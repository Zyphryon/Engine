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

#include "Zyphryon.Graphic/Service.hpp"
#include "Zyphryon.Graphic/Material.hpp"
#include "Zyphryon.Graphic/Mesh.hpp"
#include "Zyphryon.Graphic/Technique.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Builds graphic draw commands with automatic resource binding.
    class ZY_API Encoder final
    {
    public:

        /// \brief Fills the bindings a technique's signature declares, then emits the draw that reads them.
        class Binder final
        {
        public:

            /// \brief Opens a draw against a technique, with every declared texture unbound.
            ///
            /// \param Encoder   The encoder the draw is emitted through.
            /// \param Technique The technique whose signature the draw fills.
            Binder(Ref<Encoder> Encoder, ConstRef<ZyGraphic::Technique> Technique);

            /// \brief Binds an image to the texture the signature declares under the given name.
            ///
            /// \param Name  The hash of the texture's name.
            /// \param Image The image to bind, or zero to fall back to the pass's input or the technique's own.
            /// \return This binder, so the bindings of a draw read as one statement.
            ZY_INLINE Ref<Binder> SetImage(UInt64 Name, ZyGraphic::Object Image)
            {
                const ConstSpan<ZyGraphic::Schema::Texture> Textures = mTechnique.GetSchema().GetTextures();

                for (UInt32 Index = 0, Limit = Textures.GetSize(); Index < Limit; ++Index)
                {
                    if (Textures[Index].Hash == Name)
                    {
                        mCommand.Textures[Index] = Image ? Image : mEncoder.Fallback(Textures[Index]);
                        break;
                    }
                }

                if (Image)
                {
                    mVariant |= mTechnique.ResolveByTexture(Name);
                }
                return * this;
            }

            /// \brief Sets the value the technique's stencil test compares each fragment against.
            ///
            /// \param Reference The reference value, read through the mask the technique declares.
            /// \return This binder, so the bindings of a draw read as one statement.
            ZY_INLINE Ref<Binder> SetStencil(UInt8 Reference)
            {
                mCommand.Stencil = Reference;
                return * this;
            }

            /// \brief Draws through an index stream rather than straight down the vertex buffers.
            ///
            /// \param Indices The index stream, whose stride says whether an index is two bytes or four.
            /// \return This binder, so the bindings of a draw read as one statement.
            ZY_INLINE Ref<Binder> SetIndices(ConstRef<ZyGraphic::Stream> Indices)
            {
                mCommand.Indices = Indices;
                return * this;
            }

            /// \brief Draws from a mesh's vertex blocks and index buffer, ahead of any instance stream.
            ///
            /// \param Mesh The mesh the draw reads, bound as one stream per interleaved block in slot order.
            /// \return This binder, so the bindings of a draw read as one statement.
            Ref<Binder> SetMesh(ConstRef<ZyGraphic::Mesh> Mesh);

            /// \brief Turns on the features a caller enables itself, beyond the ones its bindings imply.
            ///
            /// \param Variant The bitmask of the features to add.
            /// \return This binder, so the bindings of a draw read as one statement.
            ZY_INLINE Ref<Binder> SetVariant(ZyGraphic::Technique::Key Variant)
            {
                mVariant |= Variant;
                return * this;
            }

            /// \brief Turns on the feature the technique declares under the given name.
            ///
            /// \param Name The name of the feature to add.
            /// \return This binder, so the bindings of a draw read as one statement.
            ZY_INLINE Ref<Binder> SetVariant(Text Name)
            {
                mVariant |= mTechnique.ResolveByName(Name);
                return * this;
            }

            /// \brief Applies everything a material binds, in one go.
            ///
            /// \param Material The material supplying images, samplers and the block of its own parameters.
            /// \return This binder, so the bindings of a draw read as one statement.
            Ref<Binder> Apply(ConstRef<ZyGraphic::Material> Material);

            /// \brief Applies everything a material binds, then turns on the features a caller enables beyond it.
            ///
            /// \param Material The material supplying images, samplers and the block of its own parameters.
            /// \param Variant  The bitmask of the features to add beyond the ones the material implies.
            /// \return This binder, so the bindings of a draw read as one statement.
            ZY_INLINE Ref<Binder> Apply(ConstRef<ZyGraphic::Material> Material, ZyGraphic::Technique::Key Variant)
            {
                return Apply(Material).SetVariant(Variant);
            }

            /// \brief Emits the draw the bindings were gathered for, appending it to the frame's commands.
            ///
            /// \param Instances  The instance-rate vertex stream (empty stream for a single non-instanced draw).
            /// \param Uniform    The per-instance uniform stream bound to scope #Instance (empty stream if unused).
            /// \param Parameters The draw parameters.
            void Draw(
                ConstRef<ZyGraphic::Stream>     Instances,
                ConstRef<ZyGraphic::Stream>     Uniform,
                ConstRef<ZyGraphic::Invocation> Parameters);

            /// \brief Emits the draw the bindings were gathered for, with no per-instance uniform stream.
            ///
            /// \param Instances  The instance-rate vertex stream (empty stream for a single non-instanced draw).
            /// \param Parameters The draw parameters.
            ZY_INLINE void Draw(ConstRef<ZyGraphic::Stream> Instances, ConstRef<ZyGraphic::Invocation> Parameters)
            {
                Draw(Instances, ZyGraphic::Stream(), Parameters);
            }

            /// \brief Emits the single triangle that covers the whole target, for a pass-level effect.
            ZY_INLINE void DrawFullscreen()
            {
                constexpr ZyGraphic::Invocation Parameters { .Count = 3 };

                Draw(ZyGraphic::Stream(), ZyGraphic::Stream(), Parameters);
            }

        private:

            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

            Ref<Encoder>                   mEncoder;
            ConstRef<ZyGraphic::Technique> mTechnique;
            ZyGraphic::Command             mCommand;
            ZyGraphic::Technique::Key      mVariant;
        };

    public:

        /// \brief Constructs an encoder bound to a graphic service.
        ///
        /// \param Service The graphic service used to allocate transient commands and uniforms.
        Encoder(Ref<ZyGraphic::Service> Service);

        /// \brief Resets the per-pass scratch, forgetting the material last resolved and the pass's inputs.
        void Reset();

        /// \brief Hands the pass an input, which every draw reads wherever it binds no image of its own.
        ///
        /// \param Name  The hash of the texture's name, as the techniques drawn declare it.
        /// \param Image The image read under that name.
        void SetInput(UInt64 Name, ZyGraphic::Object Image);

        /// \brief Sets the frame's uniform block bound to every subsequent draw.
        ///
        /// \param Stream The transient stream holding the per-frame uniforms.
        void SetFrame(ZyGraphic::Stream Stream);

        /// \brief Sets the pass's uniform block and input textures.
        ///
        /// \param Stream The transient stream holding the per-pass uniforms.
        void SetPass(ZyGraphic::Stream Stream);

        /// \brief Sets the rectangle every subsequent draw is clipped to.
        ///
        /// \note Read from the target's top-left on both APIs, and honoured only by a technique whose
        ///       rasterizer state enables the scissor. An empty rectangle keeps no pixel at all.
        ///
        /// \param Scissor The region to keep, in pixels.
        void SetScissor(ZyGraphic::Scissor Scissor);

        /// \brief Packs a value into a transient uniform block and binds it as the pass's.
        ///
        /// \param Block The value laid out as the technique declares the pass's block.
        template<typename Type>
        ZY_INLINE void SetPass(ConstRef<Type> Block)
            requires (!IsAnyOf<Type, ZyGraphic::Stream>)
        {
            ZyGraphic::Transient<Type> Slice = mService.AllocateInFlightUniforms<Type>(1);
            Slice[0] = Block;

            SetPass(Slice.GetStream());
        }

        /// \brief Opens a draw against a technique, to be filled with the bindings its signature declares.
        ///
        /// \param Technique The technique whose signature the draw fills.
        /// \return The binder gathering the draw's bindings.
        ZY_INLINE Binder Begin(ConstRef<ZyGraphic::Technique> Technique)
        {
            return Binder(* this, Technique);
        }

    private:

        /// \brief What a material resolves to under a technique: its variant, packed block, images and samplers.
        struct Binding final
        {
            /// The technique the material was resolved under.
            ConstPtr<ZyGraphic::Technique> Technique = nullptr;

            /// The material resolved, compared by address.
            ConstPtr<ZyGraphic::Material>  Material  = nullptr;

            /// The samplers the material supplied itself, one bit per slot, as opposed to the technique's own.
            UInt32                         Overrides = 0;

            /// The variant the material's features select.
            ZyGraphic::Technique::Key      Variant   = 0;

            /// The material's uniform block, packed into the frame's arena.
            ZyGraphic::Stream              Uniforms;

            /// The image handle for every texture the technique declares, zero where the material has none.
            Sequence<ZyGraphic::Object, ZyGraphic::Command::kMaxTextures> Textures;

            /// The sampler for every slot the technique declares, the material's own or the technique's.
            Sequence<ZyGraphic::Object, ZyGraphic::Command::kMaxSamplers> Samplers;
        };

        /// \brief Represents an image the pass hands every draw under one name.
        struct Input final
        {
            /// The hash of the texture's name.
            UInt64            Name  = 0;

            /// The image read under that name.
            ZyGraphic::Object Image = 0;
        };

        /// \brief Packs a uniform block for a frequency by resolving each declared field from a provider.
        ///
        /// \param  Frequency  The uniform block to pack.
        /// \param  Technique  The technique describing the block's layout.
        /// \param  Source     The provider supplying field values by hash.
        /// \return A transient stream holding the packed block, or an empty stream if the block declares no fields.
        template<typename Provider>
        ZY_INLINE ZyGraphic::Stream Pack(
            ZyGraphic::Frequency Frequency, ConstRef<ZyGraphic::Technique> Technique, ConstRef<Provider> Source)
        {
            ConstRef<ZyGraphic::Schema::Block> Block = Technique.GetSchema().GetUniforms(Frequency);

            if (Block.Size == 0)
            {
                return ZyGraphic::Stream();
            }

            ZyGraphic::Transient<Byte> Slice = mService.AllocateInFlightUniforms<Byte>(Block.Size);

            for (ConstRef<ZyGraphic::Schema::Uniform> Field : Block.Uniforms)
            {
                // Fall back to the technique's default when the material does not set the field.
                ConstPtr<ZyGraphic::Parameter> Parameter = Source.GetParameter(Field.Hash);

                if (Parameter == nullptr)
                {
                    Parameter = AddressOf(Field.Value);
                }

                if (Parameter->GetSlot() == ZyEnum::Cast(Field.Type))
                {
                    Parameter->Visit([&]<typename Type>(ConstRef<Type> Value)
                    {
                        Slice.Copy(ConstSpan(Value), Field.Offset);
                    });
                }
            }
            return Slice.GetStream();
        }

        /// \brief Resolves a material under a technique into \ref mBinding, keeping the last resolution while both are unchanged.
        ///
        /// \note The cache holds one entry and is dropped by \ref Reset and \ref SetFrame.
        ///
        /// \param Technique The technique whose schema names what to bind.
        /// \param Material  The material to source the variant, block, images and samplers from.
        void Resolve(ConstRef<ZyGraphic::Technique> Technique, ConstRef<ZyGraphic::Material> Material);

        /// \brief Gets what a texture slot reads when nothing binds it: the pass's input of its name, or its fallback.
        ///
        /// \param Texture The texture the technique drawn declares at the slot.
        /// \return The image to bind, which is zero when neither the pass nor the technique supplies one.
        ZyGraphic::Object Fallback(ConstRef<ZyGraphic::Schema::Texture> Texture) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<ZyGraphic::Service>                           mService;
        ZyGraphic::Stream                                 mFrame;
        ZyGraphic::Stream                                 mPass;
        ZyGraphic::Scissor                                mScissor;
        Binding                                           mBinding;
        Sequence<Input, ZyGraphic::Command::kMaxTextures> mInputs;
    };
}