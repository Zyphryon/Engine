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
#include "Zyphryon.Math/Matrix4x4.hpp"
#include "Zyphryon.Render/Graph.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the search of the occlusion, which finds the share of the sky each texel's surface sees.
    class ZY_API OcclusionHorizon final : public Pass
    {
    public:

        /// \brief Enumerates how finely the horizons are searched.
        enum class Detail : UInt8
        {
            Off,      ///< Searches nothing, leaving every surface open to the whole sky.
            Basic,    ///< Searches one slice in four steps.
            Reduced,  ///< Searches two slices in four steps.
            Standard, ///< Searches two slices in six steps.
        };

        /// \brief Represents how much the sky is held back where surfaces close in on each other.
        struct Shading final
        {
            /// The share of the sky a closed-in surface loses, where zero leaves every surface open.
            Real32 Strength = 1.0f;

            /// The farthest a surface looks for what closes it in, in world units.
            Real32 Radius   = 2.0f;

            /// \brief Serializes the state of the object to or from the specified archive.
            ///
            /// \param Archive The archive to serialize the object with.
            template<typename Serializer>
            ZY_INLINE void Serialize(Serializer Archive)
            {
                Archive.Serialize(Strength);
                Archive.Serialize(Radius);
            }

            /// \brief Provides the reflected members of this type.
            ///
            /// \return The fields of the type, in the order they are shown.
            ZY_INLINE static constexpr auto OnDescribe()
            {
                return Array(
                    ZyReflection::Field::Property<&Shading::Strength>("Strength")
                        .Between(0.0f, 1.0f)
                        .Stepped(0.01f),
                    ZyReflection::Field::Property<&Shading::Radius>("Radius")
                        .Between(0.1f, 8.0f)
                        .Stepped(0.05f));
            }
        };

    public:

        /// \brief Constructs the search of the occlusion.
        ///
        /// \param Host The service host the technique is loaded through.
        explicit OcclusionHorizon(Ref<ZyEngine::Subsystem::Host> Host);

        /// \brief Sets how finely the horizons are searched.
        ///
        /// \param Detail The detail to search at, whose \ref Detail::Off the chain answers by switching the pass off.
        ZY_INLINE void SetDetail(Detail Detail)
        {
            mDetail = Detail;
        }

        /// \brief Sets how far a surface looks for what closes it in.
        ///
        /// \param Radius The farthest to look, in world units.
        ZY_INLINE void SetRadius(Real32 Radius)
        {
            mRadius = Radius;
        }

        /// \brief Sets the view the reach was laid along, orthographic or perspective as its matrix tells.
        ///
        /// \param Inverse The matrix that carries clip space back into the space the surfaces are searched in.
        void SetView(ConstRef<Matrix4x4> Inverse);

        /// \copydoc Pass::Run
        void Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph) override;

    private:

        /// \brief The technique the search draws with.
        static constexpr Text kTechnique   = "Embedded://Technique/Occlusion/Horizon.vfx";

        /// \brief Names the variant compiled for a view whose lines of sight leave from one point.
        static constexpr Text kPerspective = "Perspective";

        /// \brief Represents the per-pass data the search technique consumes.
        struct GpuHorizonLayout final
        {
            /// The matrix that carries clip space back into the space the surfaces are searched in.
            Matrix4x4 Inverse;

            /// The direction the view looks along, of unit length, then the radius in world units.
            Vector4   Toward;

            /// The point every line of sight leaves from, unused for an orthographic view.
            Vector4   Eye;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Technique> mTechnique;
        Detail                         mDetail;
        Real32                         mRadius;
        Matrix4x4                      mInverse;
        Vector3                        mToward;
        Vector3                        mEye;
        Bool                           mPerspective;
    };
}