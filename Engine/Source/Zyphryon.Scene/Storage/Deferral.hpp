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
#include "Zyphryon.Base/Functional/Delegate.hpp"

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
            Notify,      ///< Records that a component written in place changed, for readers of its changes.
            Invoke,      ///< Calls the task carried, then destroys it.
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

        /// \brief Defines the task an \ref Operation::Invoke change carries.
        using Task = Delegate<void()>;

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

        /// \brief Marks a spread walk as started, holding each share's changes apart until it ends.
        ///
        /// \param Shares The shares the walk is cut into.
        /// \param Slots  The entity slots that exist, which is every one a change may mark while the walk runs.
        void Spread(UInt32 Shares, UInt32 Slots);

        /// \brief Makes the changes the calling thread holds from now on belong to one share of the running spread walk.
        ///
        /// \param Share The share, in the order the walk cuts them.
        void Bind(UInt32 Share);

        /// \brief Marks the spread walk as ended, appending each share's changes in share order.
        void Gather();

        /// \brief Checks whether a walk is spread over the workers right now.
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

        /// \brief Holds a change, into the calling thread's share while a walk is spread.
        ///
        /// \param Change The change.
        /// \param Mark   `true` to mark its entity, so every change of it waits behind this one, `false` otherwise.
        void Queue(ConstRef<Command> Change, Bool Mark = true);

        /// \brief Checks whether a change already waits for an entity.
        ///
        /// \param Index The slot of the entity.
        /// \return `true` if one does, `false` otherwise.
        ZY_INLINE Bool IsMarked(UInt32 Index) const
        {
            return Index < mMarks.GetSize() && mMarks[Index];
        }

        /// \brief Takes room for a value a waiting change carries, from the calling thread's share while a walk is spread.
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

        /// \brief Represents the pages values are laid out in, kept to fill again once their changes are applied.
        struct Room final
        {
            /// The pages, the ones past the current one still free.
            Sequence<Ptr<Byte>> Pages;

            /// The values too large for a page, each in a block of its own.
            Sequence<Ptr<Byte>> Oversized;

            /// The page being filled.
            UInt32              Page;

            /// The bytes of that page already taken.
            UInt32              Used;

            /// \brief Constructs room holding no page yet.
            ZY_INLINE Room()
                : Page { 0 },
                  Used { 0 }
            {
            }

            /// \brief Takes room for a value.
            ///
            /// \param Size      The bytes the value takes.
            /// \param Alignment The alignment it needs, at most a page's.
            /// \return The first byte of the room.
            Ptr<Byte> Allocate(UInt Size, UInt Alignment);

            /// \brief Frees the oversized blocks and starts over on the first page, keeping the pages.
            void Reset();

            /// \brief Frees the pages as well.
            void Release();
        };

        /// \brief Represents the changes one share of a spread walk holds, and the room their values take.
        struct Lane final
        {
            /// The changes, in the order the share asked for them.
            Sequence<Command> Commands;

            /// The room their values take, which lives until the changes are applied.
            Room              Values;
        };

        /// \brief Gets the share the calling thread holds its changes into while a walk is spread.
        ///
        /// \note Per thread, since a callback reaches the deferral through calls that never name its share.
        ///
        /// \return The share, which \ref Bind points at and which is `nullptr` before any walk was spread.
        static Ref<Ptr<Lane>> GetShare();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32              mDepth;
        Bool                mFlushing;
        Bool                mSpreading;
        Sequence<UInt8>     mMarks;
        Sequence<Command>   mCommands;
        Room                mValues;
        Sequence<Lane>      mLanes;
        UInt32              mShares;
    };
}