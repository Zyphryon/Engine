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

#include "D3D11Texture.hpp"
#include "Zyphryon.Graphic/Metadata.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void Split(TextureFormat Format, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data, Ref<Sequence<D3D11_SUBRESOURCE_DATA>> Output)
    {
        ConstPtr<Byte> Bytes = Data.GetData();

        // Slice-major, so the entry order matches what D3D11CalcSubresource indexes.
        for (UInt16 Layer = 0; Layer < Layers; ++Layer)
        {
            for (UInt8 Level = 0; Level < Levels; ++Level)
            {
                const UInt32 Pitch = GetLevelPitch(Format, Width, Level);
                const UInt32 Rows  = GetLevelRows(Format, Height, Level);

                Ref<D3D11_SUBRESOURCE_DATA> Entry = Output.Append();
                Entry.pSysMem          = Bytes;
                Entry.SysMemPitch      = Pitch;
                Entry.SysMemSlicePitch = Pitch * Rows;

                Bytes += Pitch * Rows;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static D3D11_SRV_DIMENSION GetDimension(TextureLayout Layout)
    {
        switch (Layout)
        {
        case TextureLayout::TextureCube:
            return D3D11_SRV_DIMENSION_TEXTURECUBE;
        case TextureLayout::Texture2DArray:
            return D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        default:
            return D3D11_SRV_DIMENSION_TEXTURE2D;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Texture::Texture()
        : mFormat { TextureFormat::Unspecified },
          mLevels { 1 },
          mLayers { 1 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Texture::Create(ConstRef<Device> Device, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data)
    {
        const Bool Cube = (Layout == TextureLayout::TextureCube);

        mFormat = Format;
        mLevels = Levels;
        mLayers = Cube ? 6 : Max<UInt16>(1, Layers);

        CD3D11_TEXTURE2D_DESC Description(Convert(Format), Width, Height, mLayers, mLevels);
        Description.Usage          = Convert(Storage);
        Description.BindFlags      = HasBit(Usage, Usage::Sample)  ? D3D11_BIND_SHADER_RESOURCE : 0;
        Description.CPUAccessFlags = (Storage == Storage::Dynamic) ? D3D11_CPU_ACCESS_WRITE     : 0;
        Description.MiscFlags      = Cube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

        if (HasBit(Usage, Usage::Target))
        {
            Description.BindFlags |= GetTextureMetadata(Format).IsDepth
                ? D3D11_BIND_DEPTH_STENCIL
                : D3D11_BIND_RENDER_TARGET;
        }

        Sequence<D3D11_SUBRESOURCE_DATA> Content;

        if (!Data.IsEmpty())
        {
            Content.Reserve(mLayers * mLevels);
            Split(Format, Width, Height, mLayers, mLevels, Data, Content);
        }

        const Ptr<D3D11_SUBRESOURCE_DATA> Memory = (Content.IsEmpty() ? nullptr : Content.GetData());
        if (!Check(Device.GetHandle()->CreateTexture2D(AddressOf(Description), Memory, mHandle.GetAddressOf())))
        {
            return;
        }

        if (HasBit(Usage, Usage::Sample))
        {
            const CD3D11_SHADER_RESOURCE_VIEW_DESC View(GetDimension(Layout), ConvertView(Format), 0, mLevels, 0, mLayers);
            Check(Device.GetHandle()->CreateShaderResourceView(mHandle.Get(), AddressOf(View), mView.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Texture::Update(ConstRef<Device> Device, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
    {
        constexpr D3D11_COPY_FLAGS Flags = D3D11_COPY_NO_OVERWRITE;

        const UINT       Subresource = D3D11CalcSubresource(Level, Layer, mLevels);
        const CD3D11_BOX Region(X, Y, 0, X + Width, Y + Height, 1);
        Device.GetContext()->UpdateSubresource1(mHandle.Get(), Subresource, AddressOf(Region), Data.GetData(), Pitch, 0, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Texture::Delete()
    {
        * this = Texture();
    }
}