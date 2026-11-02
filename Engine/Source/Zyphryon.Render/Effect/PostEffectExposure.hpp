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

#include "PostEffectFinal.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents one step of the exposure, which measures how bright the image is and eases toward it.
    class ZY_API PostEffectExposure final : public Pass
    {
    public:

        /// \brief The texels the meter reads the image into along each axis, all of which the adaptation averages.
        static constexpr UInt16 kMeter = 32;

        /// \brief Enumerates the step of the exposure an instance of the pass takes.
        enum class Sweep : UInt8
        {
            Measure, ///< Reads the image into the meter, as the log of its brightness weighted toward the middle.
            Adapt,   ///< Averages the meter and eases the exposure kept from the last frame toward what it asks for.
            Keep,    ///< Copies the exposure eased this frame into the one the next frame eases from.
        };

    public:

        /// \brief Constructs a step of the exposure.
        ///
        /// \param Host The service host the technique is loaded through.
        /// \param Axis The step this instance takes.
        PostEffectExposure(Ref<ZyEngine::Subsystem::Host> Host, Sweep Axis);

        /// \copydoc Pass::Run
        void Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph) override;

        /// \brief Sets how much light the image is given, which the adaptation reads its mode and range from.
        ///
        /// \param Exposure The exposure to give the image.
        void SetExposure(ConstRef<PostEffectFinal::Exposure> Exposure);

        /// \brief Sets the time the adaptation eases by, measured from the time it was last given.
        ///
        /// \param Time The time of the frame, in seconds.
        void SetTime(Real64 Time);

    private:

        /// \brief The time the exposure takes to settle on a brighter image, in seconds.
        static constexpr Real32 kBrighten  = 1.0f;

        /// \brief The time the exposure takes to settle on a darker image, in seconds, slower as an eye is.
        static constexpr Real32 kDarken    = 3.0f;

        /// \brief The longest step the adaptation eases by, in seconds, so a stalled frame never snaps.
        static constexpr Real64 kStep      = 0.25;

        /// \brief The stops an exposure with no limit of its own may stray either way.
        static constexpr Real32 kStops     = 16.0f;

        /// \brief The technique every step of the exposure draws with.
        static constexpr Text   kTechnique = "Embedded://Technique/PostEffect/Exposure.vfx";

        /// \brief Represents the per-pass data the exposure technique consumes.
        struct GpuExposureLayout final
        {
            /// The meter's texel to measure by, or the shares eased toward brighter and darker and the stops allowed.
            Vector4 Filter;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Technique> mTechnique;
        Sweep                          mSweep;
        Bool                           mAuto;
        Bool                           mFresh;
        Vector2                        mRange;
        Vector2                        mShares;
        Real64                         mTime;
    };
}