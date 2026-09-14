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
    class Encoder final
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
            /// \param Image The image to bind, or zero to leave the texture unbound.
            /// \return This binder, so the bindings of a draw read as one statement.
            ZY_INLINE Ref<Binder> SetImage(UInt64 Name, ZyGraphic::Object Image)
            {
                const ConstSpan<UInt64> Textures = mTechnique.GetSchema().GetTextures();

                for (UInt32 Index = 0, Limit = Textures.GetSize(); Index < Limit; ++Index)
                {
                    if (Textures[Index] == Name)
                    {
                        mCommand.Textures[Index] = Image;
                        break;
                    }
                }

                if (Image)
                {
                    mVariant |= mTechnique.ResolveByTexture(Name);
                }
                return * this;
            }

            /// \brief Binds a sampler to the one the signature declares under the given name.
            ///
            /// \param Name    The hash of the sampler's name.
            /// \param Sampler The sampler to read the texture through, replacing the technique's own.
            /// \return This binder, so the bindings of a draw read as one statement.
            ZY_INLINE Ref<Binder> SetSampler(UInt64 Name, ZyGraphic::Object Sampler)
            {
                const ConstSpan<ZyGraphic::Schema::Sampler> Samplers = mTechnique.GetSchema().GetSamplers();

                for (UInt32 Index = 0, Limit = Samplers.GetSize(); Sampler && Index < Limit; ++Index)
                {
                    if (Samplers[Index].Hash == Name)
                    {
                        mCommand.Samplers[Index] = Sampler;
                        break;
                    }
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

            /// \brief Emits the draw the bindings were gathered for.
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
            Ref<ZyGraphic::Command>        mCommand;
            ZyGraphic::Technique::Key      mVariant;
        };

    public:

        /// \brief Constructs an encoder bound to a graphic service.
        ///
        /// \param Service The graphic service used to allocate transient commands and uniforms.
        Encoder(Ref<ZyGraphic::Service> Service);

        /// \brief Resets the per-pass scratch, forgetting the material last resolved.
        void Reset();

        /// \brief Sets the frame's uniform block bound to every subsequent draw.
        ///
        /// \param Stream The transient stream holding the per-frame uniforms.
        void SetFrame(ZyGraphic::Stream Stream);

        /// \brief Packs a value into a transient uniform block and binds it as the frame's.
        ///
        /// \param Block The value laid out as the technique declares the frame's block.
        template<typename Type>
        ZY_INLINE void SetFrame(ConstRef<Type> Block)
            requires (!IsAnyOf<Type, ZyGraphic::Stream>)
        {
            ZyGraphic::Transient<Type> Slice = mService.AllocateInFlightUniforms<Type>(1);
            Slice[0] = Block;

            SetFrame(Slice.GetStream());
        }

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

        /// \brief Packs a uniform block for a frequency by resolving each declared field from a provider.
        ///
        /// \tparam Provider   A type exposing `GetParameter(UInt64 Hash)`.
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

        /// \brief Opens a draw against a technique, to be filled with the bindings its signature declares.
        ///
        /// \param Technique The technique whose signature the draw fills.
        /// \return The binder gathering the draw's bindings.
        ZY_INLINE Binder Begin(ConstRef<ZyGraphic::Technique> Technique)
        {
            return Binder(* this, Technique);
        }

        /// \brief Emits one (optionally instanced) draw, auto-binding all uniform scopes and samplers.
        ///
        /// \param Technique  The technique (pipeline + schema) to draw with.
        /// \param Material   The material to bind, or `nullptr` for a material-less draw.
        /// \param Instances  The instance-rate vertex stream (empty stream for a single non-instanced draw).
        /// \param Uniform    The per-instance uniform stream bound to scope #Instance (empty stream if unused).
        /// \param Parameters The draw parameters.
        /// \param Variant    The features the caller turns on itself, beyond the ones the material implies.
        void Draw(
            ConstRef<ZyGraphic::Technique>  Technique,
            ConstPtr<ZyGraphic::Material>   Material,
            ConstRef<ZyGraphic::Stream>     Instances,
            ConstRef<ZyGraphic::Stream>     Uniform,
            ConstRef<ZyGraphic::Invocation> Parameters,
            ZyGraphic::Technique::Key       Variant = 0);

        /// \brief Emits one (optionally instanced) draw without a per-instance uniform stream.
        ///
        /// \param Technique  The technique (pipeline + schema) to draw with.
        /// \param Material   The material to bind, or `nullptr` for a material-less draw.
        /// \param Instances  The instance-rate vertex stream (empty stream for a single non-instanced draw).
        /// \param Parameters The draw parameters.
        /// \param Variant    The features the caller turns on itself, beyond the ones the material implies.
        ZY_INLINE void Draw(
            ConstRef<ZyGraphic::Technique>  Technique,
            ConstPtr<ZyGraphic::Material>   Material,
            ConstRef<ZyGraphic::Stream>     Instances,
            ConstRef<ZyGraphic::Invocation> Parameters,
            ZyGraphic::Technique::Key       Variant = 0)
        {
            Draw(Technique, Material, Instances, ZyGraphic::Stream(), Parameters, Variant);
        }

        /// \brief Emits one (optionally instanced) material-less draw for a pass-level effect.
        ///
        /// \param Technique  The technique (pipeline + schema) to draw with.
        /// \param Textures   The input textures, in the technique's declared slot order.
        /// \param Instances  The instance-rate vertex stream (empty stream for a single non-instanced draw).
        /// \param Parameters The draw parameters.
        /// \param Variant    The bitmask of the features the variant to draw with was compiled from.
        void Draw(
            ConstRef<ZyGraphic::Technique>  Technique,
            ConstSpan<ZyGraphic::Object>    Textures,
            ConstRef<ZyGraphic::Stream>     Instances,
            ConstRef<ZyGraphic::Invocation> Parameters,
            ZyGraphic::Technique::Key       Variant = 0);

        /// \brief Emits one material-less draw for a pass-level effect.
        ///
        /// \param Technique  The technique (pipeline + schema) to draw with.
        /// \param Textures   The input textures, in the technique's declared slot order.
        /// \param Parameters The draw parameters.
        /// \param Variant    The bitmask of the features the variant to draw with was compiled from.
        ZY_INLINE void Draw(
            ConstRef<ZyGraphic::Technique>  Technique,
            ConstSpan<ZyGraphic::Object>    Textures,
            ConstRef<ZyGraphic::Invocation> Parameters,
            ZyGraphic::Technique::Key       Variant = 0)
        {
            Draw(Technique, Textures, ZyGraphic::Stream(), Parameters, Variant);
        }

        /// \brief Emits the single triangle that covers the whole target, for a pass-level effect.
        ///
        /// \param Technique The technique (pipeline + schema) to draw with.
        /// \param Textures  The input textures, in the technique's declared slot order.
        /// \param Variant   The bitmask of the features the variant to draw with was compiled from.
        ZY_INLINE void DrawFullscreen(
            ConstRef<ZyGraphic::Technique> Technique,
            ConstSpan<ZyGraphic::Object>   Textures = { },
            ZyGraphic::Technique::Key      Variant  = 0)
        {
            constexpr ZyGraphic::Invocation Parameters {
                .Count = 3
            };
            Draw(Technique, Textures, ZyGraphic::Stream(), Parameters, Variant);
        }

        /// \brief Emits one indexed draw over a run of a mesh, drawn with the given material.
        ///
        /// \param Technique The technique (pipeline + schema) to draw with.
        /// \param Mesh      The mesh whose buffers the run indexes into.
        /// \param Material  The material to bind, or `nullptr` for a material-less draw.
        /// \param Range     The index range to draw, and how many instances of it.
        /// \param Instances The instance-rate vertex stream (empty stream for a single non-instanced draw).
        /// \param Uniform   The per-object data stream bound to the Instance scope (empty for none).
        void Draw(
            ConstRef<ZyGraphic::Technique>  Technique,
            ConstRef<ZyGraphic::Mesh>       Mesh,
            ConstPtr<ZyGraphic::Material>   Material,
            ConstRef<ZyGraphic::Invocation> Range,
            ConstRef<ZyGraphic::Stream>     Instances,
            ConstRef<ZyGraphic::Stream>     Uniform);

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

        /// \brief Resolves a material under a technique, reusing the last resolution while both are unchanged.
        ///
        /// \note The cache holds one entry and is dropped by \ref Reset and \ref SetFrame.
        ///
        /// \param Technique The technique whose schema names what to bind.
        /// \param Material  The material to source the variant, block, images and samplers from.
        /// \return The resolved bindings.
        ConstRef<Binding> Resolve(ConstRef<ZyGraphic::Technique> Technique, ConstRef<ZyGraphic::Material> Material);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<ZyGraphic::Service> mService;
        ZyGraphic::Stream       mFrame;
        ZyGraphic::Stream       mPass;
        ZyGraphic::Scissor      mScissor;
        Binding                 mBinding;
    };
}