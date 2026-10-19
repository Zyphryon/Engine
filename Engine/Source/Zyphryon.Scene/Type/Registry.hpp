// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Metatype.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the table of component types the whole process shares.
    class ZY_API Registry final
    {
    public:

        /// \brief Registers a type, or finds the one already registered under its compiler name or a save's name.
        ///
        /// \param Info The layout and lifecycle of the type, which a type a save named first takes in place.
        /// \param Name The name it is declared under, or empty for its compiler name.
        /// \return The identifier the type answers to, never zero.
        UInt32 Register(ConstRef<Metatype> Info, Text Name);

        /// \brief Finds the type a save names, registering it as bytes when no loaded code declares it.
        ///
        /// \param Name The name the save gives it.
        /// \return The identifier, never zero.
        UInt32 Acquire(Text Name);

        /// \brief Declares a type under the name saves know it by, taking its layout and lifecycle over.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \param Info       The layout and lifecycle of the type.
        /// \param Name       The name, which no other declared type may carry.
        void Declare(UInt32 Identifier, ConstRef<Metatype> Info, Text Name);

        /// \brief Changes what a declared type is, before an entity first holds it.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \param Change     The callable, taking the type to change.
        template<typename Callable>
        ZY_INLINE void Amend(UInt32 Identifier, AnyRef<Callable> Change)
        {
            const Guard Lock(mLock);
            Change(Lookup(Identifier));
        }

        /// \brief Marks the lifecycle of a type as pointing into code about to be unloaded.
        ///
        /// \param Identifier The identifier the type answers to.
        void Retire(UInt32 Identifier);

        /// \brief Finds the type that answers to a name.
        ///
        /// \param Name The name saves know the type by, or its compiler name.
        /// \return The identifier, or zero when no type answers to the name.
        UInt32 Find(Text Name);

        /// \brief Finds the type a save named before the code declaring the same name was loaded.
        ///
        /// \param Identifier The identifier the declared type answers to.
        /// \return The identifier of the type kept as bytes under its name, or zero when there is none.
        UInt32 FindUnresolved(UInt32 Identifier);

        /// \brief Gets a registered type.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \return The type, which never moves for the life of the process.
        ConstRef<Metatype> GetMetatype(UInt32 Identifier);

        /// \brief Gets the description a type's columns are built from.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \return The type itself, or the bytes it saved to while no loaded code declares it and it is no tag.
        ConstRef<Metatype> GetColumn(UInt32 Identifier);

        /// \brief Gets how many types are registered, which is also the highest identifier handed out.
        ///
        /// \return The number of types.
        UInt32 GetCount();

    public:

        /// \brief Gets the registry the process shares.
        ///
        /// \return The registry, made the first time it is asked for.
        static Ref<Registry> Get();

    private:

        /// \brief Gets a registered type the caller may change, with the lock already held.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \return The type, which never moves.
        Ref<Metatype> Lookup(UInt32 Identifier);

        /// \brief Finds the type that answers to a name, with the lock already held.
        ///
        /// \param Name The name saves know the type by, or its compiler name.
        /// \return The identifier, or zero when no type answers to the name.
        UInt32 Search(Text Name);

        /// \brief Makes a type kept as bytes the one the code now declares, keeping only its name.
        ///
        /// \param Known The type kept as bytes, overwritten in place.
        /// \param Info  The layout and lifecycle of the type.
        static void Revive(Ref<Metatype> Known, ConstRef<Metatype> Info);

        /// \brief Copies a name into the registry, so it outlives the library that handed it in.
        ///
        /// \param Name The name to copy.
        /// \return The copy, which lives as long as the registry.
        Text Intern(Text Name);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Mutex                      mLock;
        Sequence<Unique<Metatype>> mTypes;
        Sequence<Str>              mNames;
        Table<UInt64, UInt32>      mQualified;
        Table<UInt64, UInt32>      mNamed;
    };

    /// \brief Gets the identifier a type answers to, registering it the first time it is asked about.
    ///
    /// \param Name The name it is declared under, which only the first call takes, or empty for its compiler name.
    /// \return The identifier, the same for every library in the process.
    template<typename Type>
    ZY_INLINE UInt32 IdentifierOf(Text Name = Text())
    {
        if constexpr (IsAnyOf<Type, StripAll<Type>>)
        {
            // No guard, so the common read is one load, and a race at worst registers the same type twice.
            static Atomic<UInt32> Identifier(0);

            UInt32 Value = Identifier.load(std::memory_order_relaxed);

            if (Value == 0)
            {
                Value = Registry::Get().Register(Metatype::Of<Type>(), Name);
                Identifier.store(Value, std::memory_order_relaxed);
            }
            return Value;
        }
        else
        {
            return IdentifierOf<StripAll<Type>>(Name);
        }
    }
}