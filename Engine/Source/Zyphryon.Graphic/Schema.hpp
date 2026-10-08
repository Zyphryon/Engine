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

namespace ZyGraphic
{
    /// \brief Describes the named resources a shader program samples, sets and reads, and where each one binds.
    class Schema final
    {
    public:

        /// \brief Describes a single field within a uniform block.
        struct Uniform final
        {
            /// The hash identifying this field's name.
            UInt64             Hash   = 0;

            /// The data type of the field.
            ZyGraphic::Uniform Type   = ZyGraphic::Uniform::Float;

            /// The size of the field, in bytes.
            UInt16             Size   = 0;

            /// The byte offset of the field within its owning uniform block.
            UInt16             Offset = 0;

            /// The value packed when a material does not set the field.
            Parameter          Value;
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
            UInt64             Hash   = 0;

            /// The state bound when a material does not supply its own.
            ZyGraphic::Sampler Descriptor;

            /// The sampler resource resolved from the descriptor, or zero until the technique is uploaded.
            Object             Handle = 0;
        };

        /// \brief Names a texture the program samples, with the one-texel texture bound where a material supplies none.
        struct Texture final
        {
            /// The hash identifying the name a material binds its image under.
            UInt64        Hash     = 0;

            /// The colour of the fallback's single texel, packed with red in the lowest byte.
            UInt32        Texel    = 0;

            /// The layout the fallback is created with, which must match what the program samples.
            TextureLayout Layout   = TextureLayout::Texture2D;

            /// The format the texel is stored in, which settles whether it reads back as linear or as sRGB.
            TextureFormat Format   = TextureFormat::RGBA8UIntNorm;

            /// Whether the texture declares a fallback at all.
            Bool          Declared = false;

            /// The texture created from the texel, or zero until the technique is uploaded or when none is declared.
            Object        Fallback = 0;
        };

    public:

        /// \brief Resolves every declared sampler, and creates every declared fallback texture.
        ///
        /// \param Service The graphic service used to obtain each resource.
        void Resolve(Ref<Service> Service);

        /// \brief Deletes the fallback textures created by \ref Resolve.
        ///
        /// \param Service The graphic service the textures were created through.
        void Release(Ref<Service> Service);

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

        /// \brief Declares the texture bound at a register when a material supplies none.
        ///
        /// \param Register The register the texture was declared at.
        /// \param Texel    The colour of the fallback's single texel, packed with red in the lowest byte.
        /// \param Layout   The layout the fallback is created with.
        /// \param Format   The format the texel is stored in, one of the four-byte colour formats.
        ZY_INLINE void SetFallback(UInt8 Register, UInt32 Texel, TextureLayout Layout, TextureFormat Format)
        {
            ZY_ASSERT(Register < mTextures.GetSize(), "Fallback set on an undeclared texture");

            Ref<Texture> Entry = mTextures[Register];
            Entry.Texel    = Texel;
            Entry.Layout   = Layout;
            Entry.Format   = Format;
            Entry.Declared = true;
        }

        /// \brief Declares a sampler a material may supply, taking the next sampler register.
        ///
        /// \param Name       The name a material supplies its own state under.
        /// \param Descriptor The state bound when a material supplies none.
        /// \return The register the sampler was declared at.
        ZY_INLINE UInt8 AddSampler(Text Name, ConstRef<ZyGraphic::Sampler> Descriptor)
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
        ZY_INLINE void AddUniform(Frequency Frequency, Text Name, ZyGraphic::Uniform Type, AnyRef<Parameter> Value)
        {
            const UInt16 Size = Parameter::GetSize(ZyEnum::Cast(Type));

            Ref<Block> Group = mUniforms[ZyEnum::Cast(Frequency)];
            Group.Uniforms.Append(Hash(Name), Type, Size, Group.Size, Move(Value));
            Group.Size += Size;
        }

        /// \brief Gets the uniform block declared for the given frequency.
        ///
        /// \param Frequency The frequency whose block to read.
        /// \return The block declared for the frequency, empty if it declares no field.
        ZY_INLINE ConstRef<Block> GetUniforms(Frequency Frequency) const
        {
            return mUniforms[ZyEnum::Cast(Frequency)];
        }

        /// \brief Gets the textures the program samples.
        ///
        /// \return A view over the textures, ordered so the index is the register.
        ZY_INLINE ConstSpan<Texture> GetTextures() const
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

        Array<Block, ZyEnum::Count<Frequency>()> mUniforms;
        Sequence<Texture, Command::kMaxTextures> mTextures;
        Sequence<Sampler, Command::kMaxSamplers> mSamplers;
    };
}