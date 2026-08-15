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

#include "Material.hpp"
#include "Schema.hpp"
#include "Service.hpp"
#include "Shader.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a rendering technique containing a configured pipeline.
    class Technique final : public Content::AbstractResource<Technique>
    {
    public:

        /// \brief Type alias for the bitmask selecting which features a variant compiles with.
        using Key = UInt32;

        /// \brief Type alias for an array of shader modules indexed by pipeline stage.
        using Shaders = Array<Retainer<Shader>, Enum::Count<ShaderStage>()>;

        /// \brief Specifies the groups of fixed-function state a feature replaces as a whole.
        enum class Block : UInt8
        {
            Blend,          ///< The blend factors, equations, write mask and alpha-to-coverage flag.
            Depth,          ///< The depth clip, write mask, comparison and bias.
            Stencil,        ///< The stencil read and write masks, comparisons and actions.
            Rasterizer,     ///< The fill mode, cull mode and scissor test.
            Layout,         ///< The vertex attributes and primitive topology.
        };

        /// \brief Holds every part of a pipeline a feature is allowed to replace.
        struct Layer final
        {
            /// The preprocessor macros used when compiling the shaders.
            Sequence<Macro> Macros;

            /// The fixed-function states.
            States          States;

            /// The vertex attributes the input stage consumes.
            Attributes      Attributes;

            /// The shader modules for each pipeline stage.
            Shaders         Shaders;
        };

        /// \brief Describes an optional capability that patches the base layer while it is enabled.
        struct Feature final
        {
            /// The name identifying this feature, which a key is resolved from.
            Str32  Name;

            /// The hash of the texture whose presence enables the feature, or zero if the caller enables it.
            UInt64 Texture   = 0;

            /// The hash of the parameter whose presence enables the feature, or zero if the caller enables it.
            UInt64 Parameter = 0;

            /// The bitmask of the state blocks this feature replaces, built from \ref Block.
            UInt8  Blocks    = 0;

            /// The parts replacing the base ones, each left empty at whatever the base keeps.
            Layer  Patch;
        };

        /// \brief Describes the configuration for a rendering technique.
        struct Description final
        {
            /// The resource interface the pipeline exposes to the driver, whose bindings no variant changes.
            Signature         Signature;

            /// The parts every variant starts from, before any enabled feature patches them.
            Layer             Base;

            /// The optional features, ordered so the index is the bit each one occupies in a key.
            Sequence<Feature> Features;

            /// The variants compiled up front, so drawing with one of them never waits on a shader compile.
            Sequence<Key>     Preload;
        };

    public:

        /// \brief Constructs a technique resource with the given content key.
        ///
        /// \param Key The unique content key identifying this pipeline.
        explicit Technique(AnyRef<Content::Uri> Key);

        /// \brief Sets up the technique with the given description and schema.
        ///
        /// \param Description The description containing the technique's configuration.
        /// \param Schema      The named resources the encoder fills the pipeline's bindings with.
        void Setup(AnyRef<Description> Description, AnyRef<Schema> Schema);

        /// \brief Gets the GPU pipeline handle for the technique's base variant.
        ///
        /// \return The base variant's GPU pipeline object handle.
        ZY_INLINE Object GetHandle() const
        {
            return mHandle;
        }

        /// \brief Gets the GPU pipeline handle for the given variant.
        ///
        /// \note Yields zero for a variant that has not been compiled yet, which \ref Obtain compiles.
        ///
        /// \param Key The bitmask of the features the variant compiles with.
        /// \return The variant's GPU pipeline object handle, or zero if it has not been compiled.
        ZY_INLINE Object GetHandle(Key Key) const
        {
            if (Key == 0)
            {
                return mHandle;
            }

            const ConstPtr<Object> Handle = mVariants.Find(Key);
            return (Handle ? * Handle : 0);
        }

        /// \brief Gets the GPU pipeline handle for the given variant, compiling it when it does not exist.
        ///
        /// \param Service The graphic service used to create the resource.
        /// \param Key     The bitmask of the features the variant compiles with.
        /// \return The variant's GPU pipeline object handle, falling back to the base one if it failed.
        Object Obtain(Ref<Service> Service, Key Key);

        /// \brief Resolves the key of the features the given material enables.
        ///
        /// \param Material The material whose textures and parameters decide which features turn on.
        /// \return The bitmask of the features the material enables.
        Key Resolve(ConstRef<Material> Material) const;

        /// \brief Resolves the key holding the bit of the feature declared under the given name.
        ///
        /// \note This walks the feature list, so callers resolve the keys they need once and keep them.
        ///
        /// \param Name The name of the feature to look up.
        /// \return The bitmask holding the feature's bit, or zero if the technique declares no such feature.
        ZY_INLINE Key ResolveByName(Text Name) const
        {
            for (UInt Index = 0, Limit = mDescription.Features.GetSize(); Index < Limit; ++Index)
            {
                if (mDescription.Features[Index].Name == Name)
                {
                    return (1u << Index);
                }
            }

            LOG_W("'{0}' declares no feature named '{1}'", GetKey(), Name);
            return 0;
        }

        /// \brief Resolves the key holding the bit of the feature the given texture enables.
        ///
        /// \param Name The hash of the texture's name.
        /// \return The bitmask holding the feature's bit, or zero if no feature is gated on that texture.
        ZY_INLINE Key ResolveByTexture(UInt64 Name) const
        {
            for (UInt Index = 0, Limit = mDescription.Features.GetSize(); Name && Index < Limit; ++Index)
            {
                if (mDescription.Features[Index].Texture == Name)
                {
                    return (1u << Index);
                }
            }
            return 0;
        }

        /// \brief Gets the technique's configuration description.
        ///
        /// \return The technique description.
        ZY_INLINE ConstRef<Description> GetDescription() const
        {
            return mDescription;
        }

        /// \brief Gets the schema every one of the technique's variants declares.
        ///
        /// \return The technique schema.
        ZY_INLINE ConstRef<Schema> GetSchema() const
        {
            return mSchema;
        }

        /// \brief Uploads the technique to the GPU, creating the base pipeline and every preloaded variant.
        ///
        /// \param Service The graphic service used to create the resource.
        /// \return `true` if the upload succeeded, `false` otherwise.
        Bool Upload(Ref<Service> Service);

        /// \brief Unloads the technique from the GPU, destroying every pipeline resource it compiled.
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

        /// \see Content::Resource::OnReload(Ref<Engine::Subsystem::Host>)
        void OnReload(Ref<Engine::Subsystem::Host> Host) override;

    private:

        /// \brief Assembles everything the driver needs to create a variant's pipeline.
        ///
        /// \param Key       The bitmask of the features the variant compiles with.
        /// \param Program   Receives the shader program the variant compiles.
        /// \param Signature Receives the resource interface the variant exposes.
        /// \param States    Receives the fixed-function states the variant is created with.
        void Assemble(Key Key, Ref<Program> Program, Ref<Signature> Signature, Ref<States> States) const;

        /// \brief Copies the state blocks a feature replaces over the ones being assembled.
        ///
        /// \param Destination The states each replaced block is written into.
        /// \param Source      The feature's states, which every block it replaces is complete in.
        /// \param Blocks      The bitmask of the blocks to copy, built from \ref Block.
        void Converge(Ref<States> Destination, ConstRef<States> Source, UInt8 Blocks) const;

        /// \brief Creates the pipeline backing a variant.
        ///
        /// \param Service The graphic service used to create the resource.
        /// \param Key     The bitmask of the features the variant compiles with.
        /// \return The variant's GPU pipeline object handle, or zero if it failed to compile.
        Object Compile(Ref<Service> Service, Key Key);

    public:

        /// \brief Gets the bitmask selecting a single state block.
        ///
        /// \param Block The state block to select.
        /// \return The bitmask holding the block's bit.
        ZY_INLINE static constexpr UInt8 GetBlockMask(Block Block)
        {
            return (1u << Enum::Cast(Block));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Object             mHandle;
        Description        mDescription;
        Schema             mSchema;
        Table<Key, Object> mVariants;
    };
}