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

namespace ZyInput
{
    /// \brief Represents a standard keyboard input device and its state.
    class ZY_API Keyboard final
    {
    public:

        /// \brief The total number of supported keys.
        constexpr static UInt32 kMaxKeys = ZyEnum::Count<Key>();

    public:

        /// \brief Begins a new frame, updating the keyboard state.
        void Begin();

        /// \brief Processes a single input event to update the keyboard state.
        void Process(ConstRef<Event> Event);

        /// \brief Clears all stored key states for both the current and previous frames.
        void Reset();

        /// \brief Marks every key as released, keeping the previous frame so the transition is reported.
        ///
        /// \param Output Receives one release event per input that was held.
        void ReleaseAllKeys(Ref<Sequence<Event>> Output);

        /// \brief Checks if a key was pressed during the current frame.
        ///
        /// \param Key The key to check.
        /// \return `true` if the key was pressed this frame, otherwise `false`.
        ZY_INLINE Bool IsKeyPressed(Key Key) const
        {
            return !mLastKeys.Test(ZyEnum::Cast(Key)) && mThisKeys.Test(ZyEnum::Cast(Key));
        }

        /// \brief Checks if a key is currently held down.
        ///
        /// \param Key The key to check.
        /// \return `true` if the key is held, otherwise `false`.
        ZY_INLINE Bool IsKeyHeld(Key Key) const
        {
            return mThisKeys.Test(ZyEnum::Cast(Key));
        }

        /// \brief Checks if a key was released during the current frame.
        ///
        /// \param Key The key to check.
        /// \return `true` if the key was released this frame, otherwise `false`.
        ZY_INLINE Bool IsKeyReleased(Key Key) const
        {
            return mLastKeys.Test(ZyEnum::Cast(Key)) && !mThisKeys.Test(ZyEnum::Cast(Key));
        }

        /// \brief Gets the modifier keys currently held, either side of each counting.
        ///
        /// \return The modifier keys held.
        ZY_INLINE Modifier GetModifiers() const
        {
            Modifier Result = Modifier::None;
            Result = SetOrClearBit(Result, Modifier::Shift,   IsKeyHeld(Key::LeftShift) || IsKeyHeld(Key::RightShift));
            Result = SetOrClearBit(Result, Modifier::Control, IsKeyHeld(Key::LeftCtrl)  || IsKeyHeld(Key::RightCtrl));
            Result = SetOrClearBit(Result, Modifier::Alt,     IsKeyHeld(Key::LeftAlt)   || IsKeyHeld(Key::RightAlt));
            Result = SetOrClearBit(Result, Modifier::Super,   IsKeyHeld(Key::LeftSuper) || IsKeyHeld(Key::RightSuper));
            return Result;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bitset<kMaxKeys> mLastKeys;
        Bitset<kMaxKeys> mThisKeys;
    };
}