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

#include "Remote.hpp"

#if defined(ZY_PLATFORM_WEB)
    #include <emscripten/fetch.h>
#endif // ZY_PLATFORM_WEB

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyContent
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Remote::Remote(Text Address)
        : mAddress { StrTrimRight(Address, '/') },
          mActive  { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Remote::IsAsynchronous() const
    {
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::Enumerate(Text Path, AnyRef<OnEnumerate> Callback) const
    {
        Callback(Filesystem::Result::Denied, { });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::Delete(Text Path, AnyRef<OnResult> Callback)
    {
        Callback(Filesystem::Result::Denied);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::Copy(Text Source, Text Destination, AnyRef<OnResult> Callback)
    {
        Callback(Filesystem::Result::Denied);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::Write(Text Path, AnyRef<Blob> Bytes, AnyRef<OnResult> Callback)
    {
        Callback(Filesystem::Result::Denied);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::Read(Text Path, AnyRef<OnRead> Callback)
    {
#if defined(ZY_PLATFORM_WEB)

        mQueue.Append(new Request(Str(Path), Move(Callback), Retainer<Remote>(this), 0));

        Pump();

#else

        // Nothing else here speaks over the wire yet, and answering as though it did would have the caller
        // wait for something that is never coming.
        Callback(Filesystem::Result::Denied, Blob());

#endif // ZY_PLATFORM_WEB
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::Pump()
    {
        while (mActive < kMaxInFlight && !mQueue.IsEmpty())
        {
            const Ptr<Request> Handle = mQueue.GetFront();

            mQueue.Remove(0);
            ++mActive;

            Send(Handle);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::Send(Ptr<Request> Handle)
    {
#if defined(ZY_PLATFORM_WEB)

        ++Handle->Attempt;

        emscripten_fetch_attr_t Attributes;
        emscripten_fetch_attr_init(AddressOf(Attributes));

        Blit(Attributes.requestMethod, 4, "GET");

        // The bytes are wanted whole rather than in pieces, which is what a blob is.
        Attributes.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        Attributes.userData   = Handle;
        Attributes.onsuccess  = &Remote::OnSuccess;
        Attributes.onerror    = &Remote::OnFailure;

        const Str Address = Str::Join(mAddress, '/', Handle->Path);

        emscripten_fetch(AddressOf(Attributes), Address.GetData());

#endif // ZY_PLATFORM_WEB
    }

#if defined(ZY_PLATFORM_WEB)

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::OnSuccess(Ptr<emscripten_fetch_t> Fetch)
    {
        const Ptr<Request> Handle = static_cast<Ptr<Request>>(Fetch->userData);

        // Everything wanted from the fetch is taken before it is closed, which frees what it holds.
        Blob Data = Blob::Copy(ConstSpan<Byte>(
            reinterpret_cast<ConstPtr<Byte>>(Fetch->data),
            static_cast<UInt32>(Fetch->numBytes)));
        emscripten_fetch_close(Fetch);

        Close(Handle, Filesystem::Result::Success, Move(Data));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::OnFailure(Ptr<emscripten_fetch_t> Fetch)
    {
        const Ptr<Request> Handle = static_cast<Ptr<Request>>(Fetch->userData);
        const UInt32       Status = Fetch->status;

        emscripten_fetch_close(Fetch);

        // Being told to slow down is not an answer about the file, so the same question is asked again.
        if (Status == kTooMany && Handle->Attempt < kMaxAttempts)
        {
            Ref<Remote> Owner = * Handle->Owner;

            Owner.mQueue.Append(Handle);
            --Owner.mActive;

            Owner.Pump();
            return;
        }

        // A file that is not there and an address that cannot be reached are told apart, since only one
        // of them is worth asking about again.
        Close(Handle, Status == kMissing ? Filesystem::Result::Inexistent : Filesystem::Result::Unknown, Blob());
    }

#endif // ZY_PLATFORM_WEB

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::Close(Ptr<Request> Handle, Filesystem::Result Result, AnyRef<Blob> Data)
    {
        const Retainer<Remote> Owner = Move(Handle->Owner);

        Handle->Callback(Result, Move(Data));

        delete Handle;

        // Whatever was held back for want of room goes out now that there is some.
        --Owner->mActive;

        Owner->Pump();
    }
}