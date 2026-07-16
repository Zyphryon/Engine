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

#include "Zyphryon.Graphic/Material.hpp"
#include "Zyphryon.Graphic/Schema.hpp"
#include "Zyphryon.Graphic/Service.hpp"
#include "Zyphryon.Graphic/Technique.hpp"

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

        /// \brief Sets the frame-global uniform block bound to every subsequent draw.
        ///
        /// \param Global The transient stream holding the frame-global uniforms.
        void SetGlobal(ConstRef<Graphic::Stream> Global);

        /// \brief Sets the current pass's uniform block and input textures.
        ///
        /// \param Pass The transient stream holding the per-pass uniforms.
        void SetPass(ConstRef<Graphic::Stream> Pass);

        /// \brief Packs a uniform block for a scope by resolving each schema field from a provider.
        ///
        /// \tparam Provider A type exposing `GetParameter(UInt64 Hash)`.
        /// \param  Scope    The uniform scope to pack.
        /// \param  Schema   The technique schema describing the scope's layout.
        /// \param  Source   The provider supplying field values by hash.
        /// \return A transient stream holding the packed block, or an empty stream if the scope declares no fields.
        template<typename Provider>
        ZY_INLINE Graphic::Stream Pack(Graphic::UniformScope Scope, ConstRef<Graphic::Schema> Schema, ConstRef<Provider> Source)
        {
            ConstRef<Graphic::Schema::UniformGroup> Block = Schema.GetUniforms(Scope);

            if (Block.Size == 0)
            {
                return Graphic::Stream();
            }

            Graphic::Transient<Byte> Slice = mService.AllocateTransientUniforms<Byte>(Block.Size);

            for (ConstRef<Graphic::Schema::UniformField> Field : Block.Structure)
            {
                if (const ConstPtr<Graphic::Parameter> Parameter = Source.GetParameter(Field.Hash))
                {
                    if (Parameter->GetSlot() == Enum::Cast(Field.Type))
                    {
                        Parameter->Visit([&]<typename Type>(ConstRef<Type> Value)
                        {
                            Slice.Copy(ConstSpan(Value), Field.Offset);
                        });
                    }
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

        /// \brief Emits one material-less draw for a pass-level effect.
        ///
        /// \param Technique  The technique (pipeline + schema) to draw with.
        /// \param Textures   The input textures, in the technique's declared slot order.
        /// \param Parameters The draw parameters.
        void Draw(
            ConstRef<Graphic::Technique>  Technique,
            ConstSpan<Graphic::Object>    Textures,
            ConstRef<Graphic::Invocation> Parameters);

    private:

        /// \brief Resolves and binds every texture slot the schema declares, with its sampler.
        ///
        /// \param Command  The command being assembled.
        /// \param Schema   The technique's resource schema describing the texture slots.
        /// \param Material The material to source images and samplers from.
        void BindTextures(Ref<Graphic::Command> Command, ConstRef<Graphic::Schema> Schema, ConstRef<Graphic::Material> Material);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Graphic::Service> mService;
        Graphic::Stream       mGlobal;
        Graphic::Stream       mPass;
    };
}
