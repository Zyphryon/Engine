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
        ConstRef<Graphic::Technique>  Technique,
        ConstRef<Graphic::Mesh>       Mesh,
        ConstPtr<Graphic::Material>   Material,
        ConstRef<Graphic::Invocation> Range,
        ConstRef<Graphic::Stream>     Instances,
        ConstRef<Graphic::Stream>     Uniform)
    {
        ConstRef<Graphic::Schema> Schema = Technique.GetDescription().Schema;

        const Bool   IsExtended  = Mesh.HasProperty(Graphic::Mesh::Property::Extended);
        const UInt16 IndexStride = IsExtended ? sizeof(UInt32) : sizeof(UInt16);

        Ref<Graphic::Command> Command = mService.AllocateTransientCommands(1).GetFront();

        Command.Pipeline = Technique.GetHandle();

        // Bind the frame-global, per-pass, and per-object (instance) uniform blocks.
        Command.Uniforms[Enum::Cast(Graphic::UniformScope::Global)]   = mGlobal;
        Command.Uniforms[Enum::Cast(Graphic::UniformScope::Pass)]     = mPass;
        Command.Uniforms[Enum::Cast(Graphic::UniformScope::Instance)] = Uniform;

        // Bind the run's material (uniform block, textures, and samplers) when the caller named one.
        if (Material)
        {
            const Graphic::Stream Data = Pack(Graphic::UniformScope::Material, Schema, * Material);
            Command.Uniforms[Enum::Cast(Graphic::UniformScope::Material)] = Data;

            BindTextures(Command, Schema, * Material);
        }

        // Bind one stream per interleaved block, in slot order (matching the technique's layout).
        const Graphic::Object Vertices = Mesh.GetVertices();

        for (const Graphic::VertexSlot Slot : Enum::GetValues<Graphic::VertexSlot>())
        {
            if (!Mesh.HasBinding(Slot))
            {
                continue;
            }

            const Graphic::Mesh::Binding Binding = Mesh.GetBinding(Slot);

            Bool Bound = false;

            for (ConstRef<Graphic::Stream> Stream : Command.Vertices)
            {
                Bound = Stream.Buffer  == Vertices
                     && Stream.Stride  == Binding.Stride
                     && Binding.Offset >= Stream.Offset
                     && Binding.Offset <  Stream.Offset + Binding.Stride;

                if (Bound)
                {
                    break;
                }
            }

            if (!Bound)
            {
                Command.Vertices.Append(Graphic::Stream(Vertices, Binding.Stride, Binding.Offset));
            }
        }

        // After the mesh's own, so what the batch varies is read from the stream the technique declares last.
        if (Instances.Buffer)
        {
            Command.Vertices.Append(Instances);
        }

        if (const Graphic::Object Indices  = Mesh.GetIndices(); Indices)
        {
            Command.Indices = Graphic::Stream(Indices, IndexStride, 0);
        }
        Command.Parameters = Range;
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