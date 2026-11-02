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

#include "Animation3D.hpp"
#include "Zyphryon.Graphic/Material.hpp"
#include "Zyphryon.Graphic/Mesh.hpp"
#include "Zyphryon.Math/Motion/Playback.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents a renderable model combining geometry, materials, and an optional rig to deform on.
    class ZY_API Model3D final : public ZyContent::AbstractResource<Model3D>
    {
    public:

        /// \brief Represents one clip of the model, the motion it plays and the way it plays it.
        struct Clip final
        {
            /// The hash of the clip's name.
            UInt64                Name = 0;

            /// The name the clip is shown by, which may be cut short, while lookups go by \ref Name alone.
            Str32                 Label;

            /// The motion the clip plays over the model's rig, which other models with the same bones may share.
            Retainer<Animation3D> Motion;

            /// The way the clip behaves once it reaches its end.
            Repeat                Mode = Repeat::Loop;

            /// The rate the clip plays at against its authored timing, where `1` plays as authored.
            Real32                Rate = 1.0f;
        };

    public:

        /// \brief Constructs a model resource with the given content key.
        ///
        /// \param Key The unique content key identifying this model.
        explicit Model3D(AnyRef<ZyContent::Uri> Key);

        /// \brief Sets the geometry of this model.
        ///
        /// \param Mesh The mesh resource to reference.
        ZY_INLINE void SetMesh(ConstRetainer<ZyGraphic::Mesh> Mesh)
        {
            mMesh = Mesh;
        }

        /// \brief Gets the geometry of this model.
        ///
        /// \return The referenced mesh resource, or an empty retainer when none was set.
        ZY_INLINE ConstRetainer<ZyGraphic::Mesh> GetMesh() const
        {
            return mMesh;
        }

        /// \brief Sets the bone hierarchy this model's geometry is skinned to.
        ///
        /// \param Skeleton The skeleton resource to reference.
        ZY_INLINE void SetSkeleton(ConstRetainer<Skeleton3D> Skeleton)
        {
            mSkeleton = Skeleton;
        }

        /// \brief Gets the bone hierarchy this model's geometry is skinned to.
        ///
        /// \return The referenced skeleton resource, or an empty retainer when the model does not deform.
        ZY_INLINE ConstRetainer<Skeleton3D> GetSkeleton() const
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
        ZY_INLINE void SetMaterials(AnyRef<Sequence<Retainer<ZyGraphic::Material>>> Table)
        {
            mMaterials = Move(Table);
        }

        /// \brief Gets the model's full material table.
        ///
        /// \return The materials, indexed by the slot each primitive names.
        ZY_INLINE ConstSpan<Retainer<ZyGraphic::Material>> GetMaterials() const
        {
            return mMaterials;
        }

        /// \brief Gets the material bound to the given table slot.
        ///
        /// \param Slot The material-table slot to query.
        /// \return The material resource at the slot.
        ZY_INLINE ConstRetainer<ZyGraphic::Material> GetMaterial(UInt8 Slot) const
        {
            return mMaterials[Slot];
        }

        /// \brief Replaces every clip of the model.
        ///
        /// \param Clips The clips, in authored order.
        ZY_INLINE void SetClips(AnyRef<Sequence<Clip>> Clips)
        {
            mClips = Move(Clips);
        }

        /// \brief Gets every clip of the model.
        ///
        /// \return The clips, in authored order.
        ZY_INLINE ConstSpan<Clip> GetClips() const
        {
            return mClips;
        }

        /// \brief Finds a clip by name.
        ///
        /// \param Name The hash of the clip's name.
        /// \return The clip, or `nullptr` when the model has no clip by that name.
        ZY_INLINE ConstPtr<Clip> FindClip(UInt64 Name) const
        {
            const SInt Index = mClips.Find([Name](ConstRef<Clip> Entry)
            {
                return Entry.Name == Name;
            });
            return Index < 0 ? nullptr : AddressOf(mClips[Index]);
        }

    public:

        /// \brief Uploads the model's exclusively-owned mesh and materials to the GPU.
        ///
        /// \param Service The graphic service used to create the resources.
        /// \return `true` if every upload succeeded, `false` otherwise.
        Bool Upload(Ref<ZyGraphic::Service> Service);

        /// \brief Unloads the model's exclusively-owned mesh and materials from the GPU.
        ///
        /// \param Service The graphic service used to destroy the resources.
        void Unload(Ref<ZyGraphic::Service> Service);

        /// \see ZyContent::Resource::OnCreate(Ref<ZyEngine::Subsystem::Host>)
        Bool OnCreate(Ref<ZyEngine::Subsystem::Host> Host) override
        {
            return Upload(* Host.GetService<ZyGraphic::Service>());
        }

        /// \see ZyContent::Resource::OnDelete(Ref<ZyEngine::Subsystem::Host>)
        void OnDelete(Ref<ZyEngine::Subsystem::Host> Host) override
        {
            Unload(* Host.GetService<ZyGraphic::Service>());
        }

        /// \see ZyContent::Resource::OnReload(Ref<ZyEngine::Subsystem::Host>)
        void OnReload(Ref<ZyEngine::Subsystem::Host> Host) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Mesh>               mMesh;
        Retainer<Skeleton3D>                    mSkeleton;
        Sequence<Retainer<ZyGraphic::Material>> mMaterials;
        Sequence<Clip>                          mClips;
    };
}