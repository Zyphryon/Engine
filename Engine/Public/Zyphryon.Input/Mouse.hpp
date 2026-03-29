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

namespace Input
{
    /// \brief Represents a standard mouse input device.
    class Mouse final
    {
    public:

        /// \brief The total number of supported mouse buttons.
        constexpr static UInt32 kMaxButtons = Enum::Count<Button>();

    public:

        /// \brief Constructs a new mouse input device with default state.
        ZYPHRYON_INLINE Mouse()
            : mThisX       { 0 },
              mThisY       { 0 },
              mThisScrollX { 0 },
              mThisScrollY { 0 }
        {
        }

        /// \brief Begins a new frame, updating the mouse state.
        void Begin();

        /// \brief Processes a single input event to update the mouse state.
        void Process(ConstRef<Event> Event);

        /// \brief Clears all stored mouse state.
        void Reset();

        /// \brief Gets the current X position of the mouse cursor.
        ///
        /// \return The X coordinate of the mouse cursor.
        ZYPHRYON_INLINE Real32 GetX() const
        {
            return mThisX;
        }

        /// \brief Gets the current Y position of the mouse cursor.
        ///
        /// \return The Y coordinate of the mouse cursor.
        ZYPHRYON_INLINE Real32 GetY() const
        {
            return mThisY;
        }

        /// \brief Gets the current scroll offset along the X axis.
        ///
        /// \return The scroll offset in the X direction.
        ZYPHRYON_INLINE Real32 GetScrollX() const
        {
            return mThisScrollX;
        }

        /// \brief Gets the current scroll offset along the Y axis.
        ///
        /// \return The scroll offset in the Y direction.
        ZYPHRYON_INLINE Real32 GetScrollY() const
        {
            return mThisScrollY;
        }

        /// \brief Checks if a mouse button was pressed during the current frame.
        ///
        /// \param Button The button to check.
        /// \return `true` if the button transitioned from released to pressed this frame, otherwise `false`.
        ZYPHRYON_INLINE Bool IsButtonPressed(Button Button) const
        {
            return !mLastButtons.test(Enum::Cast(Button)) && mThisButtons.test(Enum::Cast(Button));
        }

        /// \brief Checks if a mouse button is currently held down.
        ///
        /// \param Button The button to check.
        /// \return `true` if the button is currently pressed, otherwise `false`.
        ZYPHRYON_INLINE Bool IsButtonHeld(Button Button) const
        {
            return mThisButtons.test(Enum::Cast(Button));
        }

        /// \brief Checks if a mouse button was released during the current frame.
        ///
        /// \param Button The button to check.
        /// \return `true` if the button transitioned from pressed to released this frame, otherwise `false`.
        ZYPHRYON_INLINE Bool IsButtonReleased(Button Button) const
        {
            return mLastButtons.test(Enum::Cast(Button)) && !mThisButtons.test(Enum::Cast(Button));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real32              mThisX;
        Real32              mThisY;
        Real32              mThisScrollX;
        Real32              mThisScrollY;
        Bitset<kMaxButtons> mLastButtons;
        Bitset<kMaxButtons> mThisButtons;
    };
}