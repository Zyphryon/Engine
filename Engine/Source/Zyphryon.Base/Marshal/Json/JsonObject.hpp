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
    /// \brief A JSON array type forward declaration.
    class JsonArray;

    /// \brief Provides a convenient interface for manipulating JSON objects.
    class JsonObject final
    {
    public:

        /// \brief Constructs a null JSON object.
        ZY_INLINE JsonObject()
            : mNode { nullptr }
        {

        }

        /// \brief Constructs a JSON object from a JSON value node.
        ///
        /// \param Node The JSON value node that holds an object.
        ZY_INLINE JsonObject(Ref<JsonValue> Node)
            : mNode { AddressOf(Node) }
        {
            ZY_ASSERT(mNode->IsObject(), "Node is not an object.");
        }

        /// \brief Checks if the object is valid.
        ///
        /// \return `true` if the object is valid, `false` otherwise.
        ZY_INLINE Bool IsValid() const
        {
            return (mNode != nullptr && !mNode->IsNull());
        }

        /// \brief Checks if the object is null.
        ///
        /// \return `true` if the object is null, `false` otherwise.
        ZY_INLINE Bool IsNull() const
        {
            return (mNode == nullptr || mNode->IsNull());
        }

        /// \brief Checks if the object is empty.
        ///
        /// \return `true` if the object is empty, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mNode->GetObject().IsEmpty();
        }

        /// \brief Gets the number of key-value pairs in the object.
        ///
        /// \return The number of key-value pairs.
        ZY_INLINE UInt GetSize() const
        {
            return mNode->GetObject().GetSize();
        }

        /// \brief Gets the underlying JSON value node.
        ///
        /// \return A constant pointer to the JSON value node.
        ZY_INLINE ConstPtr<JsonValue> GetNode() const
        {
            return mNode;
        }

        /// \brief Gets the untyped value for the specified key.
        ///
        /// \param Key The key to look up.
        /// \return The untyped \ref JsonValue, or nullptr if the key is not found.
        ZY_INLINE ConstPtr<JsonValue> GetValue(Text Key) const
        {
            return mNode->GetObject().Find(Key);
        }

        /// \brief Checks if the specified key is present.
        ///
        /// \param Key The key to look up.
        /// \return `true` if the key is present, `false` otherwise.
        ZY_INLINE Bool Contains(Text Key) const
        {
            return mNode->GetObject().Contains(Key);
        }

        /// \brief Sets a boolean value for the specified key.
        ///
        /// \param Key   The key to associate with the value.
        /// \param Value The boolean value to set.
        ZY_INLINE void SetBool(Text Key, Bool Value)
        {
            mNode->GetObject().Assign(Key, Value);
        }

        /// \brief Gets the boolean value for the specified key.
        ///
        /// \param Key     The key to look up.
        /// \param Default The value to return if the key is not found or the value is not a boolean.
        /// \return The boolean value, or the default if the key is not found or the value is not a boolean.
        ZY_INLINE Bool GetBool(Text Key, Bool Default = false) const
        {
            const ConstPtr<JsonValue> Value = mNode->GetObject().Find(Key);

            if (Value != nullptr && Value->IsBool())
            {
                return Value->GetBool();
            }
            return Default;
        }

        /// \brief Sets a numeric value for the specified key.
        ///
        /// \param Key   The key to associate with the value.
        /// \param Value The numeric value to set.
        template<IsNumeric Type>
        ZY_INLINE void SetNumber(Text Key, Type Value)
        {
            mNode->GetObject().Assign(Key, Value);
        }

        /// \brief Gets the numeric value for the specified key.
        ///
        /// \param Key     The key to look up.
        /// \param Default The value to return if the key is not found or the value is not a number.
        /// \return The numeric value converted to the specified type, or the default if the key is not found or the value is not a number.
        template<IsNumeric Type>
        ZY_INLINE Type GetNumber(Text Key, Type Default = { }) const
        {
            const ConstPtr<JsonValue> Value = mNode->GetObject().Find(Key);

            if (Value != nullptr && Value->IsNumber())
            {
                return Value->GetNumber<Type>();
            }
            return Default;
        }

        /// \brief Sets a string value for the specified key.
        ///
        /// \param Key   The key to associate with the value.
        /// \param Value The text value to set.
        ZY_INLINE void SetString(Text Key, Text Value)
        {
            mNode->GetObject().Assign(Key, Value);
        }

        /// \brief Gets the string value for the specified key.
        ///
        /// \param Key     The key to look up.
        /// \param Default The value to return if the key is not found or the value is not a string.
        /// \return The string value, or the default if the key is not found or the value is not a string.
        ZY_INLINE Text GetString(Text Key, Text Default = Text::Empty()) const
        {
            const ConstPtr<JsonValue> Value = mNode->GetObject().Find(Key);

            if (Value != nullptr && Value->IsString())
            {
                return Value->GetString();
            }
            return Default;
        }

        /// \brief Sets an enumeration value for the specified key as its name.
        ///
        /// \param Key   The key to associate with the value.
        /// \param Value The enumeration value to set.
        template<IsEnum Type>
        ZY_INLINE void SetEnum(Text Key, Type Value)
        {
            mNode->GetObject().Assign(Key, Enum::GetName(Value));
        }

        /// \brief Gets the enumeration value for the specified key.
        ///
        /// \param Key     The key to look up.
        /// \param Default The value to return if the key is not found or the value is not a valid enumeration name.
        /// \return The parsed enumeration value, or the default if the key is not found or the value is not a valid name.
        template<IsEnum Type>
        ZY_INLINE Type GetEnum(Text Key, Type Default = {}) const
        {
            const ConstPtr<JsonValue> Value = mNode->GetObject().Find(Key);

            if (Value != nullptr && Value->IsString())
            {
                return Enum::Cast<Type>(Value->GetString(), Default);
            }
            return Default;
        }

        /// \brief Sets an array value for the specified key and returns a wrapper for it.
        ///
        /// \param Key The key to associate with the array.
        /// \return A JSON array wrapper for the newly created element.
        JsonArray SetArray(Text Key);

        /// \brief Gets the array value for the specified key.
        ///
        /// \param Key The key to look up.
        /// \return A JSON array wrapper, or a null wrapper if the key is not found or the value is not an array.
        JsonArray GetArray(Text Key) const;

        /// \brief Sets an object value for the specified key and returns a wrapper for it.
        ///
        /// \param Key The key to associate with the object.
        /// \return A JSON object wrapper for the newly created element.
        ZY_INLINE JsonObject SetObject(Text Key)
        {
            Ref<JsonValue> Value = mNode->GetObject().FindOrInsert(Key);
            Value.SetObject();

            return JsonObject(Value);
        }

        /// \brief Gets the object value for the specified key.
        ///
        /// \param Key The key to look up.
        /// \return A JSON object wrapper, or a null wrapper if the key is not found or the value is not an object.
        ZY_INLINE JsonObject GetObject(Text Key) const
        {
            const Ptr<JsonValue> Value = mNode->GetObject().Find(Key);

            if (Value != nullptr && Value->IsObject())
            {
                return JsonObject(* Value);
            }
            return JsonObject();
        }

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