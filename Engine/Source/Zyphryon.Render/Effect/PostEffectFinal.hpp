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

#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Render/Graph.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the last pass of a post effect chain, which develops the image into what the display shows.
    class ZY_API PostEffectFinal final : public Pass
    {
    public:

        /// \brief Enumerates the curves the image is compressed into the display range with.
        enum class Tonemap : UInt8
        {
            GT7,        ///< Uchimura's Gran Turismo curve, which leaves the midtones where they were authored.
            ACES,       ///< The ACES filmic curve, which adds contrast and rolls highlights further.
        };

        /// \brief Enumerates the ways the edges of the image are smoothed as it is read.
        enum class Antialias : UInt8
        {
            None,       ///< Reads the image as it lies.
            FXAA,       ///< Smooths every edge the luma shows, at the cost of a little sharpness.
        };

        /// \brief Represents how much light the image is given, and the curve it is rolled off with.
        struct Exposure final
        {
            /// The factor the image is scaled by before the curve compresses it, where one leaves it be.
            Real32  Level = 1.0f;

            /// The curve the image is compressed into the display range with.
            Tonemap Curve = Tonemap::GT7;
        };

        /// \brief Represents how the exposed image is pushed around in colour before it is shown.
        struct Grade final
        {
            /// The factor the colour's distance from grey is scaled by, where zero is grey and one leaves it be.
            Real32  Saturation = 1.0f;

            /// The push the shadows are lifted by on each channel, from -1 to 1 and zero to leave them be.
            Vector3 Lift       = Vector3::Zero();

            /// The push the midtones are bent by on each channel, from -1 to 1 and zero to leave them be.
            Vector3 Gamma      = Vector3::Zero();

            /// The push the highlights are gained by on each channel, from -1 to 1 and zero to leave them be.
            Vector3 Gain       = Vector3::Zero();

            /// The number of steps each channel is rounded to, or one and below to leave it as it lies.
            Real32  Posterize  = 0.0f;
        };

        /// \brief Represents what the glass the image is seen through leaves on it.
        struct Lens final
        {
            /// The strength the corners are drawn down with, or zero to leave them alone.
            Real32 Vignette   = 0.0f;

            /// The strength of the grain the image shows, or zero to show none.
            Real32 Grain      = 0.0f;

            /// The distance red and blue part from green at the corners, in pixels, or zero to keep them together.
            Real32 Aberration = 0.0f;
        };

    public:

        /// \brief Constructs the final pass.
        ///
        /// \param Host    The service host the technique is loaded through.
        /// \param Source  The slot of the scene image the pass develops, whose size sets the antialiasing's step.
        /// \param Surface `true` when the pass writes straight onto the display surface, `false` for a target.
        PostEffectFinal(Ref<ZyEngine::Subsystem::Host> Host, UInt32 Source, Bool Surface);

        /// \brief Sets how much light the image is given, and the curve it is rolled off with.
        ///
        /// \param Exposure The exposure to give the image.
        ZY_INLINE void SetExposure(ConstRef<Exposure> Exposure)
        {
            mExposure = Exposure;
        }

        /// \brief Gets how much light the image is given, and the curve it is rolled off with.
        ///
        /// \return The exposure the image is given.
        ZY_INLINE ConstRef<Exposure> GetExposure() const
        {
            return mExposure;
        }

        /// \brief Sets how the exposed image is pushed around in colour.
        ///
        /// \param Grade The grade to push the colour by.
        ZY_INLINE void SetGrade(ConstRef<Grade> Grade)
        {
            mGrade = Grade;
        }

        /// \brief Gets how the exposed image is pushed around in colour.
        ///
        /// \return The grade the colour is pushed by.
        ZY_INLINE ConstRef<Grade> GetGrade() const
        {
            return mGrade;
        }

        /// \brief Sets what the glass the image is seen through leaves on it.
        ///
        /// \param Lens The lens to see the image through.
        ZY_INLINE void SetLens(ConstRef<Lens> Lens)
        {
            mLens = Lens;
        }

        /// \brief Gets what the glass the image is seen through leaves on it.
        ///
        /// \return The lens the image is seen through.
        ZY_INLINE ConstRef<Lens> GetLens() const
        {
            return mLens;
        }

        /// \brief Sets the way the edges of the image are smoothed as it is read.
        ///
        /// \param Antialias The way to smooth them, or \ref Antialias::None to read the image as it lies.
        ZY_INLINE void SetAntialias(Antialias Antialias)
        {
            mAntialias = Antialias;
        }

        /// \brief Gets the way the edges of the image are smoothed as it is read.
        ///
        /// \return The way they are smoothed, which is \ref Antialias::None until one is set.
        ZY_INLINE Antialias GetAntialias() const
        {
            return mAntialias;
        }

        /// \brief Sets the share of the bloom's halo laid back over the image.
        ///
        /// \param Halo The share to lay back, or zero to lay none.
        ZY_INLINE void SetHalo(Real32 Halo)
        {
            mHalo = Halo;
        }

        /// \brief Sets the frame the grain is stirred by.
        ///
        /// \param Frame The frame, which a new value reseeds the grain with.
        ZY_INLINE void SetFrame(UInt32 Frame)
        {
            mFrame = Frame;
        }

        /// \copydoc Pass::Run
        void Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph) override;

    private:

        /// \brief The furthest a fully pushed lift may walk the shadows, in display range.
        static constexpr Real32 kLiftRange = 0.25f;

        /// \brief The technique developing the image through the exposure, curve, grade and lens.
        static constexpr Text   kTechnique = "Embedded://Technique/PostEffect/Final.vfx";

        /// \brief Represents the per-pass data the final technique consumes.
        struct GpuFinalLayout final
        {
            /// The exposure, the saturation, the vignette and the grain, in that order.
            Vector4 Grading;

            /// The offset the shadows are lifted by, with the grain's frame in `W`, below zero when nothing is graded.
            Vector4 Lift;

            /// The exponent the midtones are bent by, with the strength of the halo in `W`.
            Vector4 Gamma;

            /// The factor the highlights are gained by, with the levels each channel is stepped to in `W`.
            Vector4 Gain;

            /// A texel of the source across and up, then the aberration in pixels.
            Vector4 Lens;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Technique> mTechnique;
        UInt32                         mSource;
        Bool                           mSurface;
        Antialias                      mAntialias;
        Exposure                       mExposure;
        Grade                          mGrade;
        Lens                           mLens;
        Real32                         mHalo;
        UInt32                         mFrame;
    };
}