// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Specifies the scene configuration constants.
    enum : UInt64
    {
        /// \brief The slot below the first one an archetype is handed, fixed so archetype identifiers survive sessions.
        kMinRangeArchetypes = 1'024,

        /// \brief The last slot an archetype may be handed.
        kMaxRangeArchetypes = 65'534,

        /// \brief The number of archetypes a world holds at most.
        kMaxCountArchetypes = kMaxRangeArchetypes - kMinRangeArchetypes,

        /// \brief The first slot an entity is handed, above every slot an archetype may take.
        kMinRangeEntities   = 65'535,
    };

    /// \brief Specifies the changes a reader of a component sees, which may be several taken together.
    enum class Pull : UInt8
    {
        Added   = 0b001, ///< The entity came to see the component, where it saw none before.
        Changed = 0b010, ///< The value the entity sees was written while it kept seeing one.
        Removed = 0b100, ///< The entity no longer sees the component, taken off it or destroyed with it.
    };
    ZY_DEFINE_BITWISE_ENUM(Pull)

    /// \brief Represents the changes of a component a reader takes, as \ref Added, \ref Set and \ref Removed name them.
    ///
    /// \tparam Type  The component to observe.
    /// \tparam Kinds The changes, additions and changes together or apart, or removals alone.
    template<typename Type, Pull Kinds>
    struct Pulled final
    {
        static_assert(
            Kinds == Pull::Removed || (Kinds != Pull() && (Kinds & Pull::Removed) == Pull()),
            "A reader takes removals alone, or additions and changes");

        /// The component whose changes are read, and whose fields a reader may ask for.
        using Value = Type;

        /// The changes read.
        static constexpr Pull kKind = Kinds;

        /// The number of lists a reader of it reads, one per change in \ref kKind.
        static constexpr UInt kSize = CountBits(static_cast<UInt>(Kinds));
    };

    /// \brief Represents the entities that came to see a component.
    template<typename Type>
    using Added   = Pulled<Type, Pull::Added>;

    /// \brief Represents the entities that came to see a component or had its value written, additions first.
    template<typename Type>
    using Set     = Pulled<Type, Pull::Added | Pull::Changed>;

    /// \brief Represents the entities that stopped seeing a component, destroyed ones included.
    template<typename Type>
    using Removed = Pulled<Type, Pull::Removed>;

    /// \brief Represents a tag named by a string literal, which carries no data.
    ///
    /// \tparam Symbol The name saves know the tag by.
    template<Symbol Symbol>
    struct Tag final
    {
        /// The name saves know the tag by.
        static constexpr Text kName = Symbol;
    };

    /// \brief Represents the tag every archetype carries, which keeps it out of every walk that does not ask for it.
    using Prefab    = Tag<"Prefab">;

    /// \brief Represents the tag of a sleeping entity, which every walk that does not ask for it leaves out.
    using Asleep    = Tag<"Asleep">;

    /// \brief Represents the tag that leaves an entity, and everything beneath it, out of every save.
    using Transient = Tag<"Transient">;

    /// \brief Represents the tag of an archetype built into another, copied beneath every instance of the whole.
    using Part      = Tag<"Part">;

    /// \brief Represents the name an entity is found by among its siblings, and what tools show for it.
    struct Named final
    {
        /// The name saves carry for the component.
        static constexpr Text kName = "Named";

        /// The name, empty for none.
        Str Value;
        
        /// \brief Constructs an empty name.
        ZY_INLINE Named() = default;

        /// \brief Constructs a name.
        ///
        /// \param Name The name, unique among the entity's siblings.
        ZY_INLINE explicit Named(Text Name)
            : Value { Name }
        {
        }

        /// \brief Reads or writes the name.
        ///
        /// \param Archive The archive to read from or write to.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(Value);
        }
    };
}