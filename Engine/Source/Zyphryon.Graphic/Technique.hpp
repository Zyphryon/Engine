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

#include "Schema.hpp"
#include "Shader.hpp"
#include "Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a rendering technique containing a configured pipeline.
    class Technique final : public Content::AbstractResource<Technique>
    {
    public:

        /// \brief Type alias for an array of shader modules indexed by pipeline stage.
        using Shaders = Array<Retainer<Shader>, Enum::Count<ShaderStage>()>;

        /// \brief Describes the configuration for a rendering technique.
        struct Description
        {
            /// Preprocessor macros used during shader compilation.
            Sequence<Macro> Macros;

            /// The resource binding schema for the pipeline.
            Schema          Schema;

            /// The fixed-function pipeline states.
            States          States;

            /// The shader stages for each pipeline stage.
            Shaders         Shaders;
        };

    public:

        /// \brief Constructs a technique resource with the given content key.
        ///
        /// \param Key The unique content key identifying this pipeline.
        explicit Technique(AnyRef<Content::Uri> Key);

        /// \brief Sets up the technique with the given description.
        ///
        /// \param Description The description containing the technique's configuration.
        void Setup(AnyRef<Description> Description);

        /// \brief Gets the GPU pipeline handle for this technique.
        ///
        /// \return The technique's GPU pipeline object handle.
        ZY_INLINE Object GetHandle() const
        {
            return mHandle;
        }

        /// \brief Gets the technique's configuration description.
        ///
        /// \return The technique description.
        ZY_INLINE ConstRef<Description> GetDescription() const
        {
            return mDescription;
        }

    public:

        /// \brief Uploads the technique to the GPU, creating the pipeline resource.
        ///
        /// \param Service The graphic service used to create the resource.
        /// \return `true` if the upload succeeded, `false` otherwise.
        Bool Upload(Ref<Service> Service);

        /// \brief Unloads the technique from the GPU, destroying the pipeline resource.
        ///
        /// \param Service The graphic service used to destroy the resource.
        void Unload(Ref<Service> Service);

        /// \see Content::Resource::OnCreate(Ref<Engine::Subsystem::Host>)
        Bool OnCreate(Ref<Engine::Subsystem::Host> Host) override
        {
            return Upload(* Host.GetService<Service>());
        }

        /// \see Content::Resource::OnDelete(Ref<Engine::Subsystem::Host>)
        void OnDelete(Ref<Engine::Subsystem::Host> Host) override
        {
            Unload(* Host.GetService<Service>());
        }

    private:

        /// \brief Assembles the shader modules into a complete program.
        ///
        /// \return The assembled shader program.
        Program Assemble() const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Object      mHandle;
        Description mDescription;
    };
}