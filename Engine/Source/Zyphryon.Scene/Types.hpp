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

    /// \brief Specifies the changes a pull list records, which may be several taken together.
    enum class Pull : UInt8
    {
        Added   = 0b00000001, ///< The entity came to see the component, where it saw none before.
        Changed = 0b00000010, ///< The value the entity sees was written while it kept seeing one.
        Removed = 0b00000100, ///< The entity no longer sees the component, taken off it or destroyed with it.
    };
    ZY_DEFINE_BITWISE_ENUM(Pull)

    /// \brief Specifies how a system walks what it matches.
    enum class Schedule : UInt8
    {
        Serial,     ///< Always on the thread that progresses the world.
        Adaptive,   ///< Spread over the compute workers whenever its last walks say that pays, on one thread otherwise.
    };

    /// \brief Represents a compile-time unique tag for entities, named by a UTF-8 string literal.
    ///
    /// \tparam Symbol The compile-time UTF-8 string literal for this tag.
    template<Symbol Symbol>
    struct Tag final
    {
        /// The compile-time string literal for this tag, which it is saved under.
        static constexpr Text kName = Symbol;
    };

    /// \brief Represents the tag every archetype carries, which keeps it out of queries and passes its edits on.
    using Prefab    = Tag<"Prefab">;

    /// \brief Represents the tag that puts an entity to sleep, hiding it from what does not ask for it.
    using Asleep    = Tag<"Asleep">;

    /// \brief Represents the tag that leaves an entity out of every save.
    using Transient = Tag<"Transient">;

    /// \brief Represents the list of entities that came to see a component.
    template<typename Type>
    struct Added final
    {
        /// The component.
        using Value = Type;

        /// The change.
        static constexpr Pull kKind = Pull::Added;
    };

    /// \brief Represents the list of entities whose value of a component was written while they kept seeing one.
    template<typename Type>
    struct Changed final
    {
        /// The component.
        using Value = Type;

        /// The change.
        static constexpr Pull kKind = Pull::Changed;
    };

    /// \brief Represents the list of entities that stopped seeing a component, destroyed ones included.
    template<typename Type>
    struct Removed final
    {
        /// The component.
        using Value = Type;

        /// The change.
        static constexpr Pull kKind = Pull::Removed;
    };

    /// \brief Represents the name an entity is looked up by among its siblings, and what tools show for it.
    struct Named final
    {
        /// The name saves carry for the component.
        static constexpr Text kName = "Named";

        /// \brief Constructs an empty name.
        ZY_INLINE Named() = default;

        /// \brief Constructs a name.
        ///
        /// \param Name The name.
        ZY_INLINE explicit Named(Text Name)
            : Value { Name }
        {
        }

        /// The name, empty for none.
        Str Value;

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