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

#include "JsonValue.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A JSON object type forward declaration.
    class JsonObject;

    /// \brief Provides a convenient interface for manipulating JSON arrays.
    class JsonArray final
    {
    public:

        /// \brief Constructs a null JSON array.
        ZY_INLINE JsonArray()
            : mNode { nullptr }
        {

        }

        /// \brief Constructs a JSON array from a JSON value node.
        ///
        /// \param Node The JSON value node that holds an array.
        ZY_INLINE JsonArray(Ref<JsonValue> Node)
            : mNode { AddressOf(Node) }
        {
            ZY_ASSERT(mNode->IsArray(), "Node is not an array.");
        }

        /// \brief Checks if the array is null.
        ///
        /// \return `true` if the array is null, `false` otherwise.
        ZY_INLINE Bool IsNull() const
        {
            return (mNode == nullptr || mNode->IsNull());
        }

        /// \brief Checks if the array is empty.
        ///
        /// \return `true` if the array is empty, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mNode->GetArray().IsEmpty();
        }

        /// \brief Checks if the array is null or empty.
        ///
        /// \return `true` if the array is null or empty, `false` otherwise.
        ZY_INLINE Bool IsNullOrEmpty() const
        {
            return IsNull() || IsEmpty();
        }

        /// \brief Gets the number of elements in the array.
        ///
        /// \return The number of elements.
        ZY_INLINE UInt GetSize() const
        {
            return mNode->GetArray().GetSize();
        }

        /// \brief Appends a boolean value to the array.
        ///
        /// \param Value The boolean value to append.
        ZY_INLINE void AddBool(Bool Value)
        {
            mNode->GetArray().Append(Value);
        }

        /// \brief Gets the boolean value at the specified index.
        ///
        /// \param Index   The zero-based index of the element.
        /// \param Default The value to return if the element is not a boolean.
        /// \return The boolean value, or the default if the element is not a boolean.
        ZY_INLINE Bool GetBool(UInt Index, Bool Default = false) const
        {
            if (Index < GetSize())
            {
                ConstRef<JsonValue> Value = mNode->GetArray()[Index];

                if (Value.IsBool())
                {
                    return Value.GetBool();
                }
            }
            return Default;
        }

        /// \brief Appends a numeric value to the array.
        ///
        /// \param Value The numeric value to append.
        template<IsNumeric Type>
        ZY_INLINE void AddNumber(Type Value)
        {
            mNode->GetArray().Append(Value);
        }

        /// \brief Gets the numeric value at the specified index.
        ///
        /// \param Index   The zero-based index of the element.
        /// \param Default The value to return if the element is not a number.
        /// \return The numeric value converted to the specified type, or the default if the element is not a number.
        template<IsNumeric Type>
        ZY_INLINE Type GetNumber(UInt Index, Type Default = { }) const
        {
            if (Index < GetSize())
            {
                ConstRef<JsonValue> Value = mNode->GetArray()[Index];

                if (Value.IsNumber())
                {
                    return Value.GetNumber<Type>();
                }
            }
            return Default;
        }

        /// \brief Appends a string value to the array.
        ///
        /// \param Value The text value to append.
        ZY_INLINE void AddString(Text Value)
        {
            mNode->GetArray().Append(Str(Value));
        }

        /// \brief Gets the string value at the specified index.
        ///
        /// \param Index   The zero-based index of the element.
        /// \param Default The value to return if the element is not a string.
        /// \return The string value, or the default if the element is not a string.
        ZY_INLINE Text GetString(UInt Index, Text Default = Text::Empty()) const
        {
            if (Index < GetSize())
            {
                ConstRef<JsonValue> Value = mNode->GetArray()[Index];

                if (Value.IsString())
                {
                    return Value.GetString();
                }
            }
            return Default;
        }

        /// \brief Appends an enumeration value to the array as its name.
        ///
        /// \param Value The enumeration value to append.
        template<IsEnum Type>
        ZY_INLINE void AddEnum(Type Value)
        {
            mNode->GetArray().Append(Str(Enum::GetName(Value)));
        }

        /// \brief Gets the enumeration value at the specified index.
        ///
        /// \param Index   The zero-based index of the element.
        /// \param Default The value to return if the element is not a valid enumeration name.
        /// \return The parsed enumeration value, or the default if the element is not a valid name.
        template<IsEnum Type>
        ZY_INLINE Type GetEnum(UInt Index, Type Default = {}) const
        {
            if (Index < GetSize())
            {
                ConstRef<JsonValue> Value = mNode->GetArray()[Index];

                if (Value.IsString())
                {
                    return Enum::Cast<Type>(Value.GetString(), Default);
                }
            }
            return Default;
        }

        /// \brief Appends a new array to the array and returns a wrapper for it.
        ///
        /// \return A JSON array wrapper for the newly created element.
        ZY_INLINE JsonArray AddArray()
        {
            Ref<JsonValue> Value = mNode->GetArray().Append();
            Value.SetArray();

            return JsonArray(Value);
        }

        /// \brief Gets the array value at the specified index.
        ///
        /// \param Index The zero-based index of the element.
        /// \return A JSON array wrapper, or a null wrapper if the element is not an array.
        ZY_INLINE JsonArray GetArray(UInt Index) const
        {
            if (Index < GetSize())
            {
                Ref<JsonValue> Value = mNode->GetArray()[Index];

                if (Value.IsArray())
                {
                    return JsonArray(Value);
                }
            }
            return JsonArray();
        }

        /// \brief Appends a new object to the array and returns a wrapper for it.
        ///
        /// \return A JSON object wrapper for the newly created element.
        JsonObject AddObject();

        /// \brief Gets the object value at the specified index.
        ///
        /// \param Index The zero-based index of the element.
        /// \return A JSON object wrapper, or a null wrapper if the element is not an object.
        JsonObject GetObject(UInt Index) const;

        /// \brief Formats the JSON value into an output buffer.
        ///
        /// \param Buffer The buffer to format into.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            if (mNode)
            {
                mNode->OnFormat(Buffer);
            }
            else
            {
                Buffer.Append("Null");
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<JsonValue> mNode;
    };
}