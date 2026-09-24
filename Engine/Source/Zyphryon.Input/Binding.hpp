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
    /// \brief Specifies what physical input a binding reads.
    enum class Source : UInt8
    {
        Key,        ///< A keyboard key, reading one while it is held.
        Button,     ///< A mouse button, reading one while it is held.
        Pair,       ///< Two keyboard keys, reading minus one for the negative key and plus one for the positive.
        Axis,       ///< A mouse axis, reading how far it moved during the frame.
    };

    /// \brief Specifies which mouse axis a binding reads.
    enum class Axis : UInt8
    {
        MoveX,      ///< Horizontal movement of the cursor.
        MoveY,      ///< Vertical movement of the cursor.
        ScrollX,    ///< Horizontal scroll of the wheel.
        ScrollY,    ///< Vertical scroll of the wheel.
    };

    /// \brief Represents one physical input feeding an action, and the scale it feeds it at.
    struct Binding final
    {
        /// The kind of input the binding reads.
        Source Kind     = Source::Key;

        /// The key read by a key binding, or the key that reads plus one in a pair.
        Key    Positive = Key::Unknown;

        /// The key that reads minus one in a pair.
        Key    Negative = Key::Unknown;

        /// The mouse button read by a button binding.
        Button Button   = Button::Unknown;

        /// The mouse axis read by an axis binding.
        Axis   Axis     = Axis::MoveX;

        /// The value the input is multiplied by before it reaches the action.
        Real32 Scale    = 1.0f;

        /// \brief Creates a binding that reads a keyboard key.
        ///
        /// \param Key   The key to read.
        /// \param Scale The value the action reads while the key is held.
        /// \return The binding.
        ZY_INLINE static constexpr Binding FromKey(Key Key, Real32 Scale = 1.0f)
        {
            Binding Result;
            Result.Kind     = Source::Key;
            Result.Positive = Key;
            Result.Scale    = Scale;
            return Result;
        }

        /// \brief Creates a binding that reads a mouse button.
        ///
        /// \param Button The button to read.
        /// \param Scale  The value the action reads while the button is held.
        /// \return The binding.
        ZY_INLINE static constexpr Binding FromButton(ZyInput::Button Button, Real32 Scale = 1.0f)
        {
            Binding Result;
            Result.Kind   = Source::Button;
            Result.Button = Button;
            Result.Scale  = Scale;
            return Result;
        }

        /// \brief Creates a binding that reads two keyboard keys as the two ends of one axis.
        ///
        /// \param Negative The key that reads minus one.
        /// \param Positive The key that reads plus one.
        /// \param Scale    The value each end is multiplied by.
        /// \return The binding.
        ZY_INLINE static constexpr Binding FromKeys(Key Negative, Key Positive, Real32 Scale = 1.0f)
        {
            Binding Result;
            Result.Kind     = Source::Pair;
            Result.Negative = Negative;
            Result.Positive = Positive;
            Result.Scale    = Scale;
            return Result;
        }

        /// \brief Creates a binding that reads a mouse axis.
        ///
        /// \param Axis  The axis to read.
        /// \param Scale The value one unit of movement reads as.
        /// \return The binding.
        ZY_INLINE static constexpr Binding FromAxis(ZyInput::Axis Axis, Real32 Scale = 1.0f)
        {
            Binding Result;
            Result.Kind  = Source::Axis;
            Result.Axis  = Axis;
            Result.Scale = Scale;
            return Result;
        }
    };
}