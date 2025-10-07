// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
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

namespace Input
{
    /// \brief Represents a standard keyboard input device and its state.
    class Keyboard final
    {
    public:

        /// \brief The total number of supported keys.
        constexpr static UInt32 kMaxKeys = Enum::Count<Key>();

    public:

        /// \brief Begins a new frame, updating the keyboard state.
        void Begin();

        /// \brief Processes a single input event to update the keyboard state.
        void Process(ConstRef<Event> Event);

        /// \brief Clears all stored key states for both the current and previous frames.
        void Reset();

        /// \brief Checks if a key was pressed during the current frame.
        ///
        /// \param Key The key to check.
        /// \return `true` if the key was pressed this frame, otherwise `false`.
        ZYPHRYON_INLINE Bool IsKeyPressed(Key Key) const
        {
            return !mLastKeys.test(Enum::Cast(Key)) && mThisKeys.test(Enum::Cast(Key));
        }

        /// \brief Checks if a key is currently held down.
        ///
        /// \param Key The key to check.
        /// \return `true` if the key is held, otherwise `false`.
        ZYPHRYON_INLINE Bool IsKeyHeld(Key Key) const
        {
            return mThisKeys.test(Enum::Cast(Key));
        }

        /// \brief Checks if a key was released during the current frame.
        ///
        /// A key is considered "released" if it was held in the previous frame,
        /// but is not held in the current frame.
        ///
        /// \param Key The key to check.
        /// \return `true` if the key was released this frame, otherwise `false`.
        ZYPHRYON_INLINE Bool IsKeyReleased(Key Key) const
        {
            return mLastKeys.test(Enum::Cast(Key)) && !mThisKeys.test(Enum::Cast(Key));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bitset<kMaxKeys> mLastKeys;
        Bitset<kMaxKeys> mThisKeys;
    };
}