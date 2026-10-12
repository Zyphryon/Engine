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

#include "D3D11Binder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    static Type Pick(ConstSpan<Type> List, UInt32 Mask, UInt32 Slot)
    {
        return ((Mask >> Slot) & 1u) && Slot < List.GetSize() ? List[Slot] : Type { };
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    static Bool Differ(ConstSpan<Type> Old, UInt32 Before, ConstSpan<Type> New, UInt32 After, Ref<UInt32> First, Ref<UInt32> Count)
    {
        const UInt32 Limit = static_cast<UInt32>(Max(Old.GetSize(), New.GetSize()));

        UInt32 Last = 0;
        First = Limit;

        for (UInt32 Slot = 0; Slot < Limit; ++Slot)
        {
            if (Pick(Old, Before, Slot) != Pick(New, After, Slot))
            {
                First = Min(First, Slot);
                Last  = Slot + 1;
            }
        }
        Count = (Last > First ? Last - First : 0);
        return Count > 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Binder::Binder(ConstRef<Device> Device, ConstSpan<Buffer> Buffers, ConstSpan<Pipeline> Pipelines, ConstSpan<Sampler> Samplers, ConstSpan<Texture> Textures)
        : mDevice    { Device },
          mBuffers   { Buffers },
          mPipelines { Pipelines },
          mSamplers  { Samplers },
          mTextures  { Textures }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::Invalidate()
    {
        mCommand = Command();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::Apply(ConstRef<Command> Draw)
    {
        ApplyPipeline(Draw);
        ApplyScissor(Draw.Scissor);
        ApplyVertices(Draw.Vertices);
        ApplyIndices(Draw.Indices);
        ApplyUniforms(ShaderStage::Vertex, Draw);
        ApplyUniforms(ShaderStage::Fragment, Draw);
        ApplySamplers(ShaderStage::Vertex, Draw);
        ApplySamplers(ShaderStage::Fragment, Draw);
        ApplyTextures(ShaderStage::Vertex, Draw);
        ApplyTextures(ShaderStage::Fragment, Draw);

        mCommand = Draw;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyPipeline(ConstRef<Command> Draw)
    {
        const Ptr<ID3D11DeviceContext1> Context = mDevice.GetContext();

        ConstRef<Pipeline> Old = mPipelines[mCommand.Pipeline];
        ConstRef<Pipeline> New = mPipelines[Draw.Pipeline];

        if (Old.GetVertex() != New.GetVertex())
        {
            Context->VSSetShader(New.GetVertex(), nullptr, 0);
        }
        if (Old.GetFragment() != New.GetFragment())
        {
            Context->PSSetShader(New.GetFragment(), nullptr, 0);
        }
        if (Old.GetBlend() != New.GetBlend())
        {
            Context->OMSetBlendState(New.GetBlend(), nullptr, D3D11_DEFAULT_SAMPLE_MASK);
        }
        if (Old.GetDepth() != New.GetDepth() || mCommand.Stencil != Draw.Stencil)
        {
            Context->OMSetDepthStencilState(New.GetDepth(), Draw.Stencil);
        }
        if (Old.GetRaster() != New.GetRaster())
        {
            Context->RSSetState(New.GetRaster());
        }
        if (Old.GetLayout() != New.GetLayout())
        {
            Context->IASetInputLayout(New.GetLayout());
        }
        if (Old.GetTopology() != New.GetTopology())
        {
            Context->IASetPrimitiveTopology(New.GetTopology());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyScissor(ConstRef<Scissor> Scissor)
    {
        ConstRef<ZyGraphic::Scissor> Old = mCommand.Scissor;

        if (Old.X == Scissor.X && Old.Y == Scissor.Y && Old.Width == Scissor.Width && Old.Height == Scissor.Height)
        {
            return;
        }

        const CD3D11_RECT Rect(Scissor.X, Scissor.Y, Scissor.X + Scissor.Width, Scissor.Y + Scissor.Height);
        mDevice.GetContext()->RSSetScissorRects(1, AddressOf(Rect));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyVertices(ConstSpan<Stream> Vertices)
    {
        UInt32 First;
        UInt32 Count;

        if (!Differ<Stream>(mCommand.Vertices, ~0, Vertices, ~0, First, Count))
        {
            return;
        }

        Ptr<ID3D11Buffer> Buffers[Command::kMaxVertices];
        UINT              Strides[Command::kMaxVertices];
        UINT              Offsets[Command::kMaxVertices];

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            const Stream New = (Slot < Vertices.GetSize() ? Vertices[Slot] : Stream { });

            Buffers[Slot] = mBuffers[New.Buffer].GetHandle();
            Strides[Slot] = New.Stride;
            Offsets[Slot] = New.Offset;
        }

        mDevice.GetContext()->IASetVertexBuffers(First, Count, Buffers + First, Strides + First, Offsets + First);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyIndices(ConstRef<Stream> Indices)
    {
        if (mCommand.Indices == Indices)
        {
            return;
        }

        const DXGI_FORMAT Format = (Indices.Stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
        mDevice.GetContext()->IASetIndexBuffer(mBuffers[Indices.Buffer].GetHandle(), Format, Indices.Offset);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyUniforms(ShaderStage Stage, ConstRef<Command> Draw)
    {
        const UInt32 Before = mPipelines[mCommand.Pipeline].GetUsage(Stage).Uniforms;
        const UInt32 After  = mPipelines[Draw.Pipeline].GetUsage(Stage).Uniforms;

        UInt32 First;
        UInt32 Count;

        if (!Differ<Stream>(mCommand.Uniforms, Before, Draw.Uniforms, After, First, Count))
        {
            return;
        }

        Ptr<ID3D11Buffer> Buffers[Command::kMaxUniforms];
        UINT              Offsets[Command::kMaxUniforms];
        UINT              Lengths[Command::kMaxUniforms];

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            const Stream New = Pick<Stream>(Draw.Uniforms, After, Slot);

            Buffers[Slot] = mBuffers[New.Buffer].GetHandle();
            Offsets[Slot] = New.Offset / sizeof(Vector4);
            Lengths[Slot] = New.Stride / sizeof(Vector4);
        }

        if (const Ptr<ID3D11DeviceContext1> Context = mDevice.GetContext(); Stage == ShaderStage::Vertex)
        {
            Context->VSSetConstantBuffers1(First, Count, Buffers + First, Offsets + First, Lengths + First);
        }
        else
        {
            Context->PSSetConstantBuffers1(First, Count, Buffers + First, Offsets + First, Lengths + First);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplySamplers(ShaderStage Stage, ConstRef<Command> Draw)
    {
        const UInt32 Before = mPipelines[mCommand.Pipeline].GetUsage(Stage).Samplers;
        const UInt32 After  = mPipelines[Draw.Pipeline].GetUsage(Stage).Samplers;

        UInt32 First;
        UInt32 Count;

        if (!Differ<Object>(mCommand.Samplers, Before, Draw.Samplers, After, First, Count))
        {
            return;
        }

        Ptr<ID3D11SamplerState> States[Command::kMaxSamplers];

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            States[Slot] = mSamplers[Pick<Object>(Draw.Samplers, After, Slot)].GetHandle();
        }

        if (const Ptr<ID3D11DeviceContext1> Context = mDevice.GetContext(); Stage == ShaderStage::Vertex)
        {
            Context->VSSetSamplers(First, Count, States + First);
        }
        else
        {
            Context->PSSetSamplers(First, Count, States + First);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Binder::ApplyTextures(ShaderStage Stage, ConstRef<Command> Draw)
    {
        const UInt32 Before = mPipelines[mCommand.Pipeline].GetUsage(Stage).Textures;
        const UInt32 After  = mPipelines[Draw.Pipeline].GetUsage(Stage).Textures;

        UInt32 First;
        UInt32 Count;

        if (!Differ<Object>(mCommand.Textures, Before, Draw.Textures, After, First, Count))
        {
            return;
        }

        Ptr<ID3D11ShaderResourceView> Views[Command::kMaxTextures];

        for (UInt32 Slot = First; Slot < First + Count; ++Slot)
        {
            Views[Slot] = mTextures[Pick<Object>(Draw.Textures, After, Slot)].GetView();
        }

        if (const Ptr<ID3D11DeviceContext1> Context = mDevice.GetContext(); Stage == ShaderStage::Vertex)
        {
            Context->VSSetShaderResources(First, Count, Views + First);
        }
        else
        {
            Context->PSSetShaderResources(First, Count, Views + First);
        }
    }
}