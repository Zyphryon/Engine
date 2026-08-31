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

    Encoder::Binder::Binder(Ref<Encoder> Encoder, ConstRef<Graphic::Technique> Technique)
        : mEncoder   { Encoder },
          mTechnique { Technique },
          mCommand   { Encoder.mService.AllocateInFlightCommand() },
          mVariant   { 0 }
    {
        ConstRef<Graphic::Schema> Schema = Technique.GetSchema();

        mCommand.Uniforms[Enum::Cast(Graphic::Frequency::Frame)] = Encoder.mFrame;
        mCommand.Uniforms[Enum::Cast(Graphic::Frequency::Pass)]  = Encoder.mPass;
        mCommand.Scissor = Encoder.mScissor;

        // Every texture the signature declares holds its slot, so one left unbound still reads as zero.
        for (UInt32 Index = 0, Limit = Schema.GetTextures().GetSize(); Index < Limit; ++Index)
        {
            mCommand.Textures.Append(0);
        }

        // Samplers start at the technique's own, which a caller replaces only where it wants to.
        for (ConstRef<Graphic::Schema::Sampler> Field : Schema.GetSamplers())
        {
            mCommand.Samplers.Append(Field.Handle);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Encoder::Binder> Encoder::Binder::Apply(ConstRef<Graphic::Material> Material)
    {
        ConstRef<Graphic::Schema> Schema = mTechnique.GetSchema();

        // The material answers by name, so each image lands in the slot the signature declared it under.
        const ConstSpan<UInt64> Textures = Schema.GetTextures();

        for (UInt32 Index = 0, Limit = Textures.GetSize(); Index < Limit; ++Index)
        {
            if (ConstRetainer<Graphic::Image> Image = Material.GetImage(Textures[Index]))
            {
                mCommand.Textures[Index] = Image->GetHandle();
            }
        }

        // A material overrides a sampler only where it sets one, leaving the technique's own everywhere else.
        for (UInt32 Index = 0, Limit = Schema.GetSamplers().GetSize(); Index < Limit; ++Index)
        {
            if (const Graphic::Object Handle = Material.GetSampler(Schema.GetSamplers()[Index].Hash))
            {
                mCommand.Samplers[Index] = Handle;
            }
        }

        mCommand.Uniforms[Enum::Cast(Graphic::Frequency::Material)]
            = mEncoder.Pack(Graphic::Frequency::Material, mTechnique, Material);

        mVariant |= mTechnique.Resolve(Material);
        return * this;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Binder::Draw(
        ConstRef<Graphic::Stream>     Instances,
        ConstRef<Graphic::Stream>     Uniform,
        ConstRef<Graphic::Invocation> Parameters)
    {
        mCommand.Pipeline = mTechnique.GetHandle(mVariant);

        // A variant nothing compiled leaves the pipeline unbound, which draws nothing and says nothing.
        if (mVariant != 0 && mCommand.Pipeline == 0)
        {
            LOG_W("'{0}' has no variant {1} compiled, so the draw binds nothing",
                mTechnique.GetKey().GetUrl(), mVariant);
        }

        if (Instances.Buffer)
        {
            mCommand.Vertices.Append(Instances);
        }

        if (Uniform.Buffer)
        {
            mCommand.Uniforms[Enum::Cast(Graphic::Frequency::Instance)] = Uniform;
        }
        mCommand.Parameters = Parameters;
    }

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
        mPass    = Graphic::Stream();
        mScissor = Graphic::Scissor();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::SetFrame(Graphic::Stream Stream)
    {
        mFrame = Stream;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::SetScissor(Graphic::Scissor Scissor)
    {
        mScissor = Scissor;
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

        // The material decides which features turn on, so it selects the variant the draw is compiled for.
        Command.Pipeline = Technique.GetHandle(Material ? Technique.Resolve(* Material) : 0);

        // Bind the per-frame and per-pass uniform blocks.
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Frame)] = mFrame;
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Pass)]  = mPass;
        Command.Scissor = mScissor;

        // Bind the material uniform block, textures, and samplers from the schema.
        if (Material)
        {
            Ref<Graphic::Stream> Stream = Command.Uniforms[Enum::Cast(Graphic::Frequency::Material)];
            Stream = Pack(Graphic::Frequency::Material, Technique, * Material);

            BindTextures(Command, Technique.GetSchema(), * Material);
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
        ConstRef<Graphic::Invocation> Parameters,
        Graphic::Technique::Key       Variant)
    {
        Ref<Graphic::Command> Command = mService.AllocateInFlightCommand();

        Command.Pipeline = Technique.GetHandle(Variant);

        // Bind the per-frame and per-pass uniform blocks.
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Frame)] = mFrame;
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Pass)]  = mPass;
        Command.Scissor = mScissor;

        // Bind the caller's textures in declaration order, paired with the technique's own samplers.
        ConstRef<Graphic::Schema> Schema = Technique.GetSchema();

        for (UInt32 Index = 0, Limit = Schema.GetTextures().GetSize(); Index < Limit; ++Index)
        {
            Command.Textures.Append(Index < Textures.GetSize() ? Textures[Index] : 0);
        }

        for (ConstRef<Graphic::Schema::Sampler> Field : Schema.GetSamplers())
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
        ConstRef<Graphic::Schema> Schema = Technique.GetSchema();

        Ref<Graphic::Command> Command = mService.AllocateInFlightCommand();

        // The material decides which features turn on, so it selects the variant the draw is compiled for.
        Command.Pipeline = Technique.GetHandle(Material ? Technique.Resolve(* Material) : 0);

        // Bind the per-frame, per-pass, and per-object (instance) uniform blocks.
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Frame)]   = mFrame;
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Pass)]     = mPass;
        Command.Scissor = mScissor;
        Command.Uniforms[Enum::Cast(Graphic::Frequency::Instance)] = Uniform;

        // Bind the run's material (uniform block, textures, and samplers) when the caller named one.
        if (Material)
        {
            const Graphic::Stream Data = Pack(Graphic::Frequency::Material, Technique, * Material);
            Command.Uniforms[Enum::Cast(Graphic::Frequency::Material)] = Data;

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
            const Bool IsExtended = Mesh.HasProperty(Graphic::Mesh::Property::Extended);
            Command.Indices = Graphic::Stream(Indices, IsExtended ? sizeof(UInt32) : sizeof(UInt16), 0);
        }
        Command.Parameters = Range;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::BindTextures(
        Ref<Graphic::Command>       Command,
        ConstRef<Graphic::Schema>   Schema,
        ConstRef<Graphic::Material> Material)
    {
        for (const UInt64 Name : Schema.GetTextures())
        {
            ConstRetainer<Graphic::Image> Image = Material.GetImage(Name);

            Command.Textures.Append(Image ? Image->GetHandle() : 0);
        }

        for (ConstRef<Graphic::Schema::Sampler> Field : Schema.GetSamplers())
        {
            // Fall back to the technique's own sampler when the material supplies none.
            const Graphic::Object Handle = Material.GetSampler(Field.Hash);

            Command.Samplers.Append(Handle ? Handle : Field.Handle);
        }
    }
}