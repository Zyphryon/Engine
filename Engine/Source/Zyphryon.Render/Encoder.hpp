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
#include "Zyphryon.Graphic/Resource/Material.hpp"
#include "Zyphryon.Graphic/Resource/Mesh.hpp"
#include "Zyphryon.Graphic/Resource/Technique.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Builds graphic draw commands with automatic resource binding.
    class Encoder final
    {
    public:

        /// \brief Constructs an encoder bound to a graphic service.
        ///
        /// \param Service The graphic service used to allocate transient commands and uniforms.
        Encoder(Ref<Graphic::Service> Service);

        /// \brief Resets the per-pass scratch (collector and arena), preserving capacity for reuse.
        void Reset();

        /// \brief Sets the frame's uniform block bound to every subsequent draw.
        ///
        /// \param Stream The transient stream holding the per-frame uniforms.
        void SetFrame(Graphic::Stream Stream);

        /// \brief Sets the pass's uniform block and input textures.
        ///
        /// \param Stream The transient stream holding the per-pass uniforms.
        void SetPass(Graphic::Stream Stream);

        /// \brief Packs a uniform block for a frequency by resolving each declared field from a provider.
        ///
        /// \tparam Provider   A type exposing `GetParameter(UInt64 Hash)`.
        /// \param  Frequency  The uniform block to pack.
        /// \param  Technique  The technique describing the block's layout.
        /// \param  Source     The provider supplying field values by hash.
        /// \return A transient stream holding the packed block, or an empty stream if the block declares no fields.
        template<typename Provider>
        ZY_INLINE Graphic::Stream Pack(Graphic::Frequency Frequency, ConstRef<Graphic::Technique> Technique, ConstRef<Provider> Source)
        {
            using Reflection = Graphic::Technique::Reflection;

            ConstRef<Reflection>               Data  = Technique.GetReflection();
            ConstRef<Reflection::UniformGroup> Block = Data.Uniforms[Enum::Cast(Frequency)];

            if (Block.Size == 0)
            {
                return Graphic::Stream();
            }

            Graphic::Transient<Byte> Slice = mService.AllocateInFlightUniforms<Byte>(Block.Size);

            for (ConstRef<Reflection::UniformField> Field : Block.Structure)
            {
                // Fall back to the technique's default when the material does not set the field.
                ConstPtr<Graphic::Parameter> Parameter = Source.GetParameter(Field.Hash);

                if (Parameter == nullptr)
                {
                    Parameter = AddressOf(Field.Value);
                }

                if (Parameter->GetSlot() == Enum::Cast(Field.Type))
                {
                    Parameter->Visit([&]<typename Type>(ConstRef<Type> Value)
                    {
                        Slice.Copy(ConstSpan(Value), Field.Offset);
                    });
                }
            }
            return Slice.GetStream();
        }

        /// \brief Emits one (optionally instanced) draw, auto-binding all uniform scopes and samplers.
        ///
        /// \param Technique  The technique (pipeline + schema) to draw with.
        /// \param Material   The material to bind, or `nullptr` for a material-less draw.
        /// \param Instances  The instance-rate vertex stream (empty stream for a single non-instanced draw).
        /// \param Uniform    The per-instance uniform stream bound to scope #Instance (empty stream if unused).
        /// \param Parameters The draw parameters.
        void Draw(
            ConstRef<Graphic::Technique>  Technique,
            ConstPtr<Graphic::Material>   Material,
            ConstRef<Graphic::Stream>     Instances,
            ConstRef<Graphic::Stream>     Uniform,
            ConstRef<Graphic::Invocation> Parameters);

        /// \brief Emits one (optionally instanced) draw without a per-instance uniform stream.
        ///
        /// \param Technique  The technique (pipeline + schema) to draw with.
        /// \param Material   The material to bind, or `nullptr` for a material-less draw.
        /// \param Instances  The instance-rate vertex stream (empty stream for a single non-instanced draw).
        /// \param Parameters The draw parameters.
        ZY_INLINE void Draw(
            ConstRef<Graphic::Technique>  Technique,
            ConstPtr<Graphic::Material>   Material,
            ConstRef<Graphic::Stream>     Instances,
            ConstRef<Graphic::Invocation> Parameters)
        {
            Draw(Technique, Material, Instances, Graphic::Stream(), Parameters);
        }

        /// \brief Emits one (optionally instanced) material-less draw for a pass-level effect.
        ///
        /// \param Technique  The technique (pipeline + schema) to draw with.
        /// \param Textures   The input textures, in the technique's declared slot order.
        /// \param Instances  The instance-rate vertex stream (empty stream for a single non-instanced draw).
        /// \param Parameters The draw parameters.
        void Draw(
            ConstRef<Graphic::Technique>  Technique,
            ConstSpan<Graphic::Object>    Textures,
            ConstRef<Graphic::Stream>     Instances,
            ConstRef<Graphic::Invocation> Parameters);

        /// \brief Emits one material-less draw for a pass-level effect.
        ///
        /// \param Technique  The technique (pipeline + schema) to draw with.
        /// \param Textures   The input textures, in the technique's declared slot order.
        /// \param Parameters The draw parameters.
        ZY_INLINE void Draw(
            ConstRef<Graphic::Technique>  Technique,
            ConstSpan<Graphic::Object>    Textures,
            ConstRef<Graphic::Invocation> Parameters)
        {
            Draw(Technique, Textures, Graphic::Stream(), Parameters);
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
            ConstRef<Graphic::Technique>  Technique,
            ConstRef<Graphic::Mesh>       Mesh,
            ConstPtr<Graphic::Material>   Material,
            ConstRef<Graphic::Invocation> Range,
            ConstRef<Graphic::Stream>     Instances,
            ConstRef<Graphic::Stream>     Uniform);

    private:

        /// \brief Binds every texture and sampler the technique declares, sourced from the material.
        ///
        /// \param Command    The command being assembled.
        /// \param Reflection The technique reflection naming the textures and samplers it declares.
        /// \param Material   The material to source images and samplers from.
        void BindTextures(
            Ref<Graphic::Command>                    Command,
            ConstRef<Graphic::Technique::Reflection> Reflection,
            ConstRef<Graphic::Material>              Material);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Graphic::Service> mService;
        Graphic::Stream       mFrame;
        Graphic::Stream       mPass;
    };
}