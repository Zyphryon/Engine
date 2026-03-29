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
#include "Zyphryon.Content/Resource.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a pipeline asset within the content system.
    class Pipeline final : public Content::AbstractResource<Pipeline>
    {
        friend class AbstractResource;

    public:

        /// \brief Defines the binding tier for pipeline resources.
        enum class Tier : UInt8
        {
            Global = 0, ///< Bound at application or frame scope.
            Effect = 1, ///< Bound per effect or render pass.
            Style  = 2, ///< Bound per material or visual style.
            Object = 3, ///< Bound per individual draw or object.
        };

    public:

        /// \brief Constructs a pipeline resource with the given content key.
        ///
        /// \param Key The unique content key identifying this pipeline.
        explicit Pipeline(AnyRef<Content::Uri> Key);

        /// \brief Loads the pipeline with the specified program, textures, and properties.
        ///
        /// \param Shaders  The shader program bytecode for the pipeline.
        /// \param States   The pipeline states descriptor.
        /// \param Textures The list of texture bindings for the pipeline.
        void Load(AnyRef<Shaders> Shaders, AnyRef<States> States, AnyRef<EntryList<TextureSemantic, kMaxResources>> Textures);

        /// \brief Gets the unique identifier of the pipeline.
        ///
        /// \return The unique identifier of the pipeline.
        ZYPHRYON_INLINE Object GetID() const
        {
            return mID;
        }

        /// \brief Gets the list of texture bindings for the pipeline
        ///
        /// \return A list of texture bindings.
        ZYPHRYON_INLINE ConstSpan<Entry<TextureSemantic>> GetTextures() const
        {
            return mTextures;
        }

        /// \brief Gets the fixed states for the pipeline.
        ///
        /// \return The fixed states.
        ZYPHRYON_INLINE ConstRef<States> GetStates() const
        {
            return mStates;
        }

    private:

        /// \copydoc Resource::OnCreate(Ref<Service::Host>)
        Bool OnCreate(Ref<Service::Host> Host) override;

        /// \copydoc Resource::OnDelete(Ref<Service::Host>)
        void OnDelete(Ref<Service::Host> Host) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Object                                    mID;
        Shaders                                   mShaders;
        States                                    mStates;
        EntryList<TextureSemantic, kMaxResources> mTextures;      /// TODO: Reflection
    };
}