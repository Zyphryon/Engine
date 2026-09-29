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

        /// \brief Registers a component type, or finds it under the compiler's name or an unresolved one it declares.
        ///
        /// \param Info The layout and lifecycle of the type, which an unresolved type takes in place.
        /// \param Name The name it is declared under, or empty for none.
        /// \return The identifier the type answers to, never zero.
        UInt32 Register(ConstRef<Metatype> Info, Text Name);

        /// \brief Finds the component type a save names, declaring it unresolved when this build does not know it.
        ///
        /// \param Name The name the save gives it.
        /// \return The identifier, never zero.
        UInt32 Acquire(Text Name);

        /// \brief Declares a component type, resolving it when unresolved and naming it for saves.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \param Info       The layout and lifecycle of the type.
        /// \param Name       The name, which no resolved type may carry already, or empty to keep the one it has.
        void Declare(UInt32 Identifier, ConstRef<Metatype> Info, Text Name);

        /// \brief Makes a component type bring another along whenever an entity is given it.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \param Implied    The identifier of the component brought along.
        void Imply(UInt32 Identifier, UInt32 Implied);

        /// \brief Marks the lifecycle of a component type as pointing into code about to go away.
        ///
        /// \param Identifier The identifier the type answers to.
        void Retire(UInt32 Identifier);

        /// \brief Gives a component type a trait or takes it away, before the type is first stored.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \param Which      The trait.
        /// \param Enabled    `true` to give it, `false` to take it away.
        void SetTrait(UInt32 Identifier, Trait Which, Bool Enabled);

        /// \brief Sets how a tool shows a component type.
        ///
        /// \param Identifier   The identifier the type answers to.
        /// \param Presentation The description, whose text must outlive the type's module.
        void SetDescription(UInt32 Identifier, ConstRef<Description> Presentation);

        /// \brief Sets the fields a tool reads and writes of a component type, keeping its width and alignment.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \param Layout     The fields, which must outlive the type's module.
        void SetSchema(UInt32 Identifier, ConstRef<ZyReflection::Schema> Layout);

        /// \brief Finds the component type that answers to a name.
        ///
        /// \param Name The name saves know the type by, or the one the compiler gives it.
        /// \return The identifier, or zero when no type answers to the name.
        UInt32 Find(Text Name);

        /// \brief Finds an unresolved type left apart under the name a type answers to, since the type was known first.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \return The identifier of the unresolved type, or zero when there is none.
        UInt32 FindUnresolved(UInt32 Identifier);

        /// \brief Gets a registered component type.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \return The type, which never moves for the life of the process.
        ConstRef<Metatype> GetMetatype(UInt32 Identifier);

        /// \brief Gets the description columns of a component type are built from.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \return The type itself, or the opaque bytes while it is unresolved and not a tag.
        ConstRef<Metatype> GetColumn(UInt32 Identifier);

        /// \brief Gets how many component types are registered, which is also the highest identifier handed out.
        ///
        /// \return The number of types.
        UInt32 GetCount();

    public:

        /// \brief Gets the registry the process shares.
        ///
        /// \return The registry, made the first time it is asked for.
        static Ref<Registry> Get();

    private:

        /// \brief Gets a registered component type the caller may change, with the lock already held.
        ///
        /// \param Identifier The identifier the type answers to.
        /// \return The type.
        Ref<Metatype> Lookup(UInt32 Identifier);

        /// \brief Finds the component type that answers to a name, with the lock already held.
        ///
        /// \param Name The name.
        /// \return The identifier, or zero when no type answers to the name.
        UInt32 Search(Text Name);

        /// \brief Makes an unresolved type the one the code now declares, keeping only its name.
        ///
        /// \param Known The unresolved type.
        /// \param Info  The layout and lifecycle of the type.
        static void Revive(Ref<Metatype> Known, ConstRef<Metatype> Info);

        /// \brief Copies a name into the registry, so it outlives the library that handed it in.
        ///
        /// \param Name The name.
        /// \return The copy.
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

    /// \brief Gets the identifier a component type answers to, registering it the first time it is asked about.
    ///
    /// \param Name The name it is declared under, which only the first call takes, or empty for none.
    /// \return The identifier, the same for every library in the process.
    template<typename Type>
    ZY_INLINE UInt32 IdentifierOf(Text Name = Text())
    {
        if constexpr (IsAnyOf<Type, StripAll<Type>>)
        {
            // Starts at zero without a guard, so the common read is one load; a race only enrolls the same type twice.
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