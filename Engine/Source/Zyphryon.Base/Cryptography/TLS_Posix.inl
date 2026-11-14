// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "TLS.hpp"
#include "Zyphryon.Base/Lexical/String.hpp"
#include <openssl/pem.h>
#include <openssl/ssl.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct TLS::Backend final
    {
        /// The number of bytes moved between OpenSSL's buffers and a sequence at a time.
        static constexpr UInt32 kChunk = 4096;

        /// The OpenSSL context, which holds the protocol versions allowed and the authorities trusted.
        Ptr<SSL_CTX> Rules   = nullptr;

        /// The OpenSSL session, which owns both of its memory buffers.
        Ptr<SSL>     Session = nullptr;

        /// The memory buffer received bytes are written to, which OpenSSL reads from.
        Ptr<BIO>     Arrived = nullptr;

        /// The memory buffer OpenSSL writes the bytes to send into.
        Ptr<BIO>     Leaving = nullptr;

        void Release()
        {
            if (Session)
            {
                SSL_free(Session);
            }
            if (Rules)
            {
                SSL_CTX_free(Rules);
            }
            Rules   = nullptr;
            Session = nullptr;
            Arrived = nullptr;
            Leaving = nullptr;
        }

        void Drain(Ref<Sequence<Byte>> Output)
        {
            Byte Chunk[kChunk];

            for (int Size = BIO_read(Leaving, Chunk, kChunk); Size > 0; Size = BIO_read(Leaving, Chunk, kChunk))
            {
                Output.Append(ConstSpan<Byte>(Chunk, static_cast<UInt>(Size)));
            }
        }

        Bool Handshake(Ref<State> Stage, Ref<Sequence<Byte>> Reply)
        {
            const int Result = SSL_do_handshake(Session);

            Drain(Reply);

            if (Result == 1)
            {
                Stage = State::Open;
            }
            else if (SSL_get_error(Session, Result) != SSL_ERROR_WANT_READ)
            {
                Stage = State::Failed;
                return false;
            }
            return true;
        }
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TLS::Begin(Text Host, Bool Verified, ConstSpan<Byte> Authority, Ref<Sequence<Byte>> Reply)
    {
        mBackend->Release();
        mBackend->Rules = SSL_CTX_new(TLS_client_method());

        if (mBackend->Rules == nullptr)
        {
            mState = State::Failed;
            return false;
        }

        SSL_CTX_set_min_proto_version(mBackend->Rules, TLS1_2_VERSION);
        SSL_CTX_set_verify(mBackend->Rules, Verified ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, nullptr);

        if (Verified && Authority.GetSize() > 0)
        {
            const Ptr<BIO>        Written = BIO_new_mem_buf(Authority.GetData(), static_cast<int>(Authority.GetSize()));
            const Ptr<X509_STORE> Trusted = SSL_CTX_get_cert_store(mBackend->Rules);

            for (Ptr<X509> Each = PEM_read_bio_X509(Written, nullptr, nullptr, nullptr); Each != nullptr;
                 Each = PEM_read_bio_X509(Written, nullptr, nullptr, nullptr))
            {
                X509_STORE_add_cert(Trusted, Each);
                X509_free(Each);
            }
            BIO_free(Written);
        }
        else if (Verified)
        {
            SSL_CTX_set_default_verify_paths(mBackend->Rules);
        }

        mBackend->Session = SSL_new(mBackend->Rules);
        mBackend->Arrived = BIO_new(BIO_s_mem());
        mBackend->Leaving = BIO_new(BIO_s_mem());

        if (mBackend->Session == nullptr || mBackend->Arrived == nullptr || mBackend->Leaving == nullptr)
        {
            mState = State::Failed;
            return false;
        }

        // The session owns both buffers from here on, and frees them with itself.
        SSL_set_bio(mBackend->Session, mBackend->Arrived, mBackend->Leaving);
        SSL_set_connect_state(mBackend->Session);

        const Str Name(Host);

        SSL_set_tlsext_host_name(mBackend->Session, Name.GetData());

        if (Verified)
        {
            SSL_set1_host(mBackend->Session, Name.GetData());
        }

        mState = State::Handshaking;

        return mBackend->Handshake(mState, Reply);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TLS::Feed(ConstSpan<Byte> Wire, Ref<Sequence<Byte>> Plain, Ref<Sequence<Byte>> Reply)
    {
        if (mState != State::Handshaking && mState != State::Open)
        {
            return false;
        }
        BIO_write(mBackend->Arrived, Wire.GetData(), static_cast<int>(Wire.GetSize()));

        if (mState == State::Handshaking && !mBackend->Handshake(mState, Reply))
        {
            return false;
        }

        Byte Chunk[Backend::kChunk];

        while (mState == State::Open)
        {
            const int Size = SSL_read(mBackend->Session, Chunk, Backend::kChunk);

            if (Size > 0)
            {
                Plain.Append(ConstSpan<Byte>(Chunk, static_cast<UInt>(Size)));
                continue;
            }

            const int Reason = SSL_get_error(mBackend->Session, Size);

            if (Reason == SSL_ERROR_WANT_READ)
            {
                break;
            }
            mState = (Reason == SSL_ERROR_ZERO_RETURN ? State::Closed : State::Failed);
        }
        mBackend->Drain(Reply);

        return mState == State::Handshaking || mState == State::Open;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TLS::Seal(ConstSpan<Byte> Plain, Ref<Sequence<Byte>> Wire)
    {
        if (mState != State::Open)
        {
            return false;
        }

        while (Plain.GetSize() > 0)
        {
            const UInt Size = (Plain.GetSize() < Backend::kChunk ? Plain.GetSize() : Backend::kChunk);

            if (SSL_write(mBackend->Session, Plain.GetData(), static_cast<int>(Size)) <= 0)
            {
                mState = State::Failed;
                return false;
            }
            Plain = Plain.Slice(Size);
        }
        mBackend->Drain(Wire);

        return true;
    }
}