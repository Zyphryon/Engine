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

#include "Emitter.hpp"
#include "Zyphryon.Math/Vector3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Computes stereo placement gains for a spatial source.
    class Spatializer final
    {
    public:

        /// \brief The stereo placement result for a single source.
        struct Gains final
        {
            /// The linear gain applied to the left output channel (range [0, 1]).
            Real32 Left  = 0.0f;

            /// The linear gain applied to the right output channel (range [0, 1]).
            Real32 Right = 0.0f;
        };

    public:

        /// \brief Constructs a spatializer with default parameters.
        ZY_INLINE Spatializer()
            : mRight             { Vector3::UnitX() },
              mForward           { -Vector3::UnitZ() },
              mListenerHalfInner { kPI<Real32> },
              mListenerHalfOuter { kPI<Real32> },
              mListenerGain      { 1.0f }
        {
        }

        /// \brief Sets the listener's world-space position and orientation basis.
        ///
        /// \param Position The listener's position.
        /// \param Forward  The listener's normalized forward axis.
        /// \param Right    The listener's normalized right axis.
        ZY_INLINE void SetListener(Vector3 Position, Vector3 Forward, Vector3 Right)
        {
            mPosition = Position;
            mForward  = Forward;
            mRight    = Right;
        }

        /// \brief Sets the listener's directional cone parameters.
        ///
        /// \param InnerAngle The inner cone angle.
        /// \param OuterAngle The outer cone angle.
        /// \param OuterGain  The gain applied outside the outer cone (range [0, 1]).
        ZY_INLINE void SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain)
        {
            mListenerHalfInner = InnerAngle.GetRadians() * 0.5f;
            mListenerHalfOuter = OuterAngle.GetRadians() * 0.5f;
            mListenerGain      = OuterGain;
        }

        /// \brief Computes the stereo placement of a source relative to the current listener.
        ///
        /// \param Position The source's world-space position.
        /// \param Forward  The source's normalized forward axis (used by the emitter cone).
        /// \param Emitter  The spatial configuration of the source.
        /// \return The channel gains for the source.
        Gains Compute(Vector3 Position, Vector3 Forward, ConstRef<Emitter> Emitter) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector3 mPosition;
        Vector3 mRight;
        Vector3 mForward;
        Real32  mListenerHalfInner;
        Real32  mListenerHalfOuter;
        Real32  mListenerGain;
    };
}
