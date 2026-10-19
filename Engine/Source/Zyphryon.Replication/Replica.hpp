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

#include "Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    /// \brief Represents the identity a replica travels under, given by the publisher or authored into a save.
    class Replica final
    {
    public:

        /// The name saves carry for the component.
        static constexpr Text kName = "Replica";

    public:

        /// \brief Constructs a replica not given an identifier yet.
        ZY_INLINE Replica()
            : mIdentifier { 0 }
        {
        }

        /// \brief Constructs a replica under an identifier.
        ///
        /// \param Identifier The identifier, with \ref kPersistent set for one both sides load from disk.
        ZY_INLINE explicit Replica(UInt64 Identifier)
            : mIdentifier { Identifier }
        {
        }

        /// \brief Gets the identifier the entity is known by on both sides.
        ///
        /// \return The identifier, or zero before the publisher gave a runtime replica one.
        ZY_INLINE UInt64 GetIdentifier() const
        {
            return mIdentifier;
        }

        /// \brief Checks whether both sides load the entity from disk, so only its divergence travels.
        ///
        /// \return `true` if the entity is persistent, `false` if it lives only at runtime.
        ZY_INLINE Bool IsPersistent() const
        {
            return (mIdentifier & kPersistent) != 0;
        }

        /// \brief Checks whether the replica carries an identifier yet.
        ///
        /// \return `true` if it does, `false` for a runtime replica not yet published.
        ZY_INLINE Bool IsValid() const
        {
            return mIdentifier != 0;
        }

        /// \brief Reads or writes the identifier of a persistent replica, and nothing of a runtime one.
        ///
        /// \param Archive The archive to read from or write to.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            if constexpr (Serializer::IsWriter)
            {
                UInt64 Identifier = IsPersistent() ? mIdentifier : 0;
                Archive.Serialize(Identifier);
            }
            else
            {
                Archive.Serialize(mIdentifier);
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64 mIdentifier;
    };
}