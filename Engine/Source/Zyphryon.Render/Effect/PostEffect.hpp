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

#include "PostEffectBloom.hpp"
#include "PostEffectExposure.hpp"
#include "PostEffectTemporal.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the post effect chain, which carries a scene image through bloom and exposure onto the output.
    class ZY_API PostEffect final
    {
    public:

        /// \brief Names the ways the edges of the image are smoothed as it is read.
        using Antialias = PostEffectFinal::Antialias;

        /// \brief Names how much light the image is given, and the curve it is rolled off with.
        using Exposure  = PostEffectFinal::Exposure;

        /// \brief Names how the exposed image is pushed around in colour before it is shown.
        using Grade     = PostEffectFinal::Grade;

        /// \brief Names what the glass the image is seen through leaves on it.
        using Lens      = PostEffectFinal::Lens;

        /// \brief Names the curves the image is compressed into the display range with.
        using Tonemap   = PostEffectFinal::Tonemap;

        /// \brief Represents the glow the brightest of the image is allowed to spill.
        struct Bloom final
        {
            /// The share of the glow laid back over the image, or zero to gather none at all.
            Real32 Strength  = 0.0f;

            /// The brightness a pixel has to pass before it spills, in scene units where one is white.
            Real32 Threshold = 1.0f;

            /// \brief Serializes the state of the object to or from the specified archive.
            ///
            /// \param Archive The archive to serialize the object with.
            template<typename Serializer>
            ZY_INLINE void Serialize(Serializer Archive)
            {
                Archive.Serialize(Strength);
                Archive.Serialize(Threshold);
            }

            /// \brief Provides the reflected members of this type.
            ///
            /// \return The fields of the type, in the order they are shown.
            ZY_INLINE static constexpr auto OnDescribe()
            {
                return Array(
                    ZyReflection::Field::Property<&Bloom::Strength>("Strength")
                        .Between(0.0f, 4.0f)
                        .Stepped(0.01f),
                    ZyReflection::Field::Property<&Bloom::Threshold>("Threshold")
                        .Between(0.0f, 64.0f)
                        .Stepped(0.05f));
            }
        };

    public:

        /// \brief Constructs the chain, declaring its targets and appending its passes to the blueprint.
        ///
        /// \param Host      The service host the techniques are loaded through.
        /// \param Blueprint The blueprint the chain declares its targets and passes into.
        /// \param Scene     The slot of the scene image the chain reads, whose format the bloom is gathered in.
        /// \param Output    The slot the final image is written into, or \ref Pass::kNone for the display surface.
        PostEffect(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint, UInt32 Scene, UInt32 Output);

        /// \brief Stirs the grain and eases the exposure by the time given.
        ///
        /// \param Time The time of the frame, in seconds.
        void Advance(Real64 Time);

        /// \brief Sets the glow the brightest of the image is allowed to spill.
        ///
        /// \param Bloom The glow to allow.
        void SetBloom(ConstRef<Bloom> Bloom);

        /// \brief Sets the fraction of the output the first level of the bloom is gathered at.
        ///
        /// \param Sizing The fraction to gather it at, which every level below halves again.
        ZY_INLINE void SetBloomScale(Target::Scale Sizing)
        {
            mBloom.SetScale(Sizing);
        }

        /// \brief Sets how much light the image is given, and the curve it is rolled off with.
        ///
        /// \param Exposure The exposure to give the image.
        ZY_INLINE void SetExposure(ConstRef<Exposure> Exposure)
        {
            mFinal->SetExposure(Exposure);
            mExposure.SetExposure(Exposure);
        }

        /// \brief Sets how the exposed image is pushed around in colour.
        ///
        /// \param Grade The grade to push the colour by.
        ZY_INLINE void SetGrade(ConstRef<Grade> Grade)
        {
            mFinal->SetGrade(Grade);
        }

        /// \brief Sets what the glass the image is seen through leaves on it.
        ///
        /// \param Lens The lens to see the image through.
        ZY_INLINE void SetLens(ConstRef<Lens> Lens)
        {
            mFinal->SetLens(Lens);
        }

        /// \brief Sets the way the edges of the image are smoothed as it is read.
        ///
        /// \param Antialias The way to smooth them, or \ref Antialias::None to read the image as it lies.
        ZY_INLINE void SetAntialias(Antialias Antialias)
        {
            mFinal->SetAntialias(Antialias);
            mTemporal.SetActive(Antialias == PostEffect::Antialias::Temporal);
        }

        /// \brief Gets where the next image is to be drawn for the temporal smoothing to blend it back.
        ///
        /// \return The offset from where the view stands, in pixels, or zero unless the image is smoothed over time.
        ZY_INLINE Vector2 GetJitter() const
        {
            return mTemporal.GetJitter();
        }

        /// \brief Sets where a point standing still was on the screen in the image before.
        ///
        /// \param Shift The move from this image back to the last, in clip space.
        /// \param Scale The size the last image stood at against this one along each axis, one when the zoom held.
        ZY_INLINE void SetShift(Vector2 Shift, Vector2 Scale)
        {
            mTemporal.SetShift(Shift, Scale);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        PostEffectTemporal   mTemporal;
        PostEffectBloom      mBloom;
        PostEffectExposure   mExposure;
        Ptr<PostEffectFinal> mFinal;
    };
}