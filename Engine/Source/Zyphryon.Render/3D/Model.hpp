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

#include "Skeleton.hpp"
#include "Zyphryon.Graphic/Mesh.hpp"
#include "Zyphryon.Graphic/Material.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Represents a renderable model combining geometry, materials, and an optional rig to deform on.
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

        /// \brief Sets the bone hierarchy this model's geometry is skinned to.
        ///
        /// \param Skeleton The skeleton resource to reference.
        ZY_INLINE void SetSkeleton(ConstRetainer<Render::Skeleton> Skeleton)
        {
            mSkeleton = Skeleton;
        }

        /// \brief Gets the bone hierarchy this model's geometry is skinned to.
        ///
        /// \return The referenced skeleton resource, or null when the model does not deform.
        ZY_INLINE ConstRetainer<Render::Skeleton> GetSkeleton() const
        {
            return mSkeleton;
        }

        /// \brief Gets the local-space axis-aligned bounds of the model's geometry at rest.
        ///
        /// \return The mesh's bind-pose bounds, or an invalid box when the model has no mesh.
        ZY_INLINE Box GetExtent() const
        {
            return mMesh ? mMesh->GetExtent() : Box::Invalid();
        }

        /// \brief Replaces the material table.
        ///
        /// \param Table The materials, ordered so each entry sits at the slot its primitives index by.
        ZY_INLINE void SetMaterials(AnyRef<Sequence<Retainer<Graphic::Material>>> Table)
        {
            mMaterials = Move(Table);
        }

        /// \brief Gets the model's full material table.
        ///
        /// \return A read-only view over the material table.
        ZY_INLINE ConstSpan<Retainer<Graphic::Material>> GetMaterials() const
        {
            return mMaterials;
        }

        /// \brief Gets the material bound to the given table slot.
        ///
        /// \param Slot The material-table slot to query.
        /// \return The material resource at the slot.
        ZY_INLINE ConstRetainer<Graphic::Material> GetMaterial(UInt8 Slot) const
        {
            return mMaterials[Slot];
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
        Retainer<Render::Skeleton>            mSkeleton;
        Sequence<Retainer<Graphic::Material>> mMaterials;
    };
}