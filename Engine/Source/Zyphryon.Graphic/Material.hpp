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

        /// \brief Binds an image to the texture a technique declares under the same name.
        struct TextureEntry final
        {
            /// The hash identifying the texture's name.
            UInt64          Hash = 0;

            /// The image bound to the texture.
            Retainer<Image> Image;
        };

        /// \brief Supplies the state for the dynamic sampler a technique declares under the same name.
        struct SamplerEntry final
        {
            /// The hash identifying the sampler's name.
            UInt64  Hash   = 0;

            /// The state requested in place of the technique's default.
            Sampler Descriptor;

            /// The sampler resource resolved from the descriptor, or zero until the material is uploaded.
            Object  Handle = 0;
        };

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

        /// \brief Binds an image to the named texture, replacing any image already bound to it.
        ///
        /// \param Name  The pre-hashed texture name.
        /// \param Image The image resource to bind.
        void SetImage(UInt64 Name, ConstRetainer<Image> Image);

        /// \brief Gets the image bound to the named texture.
        ///
        /// \param Name The pre-hashed texture name.
        /// \return The image resource bound to the texture, or null if none.
        Retainer<Image> GetImage(UInt64 Name) const;

        /// \brief Gets every texture bound by this material.
        ///
        /// \return A span over the material's texture entries.
        ZY_INLINE ConstSpan<TextureEntry> GetImages() const
        {
            return mTextures;
        }

        /// \brief Sets the state for the named sampler, replacing the technique's default.
        ///
        /// \param Name       The pre-hashed sampler name.
        /// \param Descriptor The sampler state to apply.
        void SetSampler(UInt64 Name, Sampler Descriptor);

        /// \brief Gets the sampler resource bound to the named sampler.
        ///
        /// \param Name The pre-hashed sampler name.
        /// \return The sampler resource bound to the texture, or null 0 if none.
        Object GetSampler(UInt64 Name) const;

        /// \brief Gets every sampler set by this material.
        ///
        /// \return A span over the material's sampler entries.
        ZY_INLINE ConstSpan<SamplerEntry> GetSamplers() const
        {
            return mSamplers;
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

        Object                                        mHandle;
        Sequence<TextureEntry, Command::kMaxTextures> mTextures;
        Sequence<SamplerEntry, Command::kMaxSamplers> mSamplers;
        Table<UInt64, Parameter>                      mParameters;
    };
}