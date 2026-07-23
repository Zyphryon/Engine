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

#include "Mesh.hpp"
#include "Material.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
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
        ZY_INLINE void SetMesh(ConstRetainer<Mesh> Mesh)
        {
            mMesh = Mesh;
        }

        /// \brief Gets the geometry of this model.
        ///
        /// \return The referenced mesh resource, or null if unset.
        ZY_INLINE ConstRetainer<Mesh> GetMesh() const
        {
            return mMesh;
        }

        /// \brief Gets the local-space axis-aligned bounds of the model's geometry.
        ///
        /// \return The mesh's bounding box, or an invalid box when the model has no mesh.
        ZY_INLINE Box GetBounds() const
        {
            return mMesh ? mMesh->GetBounds() : Box::Invalid();
        }

        /// \brief Appends a material to the table, in the order its primitives reference by slot.
        ///
        /// \param Material The material resource to append.
        /// \return The material's slot index, matching `Mesh::Primitive::Material`.
        UInt8 AddMaterial(ConstRetainer<Material> Material);

        /// \brief Gets the material bound to the given table slot.
        ///
        /// \param Slot The material-table slot to query.
        /// \return The material resource at the slot.
        ZY_INLINE ConstRetainer<Material> GetMaterial(UInt8 Slot) const
        {
            return mMaterials[Slot];
        }

        /// \brief Gets the model's full material table.
        ///
        /// \return A read-only view over the material table.
        ZY_INLINE ConstSpan<Retainer<Material>> GetMaterials() const
        {
            return mMaterials;
        }

    public:

        /// \brief Uploads the model's exclusively-owned mesh and materials to the GPU.
        ///
        /// \param Service The graphic service used to create the resources.
        /// \return `true` on success.
        Bool Upload(Ref<Service> Service);

        /// \brief Unloads the model's exclusively-owned mesh and materials from the GPU.
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

        Retainer<Mesh>               mMesh;
        Sequence<Retainer<Material>> mMaterials;
    };
}
