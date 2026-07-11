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
#include "Zyphryon.Math/Angle.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Represents a sound emitter with spatial properties for 3D audio.
    class Emitter final
    {
    public:

        /// \brief Constructs an emitter with default parameters.
        ZY_INLINE Emitter()
            : mAttenuation   { Attenuation::Inverse },
              mInnerRadius   { 1.0f },
              mInnerAngle    { Angle::FromDegrees(360.0f) },
              mOuterRadius   { 100.0f },
              mOuterAngle    { Angle::FromDegrees(360.0f) },
              mOuterGain     { 0.0f },
              mDopplerFactor { 1.0f }
        {
        }

        /// \brief Sets the attenuation model for the emitter.
        ///
        /// \param Attenuation The attenuation model to use.
        ZY_INLINE void SetAttenuation(Attenuation Attenuation)
        {
            mAttenuation = Attenuation;
        }

        /// \brief Gets the attenuation model of the emitter.
        ///
        /// \return The attenuation model.
        ZY_INLINE Attenuation GetAttenuation() const
        {
            return mAttenuation;
        }

        /// \brief Sets the inner and outer radius for distance attenuation.
        ///
        /// \param InnerRadius The inner radius where the sound is at full volume.
        /// \param OuterRadius The outer radius where the sound is fully attenuated.
        ZY_INLINE void SetRadius(Real32 InnerRadius, Real32 OuterRadius)
        {
            ZY_ASSERT(InnerRadius >= 0.0f, "Inner radius must be non-negative");
            ZY_ASSERT(OuterRadius >= InnerRadius, "Outer radius must be greater than or equal to inner radius");

            mInnerRadius = InnerRadius;
            mOuterRadius = OuterRadius;
        }

        /// \brief Gets the inner radius of the emitter.
        ///
        /// \return The inner radius.
        ZY_INLINE Real32 GetInnerRadius() const
        {
            return mInnerRadius;
        }

        /// \brief Gets the outer radius of the emitter.
        ///
        /// \return The outer radius.
        ZY_INLINE Real32 GetOuterRadius() const
        {
            return mOuterRadius;
        }

        /// \brief Sets the cone parameters for directional sound attenuation.
        ///
        /// \param InnerAngle The inner cone angle.
        /// \param OuterAngle The outer cone angle.
        /// \param OuterGain  The gain applied outside the outer cone (range [0, 1]).
        ZY_INLINE void SetCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain)
        {
            ZY_ASSERT(InnerAngle.IsValid(), "Inner angle must be normalized (0 <= angle < 2π)");
            ZY_ASSERT(OuterAngle.IsValid(), "Outer angle must be normalized (0 <= angle < 2π)");
            ZY_ASSERT(InnerAngle <= OuterAngle, "Inner angle cannot exceed outer angle");
            ZY_ASSERT(IsBetween(OuterGain, 0.0f, 1.0f), "Outer gain must be [0,1]");

            mInnerAngle = InnerAngle;
            mOuterAngle = OuterAngle;
            mOuterGain  = OuterGain;
        }

        /// \brief Gets the inner cone angle of the emitter.
        ///
        /// \return The inner cone angle.
        ZY_INLINE Angle GetInnerAngle() const
        {
            return mInnerAngle;
        }

        /// \brief Gets the outer cone angle of the emitter.
        ///
        /// \return The outer cone angle.
        ZY_INLINE Angle GetOuterAngle() const
        {
            return mOuterAngle;
        }

        /// \brief Gets the outer cone gain of the emitter.
        ///
        /// \return The outer cone gain.
        ZY_INLINE Real32 GetOuterGain() const
        {
            return mOuterGain;
        }

        /// \brief Sets the Doppler effect factor for the emitter.
        ///
        /// \param Factor The Doppler factor (non-negative).
        ZY_INLINE void SetDopplerFactor(Real32 Factor)
        {
            ZY_ASSERT(Factor >= 0.0f, "Doppler factor must be non-negative");

            mDopplerFactor = Factor;
        }

        /// \brief Gets the Doppler effect factor of the emitter.
        ///
        /// \return The Doppler factor.
        ZY_INLINE Real32 GetDopplerFactor() const
        {
            return mDopplerFactor;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Attenuation mAttenuation;
        Real32      mInnerRadius;
        Angle       mInnerAngle;
        Real32      mOuterRadius;
        Angle       mOuterAngle;
        Real32      mOuterGain;
        Real32      mDopplerFactor;
    };
}