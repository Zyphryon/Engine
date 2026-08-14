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

#include "Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Describes the named resources a shader program samples, sets and reads, and where each one binds.
    class Schema final
    {
    public:

        /// \brief Describes a single field within a uniform block.
        struct Uniform final
        {
            /// The hash identifying this field's name.
            UInt64           Hash   = 0;

            /// The data type of the field.
            Graphic::Uniform Type   = Graphic::Uniform::Float;

            /// The size of the field, in bytes.
            UInt16           Size   = 0;

            /// The byte offset of the field within its owning uniform block.
            UInt16           Offset = 0;

            /// The value packed when a material does not set the field.
            Parameter        Value;
        };

        /// \brief Describes the layout of the uniform block declared for one frequency.
        struct Block final
        {
            /// The total size of the block, in bytes.
            UInt32            Size = 0;

            /// The uniforms contained within the block, in declaration order.
            Sequence<Uniform> Uniforms;
        };

        /// \brief Names a sampler a material may supply, with the state bound when it does not.
        struct Sampler final
        {
            /// The hash identifying this sampler's name.
            UInt64           Hash   = 0;

            /// The state bound when a material does not supply its own.
            Graphic::Sampler Descriptor;

            /// The sampler resource resolved from the descriptor, or zero until the technique is uploaded.
            Object           Handle = 0;
        };

    public:

        /// \brief Resolves every declared sampler into the resource backing its descriptor.
        ///
        /// \param Service The graphic service used to obtain each sampler resource.
        void Resolve(Ref<Service> Service);

        /// \brief Declares a texture the program samples, taking the next texture register.
        ///
        /// \param Name The name a material binds its image under.
        /// \return The register the texture was declared at.
        ZY_INLINE UInt8 AddTexture(Text Name)
        {
            ZY_ASSERT(mTextures.GetSize() < Command::kMaxTextures, "Schema declares more textures than a draw binds");

            mTextures.Append(Hash(Name));
            return static_cast<UInt8>(mTextures.GetSize() - 1);
        }

        /// \brief Declares a sampler a material may supply, taking the next sampler register.
        ///
        /// \param Name       The name a material supplies its own state under.
        /// \param Descriptor The state bound when a material supplies none.
        /// \return The register the sampler was declared at.
        ZY_INLINE UInt8 AddSampler(Text Name, ConstRef<Graphic::Sampler> Descriptor)
        {
            ZY_ASSERT(mSamplers.GetSize() < Command::kMaxSamplers, "Schema declares more samplers than a draw binds");

            mSamplers.Append(Hash(Name), Descriptor, 0);
            return static_cast<UInt8>(mSamplers.GetSize() - 1);
        }

        /// \brief Declares a uniform field, appending it to its frequency's block and growing that block.
        ///
        /// \param Frequency The block the field belongs to.
        /// \param Name      The name a material sets the field under.
        /// \param Type      The data type of the field.
        /// \param Value     The value packed when a material does not set the field.
        ZY_INLINE void AddUniform(Frequency Frequency, Text Name, Graphic::Uniform Type, AnyRef<Parameter> Value)
        {
            const UInt16 Size = Parameter::GetSize(Enum::Cast(Type));

            Ref<Block> Group = mUniforms[Enum::Cast(Frequency)];
            Group.Uniforms.Append(Hash(Name), Type, Size, Group.Size, Move(Value));
            Group.Size += Size;
        }

        /// \brief Gets the uniform block declared for the given frequency.
        ///
        /// \param Frequency The frequency whose block to read.
        /// \return The block declared for the frequency, empty if it declares no field.
        ZY_INLINE ConstRef<Block> GetUniforms(Frequency Frequency) const
        {
            return mUniforms[Enum::Cast(Frequency)];
        }

        /// \brief Gets the hashed names of the textures the program samples.
        ///
        /// \return A view over the texture names, ordered so the index is the register.
        ZY_INLINE ConstSpan<UInt64> GetTextures() const
        {
            return mTextures;
        }

        /// \brief Gets the samplers a material may supply.
        ///
        /// \return A view over the samplers, ordered so the index is the register.
        ZY_INLINE ConstSpan<Sampler> GetSamplers() const
        {
            return mSamplers;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Block, Enum::Count<Frequency>()>   mUniforms;
        Sequence<UInt64, Command::kMaxTextures>  mTextures;
        Sequence<Sampler, Command::kMaxSamplers> mSamplers;
    };
}