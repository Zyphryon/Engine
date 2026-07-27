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

#include "Zyphryon.Graphic/Resource/Mesh.hpp"
#include "Zyphryon.Graphic/Resource/Material.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Represents a model resource combining materials, and meshes.
    class Model final : public Content::AbstractResource<Model>
    {
    public:

        /// \brief Constructs a model resource with the given content key.
        ///
        /// \param Key The unique content key identifying this model.
        explicit Model(AnyRef<Content::Uri> Key);

        /// \brief Sets the geometry of this model.
        ///
        /// \param Mesh The mesh resource to reference.
        ZY_INLINE void SetMesh(ConstRetainer<Graphic::Mesh> Mesh)
        {
            mMesh = Mesh;
        }

        /// \brief Gets the geometry of this model.
        ///
        /// \return The referenced mesh resource, or null if unset.
        ZY_INLINE ConstRetainer<Graphic::Mesh> GetMesh() const
        {
            return mMesh;
        }

        /// \brief Sets the bounds the model is culled by, overriding the ones its mesh reports.
        ///
        /// Skinned geometry needs this. A mesh only knows the extent of its bind pose, so a model that raises
        /// an arm or swings a blade reaches outside it and gets culled while still on screen. Give such a model
        /// bounds wide enough for every pose it can strike, and leave it unset for anything that does not
        /// deform.
        ///
        /// \param Bounds The bounding box to cull by, or an invalid box to fall back on the mesh.
        ZY_INLINE void SetBounds(ConstRef<Box> Bounds)
        {
            mBounds = Bounds;
        }

        /// \brief Gets the local-space axis-aligned bounds the model is culled by.
        ///
        /// \return The overriding bounds when one was set, otherwise the mesh's own bind-pose bounds, or an
        ///         invalid box when the model has neither.
        ZY_INLINE Box GetBounds() const
        {
            if (mBounds.IsValid())
            {
                return mBounds;
            }
            return mMesh ? mMesh->GetBounds() : Box::Invalid();
        }

        /// \brief Checks whether the model carries bounds of its own rather than borrowing its mesh's.
        ///
        /// \return `true` when bounds were set explicitly, otherwise `false`.
        ZY_INLINE Bool HasBounds() const
        {
            return mBounds.IsValid();
        }

        /// \brief Appends a material to the table, in the order its primitives reference by slot.
        ///
        /// \param Material The material resource to append.
        /// \return The material's slot index, matching `Mesh::Primitive::Material`.
        UInt8 AddMaterial(ConstRetainer<Graphic::Material> Material);

        /// \brief Gets the material bound to the given table slot.
        ///
        /// \param Slot The material-table slot to query.
        /// \return The material resource at the slot.
        ZY_INLINE ConstRetainer<Graphic::Material> GetMaterial(UInt8 Slot) const
        {
            return mMaterials[Slot];
        }

        /// \brief Gets the model's full material table.
        ///
        /// \return A read-only view over the material table.
        ZY_INLINE ConstSpan<Retainer<Graphic::Material>> GetMaterials() const
        {
            return mMaterials;
        }

    public:

        /// \brief Uploads the model's exclusively-owned mesh and materials to the GPU.
        ///
        /// \param Service The graphic service used to create the resources.
        /// \return `true` on success.
        Bool Upload(Ref<Graphic::Service> Service);

        /// \brief Unloads the model's exclusively-owned mesh and materials from the GPU.
        ///
        /// \param Service The graphic service used to destroy the resources.
        void Unload(Ref<Graphic::Service> Service);

        /// \see Content::Resource::OnCreate(Ref<Engine::Subsystem::Host>)
        Bool OnCreate(Ref<Engine::Subsystem::Host> Host) override
        {
            return Upload(* Host.GetService<Graphic::Service>());
        }

        /// \see Content::Resource::OnDelete(Ref<Engine::Subsystem::Host>)
        void OnDelete(Ref<Engine::Subsystem::Host> Host) override
        {
            Unload(* Host.GetService<Graphic::Service>());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Graphic::Mesh>               mMesh;
        Box                                   mBounds = Box::Invalid();
        Sequence<Retainer<Graphic::Material>> mMaterials;
    };
}
