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

#include "Binding.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyInput
{
    /// \brief Represents the named actions a game reads, and the physical inputs bound to each.
    class ZY_API Mapping final
    {
    public:

        /// \brief Adds a binding to an action, defining the action if it is new.
        ///
        /// \param Action  The name of the action.
        /// \param Binding The physical input to feed the action.
        void Bind(Text Action, ConstRef<Binding> Binding);

        /// \brief Removes every binding of an action, leaving the action defined.
        ///
        /// \param Action The name of the action.
        void Unbind(Text Action);

        /// \brief Adds a value to an action for the next update, defining the action if it is new.
        ///
        /// \note The next \ref Update reads the value once and drops it.
        ///
        /// \param Action The name of the action.
        /// \param Value  The value to add, summed with the bindings the way a mouse axis is.
        void Feed(Text Action, Real32 Value);

        /// \brief Removes every action and binding.
        void Clear();

        /// \brief Reads every binding and advances each action by one frame.
        ///
        /// \param Keyboard The keyboard state of the current frame.
        /// \param Mouse    The mouse state of the current frame.
        void Update(ConstRef<Keyboard> Keyboard, ConstRef<Mouse> Mouse);

        /// \brief Replaces the bindings of every defined action the document names.
        ///
        /// \note Actions the document leaves out keep their bindings, and names it holds that no action has are ignored.
        ///
        /// \param Root The object holding one array of bindings per action name.
        void Load(JsonObject Root);

        /// \brief Writes the bindings of every action.
        ///
        /// \param Root The object to write one array of bindings per action name under.
        void Save(JsonObject Root) const;

        /// \brief Visits every action that was pressed or released during the current frame.
        ///
        /// \param Callback The callable invoked with each action's identifier and the change it went through.
        template<typename Callable>
        ZY_INLINE void ForEachChange(AnyRef<Callable> Callback) const
        {
            mActions.ForEach([&](UInt64 Identifier, ConstRef<Entry> Entry)
            {
                if (Entry.Down != Entry.WasDown)
                {
                    Callback(Identifier, Entry.Down ? Action::Pressed : Action::Released);
                }
            });
        }

        /// \brief Checks whether an action was pressed during the current frame.
        ///
        /// \param Action The identifier of the action.
        /// \return `true` if the action went from released to held this frame, otherwise `false`.
        ZY_INLINE Bool IsPressed(UInt64 Action) const
        {
            const ConstPtr<Entry> Entry = mActions.Find(Action);
            return Entry && Entry->Down && !Entry->WasDown;
        }

        /// \brief Checks whether an action is held.
        ///
        /// \param Action The identifier of the action.
        /// \return `true` if the action is held this frame, otherwise `false`.
        ZY_INLINE Bool IsHeld(UInt64 Action) const
        {
            const ConstPtr<Entry> Entry = mActions.Find(Action);
            return Entry && Entry->Down;
        }

        /// \brief Checks whether an action was released during the current frame.
        ///
        /// \param Action The identifier of the action.
        /// \return `true` if the action went from held to released this frame, otherwise `false`.
        ZY_INLINE Bool IsReleased(UInt64 Action) const
        {
            const ConstPtr<Entry> Entry = mActions.Find(Action);
            return Entry && !Entry->Down && Entry->WasDown;
        }

        /// \brief Gets the value an action reads this frame.
        ///
        /// \param Action The identifier of the action.
        /// \return The keys and buttons bound to it, kept within [-1, 1], plus how far its mouse axes moved.
        ZY_INLINE Real32 GetValue(UInt64 Action) const
        {
            const ConstPtr<Entry> Entry = mActions.Find(Action);
            return Entry ? Entry->Value : 0.0f;
        }

    private:

        /// \brief Holds one action's bindings and the state they produced.
        struct Entry final
        {
            /// The name the action was defined under, kept for saving.
            Str               Name;

            /// The physical inputs feeding the action.
            Sequence<Binding> Bindings;

            /// The value the bindings read this frame.
            Real32            Value   = 0.0f;

            /// The value fed from code since the last update, waiting for the next one.
            Real32            Fed     = 0.0f;

            /// `true` when the action is held this frame.
            Bool              Down    = false;

            /// `true` when the action was held the frame before.
            Bool              WasDown = false;
        };

        /// \brief Gets the entry of an action, defining the action if it is new.
        ///
        /// \param Action The name of the action.
        /// \return The action's entry.
        Ref<Entry> Define(Text Action);

        /// \brief Reads the value a single binding feeds its action this frame.
        ///
        /// \param Binding  The binding to read.
        /// \param Keyboard The keyboard state of the current frame.
        /// \param Mouse    The mouse state of the current frame.
        /// \return The binding's value, already scaled.
        static Real32 Read(ConstRef<Binding> Binding, ConstRef<Keyboard> Keyboard, ConstRef<Mouse> Mouse);

    public:

        /// \brief Gets the identifier an action is queried by.
        ///
        /// \param Action The name of the action.
        /// \return The identifier, the same on every build, so it can be a compile-time constant.
        ZY_INLINE static constexpr UInt64 Identify(Text Action)
        {
            return Hash(Action);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<UInt64, Entry> mActions;
    };
}