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

#include "Zyphryon.Base/Container/Table.hpp"
#include "Zyphryon.Base/Container/Sequence.hpp"
#include "Zyphryon.Base/Container/Variant.hpp"
#include "Zyphryon.Base/Lexical/String.hpp"
#include "Zyphryon.Base/Metadata/Enumerator.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A JSON value that can represent any JSON type.
    class JsonValue final
    {
    public:

        /// \brief A JSON array type alias.
        using Array  = Sequence<JsonValue>;

        /// \brief A JSON object type alias.
        using Object = Table<Str, JsonValue>;

    public:

        /// \brief Constructs a null JSON value.
        ZY_INLINE JsonValue() = default;

        /// \brief Constructs a JSON boolean value.
        ///
        /// \param Value The boolean value to store.
        ZY_INLINE JsonValue(Bool Value)
            : mData { Value }
        {
        }

        /// \brief Constructs a JSON number value from a numeric type.
        ///
        /// \param Value The numeric value to store.
        template<IsNumeric Type>
        ZY_INLINE JsonValue(Type Value)
            : mData { static_cast<Real64>(Value) }
        {
        }

        /// \brief Constructs a JSON string value.
        ///
        /// \param Value The text value to store.
        ZY_INLINE JsonValue(Text Value)
        {
            mData.Emplace<Str>(Value);
        }

        /// \brief Checks if the value is null.
        ///
        /// \return `true` if the value is null, `false` otherwise.
        ZY_INLINE Bool IsNull() const
        {
            return mData.IsEmpty();
        }

        /// \brief Checks if the value is a boolean.
        ///
        /// \return `true` if the value is a boolean, `false` otherwise.
        ZY_INLINE Bool IsBool() const
        {
            return mData.IsHolding<Bool>();
        }

        /// \brief Checks if the value is a number.
        ///
        /// \return `true` if the value is a number, `false` otherwise.
        ZY_INLINE Bool IsNumber() const
        {
            return mData.IsHolding<Real64>();
        }

        /// \brief Checks if the value is a string.
        ///
        /// \return `true` if the value is a string, `false` otherwise.
        ZY_INLINE Bool IsString() const
        {
            return mData.IsHolding<Str>();
        }

        /// \brief Checks if the value is an array.
        ///
        /// \return `true` if the value is an array, `false` otherwise.
        ZY_INLINE Bool IsArray() const
        {
            return mData.IsHolding<Array>();
        }

        /// \brief Checks if the value is an object.
        ///
        /// \return `true` if the value is an object, `false` otherwise.
        ZY_INLINE Bool IsObject() const
        {
            return mData.IsHolding<Object>();
        }

        /// \brief Gets the boolean value.
        ///
        /// \return The stored boolean value.
        ZY_INLINE Bool GetBool() const
        {
            return mData.Get<Bool>();
        }

        /// \brief Gets the number value as the specified numeric type.
        ///
        /// \tparam Type The numeric type to return.
        /// \return The stored number converted to the specified type.
        template<IsNumeric Type>
        ZY_INLINE Type GetNumber() const
        {
            return static_cast<Type>(mData.Get<Real64>());
        }

        /// \brief Gets the string value.
        ///
        /// \return The stored string value.
        ZY_INLINE Text GetString() const
        {
            return mData.Get<Str>();
        }

        /// \brief Gets the array value.
        ///
        /// \return A reference to the stored array.
        ZY_INLINE Ref<Array> GetArray()
        {
            return mData.Get<Array>();
        }

        /// \brief Gets the array value.
        ///
        /// \return A constant reference to the stored array.
        ZY_INLINE ConstRef<Array> GetArray() const
        {
            return mData.Get<Array>();
        }

        /// \brief Gets the object value.
        ///
        /// \return A reference to the stored object.
        ZY_INLINE Ref<Object> GetObject()
        {
            return mData.Get<Object>();
        }

        /// \brief Gets the object value.
        ///
        /// \return A constant reference to the stored object.
        ZY_INLINE ConstRef<Object> GetObject() const
        {
            return mData.Get<Object>();
        }

        /// \brief Sets the value to a boolean.
        ///
        /// \param Value The boolean value to set.
        ZY_INLINE void SetBoolean(Bool Value)
        {
            mData.Emplace<Bool>(Value);
        }

        /// \brief Sets the value to a number.
        ///
        /// \tparam Type The numeric type of the value.
        /// \param Value The numeric value to set.
        template<IsNumeric Type>
        ZY_INLINE void SetNumber(Type Value)
        {
            mData.Emplace<Real64>(static_cast<Real64>(Value));
        }

        /// \brief Sets the value to a string.
        ///
        /// \param Value The text value to set.
        ZY_INLINE void SetString(Text Value)
        {
            mData.Emplace<Str>(Value);
        }

        /// \brief Sets the value to an empty array and returns a reference to it.
        ///
        /// \return A reference to the newly created empty array.
        ZY_INLINE Ref<Array> SetArray()
        {
            return mData.Emplace<Array>();
        }

        /// \brief Sets the value to an empty object and returns a reference to it.
        ///
        /// \return A reference to the newly created empty object.
        ZY_INLINE Ref<Object> SetObject()
        {
            return mData.Emplace<Object>();
        }

        /// \brief Formats the JSON value into an output buffer.
        ///
        /// \param Buffer The buffer to format into.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            if (IsNull())
            {
                Buffer.Append("Null");
                return;
            }

            static constexpr Format::Pattern<5> kPattern("{0}");

            mData.Visit([&]<typename T0>(AnyRef<T0> Value)
            {
                Format::Processor<Output>::Format(Buffer, kPattern, Value);
            });
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Variant<Bool, Real64, Str, Array, Object> mData;
    };
}