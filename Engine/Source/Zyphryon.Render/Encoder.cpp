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

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Encoder::Binder::Binder(Ref<Encoder> Encoder, ConstRef<ZyGraphic::Technique> Technique)
        : mEncoder   { Encoder },
          mTechnique { Technique },
          mCommand   { Encoder.mService.AllocateInFlightCommand() },
          mVariant   { 0 }
    {
        ConstRef<ZyGraphic::Schema> Schema = Technique.GetSchema();

        mCommand.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Frame)] = Encoder.mFrame;
        mCommand.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Pass)]  = Encoder.mPass;
        mCommand.Scissor = Encoder.mScissor;

        // Every texture the signature declares holds its slot, so one left unbound still reads as zero.
        for (UInt32 Index = 0, Limit = Schema.GetTextures().GetSize(); Index < Limit; ++Index)
        {
            mCommand.Textures.Append(0);
        }

        // Samplers start at the technique's own, which a caller replaces only where it wants to.
        for (ConstRef<ZyGraphic::Schema::Sampler> Field : Schema.GetSamplers())
        {
            mCommand.Samplers.Append(Field.Handle);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Encoder::Binder> Encoder::Binder::Apply(ConstRef<ZyGraphic::Material> Material)
    {
        ConstRef<Binding> Bound = mEncoder.Resolve(mTechnique, Material);

        // The material answers by name, so each image lands in the slot the signature declared it under.
        for (UInt32 Index = 0, Limit = Bound.Textures.GetSize(); Index < Limit; ++Index)
        {
            if (const ZyGraphic::Object Handle = Bound.Textures[Index])
            {
                mCommand.Textures[Index] = Handle;
            }
        }

        // A material overrides a sampler only where it sets one, leaving the technique's own everywhere else.
        for (UInt32 Index = 0, Limit = Bound.Samplers.GetSize(); Index < Limit; ++Index)
        {
            if (Bound.Overrides & (1u << Index))
            {
                mCommand.Samplers[Index] = Bound.Samplers[Index];
            }
        }

        mCommand.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Material)] = Bound.Uniforms;

        mVariant |= Bound.Variant;
        return * this;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Binder::Draw(
        ConstRef<ZyGraphic::Stream>     Instances,
        ConstRef<ZyGraphic::Stream>     Uniform,
        ConstRef<ZyGraphic::Invocation> Parameters)
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
            mCommand.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Instance)] = Uniform;
        }
        mCommand.Parameters = Parameters;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Encoder::Encoder(Ref<ZyGraphic::Service> Service)
        : mService { Service }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Reset()
    {
        mPass    = ZyGraphic::Stream();
        mScissor = ZyGraphic::Scissor();
        mBinding = Binding();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::SetFrame(ZyGraphic::Stream Stream)
    {
        mFrame   = Stream;
        mBinding = Binding();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::SetScissor(ZyGraphic::Scissor Scissor)
    {
        mScissor = Scissor;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::SetPass(ZyGraphic::Stream Stream)
    {
        mPass = Stream;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Draw(
        ConstRef<ZyGraphic::Technique>  Technique,
        ConstPtr<ZyGraphic::Material>   Material,
        ConstRef<ZyGraphic::Stream>     Instances,
        ConstRef<ZyGraphic::Stream>     Uniform,
        ConstRef<ZyGraphic::Invocation> Parameters,
        ZyGraphic::Technique::Key       Variant)
    {
        Ref<ZyGraphic::Command> Command = mService.AllocateInFlightCommand();

        // The material decides which features turn on, and the caller adds whatever it turns on itself.
        const ConstPtr<Binding> Bound = Material ? AddressOf(Resolve(Technique, * Material)) : nullptr;

        const ZyGraphic::Technique::Key Features = (Bound ? Bound->Variant : 0) | Variant;

        Command.Pipeline = Technique.GetHandle(Features);

        // A variant nothing compiled leaves the pipeline unbound, which draws nothing and says nothing.
        if (Features != 0 && Command.Pipeline == 0)
        {
            LOG_W("'{0}' has no variant {1} compiled, so the draw binds nothing",
                Technique.GetKey().GetUrl(), Features);
        }

        // Bind the per-frame and per-pass uniform blocks.
        Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Frame)] = mFrame;
        Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Pass)]  = mPass;
        Command.Scissor = mScissor;

        // Bind the material uniform block, textures, and samplers from the schema.
        if (Bound)
        {
            Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Material)] = Bound->Uniforms;
            Command.Textures = Bound->Textures;
            Command.Samplers = Bound->Samplers;
        }

        // Bind the per-instance vertex stream and, if present, the per-instance uniform block.
        if (Instances.Buffer)
        {
            Command.Vertices.Append(Instances);
        }

        if (Uniform.Buffer)
        {
            Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Instance)] = Uniform;
        }
        Command.Parameters = Parameters;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Draw(
        ConstRef<ZyGraphic::Technique>  Technique,
        ConstSpan<ZyGraphic::Object>    Textures,
        ConstRef<ZyGraphic::Stream>     Instances,
        ConstRef<ZyGraphic::Invocation> Parameters,
        ZyGraphic::Technique::Key       Variant)
    {
        Ref<ZyGraphic::Command> Command = mService.AllocateInFlightCommand();

        Command.Pipeline = Technique.GetHandle(Variant);

        // Bind the per-frame and per-pass uniform blocks.
        Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Frame)] = mFrame;
        Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Pass)]  = mPass;
        Command.Scissor = mScissor;

        // Bind the caller's textures in declaration order, paired with the technique's own samplers.
        ConstRef<ZyGraphic::Schema> Schema = Technique.GetSchema();

        for (UInt32 Index = 0, Limit = Schema.GetTextures().GetSize(); Index < Limit; ++Index)
        {
            Command.Textures.Append(Index < Textures.GetSize() ? Textures[Index] : 0);
        }

        for (ConstRef<ZyGraphic::Schema::Sampler> Field : Schema.GetSamplers())
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
        ConstRef<ZyGraphic::Technique>  Technique,
        ConstRef<ZyGraphic::Mesh>       Mesh,
        ConstPtr<ZyGraphic::Material>   Material,
        ConstRef<ZyGraphic::Invocation> Range,
        ConstRef<ZyGraphic::Stream>     Instances,
        ConstRef<ZyGraphic::Stream>     Uniform)
    {
        ConstRef<ZyGraphic::Schema> Schema = Technique.GetSchema();

        Ref<ZyGraphic::Command> Command = mService.AllocateInFlightCommand();

        // The material decides which features turn on, so it selects the variant the draw is compiled for.
        const ConstPtr<Binding> Bound = Material ? AddressOf(Resolve(Technique, * Material)) : nullptr;

        Command.Pipeline = Technique.GetHandle(Bound ? Bound->Variant : 0);

        // Bind the per-frame, per-pass, and per-object (instance) uniform blocks.
        Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Frame)]   = mFrame;
        Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Pass)]     = mPass;
        Command.Scissor = mScissor;
        Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Instance)] = Uniform;

        // Bind the run's material (uniform block, textures, and samplers) when the caller named one.
        if (Bound)
        {
            Command.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Material)] = Bound->Uniforms;
            Command.Textures = Bound->Textures;
            Command.Samplers = Bound->Samplers;
        }

        // Bind one stream per interleaved block, in slot order (matching the technique's layout).
        const ZyGraphic::Object Vertices = Mesh.GetVertices();

        for (const ZyGraphic::VertexSlot Slot : ZyEnum::GetValues<ZyGraphic::VertexSlot>())
        {
            if (!Mesh.HasBinding(Slot))
            {
                continue;
            }

            const ZyGraphic::Mesh::Binding Binding = Mesh.GetBinding(Slot);

            Bool Bound = false;

            for (ConstRef<ZyGraphic::Stream> Stream : Command.Vertices)
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
                Command.Vertices.Append(ZyGraphic::Stream(Vertices, Binding.Stride, Binding.Offset));
            }
        }

        // After the mesh's own, so what the batch varies is read from the stream the technique declares last.
        if (Instances.Buffer)
        {
            Command.Vertices.Append(Instances);
        }

        if (const ZyGraphic::Object Indices  = Mesh.GetIndices(); Indices)
        {
            const Bool IsExtended = Mesh.HasProperty(ZyGraphic::Mesh::Property::Extended);
            Command.Indices = ZyGraphic::Stream(Indices, IsExtended ? sizeof(UInt32) : sizeof(UInt16), 0);
        }
        Command.Parameters = Range;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstRef<Encoder::Binding> Encoder::Resolve(ConstRef<ZyGraphic::Technique> Technique, ConstRef<ZyGraphic::Material> Material)
    {
        Ref<Binding> Bound = mBinding;

        // A run of draws sharing a material under one technique packs and resolves it once.
        if (Bound.Technique == AddressOf(Technique) && Bound.Material == AddressOf(Material))
        {
            return Bound;
        }

        ConstRef<ZyGraphic::Schema> Schema = Technique.GetSchema();

        Bound.Technique = AddressOf(Technique);
        Bound.Material  = AddressOf(Material);
        Bound.Overrides = 0;
        Bound.Variant   = Technique.Resolve(Material);
        Bound.Uniforms  = Pack(ZyGraphic::Frequency::Material, Technique, Material);

        Bound.Textures.Clear();

        for (const UInt64 Name : Schema.GetTextures())
        {
            ConstRetainer<ZyGraphic::Image> Image = Material.GetImage(Name);

            Bound.Textures.Append(Image ? Image->GetHandle() : 0);
        }

        Bound.Samplers.Clear();

        for (UInt32 Index = 0, Limit = Schema.GetSamplers().GetSize(); Index < Limit; ++Index)
        {
            ConstRef<ZyGraphic::Schema::Sampler> Field = Schema.GetSamplers()[Index];

            // Fall back to the technique's own sampler when the material supplies none.
            if (const ZyGraphic::Object Handle = Material.GetSampler(Field.Hash))
            {
                Bound.Samplers.Append(Handle);
                Bound.Overrides |= (1u << Index);
            }
            else
            {
                Bound.Samplers.Append(Field.Handle);
            }
        }
        return Bound;
    }
}