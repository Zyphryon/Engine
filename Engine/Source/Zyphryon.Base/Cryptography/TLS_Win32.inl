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
#include "Zyphryon.Base/Lexical/Unicode.hpp"

#define SECURITY_WIN32
#define SCHANNEL_USE_BLACKLISTS
#include <windows.h>
#include <subauth.h>
#include <security.h>
#include <schannel.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct TLS::Backend final
    {
        /// The most UTF-16 code units a server's host name takes, its terminator included.
        static constexpr UInt kMaxHost      = 256;

        /// The most bytes one authority's certificate takes once decoded from PEM.
        static constexpr UInt kMaxAuthority = 8192;

        /// The Schannel credentials of the client, which carry no certificate of its own.
        CredHandle                Credentials { };

        /// The security context Schannel keeps the session's keys in.
        CtxtHandle                Context     { };

        /// `true` while \ref Credentials is acquired, `false` before that and once it is freed.
        Bool                      Granted     = false;

        /// `true` while \ref Context exists, `false` before the first handshake step and once it is deleted.
        Bool                      Begun       = false;

        /// `true` to refuse an untrusted certificate or one for another name, `false` to accept any.
        Bool                      Verified    = false;

        /// The server's host name in UTF-16, terminated.
        Sequence<Wide, kMaxHost>  Host;

        /// The PEM certificates of the only authorities trusted, or empty to trust the system's.
        Sequence<Byte>            Authority;

        /// The sizes of a record's header and trailer and the most bytes one record encrypts.
        SecPkgContext_StreamSizes Sizes       { };

        /// The received bytes not yet consumed, kept until the rest of their message arrives.
        Sequence<Byte>            Inbound;

        void Release()
        {
            if (Begun)
            {
                DeleteSecurityContext(AddressOf(Context));
            }
            if (Granted)
            {
                FreeCredentialsHandle(AddressOf(Credentials));
            }
            Begun   = false;
            Granted = false;

            Inbound.Clear();
        }

        Bool Verify()
        {
            static constexpr Text kClosing = "-----END CERTIFICATE-----";

            PCCERT_CONTEXT Shown = nullptr;

            const SECURITY_STATUS Status = QueryContextAttributesW(
                AddressOf(Context), SECPKG_ATTR_REMOTE_CERT_CONTEXT, AddressOf(Shown));

            if (Status != SEC_E_OK)
            {
                return false;
            }

            const HCERTSTORE Trusted = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, nullptr);
            const Text       Written(reinterpret_cast<ConstPtr<Char>>(Authority.GetData()), Authority.GetSize());

            for (UInt Cursor = 0; Cursor < Written.GetSize();)
            {
                const Text Rest = Written.Slice(Cursor);
                UInt       End  = 0;

                while (End + kClosing.GetSize() <= Rest.GetSize() && Rest.Slice(End, kClosing.GetSize()) != kClosing)
                {
                    ++End;
                }
                if (End + kClosing.GetSize() > Rest.GetSize())
                {
                    break;
                }
                End += kClosing.GetSize();

                Byte  Encoded[kMaxAuthority];
                DWORD Size = sizeof(Encoded);

                const BOOL Decoded = CryptStringToBinaryA(
                    Rest.GetData(),
                    static_cast<DWORD>(End),
                    CRYPT_STRING_BASE64HEADER,
                    Encoded,
                    AddressOf(Size),
                    nullptr,
                    nullptr);

                if (Decoded)
                {
                    CertAddEncodedCertificateToStore(
                        Trusted, X509_ASN_ENCODING, Encoded, Size, CERT_STORE_ADD_ALWAYS, nullptr);
                }
                Cursor += End;
            }

            CERT_CHAIN_ENGINE_CONFIG Rules { };
            Rules.cbSize         = sizeof(Rules);
            Rules.hExclusiveRoot = Trusted;

            CERT_CHAIN_PARA Asked { };
            Asked.cbSize = sizeof(Asked);

            HCERTCHAINENGINE     Engine  = nullptr;
            PCCERT_CHAIN_CONTEXT Chain   = nullptr;
            Bool                 Passed = false;

            if (CertCreateCertificateChainEngine(AddressOf(Rules), AddressOf(Engine))
                && CertGetCertificateChain(
                    Engine, Shown, nullptr, Shown->hCertStore, AddressOf(Asked), 0, nullptr, AddressOf(Chain)))
            {
                SSL_EXTRA_CERT_CHAIN_POLICY_PARA Named { };
                Named.cbSize         = sizeof(Named);
                Named.dwAuthType     = AUTHTYPE_SERVER;
                Named.pwszServerName = Host.GetData();

                CERT_CHAIN_POLICY_PARA Policy { };
                Policy.cbSize            = sizeof(Policy);
                Policy.pvExtraPolicyPara = AddressOf(Named);

                CERT_CHAIN_POLICY_STATUS Verdict { };
                Verdict.cbSize = sizeof(Verdict);

                Passed = CertVerifyCertificateChainPolicy(
                    CERT_CHAIN_POLICY_SSL, Chain, AddressOf(Policy), AddressOf(Verdict)) && Verdict.dwError == 0;
            }

            if (Chain)
            {
                CertFreeCertificateChain(Chain);
            }
            if (Engine)
            {
                CertFreeCertificateChainEngine(Engine);
            }
            CertCloseStore(Trusted, 0);
            CertFreeCertificateContext(Shown);

            return Passed;
        }

        Bool Handshake(Ref<State> Stage, Ref<Sequence<Byte>> Reply)
        {
            constexpr DWORD kAsked = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY
                                   | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM    | ISC_REQ_USE_SUPPLIED_CREDS;

            while (Stage == State::Handshaking)
            {
                SecBuffer Taken[2] =
                {
                    { static_cast<ULONG>(Inbound.GetSize()), SECBUFFER_TOKEN, Inbound.GetData() },
                    { 0, SECBUFFER_EMPTY, nullptr },
                };
                SecBuffer Given[1] =
                {
                    { 0, SECBUFFER_TOKEN, nullptr },
                };
                SecBufferDesc Input  = { SECBUFFER_VERSION, 2, Taken };
                SecBufferDesc Output = { SECBUFFER_VERSION, 1, Given };
                DWORD         Agreed = 0;

                const SECURITY_STATUS Status = InitializeSecurityContextW(
                    AddressOf(Credentials),
                    Begun ? AddressOf(Context) : nullptr,
                    Host.GetData(),
                    kAsked | (Verified && Authority.IsEmpty() ? 0 : ISC_REQ_MANUAL_CRED_VALIDATION),
                    0,
                    0,
                    Begun ? AddressOf(Input) : nullptr,
                    0,
                    AddressOf(Context),
                    AddressOf(Output),
                    AddressOf(Agreed),
                    nullptr);

                if (Given[0].pvBuffer != nullptr)
                {
                    Reply.Append(ConstSpan<Byte>(static_cast<ConstPtr<Byte>>(Given[0].pvBuffer), Given[0].cbBuffer));

                    FreeContextBuffer(Given[0].pvBuffer);
                }

                if (Status == SEC_E_INCOMPLETE_MESSAGE)
                {
                    return true;
                }

                if (Status != SEC_E_OK && Status != SEC_I_CONTINUE_NEEDED)
                {
                    Stage = State::Failed;
                    return false;
                }

                // Schannel reports what it did not consume, which is the start of the next message.
                if (Begun)
                {
                    const UInt Left = (Taken[1].BufferType == SECBUFFER_EXTRA ? Taken[1].cbBuffer : 0);

                    Inbound.Remove(0, Inbound.GetSize() - Left);
                }
                Begun = true;

                if (Status == SEC_E_OK)
                {
                    // Schannel validates against the system's authorities only, so given ones are checked here.
                    if (Verified && !Authority.IsEmpty() && !Verify())
                    {
                        Stage = State::Failed;
                        return false;
                    }
                    QueryContextAttributesW(AddressOf(Context), SECPKG_ATTR_STREAM_SIZES, AddressOf(Sizes));

                    Stage = State::Open;
                }
                else if (Inbound.IsEmpty())
                {
                    return true;
                }
            }
            return true;
        }

        Bool Decrypt(Ref<State> Stage, Ref<Sequence<Byte>> Plain, Ref<Sequence<Byte>> Reply)
        {
            while (Stage == State::Open && !Inbound.IsEmpty())
            {
                SecBuffer Parts[4] =
                {
                    { static_cast<ULONG>(Inbound.GetSize()), SECBUFFER_DATA, Inbound.GetData() },
                    { 0, SECBUFFER_EMPTY, nullptr },
                    { 0, SECBUFFER_EMPTY, nullptr },
                    { 0, SECBUFFER_EMPTY, nullptr },
                };
                SecBufferDesc Record = { SECBUFFER_VERSION, 4, Parts };

                const SECURITY_STATUS Status = DecryptMessage(AddressOf(Context), AddressOf(Record), 0, nullptr);

                if (Status == SEC_E_INCOMPLETE_MESSAGE)
                {
                    return true;
                }

                if (Status != SEC_E_OK && Status != SEC_I_RENEGOTIATE)
                {
                    Stage = (Status == SEC_I_CONTEXT_EXPIRED ? State::Closed : State::Failed);
                    return false;
                }

                // Schannel decrypts in place, so the plain bytes and the leftover both point into the buffer.
                UInt Left = 0;

                for (ConstRef<SecBuffer> Part : Parts)
                {
                    if (Part.BufferType == SECBUFFER_DATA)
                    {
                        Plain.Append(ConstSpan<Byte>(static_cast<ConstPtr<Byte>>(Part.pvBuffer), Part.cbBuffer));
                    }
                    else if (Part.BufferType == SECBUFFER_EXTRA)
                    {
                        Left = Part.cbBuffer;
                    }
                }
                Inbound.Remove(0, Inbound.GetSize() - Left);

                if (Status == SEC_I_RENEGOTIATE)
                {
                    Stage = State::Handshaking;

                    if (!Handshake(Stage, Reply))
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TLS::Begin(Text Host, Bool Verified, ConstSpan<Byte> Authority, Ref<Sequence<Byte>> Reply)
    {
        mBackend->Release();
        mBackend->Host      = Unicode::Widen<Backend::kMaxHost>(Host);
        mBackend->Verified  = Verified;
        mBackend->Authority = Authority;

        const Bool Automatic = Verified && Authority.GetSize() == 0;

        SCH_CREDENTIALS Wanted { };
        Wanted.dwVersion = SCH_CREDENTIALS_VERSION;
        Wanted.dwFlags   = SCH_USE_STRONG_CRYPTO | SCH_CRED_NO_DEFAULT_CREDS
                         | (Automatic ? SCH_CRED_AUTO_CRED_VALIDATION : SCH_CRED_MANUAL_CRED_VALIDATION);

        const SECURITY_STATUS Status = AcquireCredentialsHandleW(
            nullptr,
            const_cast<LPWSTR>(UNISP_NAME_W),
            SECPKG_CRED_OUTBOUND,
            nullptr,
            AddressOf(Wanted),
            nullptr,
            nullptr,
            AddressOf(mBackend->Credentials),
            nullptr);

        if (Status != SEC_E_OK)
        {
            mState = State::Failed;
            return false;
        }

        mBackend->Granted = true;
        mState            = State::Handshaking;

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
        mBackend->Inbound.Append(Wire);

        if (mState == State::Handshaking && !mBackend->Handshake(mState, Reply))
        {
            return false;
        }
        return mBackend->Decrypt(mState, Plain, Reply);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TLS::Seal(ConstSpan<Byte> Plain, Ref<Sequence<Byte>> Wire)
    {
        if (mState != State::Open)
        {
            return false;
        }

        ConstRef<SecPkgContext_StreamSizes> Sizes = mBackend->Sizes;

        while (Plain.GetSize() > 0)
        {
            const UInt Size  = (Plain.GetSize() < Sizes.cbMaximumMessage ? Plain.GetSize() : Sizes.cbMaximumMessage);
            const UInt Start = Wire.GetSize();

            Wire.Advance(Sizes.cbHeader + Size + Sizes.cbTrailer);

            const Ptr<Byte> Record = Wire.GetData() + Start;

            Copy(Record + Sizes.cbHeader, Size, Plain.GetData());

            SecBuffer Parts[4] =
            {
                { Sizes.cbHeader,           SECBUFFER_STREAM_HEADER,  Record },
                { static_cast<ULONG>(Size), SECBUFFER_DATA,           Record + Sizes.cbHeader },
                { Sizes.cbTrailer,          SECBUFFER_STREAM_TRAILER, Record + Sizes.cbHeader + Size },
                { 0,                        SECBUFFER_EMPTY,          nullptr },
            };
            SecBufferDesc Sealed = { SECBUFFER_VERSION, 4, Parts };

            if (EncryptMessage(AddressOf(mBackend->Context), 0, AddressOf(Sealed), 0) != SEC_E_OK)
            {
                mState = State::Failed;
                return false;
            }

            // The trailer may come out shorter than the space reserved for it.
            const UInt Used = Start + Parts[0].cbBuffer + Parts[1].cbBuffer + Parts[2].cbBuffer;

            if (Used < Wire.GetSize())
            {
                Wire.Remove(Used, Wire.GetSize() - Used);
            }
            Plain = Plain.Slice(Size);
        }
        return true;
    }
}