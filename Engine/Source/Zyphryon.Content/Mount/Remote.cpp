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

namespace Content
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Remote::Remote(Text Address)
        : mAddress { Address },
          mActive  { 0 }
    {
        if (StrEndsWith(mAddress, "/"))
        {
            mAddress.RemoveLast();
        }
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

        mQueue.Append(new Request(Str(Path), Move(Callback), this, 0));

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

        Attributes.onsuccess  = [](Ptr<emscripten_fetch_t> Fetch)
        {
            const Ptr<Request> Handle = static_cast<Ptr<Request>>(Fetch->userData);
            const UInt32       Size   = static_cast<UInt32>(Fetch->numBytes);
            const Ptr<Byte>    Data   = new Byte[Size];

            // Everything wanted from the fetch is taken before it is closed, which frees what it holds.
            Blit(Data, Size, Fetch->data);
            emscripten_fetch_close(Fetch);

            Close(Handle, Filesystem::Result::Success, Data, Size);
        };

        Attributes.onerror    = [](Ptr<emscripten_fetch_t> Fetch)
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
            Close(Handle, Status == 404 ? Filesystem::Result::Inexistent : Filesystem::Result::Unknown, nullptr, 0);
        };

        Str Address = Str::Print<"{0}/{1}">(mAddress, Handle->Path);
        Address.Append('\0');

        emscripten_fetch(AddressOf(Attributes), Address.GetData());

#endif // ZY_PLATFORM_WEB
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::Close(Ptr<Request> Handle, Filesystem::Result Result, Ptr<Byte> Data, UInt32 Size)
    {
        Ref<Remote> Owner = * Handle->Owner;

        if (Data)
        {
            Handle->Callback(Result, Blob(Data, Size, [](Ptr<Byte> Address)
            {
                delete[] Address;
            }));
        }
        else
        {
            Handle->Callback(Result, Blob());
        }

        delete Handle;

        // Whatever was held back for want of room goes out now that there is some.
        --Owner.mActive;

        Owner.Pump();
    }
}