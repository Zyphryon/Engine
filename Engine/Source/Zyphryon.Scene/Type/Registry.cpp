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

#include "Registry.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Registry::Register(ConstRef<Metatype> Info, Text Name)
    {
        const Guard Lock(mLock);

        // A type arrives under the compiler's name, which keys it for every library in the process.
        const UInt64 Key = Hash(Info.Name);

        if (const ConstPtr<UInt32> Found = mQualified.Find(Key))
        {
            Ref<Metatype> Known = Lookup(* Found);

            ZY_ASSERT(!Known.Has(Trait::Loaded) || Known.GetSize() == Info.GetSize(),
                "Two component types share a name and differ in size");

            if (!Known.Has(Trait::Loaded))
            {
                Revive(Known, Info);
            }
            return * Found;
        }

        // A save read before the type was declared left it unresolved under the name, so it takes that identifier.
        const UInt32 Unresolved = Name.IsEmpty() ? 0 : Search(Name);

        if (Unresolved && !Lookup(Unresolved).Has(Trait::Loaded))
        {
            Revive(Lookup(Unresolved), Info);
            mQualified.Assign(Key, Unresolved);
            return Unresolved;
        }

        Ref<Unique<Metatype>> Entry = mTypes.Append(Unique<Metatype>::Create(Info));
        Entry->Name = Intern(Info.Name);

        // Identifiers start at one, so zero names no type.
        const UInt32 Identifier = static_cast<UInt32>(mTypes.GetSize());
        mQualified.Assign(Key, Identifier);
        return Identifier;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Registry::Acquire(Text Name)
    {
        const Guard Lock(mLock);

        if (const UInt32 Known = Search(Name))
        {
            return Known;
        }

        // Saved and kept off instances until code declares what it is, since nothing here knows how it is inherited.
        Metatype Info;
        Info.Name   = Intern(Name);
        Info.Layout = Metatype::Unitialized().Layout;
        Info.Traits = Trait::Serializable | Trait::Local;
        mTypes.Append(Unique<Metatype>::Create(Info));

        const UInt32 Identifier = static_cast<UInt32>(mTypes.GetSize());
        mNamed.Assign(Hash(Name), Identifier);
        mQualified.Assign(Hash(Name), Identifier);
        return Identifier;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Registry::Declare(UInt32 Identifier, ConstRef<Metatype> Info, Text Name)
    {
        const Guard Lock(mLock);

        Ref<Metatype> Known = Lookup(Identifier);

        // A module imported again brings back the code its types point into.
        if (!Known.Has(Trait::Loaded))
        {
            Revive(Known, Info);
        }

        if (Name.IsEmpty() || Known.Name == Name)
        {
            return;
        }

        mNamed.Erase(Hash(Known.Name), Identifier);

        // An unresolved type holding the name gives it up, and each world then moves its values over.
        ZY_ASSERT(!mNamed.Contains(Hash(Name)) || !Lookup(* mNamed.Find(Hash(Name))).Has(Trait::Loaded),
            "Two component types are declared under one name");

        Known.Name = Intern(Name);
        mNamed.Assign(Hash(Known.Name), Identifier);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Registry::Imply(UInt32 Identifier, UInt32 Implied)
    {
        const Guard Lock(mLock);

        ZY_ASSERT(Identifier != Implied, "A component cannot bring itself along");

        if (Ref<Sequence<UInt32>> Implies = Lookup(Identifier).Implies; !Implies.Contains(Implied))
        {
            Implies.Append(Implied);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Registry::Retire(UInt32 Identifier)
    {
        const Guard Lock(mLock);

        Ref<Metatype> Info = Lookup(Identifier);

        // Null them, so a late call fails here instead of jumping into unloaded code.
        Info.Construct    = nullptr;
        Info.Destruct     = nullptr;
        Info.Relocate     = nullptr;
        Info.Copy         = nullptr;
        Info.Save         = nullptr;
        Info.Load         = nullptr;
        Info.Presentation = Description();
        Info.Layout       = ZyReflection::Schema(Info.GetSize(), Info.GetAlignment());
        Info.Set(Trait::Loaded, false);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Registry::SetTrait(UInt32 Identifier, Trait Which, Bool Enabled)
    {
        const Guard Lock(mLock);
        Lookup(Identifier).Set(Which, Enabled);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Registry::SetDescription(UInt32 Identifier, ConstRef<Description> Presentation)
    {
        const Guard Lock(mLock);
        Lookup(Identifier).Presentation = Presentation;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Registry::SetSchema(UInt32 Identifier, ConstRef<ZyReflection::Schema> Layout)
    {
        const Guard Lock(mLock);

        Ref<Metatype> Info = Lookup(Identifier);
        Info.Layout = ZyReflection::Schema(Layout.GetFields(), Layout.GetName(), Info.GetSize(), Info.GetAlignment());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Registry::Find(Text Name)
    {
        const Guard Lock(mLock);
        return Search(Name);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Registry::FindUnresolved(UInt32 Identifier)
    {
        const Guard Lock(mLock);
        const Text  Name = Lookup(Identifier).Name;

        for (UInt32 Other = 1; Other <= mTypes.GetSize(); ++Other)
        {
            if (Other != Identifier && !mTypes[Other - 1]->Has(Trait::Loaded) && mTypes[Other - 1]->Name == Name)
            {
                return Other;
            }
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstRef<Metatype> Registry::GetMetatype(UInt32 Identifier)
    {
        const Guard Lock(mLock);
        return Lookup(Identifier);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstRef<Metatype> Registry::GetColumn(UInt32 Identifier)
    {
        ConstRef<Metatype> Info = GetMetatype(Identifier);
        return Info.Has(Trait::Loaded) || Info.IsTag() ? Info : Metatype::Unitialized();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Registry::GetCount()
    {
        const Guard Lock(mLock);
        return static_cast<UInt32>(mTypes.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Registry> Registry::Get()
    {
        static Registry Instance;
        return Instance;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Metatype> Registry::Lookup(UInt32 Identifier)
    {
        ZY_ASSERT(Identifier > 0 && Identifier <= mTypes.GetSize(), "No component type answers to that identifier");

        return * mTypes[Identifier - 1];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Registry::Search(Text Name)
    {
        const UInt64 Key = Hash(Name);

        if (const ConstPtr<UInt32> Found = mNamed.Find(Key); Found && Lookup(* Found).Name == Name)
        {
            return * Found;
        }

        // The compiler's name is only kept as a key, so trust the hash, as enrolling does.
        const ConstPtr<UInt32> Found = mQualified.Find(Key);
        return Found ? * Found : 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Registry::Revive(Ref<Metatype> Known, ConstRef<Metatype> Info)
    {
        const Text Name = Known.Name;
        Known      = Info;
        Known.Name = Name;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Text Registry::Intern(Text Name)
    {
        return mNames.Append(Name);
    }
}