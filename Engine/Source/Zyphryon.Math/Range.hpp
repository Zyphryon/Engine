// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2025-2026 Zyphryon contributors (see AUTHORS.md)
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Random.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a pair of bounds a value is taken between.
    template<typename Type>
    class AnyRange final
    {
    public:

        /// \brief Constructs a range holding nothing but the type's own zero.
        ZY_INLINE constexpr AnyRange()
            : mMinimum { Type(0) },
              mMaximum { Type(0) }
        {
        }

        /// \brief Constructs a range holding a single value, which is both of its bounds.
        ///
        /// \param Value The bound to hold at either end.
        ZY_INLINE constexpr explicit AnyRange(Type Value)
            : mMinimum { Value },
              mMaximum { Value }
        {
        }

        /// \brief Constructs a range between a pair of bounds, in whichever order they are given.
        ///
        /// \param Minimum The bound at one end.
        /// \param Maximum The bound at the other.
        ZY_INLINE constexpr AnyRange(Type Minimum, Type Maximum)
            : mMinimum { ::Min(Minimum, Maximum) },
              mMaximum { ::Max(Minimum, Maximum) }
        {
        }

        /// \brief Gets the lower of the two bounds.
        ///
        /// \return The bound nothing taken from the range falls below.
        ZY_INLINE constexpr Type GetMinimum() const
        {
            return mMinimum;
        }

        /// \brief Gets the upper of the two bounds.
        ///
        /// \return The bound nothing taken from the range rises above.
        ZY_INLINE constexpr Type GetMaximum() const
        {
            return mMaximum;
        }

        /// \brief Gets how far apart the two bounds stand.
        ///
        /// \return The distance from one bound to the other, which is never negative.
        ZY_INLINE constexpr Type GetLength() const
        {
            return mMaximum - mMinimum;
        }

        /// \brief Checks whether the range holds a single value.
        ///
        /// \return `true` when both bounds are the same, `false` otherwise.
        ZY_INLINE constexpr Bool IsFixed() const
        {
            return mMinimum == mMaximum;
        }

        /// \brief Checks whether a value falls within the bounds, either of which it may sit on.
        ///
        /// \param Value The value to test.
        /// \return `true` when the value falls within the bounds, `false` otherwise.
        ZY_INLINE constexpr Bool Contains(Type Value) const
        {
            return Value >= mMinimum && Value <= mMaximum;
        }

        /// \brief Brings a value within the bounds, leaving it alone when it already sits inside them.
        ///
        /// \param Value The value to bring in.
        /// \return The value, moved onto the nearer bound when it fell outside.
        ZY_INLINE constexpr Type Clamp(Type Value) const
        {
            return ::Clamp(Value, mMinimum, mMaximum);
        }

        /// \brief Walks from one bound to the other.
        ///
        /// \param Amount How far along to walk, from zero at the lower bound to one at the upper.
        /// \return The value that far between the bounds.
        ZY_INLINE constexpr Type Lerp(Real32 Amount) const
        {
            return static_cast<Type>(mMinimum + (mMaximum - mMinimum) * Amount);
        }

        /// \brief Takes a value from somewhere between the bounds.
        ///
        /// \param Random The source the value is drawn from.
        /// \return A value between the bounds, either of which it may land on.
        ZY_INLINE Type Pick(Ref<Random> Random) const
        {
            if constexpr (IsReal<Type>)
            {
                return Random.template NextReal<Type>(mMinimum, mMaximum);
            }
            else
            {
                return Random.template NextInteger<Type>(mMinimum, mMaximum);
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type mMinimum;
        Type mMaximum;
    };

    /// \brief Represents a pair of floating-point bounds.
    using Range    = AnyRange<Real32>;

    /// \brief Represents a pair of signed integer bounds.
    using IntRange = AnyRange<SInt32>;
}