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

#include "Shader.hpp"
#include "Zyphryon.Graphic/Service.hpp"

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

        /// \brief Describes the shader-side data needed to fill the pipeline's bindings.
        struct Reflection final
        {
            /// \brief Describes a single field within a uniform block.
            struct UniformField final
            {
                /// The hash identifying this field's name.
                UInt64    Hash   = 0;

                /// The data type of the field.
                Uniform   Type   = Uniform::Float;

                /// The size of the field, in bytes.
                UInt16    Size   = 0;

                /// The byte offset of the field within its owning uniform block.
                UInt16    Offset = 0;

                /// The value packed when a material does not set the field.
                Parameter Value;
            };

            /// \brief Describes the layout of the uniform block declared for one frequency.
            struct UniformGroup final
            {
                /// The total size of the block, in bytes.
                UInt32                 Size = 0;

                /// The fields contained within the block, in declaration order.
                Sequence<UniformField> Structure;
            };

            /// \brief Names a sampler a material may supply, with the state bound when it does not.
            struct SamplerField final
            {
                /// The hash identifying this sampler's name.
                UInt64  Hash   = 0;

                /// The state bound when a material does not supply its own.
                Sampler Descriptor;

                /// The sampler resource resolved from the descriptor, or zero until the technique is uploaded.
                Object  Handle = 0;
            };

            /// \brief Appends a field to a uniform block, assigning its offset and growing the block.
            ///
            /// \param Frequency The block the field belongs to.
            /// \param Name      The name of the field.
            /// \param Type      The data type of the field.
            /// \param Value     The value packed when a material does not set the field.
            ZY_INLINE void AddUniform(Frequency Frequency, Text Name, Uniform Type, AnyRef<Parameter> Value)
            {
                const UInt16 Size = Parameter::GetSize(Enum::Cast(Type));

                Ref<UniformGroup> Group = Uniforms[Enum::Cast(Frequency)];
                Group.Structure.Append(Hash(Name), Type, Size, Group.Size, Move(Value));
                Group.Size += Size;
            }

            /// The uniform block layout declared for each frequency.
            Array<UniformGroup, Enum::Count<Frequency>()> Uniforms;

            /// The names of the textures the program samples, ordered so the index is the register.
            Sequence<UInt64, Command::kMaxTextures>       Textures;

            /// The named samplers a material may supply, ordered so the index is the register.
            Sequence<SamplerField, Command::kMaxSamplers> Samplers;
        };

        /// \brief Describes the configuration for a rendering technique.
        struct Description final
        {
            /// Preprocessor macros used when compiling the technique's shaders.
            Sequence<Macro> Macros;

            /// The resource interface the pipeline exposes to the driver.
            Signature       Signature;

            /// The fixed-function pipeline states.
            States          States;

            /// The shader modules for each pipeline stage.
            Shaders         Shaders;
        };

    public:

        /// \brief Constructs a technique resource with the given content key.
        ///
        /// \param Key The unique content key identifying this pipeline.
        explicit Technique(AnyRef<Content::Uri> Key);

        /// \brief Sets up the technique with the given description and reflection.
        ///
        /// \param Description The description containing the technique's configuration.
        /// \param Reflection  The shader-side data the encoder needs to fill the pipeline's bindings.
        void Setup(AnyRef<Description> Description, AnyRef<Reflection> Reflection);

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

        /// \brief Gets the technique's shader-side reflection data.
        ///
        /// \return The technique reflection.
        ZY_INLINE ConstRef<Reflection> GetReflection() const
        {
            return mReflection;
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
        Reflection  mReflection;
    };
}