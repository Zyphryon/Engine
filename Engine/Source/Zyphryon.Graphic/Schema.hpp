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

#include "Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Describes the resource interface of a shader program.
    class Schema final
    {
    public:

        /// \brief Describes a single field within a uniform group.
        struct UniformField final
        {
            /// The hash identifying this field's name.
            UInt64      Hash;

            /// The data type of the field.
            UniformType Type;

            /// The size of the field, in bytes.
            UInt16      Size;

            /// The byte offset of the field within its owning uniform block.
            UInt16      Offset;

            /// The default value of the uniform field.
            Parameter   Value;
        };

        /// \brief Describes the layout of a uniform group for a given uniform scope.
        struct UniformGroup final
        {
            /// The total size of the group, in bytes.
            UInt32                 Size;

            /// The fields contained within the group, in declaration order.
            Sequence<UniformField> Structure;
        };

    public:

        /// \brief Registers a sampler for a given texture slot.
        ///
        /// \param Slot    The logical texture slot to associate with the sampler.
        /// \param Sampler The sampler description to register.
        ZY_INLINE void AddSampler(TextureSlot Slot, Sampler Sampler)
        {
            mSamplers[Enum::Cast(Slot)] = Sampler;
        }

        /// \brief Gets the sampler registered for a given texture slot.
        ///
        /// \param Slot The logical texture slot to query.
        /// \return The sampler description associated with the slot.
        ZY_INLINE Sampler GetSampler(TextureSlot Slot) const
        {
            return mSamplers[Enum::Cast(Slot)];
        }

        /// \brief Registers a texture slot used by the shader.
        ///
        /// \param Slot The logical texture slot to add.
        ZY_INLINE void AddTexture(TextureSlot Slot)
        {
            mTextures.Append(Slot);
        }

        /// \brief Gets all texture slots registered with this schema.
        ///
        /// \return A span over the registered texture slots.
        ZY_INLINE ConstSpan<TextureSlot> GetTextures() const
        {
            return mTextures;
        }

        /// \brief Registers a uniform within a uniform group.
        ///
        /// \param Scope The uniform group the uniform belongs to.
        /// \param Name  The uniform name.
        /// \param Type  The data type of the uniform.
        /// \param Value The default value of the uniform.
        ZY_INLINE void AddUniform(UniformScope Scope, Text Name, UniformType Type, AnyRef<Parameter> Value)
        {
            const UInt16 Size = Parameter::GetSize(Enum::Cast(Type));

            Ref<UniformGroup> Group = mUniforms[Enum::Cast(Scope)];
            Ref<UniformField> Field = Group.Structure.Append(Hash(Name), Type, Size, Group.Size, Move(Value));
            Group.Size += Size;
        }

        /// \brief Gets the uniform group declared for a given scope.
        ///
        /// \param Scope The uniform group to query.
        /// \return The group's total size and fields, in declaration order.
        ZY_INLINE ConstRef<UniformGroup> GetUniforms(UniformScope Scope) const
        {
            return mUniforms[Enum::Cast(Scope)];
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<TextureSlot, Command::kMaxTextures> mTextures;
        Array<Sampler, Command::kMaxTextures>        mSamplers;
        Array<UniformGroup, Command::kMaxUniforms>   mUniforms;
    };
}
