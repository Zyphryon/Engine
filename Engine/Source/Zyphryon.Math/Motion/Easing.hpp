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

#include "Zyphryon.Math/Angle.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    /// \brief Specifies the curve an eased value follows from start to end.
    enum class Easing : UInt8
    {
        Linear,        ///< No easing, a constant rate.
        InQuad,        ///< Quadratic acceleration.
        OutQuad,       ///< Quadratic deceleration.
        InOutQuad,     ///< Quadratic acceleration, then deceleration.
        InCubic,       ///< Cubic acceleration.
        OutCubic,      ///< Cubic deceleration.
        InOutCubic,    ///< Cubic acceleration, then deceleration.
        Smoothstep,    ///< Smooth acceleration, then deceleration.
        InSine,        ///< Sinusoidal acceleration.
        OutSine,       ///< Sinusoidal deceleration.
        InOutSine,     ///< Sinusoidal acceleration, then deceleration.
        InExpo,        ///< Exponential acceleration.
        OutExpo,       ///< Exponential deceleration.
        InCirc,        ///< Circular acceleration.
        OutCirc,       ///< Circular deceleration.
        InElastic,     ///< Elastic overshoot while accelerating.
        OutElastic,    ///< Elastic overshoot while decelerating.
        InOutElastic,  ///< Elastic overshoot at both ends.
        InBounce,      ///< Bounce while accelerating.
        OutBounce,     ///< Bounce while decelerating.
        InOutBounce,   ///< Bounce at both ends.
        InBack,        ///< Overshoot while accelerating.
        OutBack,       ///< Overshoot while decelerating.
        InOutBack      ///< Overshoot at both ends.
    };

    /// \brief Eases linearly, leaving the time as it is.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The unmodified time value.
    template<typename Type>
    constexpr Type Linear(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time;
    }

    /// \brief Eases in quadratically, accelerating from zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following quadratic acceleration.
    template<typename Type>
    constexpr Type InQuad(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time * Time;
    }

    /// \brief Eases out quadratically, decelerating to zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following quadratic deceleration.
    template<typename Type>
    constexpr Type OutQuad(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time * (2 - Time);
    }

    /// \brief Eases in and out quadratically, accelerating until halfway and then decelerating.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with quadratic acceleration/deceleration.
    template<typename Type>
    constexpr Type InOutQuad(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time < 0.5 ? 2 * Time * Time : -1 + (4 - 2 * Time) * Time;
    }

    /// \brief Eases in cubically, accelerating from zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following cubic acceleration.
    template<typename Type>
    constexpr Type InCubic(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time * Time * Time;
    }

    /// \brief Eases out cubically, decelerating to zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following cubic deceleration.
    template<typename Type>
    constexpr Type OutCubic(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        --Time;
        return Time * Time * Time + 1;
    }

    /// \brief Eases in and out cubically, accelerating until halfway and then decelerating.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with cubic acceleration/deceleration.
    template<typename Type>
    constexpr Type InOutCubic(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time < 0.5 ? 4 * Time * Time * Time : (Time - 1) * (2 * Time - 2) * (2 * Time - 2) + 1;
    }

    /// \brief Eases in and out along the smoothstep curve.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The smoothed eased value.
    template<typename Type>
    constexpr Type Smoothstep(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time * Time * (3 - 2 * Time);
    }

    /// \brief Eases in along a sine curve, accelerating from zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following sinusoidal acceleration.
    template<typename Type>
    ZY_INLINE Type InSine(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Type(1) - Angle::Cosine(Time * kPI<Type> / 2);
    }

    /// \brief Eases out along a sine curve, decelerating to zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following sinusoidal deceleration.
    template<typename Type>
    ZY_INLINE Type OutSine(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Angle::Sine(Time * kPI<Type> / 2);
    }

    /// \brief Eases in and out along a sine curve, accelerating until halfway and then decelerating.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with sinusoidal acceleration/deceleration.
    template<typename Type>
    ZY_INLINE Type InOutSine(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return -(Angle::Cosine(kPI<Type> * Time) - 1) / 2;
    }

    /// \brief Eases in exponentially, accelerating from zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following exponential acceleration.
    template<typename Type>
    ZY_INLINE Type InExpo(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time == 0 ? 0 : Pow(Type(2), 10 * (Time - 1));
    }

    /// \brief Eases out exponentially, decelerating to zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following exponential deceleration.
    template<typename Type>
    ZY_INLINE Type OutExpo(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time == 1 ? 1 : 1 - Pow(Type(2), -10 * Time);
    }

    /// \brief Eases in along a circular arc, accelerating from zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following circular acceleration.
    template<typename Type>
    ZY_INLINE Type InCirc(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return 1 - Sqrt(1 - Time * Time);
    }

    /// \brief Eases out along a circular arc, decelerating to zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value following circular deceleration.
    template<typename Type>
    ZY_INLINE Type OutCirc(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        --Time;
        return Sqrt(1 - Time * Time);
    }

    /// \brief Eases in with an elastic overshoot, accelerating from zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with elastic spring effect (acceleration).
    template<typename Type>
    ZY_INLINE Type InElastic(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time == 0 ? 0 : Time == 1 ? 1
            : -Pow(Type(2), 10 * Time - 10) * Angle::Sine((Time * 10 - 10.75) * (kTwoPI<Type> / 3));
    }

    /// \brief Eases out with an elastic overshoot, decelerating to zero velocity.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with elastic spring effect (deceleration).
    template<typename Type>
    ZY_INLINE Type OutElastic(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        constexpr Type C4 = kTwoPI<Type> / 3;
        return Time == 0 ? 0 : Time == 1 ? 1 : Pow(Type(2), -10 * Time) * Angle::Sine((Time * 10 - 0.75) * C4) + 1;
    }

    /// \brief Eases in and out with an elastic overshoot at both ends.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with elastic spring effect (both directions).
    template<typename Type>
    ZY_INLINE Type InOutElastic(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        constexpr Type C5 = kTwoPI<Type> / 4.5;
        return Time == 0 ? 0 : Time == 1 ? 1 : Time < 0.5
            ? -(Pow(Type(2), 20 * Time - 10) * Angle::Sine((20 * Time - 11.125) * C5)) / 2
            : (Pow(Type(2), -20 * Time + 10) * Angle::Sine((20 * Time - 11.125) * C5)) / 2 + 1;
    }

    /// \brief Eases out with a bounce, decelerating into the end.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with bounce effect (deceleration).
    template<typename Type>
    constexpr Type OutBounce(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        constexpr Type N1 = 7.5625;
        constexpr Type D1 = 2.75;

        if (Time < 1 / D1)
        {
            return N1 * Time * Time;
        }
        if (Time < 2 / D1)
        {
            Time -= 1.5 / D1;
            return N1 * Time * Time + 0.75;
        }
        if (Time < 2.5 / D1)
        {
            Time -= 2.25 / D1;
            return N1 * Time * Time + 0.9375;
        }

        Time -= 2.625 / D1;
        return N1 * Time * Time + 0.984375;
    }

    /// \brief Eases in with a bounce, accelerating away from the start.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with bounce effect (acceleration).
    template<typename Type>
    constexpr Type InBounce(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return 1 - OutBounce(1 - Time);
    }

    /// \brief Eases in and out with a bounce at both ends.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with bounce effect (both directions).
    template<typename Type>
    constexpr Type InOutBounce(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        return Time < 0.5 ? (1 - OutBounce(1 - 2 * Time)) / 2 : (1 + OutBounce(2 * Time - 1)) / 2;
    }

    /// \brief Eases in with an overshoot, pulling back before it accelerates.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with overshoot (acceleration).
    template<typename Type>
    constexpr Type InBack(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        constexpr Type C1 = 1.70158;
        constexpr Type C3 = C1 + 1;
        return C3 * Time * Time * Time - C1 * Time * Time;
    }

    /// \brief Eases out with an overshoot, passing the end before it settles.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with overshoot (deceleration).
    template<typename Type>
    ZY_INLINE Type OutBack(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        constexpr Type C1 = 1.70158;
        constexpr Type C3 = C1 + 1;
        return 1 + C3 * Pow(Time - 1, Type(3)) + C1 * Pow(Time - 1, Type(2));
    }

    /// \brief Eases in and out with an overshoot at both ends.
    ///
    /// \param Time The normalized time value in [0, 1].
    /// \return The eased value with overshoot (both directions).
    template<typename Type>
    ZY_INLINE Type InOutBack(Type Time)
    {
        ZY_ASSERT(Time >= Type(0) && Time <= Type(1), "Easing time must be normalized in [0, 1]");

        constexpr Type C2 = 1.70158 * 1.525;
        return Time < 0.5
            ? (Pow(2 * Time, Type(2)) * ((C2 + 1) * 2 * Time - C2)) / 2
            : (Pow(2 * Time - 2, Type(2)) * ((C2 + 1) * (Time * 2 - 2) + C2) + 2) / 2;
    }

    /// \brief Applies the specified easing function to a time value.
    ///
    /// \param Function The easing function to apply.
    /// \param Time     The normalized time value in [0, 1].
    /// \return The eased value according to the specified function.
    template<typename Type>
    ZY_INLINE Type Ease(Easing Function, Type Time)
    {
        switch (Function)
        {
        case Easing::Linear:
            return Linear(Time);
        case Easing::InQuad:
            return InQuad(Time);
        case Easing::OutQuad:
            return OutQuad(Time);
        case Easing::InOutQuad:
            return InOutQuad(Time);
        case Easing::InCubic:
            return InCubic(Time);
        case Easing::OutCubic:
            return OutCubic(Time);
        case Easing::InOutCubic:
            return InOutCubic(Time);
        case Easing::Smoothstep:
            return Smoothstep(Time);
        case Easing::InSine:
            return InSine(Time);
        case Easing::OutSine:
            return OutSine(Time);
        case Easing::InOutSine:
            return InOutSine(Time);
        case Easing::InExpo:
            return InExpo(Time);
        case Easing::OutExpo:
            return OutExpo(Time);
        case Easing::InCirc:
            return InCirc(Time);
        case Easing::OutCirc:
            return OutCirc(Time);
        case Easing::InElastic:
            return InElastic(Time);
        case Easing::OutElastic:
            return OutElastic(Time);
        case Easing::InOutElastic:
            return InOutElastic(Time);
        case Easing::InBounce:
            return InBounce(Time);
        case Easing::OutBounce:
            return OutBounce(Time);
        case Easing::InOutBounce:
            return InOutBounce(Time);
        case Easing::InBack:
            return InBack(Time);
        case Easing::OutBack:
            return OutBack(Time);
        case Easing::InOutBack:
            return InOutBack(Time);
        default:
            return Linear(Time);
        }
    }
}