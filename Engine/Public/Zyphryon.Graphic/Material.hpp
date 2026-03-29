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

#include "Texture.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a material asset within the content system.
    class Material final : public Content::AbstractResource<Material>
    {
        friend class AbstractResource;

    public:

        /// \brief Constructs a material resource with the given content key.
        ///
        /// \param Key The unique content key identifying this material.
        explicit Material(AnyRef<Content::Uri> Key);

        /// \brief Gets the unique object ID of the material.
        ///
        /// \return The material's object ID.
        ZYPHRYON_INLINE Object GetID() const
        {
            return mID;
        }

        /// \brief Sets a texture for the specified semantic slot.
        ///
        /// \param Semantic The texture semantic slot to set
        /// \param Texture  The texture to assign to the slot.
        ZYPHRYON_INLINE void SetTexture(TextureSemantic Semantic, ConstTracker<Texture> Texture)
        {
            mTextures[Enum::Cast(Semantic)] = Texture;
        }

        /// \brief Gets the texture assigned to the specified semantic slot.
        ///
        /// \param Semantic The texture semantic slot to retrieve.
        /// \return The texture assigned to the slot.
        ZYPHRYON_INLINE ConstTracker<Texture> GetTexture(TextureSemantic Semantic) const
        {
            return mTextures[Enum::Cast(Semantic)];
        }

        /// \brief Sets a sampler for the specified semantic slot.
        ///
        /// \param Semantic The texture semantic slot to set
        /// \param Sampler  The sampler to assign to the slot.
        ZYPHRYON_INLINE void SetSampler(TextureSemantic Semantic, Sampler Sampler)
        {
            mSamplers[Enum::Cast(Semantic)] = Sampler;
        }

        /// \brief Gets the sampler assigned to the specified semantic slot.
        ///
        /// \param Semantic The texture semantic slot to retrieve.
        /// \return The sampler assigned to the slot.
        ZYPHRYON_INLINE Sampler GetSampler(TextureSemantic Semantic) const
        {
            return mSamplers[Enum::Cast(Semantic)];
        }

        /// \brief Sets a parameter value at the specified byte offset.
        ///
        /// \param Offset    The byte offset within the parameter buffer.
        /// \param Parameter The parameter value to set.
        template<typename Type>
        ZYPHRYON_INLINE void SetParameter(UInt32 Offset, Type Parameter)
        {
            static_assert(IsTrivial<Type>, "Material::SetParameter only supports trivially copyable types.");

            constexpr UInt Size = sizeof(Type);

            if (Offset + Size > mParameters.size())
            {
                mParameters.resize(Offset + Size);
            }

            std::memcpy(mParameters.data() + Offset, std::addressof(Parameter), Size);
        }

        /// \brief Gets a span of the material's parameter buffer.
        ///
        /// \return A constant span of the parameter buffer bytes.
        ZYPHRYON_INLINE ConstSpan<Byte> GetParameters() const
        {
            return mParameters;
        }

    private:

        /// \copydoc Resource::OnCreate(Ref<Service::Host>)
        Bool OnCreate(Ref<Service::Host> Host) override;

        /// \copydoc Resource::OnDelete(Ref<Service::Host>)
        void OnDelete(Ref<Service::Host> Host) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Object                                 mID;
        Array<Tracker<Texture>, kMaxResources> mTextures;
        Array<Sampler, kMaxResources>          mSamplers;
        Vector<Byte>                           mParameters;
    };
}