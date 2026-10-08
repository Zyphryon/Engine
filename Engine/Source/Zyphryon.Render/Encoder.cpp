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
          mCommand   { },
          mVariant   { 0 }
    {
        ConstRef<ZyGraphic::Schema> Schema = Technique.GetSchema();

        mCommand.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Frame)] = Encoder.mFrame;
        mCommand.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Pass)]  = Encoder.mPass;
        mCommand.Scissor = Encoder.mScissor;

        // Every texture the signature declares holds its slot, starting at the pass's input or its fallback.
        for (ConstRef<ZyGraphic::Schema::Texture> Field : Schema.GetTextures())
        {
            mCommand.Textures.Append(Encoder.Fallback(Field));
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
        mEncoder.Resolve(mTechnique, Material);

        ConstRef<Binding> Bound = mEncoder.mBinding;

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

    Ref<Encoder::Binder> Encoder::Binder::SetMesh(ConstRef<ZyGraphic::Mesh> Mesh)
    {
        // Bind one stream per interleaved block, in slot order (matching the technique's layout).
        const ZyGraphic::Object Vertices = Mesh.GetVertices();

        for (const ZyGraphic::VertexSlot Slot : ZyEnum::GetValues<ZyGraphic::VertexSlot>())
        {
            if (!Mesh.HasBinding(Slot))
            {
                continue;
            }

            const ZyGraphic::Mesh::Binding Block = Mesh.GetBinding(Slot);

            Bool Bound = false;

            for (ConstRef<ZyGraphic::Stream> Stream : mCommand.Vertices)
            {
                Bound = Stream.Buffer == Vertices
                     && Stream.Stride == Block.Stride
                     && Block.Offset  >= Stream.Offset
                     && Block.Offset  <  Stream.Offset + Block.Stride;

                if (Bound)
                {
                    break;
                }
            }

            if (!Bound)
            {
                mCommand.Vertices.Append(ZyGraphic::Stream(Vertices, Block.Stride, Block.Offset));
            }
        }

        if (const ZyGraphic::Object Indices = Mesh.GetIndices(); Indices)
        {
            const Bool IsExtended = Mesh.HasProperty(ZyGraphic::Mesh::Property::Extended);
            mCommand.Indices = ZyGraphic::Stream(Indices, IsExtended ? sizeof(UInt32) : sizeof(UInt16), 0);
        }
        return * this;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::Binder::Draw(
        ConstRef<ZyGraphic::Stream>     Instances,
        ConstRef<ZyGraphic::Stream>     Uniform,
        ConstRef<ZyGraphic::Invocation> Parameters)
    {
        mCommand.Pipeline = mTechnique.Obtain(mEncoder.mService, mVariant);

        if (Instances.Buffer)
        {
            mCommand.Vertices.Append(Instances);
        }

        if (Uniform.Buffer)
        {
            mCommand.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Instance)] = Uniform;
        }
        mCommand.Parameters = Parameters;

        mEncoder.mService.AllocateInFlightCommand() = mCommand;
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
        mInputs.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Encoder::SetInput(UInt64 Name, ZyGraphic::Object Image)
    {
        mInputs.Append(Name, Image);
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

    void Encoder::Resolve(ConstRef<ZyGraphic::Technique> Technique, ConstRef<ZyGraphic::Material> Material)
    {
        // A run of draws sharing a material under one technique packs and resolves it once.
        if (mBinding.Technique == AddressOf(Technique) && mBinding.Material == AddressOf(Material))
        {
            return;
        }

        ConstRef<ZyGraphic::Schema> Schema = Technique.GetSchema();

        mBinding.Technique = AddressOf(Technique);
        mBinding.Material  = AddressOf(Material);
        mBinding.Overrides = 0;
        mBinding.Variant   = Technique.Resolve(Material);
        mBinding.Uniforms  = Pack(ZyGraphic::Frequency::Material, Technique, Material);

        mBinding.Textures.Clear();

        for (ConstRef<ZyGraphic::Schema::Texture> Field : Schema.GetTextures())
        {
            ConstRetainer<ZyGraphic::Image> Image = Material.GetImage(Field.Hash);

            mBinding.Textures.Append(Image ? Image->GetHandle() : 0);
        }

        mBinding.Samplers.Clear();

        for (UInt32 Index = 0, Limit = Schema.GetSamplers().GetSize(); Index < Limit; ++Index)
        {
            ConstRef<ZyGraphic::Schema::Sampler> Field = Schema.GetSamplers()[Index];

            // Fall back to the technique's own sampler when the material supplies none.
            if (const ZyGraphic::Object Handle = Material.GetSampler(Field.Hash))
            {
                mBinding.Samplers.Append(Handle);
                mBinding.Overrides |= (1u << Index);
            }
            else
            {
                mBinding.Samplers.Append(Field.Handle);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ZyGraphic::Object Encoder::Fallback(ConstRef<ZyGraphic::Schema::Texture> Texture) const
    {
        for (ConstRef<Input> Given : mInputs)
        {
            if (Given.Name == Texture.Hash && Given.Image)
            {
                return Given.Image;
            }
        }
        return Texture.Fallback;
    }
}