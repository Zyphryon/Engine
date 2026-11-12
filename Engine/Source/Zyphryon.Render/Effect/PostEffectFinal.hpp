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
        friend class PostEffect;

    public:

        /// \brief Enumerates the curves the image is compressed into the display range with.
        enum class Tonemap : UInt8
        {
            Neutral,    ///< Khronos's PBR Neutral curve, which keeps colours as authored until they near white.
            ACES,       ///< The ACES filmic curve, which adds contrast and rolls highlights further.
            AgX,        ///< The AgX curve, which keeps hues as bright colours wash toward white.
        };

        /// \brief Enumerates the ways the edges of the image are smoothed as it is read.
        enum class Antialias : UInt8
        {
            None,       ///< Reads the image as it lies.
            FXAA,       ///< Smooths every edge the luma shows, at the cost of a little sharpness.
            Temporal,   ///< Steadies the image over the ones before it, then smooths and sharpens what is left.
        };

        /// \brief Represents how much light the image is given, and the curve it is rolled off with.
        struct Exposure final
        {
            /// `true` to measure the image and ease toward the exposure it asks for, `false` to hold the level alone.
            Bool    Auto    = false;

            /// The factor the image is scaled by before the curve, on top of the measured one when automatic.
            Real32  Level   = 1.0f;

            /// The least factor an automatic exposure eases to, or zero for no limit.
            Real32  Minimum = 0.25f;

            /// The most factor an automatic exposure eases to, or zero for no limit.
            Real32  Maximum = 4.0f;

            /// The curve the image is compressed into the display range with.
            Tonemap Curve   = Tonemap::ACES;

            /// \brief Serializes the state of the object to or from the specified archive.
            ///
            /// \param Archive The archive to serialize the object with.
            template<typename Serializer>
            ZY_INLINE void Serialize(Serializer Archive)
            {
                Archive.Serialize(Auto);
                Archive.Serialize(Level);
                Archive.Serialize(Minimum);
                Archive.Serialize(Maximum);
                Archive.Serialize(Curve);
            }

            /// \brief Provides the reflected members of this type.
            ///
            /// \return The fields of the type, in the order they are shown.
            ZY_INLINE static constexpr auto OnDescribe()
            {
                return Array(
                    ZyReflection::Field::Property<&Exposure::Auto>("Auto"),
                    ZyReflection::Field::Property<&Exposure::Level>("Level")
                        .Between(0.0f, 64.0f)
                        .Stepped(0.01f),
                    ZyReflection::Field::Property<&Exposure::Minimum>("Minimum")
                        .Between(0.0f, 64.0f)
                        .Stepped(0.01f),
                    ZyReflection::Field::Property<&Exposure::Maximum>("Maximum")
                        .Between(0.0f, 64.0f)
                        .Stepped(0.01f),
                    ZyReflection::Field::Property<&Exposure::Curve>("Curve"));
            }
        };

        /// \brief Represents how the exposed image is pushed around in colour before it is shown.
        struct Grade final
        {
            /// The factor the colour's distance from grey is scaled by, where zero is grey and one leaves it be.
            Real32  Saturation = 1.0f;

            /// The push black is raised or lowered by on each channel, from -0.25 to 0.25 of the display range.
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

            /// The frames a second the grain is stirred at, a film's 24 by default, or zero to hold it still.
            Real32 Cadence    = 24.0f;

            /// \brief Serializes the state of the object to or from the specified archive.
            ///
            /// \param Archive The archive to serialize the object with.
            template<typename Serializer>
            ZY_INLINE void Serialize(Serializer Archive)
            {
                Archive.Serialize(Vignette);
                Archive.Serialize(Grain);
                Archive.Serialize(Aberration);
                Archive.Serialize(Cadence);
            }

            /// \brief Provides the reflected members of this type.
            ///
            /// \return The fields of the type, in the order they are shown.
            ZY_INLINE static constexpr auto OnDescribe()
            {
                return Array(
                    ZyReflection::Field::Property<&Lens::Vignette>("Vignette")
                        .Between(0.0f, 2.0f)
                        .Stepped(0.01f),
                    ZyReflection::Field::Property<&Lens::Grain>("Grain")
                        .Between(0.0f, 1.0f)
                        .Stepped(0.01f),
                    ZyReflection::Field::Property<&Lens::Aberration>("Aberration")
                        .Between(0.0f, 16.0f)
                        .Stepped(0.1f),
                    ZyReflection::Field::Property<&Lens::Cadence>("Cadence")
                        .Between(0.0f, 60.0f)
                        .Stepped(1.0f));
            }
        };

    public:

        /// \brief Constructs the final pass.
        ///
        /// \param Host   The service host the technique is loaded through.
        /// \param Source The slot of the scene image the pass develops, whose size sets the antialiasing's step.
        PostEffectFinal(Ref<ZyEngine::Subsystem::Host> Host, UInt32 Source);

        /// \copydoc Pass::Run
        void Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph) override;

    private:

        /// \brief The frames the grain counts before it wraps, the most a float carries whole.
        static constexpr Real64 kFrames    = 16777216.0;

        /// \brief The technique developing the image through the exposure, curve, grade and lens.
        static constexpr Text   kTechnique = "Embedded://Technique/PostEffect/Final.vfx";

        /// \brief Represents the per-pass data the final technique consumes.
        struct GpuFinalLayout final
        {
            /// The exposure, the saturation, twice the vignette and the grain, in that order.
            Vector4 Grading;

            /// The offset the shadows are lifted by, with the grain's frame in `W`, below zero when nothing is graded.
            Vector4 Lift;

            /// The exponent the midtones are bent by, with the strength of the halo in `W`.
            Vector4 Gamma;

            /// The factor the highlights are gained by, with the steps between the levels in `W`, or zero for none.
            Vector4 Gain;

            /// The texel of the source across and up, then the aberration across the image in texels.
            Vector4 Lens;
        };

        /// \brief Sets how much light the image is given, and the curve it is rolled off with.
        ///
        /// \param Exposure The exposure to give the image.
        ZY_INLINE void SetExposure(ConstRef<Exposure> Exposure)
        {
            mExposure = Exposure;
        }

        /// \brief Sets how the exposed image is pushed around in colour.
        ///
        /// \param Grade The grade to push the colour by.
        ZY_INLINE void SetGrade(ConstRef<Grade> Grade)
        {
            mGrade = Grade;
        }

        /// \brief Sets what the glass the image is seen through leaves on it.
        ///
        /// \param Lens The lens to see the image through.
        ZY_INLINE void SetLens(ConstRef<Lens> Lens)
        {
            mLens = Lens;
        }

        /// \brief Sets the way the edges of the image are smoothed as it is read.
        ///
        /// \param Antialias The way to smooth them, or \ref Antialias::None to read the image as it lies.
        ZY_INLINE void SetAntialias(Antialias Antialias)
        {
            mAntialias = Antialias;
        }

        /// \brief Sets the share of the bloom's halo laid back over the image.
        ///
        /// \param Halo The share to lay back, or zero to lay none.
        ZY_INLINE void SetHalo(Real32 Halo)
        {
            mHalo = Halo;
        }

        /// \brief Sets the time the grain is stirred by, counted into frames at the lens's cadence.
        ///
        /// \param Time The time of the frame, in seconds.
        ZY_INLINE void SetTime(Real64 Time)
        {
            const Real64 Count = Floor(Time * mLens.Cadence);
            mFrame = static_cast<UInt32>(Count - Floor(Count / kFrames) * kFrames);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Technique> mTechnique;
        UInt32                         mSource;
        Antialias                      mAntialias;
        Exposure                       mExposure;
        Grade                          mGrade;
        Lens                           mLens;
        Real32                         mHalo;
        UInt32                         mFrame;
    };
}