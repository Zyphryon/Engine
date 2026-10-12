// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Loader::Open()
    {
        // The vendor-neutral GLX first, then the single library a system without libglvnd ships.
        if (!mLibrary.Open("libGLX.so.0") && !mLibrary.Open("libGL.so.1"))
        {
            return false;
        }

        mFinder = mLibrary.GetSymbol("glXGetProcAddressARB");
        return mFinder != nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<void> Loader::Find(Text Name) const
    {
        using Procedure = void (*)();
        using Finder    = Procedure (*)(ConstPtr<UInt8>);

        if (mFinder == nullptr)
        {
            return nullptr;
        }

        Char Buffer[128];
        ZY_ASSERT(Name.GetSize() < sizeof(Buffer), "OpenGL: An entry point name does not fit the lookup buffer");

        Copy(Buffer, Name.GetSize(), Name.GetData());
        Buffer[Name.GetSize()] = '\0';

        // GLX hands back a stub for any name at all, so what it answers proves nothing; the version does.
        if (const Procedure Address = reinterpret_cast<Finder>(mFinder)(reinterpret_cast<ConstPtr<UInt8>>(Buffer)))
        {
            return reinterpret_cast<Ptr<void>>(Address);
        }
        return mLibrary.GetSymbol(Name);
    }
}