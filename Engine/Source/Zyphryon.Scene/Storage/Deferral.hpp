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

#include "Handle.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the running walks and the changes they hold until the outermost one ends, in order.
    class ZY_API Deferral final
    {
    public:

        /// \brief Specifies what a waiting change does.
        enum class Operation : UInt8
        {
            Instantiate, ///< Makes a waiting entity from an archetype.
            Add,         ///< Gives a default component.
            Set,         ///< Gives or overwrites a component with the value carried.
            Remove,      ///< Takes a component off.
            Destroy,     ///< Destroys an entity and everything beneath it.
            Attach,      ///< Makes an entity the last child of another, or takes it from its parent.
            Rebase,      ///< Makes an entity read from another archetype.
            Purge,       ///< Takes a component off every entity.
            Override,    ///< Gives an entity its own copy of a component its archetype lends it.
        };

        /// \brief Represents one waiting change.
        struct Command final
        {
            /// What the change does.
            Operation Kind;

            /// The component it is about, or zero.
            UInt32    Identifier;

            /// The entity it changes.
            Handle    Actor;

            /// The other entity it involves, or one that names nothing.
            Handle    Other;

            /// The value it carries, or `nullptr`.
            Ptr<Byte> Payload;
        };

    public:

        /// \brief Constructs a deferral with no walk running and no change waiting.
        Deferral();

        /// \brief Gives back the room values took.
        ~Deferral();

        /// \brief Deferrals are not copied, since waiting changes point into their room.
        Deferral(ConstRef<Deferral> Other) = delete;

        /// \brief Checks whether a walk is running.
        ///
        /// \return `true` if one is, `false` otherwise.
        ZY_INLINE Bool IsWalking() const
        {
            return mDepth > 0;
        }

        /// \brief Marks a walk as started.
        ZY_INLINE void Enter()
        {
            ++mDepth;
        }

        /// \brief Marks a walk as ended.
        ///
        /// \return `true` when the outermost walk ended outside a flush, so what waits may be applied.
        ZY_INLINE Bool Leave()
        {
            return --mDepth == 0 && !mFlushing;
        }

        /// \brief Marks a spread walk as started or ended.
        ///
        /// \param Spreading `true` when it starts, `false` when it ends.
        ZY_INLINE void SetSpreading(Bool Spreading)
        {
            mSpreading = Spreading;
        }

        /// \brief Checks whether a walk is spread over the workers right now, when only writes in place are allowed.
        ///
        /// \return `true` if one is, `false` otherwise.
        ZY_INLINE Bool IsSpreading() const
        {
            return mSpreading;
        }

        /// \brief Marks the waiting changes as being applied or done, so a walk they start applies nothing more.
        ///
        /// \param Flushing `true` while they are applied, `false` once they are done.
        ZY_INLINE void SetFlushing(Bool Flushing)
        {
            mFlushing = Flushing;
        }

        /// \brief Holds a change and marks its entity.
        ///
        /// \param Change The change.
        void Queue(ConstRef<Command> Change);

        /// \brief Checks whether a change already waits for an entity.
        ///
        /// \param Index The slot of the entity.
        /// \return `true` if one does, `false` otherwise.
        ZY_INLINE Bool IsMarked(UInt32 Index) const
        {
            return Index < mMarks.GetSize() && mMarks[Index];
        }

        /// \brief Takes room for a value a waiting change carries.
        ///
        /// \param Size      The bytes the value takes.
        /// \param Alignment The alignment it needs, at most 64.
        /// \return The first byte of the room, which stays put until the change is applied.
        Ptr<Byte> Allocate(UInt Size, UInt Alignment);

        /// \brief Gets the changes waiting, in the order they were asked for.
        ///
        /// \return The changes, which move when another one is held.
        ZY_INLINE ConstSpan<Command> GetCommands() const
        {
            return mCommands;
        }

        /// \brief Forgets every applied change and every mark, keeping the room to fill again.
        void Clear();

    private:

        /// \brief The bytes of one page of values.
        static constexpr UInt kPageBytes     = 16'384;

        /// \brief The alignment of every page and of every value kept apart.
        static constexpr UInt kPageAlignment = 64;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32              mDepth;
        Bool                mFlushing;
        Bool                mSpreading;
        Sequence<UInt8>     mMarks;
        Sequence<Command>   mCommands;
        Sequence<Ptr<Byte>> mPages;
        Sequence<Ptr<Byte>> mOversized;
        UInt32              mPage;
        UInt32              mUsed;
    };
}