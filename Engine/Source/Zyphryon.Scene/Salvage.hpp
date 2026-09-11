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

#include "Builder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents what an entity carried that the world had no name for, kept exactly as it was read.
    class Salvage final
    {
    public:

        /// \brief Represents one component as it stood in the archive, name and payload together.
        struct Record final
        {
            /// The relation the component was held under, or nothing when it stood on its own.
            Str64          Relation;

            /// The name the component was written under.
            Str64          Name;

            /// The payload, byte for byte as it was read.
            Sequence<Byte> Data;
        };

    public:

        /// \brief Keeps a component the world could not place.
        ///
        /// \param Relation The relation the component was held under, or empty when it stood on its own.
        /// \param Name     The name the component was written under.
        /// \param Data     The payload, which is copied whole since nothing here understands it.
        ZY_INLINE void Keep(Text Relation, Text Name, ConstSpan<Byte> Data)
        {
            Ref<Record> Kept = mRecords.Append();
            Kept.Relation    = Relation;
            Kept.Name        = Name;
            Kept.Data.Append(Data);
        }

        /// \brief Gets everything being kept, in the order it was read.
        ///
        /// \return The components the world had no name for.
        ZY_INLINE ConstSpan<Record> GetRecords() const
        {
            return ConstSpan<Record>(mRecords.GetData(), mRecords.GetSize());
        }

        /// \brief Lets go of what is kept under a name, which is what a world does once it knows the name.
        ///
        /// \param Name The name to let go of.
        ZY_INLINE void Forget(Text Name)
        {
            mRecords.RemoveSomeIf([Name](ConstRef<Record> Kept)
            {
                return Kept.Name == Name;
            });
        }

        /// \brief Checks whether anything is still being kept.
        ///
        /// \return `true` when nothing is kept, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mRecords.IsEmpty();
        }

    public:

        /// \brief Declares the component to the scene, so everything registration needs stands here.
        ///
        /// \return The declaration the scene service applies in two passes.
        ZY_INLINE static constexpr auto OnDeclare()
        {
            return DSL::Declare<Salvage>("Salvage", DSL::Local, DSL::Sparse);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Record> mRecords;
    };
}