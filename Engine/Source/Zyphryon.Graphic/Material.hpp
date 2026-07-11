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

#include "Image.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a material resource combining textures, samplers, and uniform data.
    class Material final : public Content::AbstractResource<Material>
    {
    public:

        /// \brief Constructs a material resource with the given content key.
        ///
        /// \param Key The unique content key identifying this material.
        explicit Material(AnyRef<Content::Uri> Key);

        /// \brief Gets the resource handle for this material.
        ///
        /// \return The material's object handle.
        ZY_INLINE Object GetHandle() const
        {
            return mHandle;
        }

        /// \brief Sets the texture image for the specified texture slot.
        ///
        /// \param Slot  The texture slot to bind the image to.
        /// \param Image The image resource to bind.
        ZY_INLINE void SetImage(TextureSlot Slot, ConstRetainer<Image> Image)
        {
            mTextures[Enum::Cast(Slot)] = Image;
        }

        /// \brief Gets the texture image bound to the specified texture slot.
        ///
        /// \param Slot The texture slot to query.
        /// \return The image resource bound to the slot, or null if none.
        ZY_INLINE ConstRetainer<Image> GetImage(TextureSlot Slot) const
        {
            return mTextures[Enum::Cast(Slot)];
        }

        /// \brief Sets the sampler state for the specified texture slot.
        ///
        /// \param Slot    The texture slot to configure.
        /// \param Sampler The sampler state to apply.
        ZY_INLINE void SetSampler(TextureSlot Slot, Sampler Sampler)
        {
            mSamplers[Enum::Cast(Slot)] = Sampler;
        }

        /// \brief Gets the sampler state for the specified texture slot.
        ///
        /// \param Slot The texture slot to query.
        /// \return The sampler state bound to the slot.
        ZY_INLINE Sampler GetSampler(TextureSlot Slot) const
        {
            return mSamplers[Enum::Cast(Slot)];
        }

        /// \brief Sets a uniform value identified by a pre-hashed name.
        ///
        /// \param Key   The pre-hashed uniform name.
        /// \param Value The uniform value to assign.
        template<typename Type>
        ZY_INLINE void SetParameter(UInt64 Key, AnyRef<Type> Value)
        {
            mParameters.Assign(Key, Forward<Type>(Value));
        }

        /// \brief Removes a uniform value by its pre-hashed name.
        ///
        /// \param Key The pre-hashed uniform name to remove.
        ZY_INLINE void UnsetParameter(UInt64 Key)
        {
            mParameters.Erase(Key);
        }

        /// \brief Gets a uniform value by its pre-hashed name.
        ///
        /// \param Key The pre-hashed uniform name.
        /// \return Pointer to the uniform value, or null if not found.
        ZY_INLINE ConstPtr<Parameter> GetParameter(UInt64 Key) const
        {
            return mParameters.Find(Key);
        }

    public:

        /// \brief Uploads the material to the GPU, creating the material resource.
        ///
        /// \param Service The graphic service used to create the resource.
        /// \return `true` if the upload succeeded, `false` otherwise.
        Bool Upload(Ref<Service> Service);

        /// \brief Unloads the material from the GPU, destroying the material resource.
        ///
        /// \param Service The graphic service used to destroy the resource.
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

        Object                                             mHandle;
        Array<Retainer<Image>, Enum::Count<TextureSlot>()> mTextures;
        Array<Sampler, Enum::Count<TextureSlot>()>         mSamplers;
        Table<UInt64, Parameter>                           mParameters;
    };
}