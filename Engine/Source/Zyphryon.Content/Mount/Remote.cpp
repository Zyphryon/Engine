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
        : mAddress { Address }
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

        // The request outlives this call, so what answers it is put somewhere the call does not own.
        const Ptr<Request> Handle = new Request(Move(Callback), this);

        emscripten_fetch_attr_t Attributes;
        emscripten_fetch_attr_init(AddressOf(Attributes));

        Blit(Attributes.requestMethod, 4, "GET");

        // The bytes are handed over whole rather than in pieces, which is what a blob wants, and the fetch
        // keeps them until it is freed: what arrives is taken over below rather than copied out of it.
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

            OnSucceed(Handle, Data, Size);
        };

        Attributes.onerror    = [](Ptr<emscripten_fetch_t> Fetch)
        {
            const Ptr<Request> Handle = static_cast<Ptr<Request>>(Fetch->userData);
            const UInt32       Status = Fetch->status;

            emscripten_fetch_close(Fetch);

            OnFail(Handle, Status);
        };

        const Str Address = Str::Print<"{0}/{1}">(mAddress, Path);

        emscripten_fetch(AddressOf(Attributes), Address.GetData());

#else

        Callback(Filesystem::Result::Denied, Blob());

#endif // ZY_PLATFORM_WEB
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::OnSucceed(Ptr<Request> Handle, Ptr<Byte> Data, UInt32 Size)
    {
        Handle->Callback(Filesystem::Result::Success, Blob(Data, Size, [](Ptr<Byte> Address)
        {
            delete[] Address;
        }));

        delete Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Remote::OnFail(Ptr<Request> Handle, UInt32 Status)
    {
        const Filesystem::Result Result = (Status == 404)
            ? Filesystem::Result::Inexistent
            : Filesystem::Result::Unknown;

        Handle->Callback(Result, Blob());

        delete Handle;
    }
}