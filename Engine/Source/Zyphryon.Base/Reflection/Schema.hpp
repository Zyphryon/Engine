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

#include "Field.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Reflection
{
    /// \brief Represents the fields of one type, in the order they were declared.
    class Schema final
    {
    public:

        /// \brief Constructs a schema over no fields.
        ZY_INLINE constexpr Schema()
            : mFields { },
              mName   { },
              mSize   { 0 },
              mAlign  { 0 }
        {
        }

        /// \brief Constructs a schema over the fields of a type of the given shape.
        ///
        /// \param Fields The fields to view, which must outlive the schema.
        /// \param Size   The width of the type the fields are reached through, or zero when it is not known.
        /// \param Align  The alignment of the type the fields are reached through, or zero when it is not known.
        ZY_INLINE constexpr explicit Schema(
            Span<const Field> Fields, Text Name = Text::Empty(), UInt Size = 0, UInt Align = 0)
            : mFields { Fields },
              mName   { Name },
              mSize   { static_cast<UInt16>(Size) },
              mAlign  { static_cast<UInt8>(Align) }
        {
        }

        /// \brief Checks whether the schema describes no fields.
        ///
        /// \return `true` when the schema describes nothing, otherwise `false`.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return mFields.IsEmpty();
        }

        /// \brief Gets the name the type named itself with.
        ///
        /// \return The name of the type, empty when it never named itself.
        ZY_INLINE constexpr Text GetName() const
        {
            return mName;
        }

        /// \brief Gets the number of fields the schema describes.
        ///
        /// \return The count of fields.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return mFields.GetSize();
        }

        /// \brief Gets the width of the type the schema describes.
        ///
        /// \return The width in bytes, or zero when the schema was taken from a field rather than from a type.
        ZY_INLINE constexpr UInt GetWidth() const
        {
            return mSize;
        }

        /// \brief Gets the fields the schema describes, in the order they were declared.
        ///
        /// \return The fields of the schema.
        ZY_INLINE constexpr ConstSpan<Field> GetFields() const
        {
            return mFields;
        }

        /// \brief Gets the field at the specified position.
        ///
        /// \param Index The position of the field.
        /// \return The field at that position.
        ZY_INLINE constexpr ConstRef<Field> operator[](UInt Index) const
        {
            return mFields[Index];
        }

        /// \brief Finds a field by name.
        ///
        /// \param Name The name of the field to find.
        /// \return A pointer to the field, or `nullptr` when the schema has no field by that name.
        ZY_INLINE constexpr ConstPtr<Field> Find(Text Name) const
        {
            for (UInt Index = 0; Index < mFields.GetSize(); ++Index)
            {
                if (mFields[Index].GetName() == Name)
                {
                    return AddressOf(mFields[Index]);
                }
            }
            return nullptr;
        }

    public:

        /// \brief Gets the schema of a described type.
        ///
        /// \tparam Type The type whose schema to get.
        /// \return The schema of that type.
        template<typename Type>
        ZY_INLINE static constexpr Schema Of()
        {
            static_assert(IsDescribed<Type>, "The type has no Reflection::Describe of its own");

            return Schema(
                ConstSpan<Field>(Describe<Type>::kFields), Detail::TagOf<Type>(), sizeof(Type), alignof(Type));
        }

        /// \brief Gets the schema of the value a nested field steps into.
        ///
        /// \param Value The field to step into.
        /// \return The schema of the value, empty when the field steps into nothing.
        ZY_INLINE static constexpr Schema Of(ConstRef<Field> Value)
        {
            return Schema(Value.GetNested());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Span<const Field> mFields;
        Text              mName;
        UInt16            mSize;
        UInt8             mAlign;
    };
}

/// \def ZY_REFLECT
/// \brief Lays down the fields of a type, once, in storage that lasts as long as the program does.
///
/// \note A type spelled with a comma in it, such as a template with two arguments, needs a `using` alias first.
///
/// \param Type The type being described, spelled as it is from the global namespace.
/// \param ...  The fields of the type, in the order they are shown.
#define ZY_REFLECT(Type, ...)                             \
    template<>                                            \
    struct Reflection::Describe<Type> final               \
    {                                                     \
        static constexpr Array kFields = { __VA_ARGS__ }; \
    }

/// \def ZY_REFLECT_CLASSIFIED
/// \brief Names a type reflection cannot name of its own accord, which then travels as its own bytes under a tag.
///
/// \note The type has to be trivially copyable and no wider than a value carries inline.
///
/// \param Type The type being named, spelled as it is from the global namespace.
/// \param Name The tag whoever edits the value matches on.
#define ZY_REFLECT_CLASSIFIED(Type, Name)                                     \
    template<>                                                                \
    struct Reflection::Classify<Type> final                                   \
    {                                                                         \
        static constexpr Reflection::Kind kValue = Reflection::Kind::Foreign; \
                                                                              \
        static constexpr Char             kTag[] = Name;                      \
    }