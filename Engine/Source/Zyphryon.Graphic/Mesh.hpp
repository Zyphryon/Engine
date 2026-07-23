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

#include "Service.hpp"
#include "Zyphryon.Content/Resource.hpp"
#include "Zyphryon.Math/Geometry/Box.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a GPU-backed geometry asset: one shared vertex/index buffer split into drawable primitives.
    class Mesh final : public Content::AbstractResource<Mesh>
    {
    public:

        /// \brief Describes one vertex attribute's physical placement in the shared buffer, keyed by `VertexSlot`.
        struct Binding final
        {
            /// The element format of this attribute.
            VertexFormat Format = VertexFormat::Float32x4;

            /// The byte stride between consecutive vertices in this attribute's interleaving group; `0` if absent.
            UInt16       Stride = 0;

            /// The byte offset where this attribute's data begins within the shared buffer.
            UInt32       Offset = 0;
        };

        /// \brief Describes one drawable subrange of the mesh, sharing its bindings and drawn with one material slot.
        struct Primitive final
        {
            /// The draw range into the mesh's buffers: index count, base vertex, and first-index byte offset.
            Invocation Range;

            /// The slot into the owning model's material table this primitive is drawn with.
            UInt8      Material = 0;
        };

        /// \brief Enumerates optional per-mesh flags.
        enum class Property : UInt8
        {
            None     = 0,      ///< No flags set.
            Extended = 1 << 0, ///< Index buffer is 32-bit (else 16-bit).
        };
        ZY_DEFINE_BITWISE_FRIEND_ENUM(Property)

    public:

        /// \brief Constructs a mesh resource with the given content key.
        ///
        /// \param Key The unique content key identifying this mesh.
        explicit Mesh(AnyRef<Content::Uri> Key);

        /// \brief Sets the mesh's full property mask, replacing any previously set flags.
        ///
        /// \param Value The complete set of properties to assign.
        ZY_INLINE void SetProperties(Property Value)
        {
            mProperties = Value;
        }

        /// \brief Tests whether a property is enabled.
        ///
        /// \param Value The property to test.
        /// \return `true` if the property is set, `false` otherwise.
        ZY_INLINE Bool HasProperty(Property Value) const
        {
            return HasBit(mProperties, Value);
        }

        /// \brief Gets the mesh's full property mask.
        ///
        /// \return The set of enabled properties.
        ZY_INLINE Property GetProperties() const
        {
            return mProperties;
        }

        /// \brief Appends a primitive that draws a subrange of the shared buffers.
        ///
        /// \param Definition The draw range and material slot.
        void AddPrimitive(ConstRef<Primitive> Definition);

        /// \brief Gets the primitives that compose this mesh.
        ///
        /// \return A read-only view over the mesh's primitives.
        ZY_INLINE ConstSpan<Primitive> GetPrimitives() const
        {
            return mPrimitives;
        }

        /// \brief Sets the local-space axis-aligned bounds enclosing every vertex position.
        ///
        /// \param Bounds The bounding box in the mesh's own coordinate space.
        ZY_INLINE void SetBounds(ConstRef<Box> Bounds)
        {
            mBounds = Bounds;
        }

        /// \brief Gets the local-space axis-aligned bounds enclosing every vertex position.
        ///
        /// \return The bounding box in the mesh's own coordinate space.
        ZY_INLINE ConstRef<Box> GetBounds() const
        {
            return mBounds;
        }

        /// \brief Sets the shared vertex buffer's CPU data, uploaded on the next `Upload`.
        ///
        /// \param Data The raw vertex data holding every attribute region for the mesh.
        void SetVertices(AnyRef<Blob> Data);

        /// \brief Gets the shared vertex buffer handle.
        ///
        /// \return The GPU buffer object, or `0` before upload.
        ZY_INLINE Object GetVertices() const
        {
            return mVertices;
        }

        /// \brief Declares the physical placement of a vertex attribute within the shared buffer.
        ///
        /// \param Slot   The attribute's semantic slot.
        /// \param Format The attribute's element format.
        /// \param Stride The byte stride between consecutive vertices in the attribute's interleaving group.
        /// \param Offset The byte offset where the attribute's data begins within the shared buffer.
        void SetBinding(VertexSlot Slot, VertexFormat Format, UInt16 Stride, UInt32 Offset);

        /// \brief Gets the physical placement of a vertex attribute.
        ///
        /// \param Slot The attribute's semantic slot.
        /// \return The attribute's binding; its `Stride` is `0` when the mesh does not provide the attribute.
        ZY_INLINE Binding GetBinding(VertexSlot Slot) const
        {
            return mBindings[Enum::Cast(Slot)];
        }

        /// \brief Tests whether the mesh provides a vertex attribute.
        ///
        /// \param Slot The attribute's semantic slot.
        /// \return `true` if the attribute is present, `false` otherwise.
        ZY_INLINE Bool HasBinding(VertexSlot Slot) const
        {
            return mBindings[Enum::Cast(Slot)].Stride != 0;
        }

        /// \brief Sets the shared index buffer's CPU data.
        ///
        /// \param Data The raw index data uploaded on the next `Upload`.
        void SetIndices(AnyRef<Blob> Data);

        /// \brief Gets the shared index buffer handle.
        ///
        /// \return The GPU buffer object, or `0` for non-indexed geometry or before upload.
        ZY_INLINE Object GetIndices() const
        {
            return mIndices;
        }

    public:

        /// \brief Uploads the shared vertex/index buffers to the GPU, creating the buffer resources.
        ///
        /// \param Service The graphic service used to create the resources.
        /// \return `true` if the vertex buffer uploaded successfully, `false` otherwise.
        Bool Upload(Ref<Service> Service);

        /// \brief Unloads the shared vertex/index buffers from the GPU, destroying the buffer resources.
        ///
        /// \param Service The graphic service used to destroy the resources.
        void Unload(Ref<Service> Service);

        /// \see Content::Resource::OnCreate(Ref<Engine::Subsystem::Host>)
        Bool OnCreate(Ref<Engine::Subsystem::Host> Host) override
        {
            return Upload(* Host.GetService<Service>());
        }

        /// \see Content::Resource::OnDelete(Ref<Engine::Subsystem::Host>)
        void OnDelete(Ref<Engine::Subsystem::Host> Host) override
        {
            Unload(* Host.GetService<Service>());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Property                                  mProperties;
        Object                                    mVertices;
        Object                                    mIndices;
        Array<Binding, Enum::Count<VertexSlot>()> mBindings;
        Sequence<Primitive>                       mPrimitives;
        Box                                       mBounds;
        Blob                                      mGeometry;
        Blob                                      mElements;
    };
}
