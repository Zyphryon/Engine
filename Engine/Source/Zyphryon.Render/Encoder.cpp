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

    void Encoder::SetFrame(Graphic::Stream Stream)
    {
        mFrame = Stream;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::SetPass(Graphic::Stream Stream)
    {
        mPass = Stream;
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
        Ref<Graphic::Command> Command = mService.AllocateInFlightCommand();

        Command.Pipeline = Technique.GetHandle();

        // Bind the per-frame and per-pass uniform blocks.
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Frame)] = mFrame;
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Pass)]  = mPass;

        // Bind the material uniform block, textures, and samplers from the schema.
        if (Material)
        {
            Ref<Graphic::Stream> Stream = Command.Uniforms[Enum::Cast(Graphic::Frequency::Material)];
            Stream = Pack(Graphic::Frequency::Material, Technique, * Material);

            BindTextures(Command, Technique.GetReflection(), * Material);
        }

        // Bind the per-instance vertex stream and, if present, the per-instance uniform block.
        if (Instances.Buffer)
        {
            Command.Vertices.Append(Instances);
        }

        if (Uniform.Buffer)
        {
            Command.Uniforms[Enum::Cast(Graphic::Frequency::Instance)] = Uniform;
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
        Ref<Graphic::Command> Command = mService.AllocateInFlightCommand();

        Command.Pipeline = Technique.GetHandle();

        // Bind the per-frame and per-pass uniform blocks.
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Frame)] = mFrame;
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Pass)]  = mPass;

        // Bind the caller's textures in declaration order, paired with the technique's own samplers.
        ConstRef<Graphic::Technique::Reflection> Reflection = Technique.GetReflection();

        for (UInt32 Index = 0, Limit = Reflection.Textures.GetSize(); Index < Limit; ++Index)
        {
            Command.Textures.Append(Index < Textures.GetSize() ? Textures[Index] : 0);
        }

        for (ConstRef<Graphic::Technique::Reflection::SamplerField> Field : Reflection.Samplers)
        {
            Command.Samplers.Append(Field.Handle);
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
        ConstRef<Graphic::Technique::Reflection> Reflection = Technique.GetReflection();

        Ref<Graphic::Command> Command = mService.AllocateInFlightCommand();

        Command.Pipeline = Technique.GetHandle();

        // Bind the per-frame, per-pass, and per-object (instance) uniform blocks.
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Frame)]   = mFrame;
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Pass)]     = mPass;
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Instance)] = Uniform;

        // Bind the run's material (uniform block, textures, and samplers) when the caller named one.
        if (Material)
        {
            const Graphic::Stream Data = Pack(Graphic::Frequency::Material, Technique, * Material);
            Command.Uniforms[Enum::Cast(Graphic::Frequency::Material)] = Data;

            BindTextures(Command, Reflection, * Material);
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
            const Bool IsExtended = Mesh.HasProperty(Graphic::Mesh::Property::Extended);
            Command.Indices = Graphic::Stream(Indices, IsExtended ? sizeof(UInt32) : sizeof(UInt16), 0);
        }
        Command.Parameters = Range;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::BindTextures(
        Ref<Graphic::Command>                    Command,
        ConstRef<Graphic::Technique::Reflection> Reflection,
        ConstRef<Graphic::Material>              Material)
    {
        for (const UInt64 Name : Reflection.Textures)
        {
            ConstRetainer<Graphic::Image> Image = Material.GetImage(Name);

            Command.Textures.Append(Image ? Image->GetHandle() : 0);
        }

        for (ConstRef<Graphic::Technique::Reflection::SamplerField> Field : Reflection.Samplers)
        {
            // Fall back to the technique's own sampler when the material supplies none.
            const Graphic::Object Handle = Material.GetSampler(Field.Hash);

            Command.Samplers.Append(Handle ? Handle : Field.Handle);
        }
    }
}