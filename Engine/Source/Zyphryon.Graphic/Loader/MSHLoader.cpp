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

#include "MSHLoader.hpp"
#include "Zyphryon.Graphic/Mesh.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ReadCompressedBuffer(Ref<Reader> Body, Ref<Blob> Result)
    {
        const UInt32          Size    = Body.Read<UInt32>();
        const ConstSpan<Byte> Payload = Body.ReadBlock<UInt32, Byte>();

        if (Size == 0 || Payload.IsEmpty())
        {
            return false;
        }

        Result = Blob::Allocate<Byte>(Size);

        if (Size != Payload.GetSize())
        {
            return LZ4Decode(Payload, Result.GetData<Byte>(), Size) == Size;
        }

        Result.Copy<Byte>(Payload.GetData(), Size);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MSHLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        Reader Input(Data.GetData(), Data.GetSize());

        if (Input.Read<UInt32>() != ('Z' | ('M' << 8) | ('S' << 16) | ('H' << 24)))
        {
            LOG_W("'{}' is not a ZMSH file (bad magic)", Scope.GetResource()->GetKey());
            return false;
        }

        if (Input.Read<UInt16>() != 1)
        {
            LOG_W("'{}' has an unsupported ZMSH version", Scope.GetResource()->GetKey());
            return false;
        }

        const Retainer<Mesh> Asset = Retainer<Mesh>::Cast(Scope.GetResource());

        // Walk the tag-length-value chunk stream; unrecognized tags are skipped by their length.
        while (Input.GetAvailable() >= 2 * sizeof(UInt32))
        {
            const UInt32 Tag    = Input.Read<UInt32>();
            const UInt32 Length = Input.Read<UInt32>();

            if (Length > Input.GetAvailable())
            {
                LOG_W("'{}' has a truncated ZMSH chunk", Scope.GetResource()->GetKey());
                return false;
            }

            Reader Body = Input.Split(Length);

            switch (Tag)
            {
            case ('M' | ('E' << 8) | ('S' << 16) | ('H' << 24)):
            {
                Asset->SetProperties(Body.Read<Mesh::Property>());
                Asset->SetBounds(Body.Read<Box>());

                for (UInt Count = Body.Read<UInt8>(); Count > 0; --Count)
                {
                    const VertexSlot   Slot   = Body.Read<VertexSlot>();
                    const VertexFormat Format = Body.Read<VertexFormat>();
                    const UInt16       Stride = Body.Read<UInt16>();
                    const UInt32       Offset = Body.Read<UInt32>();
                    Asset->SetBinding(Slot, Format, Stride, Offset);
                }

                for (UInt Count = Body.Read<UInt16>(); Count > 0; --Count)
                {
                    Asset->AddPrimitive(Body.Read<Mesh::Primitive>());
                }
                break;
            }
            case ('V' | ('B' << 8) | ('U' << 16) | ('F' << 24)):
            {
                Blob Vertices;

                if (ReadCompressedBuffer(Body, Vertices))
                {
                    Asset->SetVertices(Move(Vertices));
                }
                else
                {
                    LOG_W("'{}' has an invalid vertex buffer", Scope.GetResource()->GetKey());
                    return false;
                }
                break;
            }
            case ('I' | ('B' << 8) | ('U' << 16) | ('F' << 24)):
            {
                Blob Indices;

                if (ReadCompressedBuffer(Body, Indices))
                {
                    Asset->SetIndices(Move(Indices));
                }
                else
                {
                    LOG_W("'{}' has an invalid index buffer", Scope.GetResource()->GetKey());
                    return false;
                }
                break;
            }
            default:
                break;
            }
        }
        return true;
    }
}
