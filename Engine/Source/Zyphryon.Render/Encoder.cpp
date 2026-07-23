// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Encoder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Encoder::Encoder(Ref<Graphic::Service> Service)
        : mService { Service }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Reset()
    {
        mPass = Graphic::Stream();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::SetGlobal(ConstRef<Graphic::Stream> Global)
    {
        mGlobal = Global;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::SetPass(ConstRef<Graphic::Stream> Pass)
    {
        mPass = Pass;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Draw(
        ConstRef<Graphic::Technique>  Technique,
        ConstPtr<Graphic::Material>   Material,
        ConstRef<Graphic::Stream>     Instances,
        ConstRef<Graphic::Stream>     Uniform,
        ConstRef<Graphic::Invocation> Parameters)
    {
        Ref<Graphic::Command> Command = mService.AllocateTransientCommands(1).GetFront();

        Command.Pipeline = Technique.GetHandle();

        // Bind the frame-global and per-pass uniform blocks.
        Command.Uniforms[Enum::Cast(Graphic::UniformScope::Global)] = mGlobal;
        Command.Uniforms[Enum::Cast(Graphic::UniformScope::Pass)]   = mPass;

        // Bind the material uniform block, textures, and samplers from the schema.
        if (Material)
        {
            ConstRef<Graphic::Schema> Schema = Technique.GetDescription().Schema;

            Ref<Graphic::Stream> Stream = Command.Uniforms[Enum::Cast(Graphic::UniformScope::Material)];
            Stream = Pack(Graphic::UniformScope::Material, Schema, * Material);

            BindTextures(Command, Schema, * Material);
        }

        // Bind the per-instance vertex stream and, if present, the per-instance uniform block.
        if (Instances.Buffer)
        {
            Command.Vertices.Append(Instances);
        }

        if (Uniform.Buffer)
        {
            Command.Uniforms[Enum::Cast(Graphic::UniformScope::Instance)] = Uniform;
        }
        Command.Parameters = Parameters;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Draw(
        ConstRef<Graphic::Technique>  Technique,
        ConstSpan<Graphic::Object>    Textures,
        ConstRef<Graphic::Stream>     Instances,
        ConstRef<Graphic::Invocation> Parameters)
    {
        Ref<Graphic::Command> Command = mService.AllocateTransientCommands(1).GetFront();

        Command.Pipeline = Technique.GetHandle();

        // Bind the frame-global and per-pass uniform blocks.
        Command.Uniforms[Enum::Cast(Graphic::UniformScope::Global)] = mGlobal;
        Command.Uniforms[Enum::Cast(Graphic::UniformScope::Pass)]   = mPass;

        // Bind the input textures paired with the technique's default samplers.
        ConstRef<Graphic::Schema> Schema = Technique.GetDescription().Schema;

        UInt32 Index = 0;

        for (const Graphic::TextureSlot Slot : Schema.GetTextures())
        {
            Command.Textures.Append(Index < Textures.GetSize() ? Textures[Index] : 0);
            Command.Samplers.Append(Schema.GetSampler(Slot));

            ++Index;
        }

        // Bind the per-instance vertex stream.
        if (Instances.Buffer)
        {
            Command.Vertices.Append(Instances);
        }
        Command.Parameters = Parameters;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Draw(
        ConstRef<Graphic::Technique> Technique,
        ConstRef<Graphic::Model>     Model,
        ConstRef<Graphic::Stream>    Uniform)
    {
        ConstRetainer<Graphic::Mesh> Mesh = Model.GetMesh();

        ConstRef<Graphic::Schema>                    Schema    = Technique.GetDescription().Schema;
        const ConstSpan<Retainer<Graphic::Material>> Materials = Model.GetMaterials();

        const Graphic::Object VertexBuffer = Mesh->GetVertices();
        const Graphic::Object IndexBuffer  = Mesh->GetIndices();

        const Bool   IsExtended  = Mesh->HasProperty(Graphic::Mesh::Property::Extended);
        const UInt16 IndexStride = IsExtended ? sizeof(UInt32) : sizeof(UInt16);

        for (ConstRef<Graphic::Mesh::Primitive> Primitive : Mesh->GetPrimitives())
        {
            Ref<Graphic::Command> Command = mService.AllocateTransientCommands(1).GetFront();

            Command.Pipeline = Technique.GetHandle();

            // Bind the frame-global, per-pass, and per-object (instance) uniform blocks.
            Command.Uniforms[Enum::Cast(Graphic::UniformScope::Global)]   = mGlobal;
            Command.Uniforms[Enum::Cast(Graphic::UniformScope::Pass)]     = mPass;
            Command.Uniforms[Enum::Cast(Graphic::UniformScope::Instance)] = Uniform;

            // Bind the primitive's material (uniform block, textures, and samplers) when the model provides one.
            if (Primitive.Material < Materials.GetSize())
            {
                ConstRef<Graphic::Material> Material = * Materials[Primitive.Material];

                const Graphic::Stream Data = Pack(Graphic::UniformScope::Material, Schema, Material);
                Command.Uniforms[Enum::Cast(Graphic::UniformScope::Material)] = Data;

                BindTextures(Command, Schema, Material);
            }

            // Bind every present attribute as a vertex stream, in slot order (matching the technique's layout).
            for (const Graphic::VertexSlot Slot : Enum::GetValues<Graphic::VertexSlot>())
            {
                if (Mesh->HasBinding(Slot))
                {
                    const Graphic::Mesh::Binding Binding = Mesh->GetBinding(Slot);
                    Command.Vertices.Append(Graphic::Stream(VertexBuffer, Binding.Stride, Binding.Offset));
                }
            }

            if (IndexBuffer)
            {
                Command.Indices = Graphic::Stream(IndexBuffer, IndexStride, 0);
            }
            Command.Parameters = Primitive.Range;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::BindTextures(Ref<Graphic::Command> Command, ConstRef<Graphic::Schema> Schema, ConstRef<Graphic::Material> Material)
    {
        for (const Graphic::TextureSlot Slot : Schema.GetTextures())
        {
            Graphic::Object  Texture = 0;
            Graphic::Sampler Sampler = Schema.GetSampler(Slot);

            if (ConstRetainer<Graphic::Image> Image = Material.GetImage(Slot))
            {
                Texture = Image->GetHandle();

                if (Graphic::Sampler Override = Material.GetSampler(Slot); !Override.IsDefault())
                {
                    Sampler = Override;
                }
            }

            Command.Textures.Append(Texture);
            Command.Samplers.Append(Sampler);
        }
    }
}
