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

#include "Reader.hpp"
#include "Writer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Concept that defines the serialization interface for types.
    template<typename Type, typename Serializer>
    concept IsSerializable = requires(Ref<Type> Object, Ref<Serializer> Archive)
    {
        Object.Serialize(Archive);
    };

    /// \brief Concept that checks if a type is a collection.
    template<typename Type>
    concept IsContainer = IsContiguous<Type> && requires
    {
        typename Type::Element;
    };

    /// \brief Concept that checks if a type can be resized.
    template<typename Type>
    concept IsResizable = requires(Ref<Type> Object, UInt Size)
    {
        Object.Advance(Size);
    };

    /// \brief A unified reader/writer for serializing values to and from a binary archive.
    ///
    /// \tparam Object The underlying archive type, either \c Reader or \c Writer.
    template<typename Object>
    class Archive
    {
    public:

        /// \brief `true` if this archive reads values from a buffer, `false` otherwise.
        static constexpr Bool IsReader = IsAnyOf<Object, Reader>;

        /// \brief `true` if this archive writes values to a buffer, `false` otherwise.
        static constexpr Bool IsWriter = IsAnyOf<Object, Writer>;

    public:

        /// \brief Constructs an archive wrapping the given reader or writer.
        ///
        /// \param Archive The underlying reader or writer to delegate to.
        ZY_INLINE Archive(Ref<Object> Archive)
            : mArchive{Archive}
        {
        }

        /// \brief Serializes a boolean value.
        ///
        /// \param Value The value to read into or write from.
        ZY_INLINE void Serialize(Text, Ref<Bool> Value)
        {
            if constexpr (IsReader)
            {
                Value = mArchive.template Read<Bool>();
            }
            else
            {
                mArchive.template Write<Bool>(Value);
            }
        }

        /// \brief Serializes an enumeration value via its underlying integral type.
        ///
        /// \param Value The value to read into or write from.
        template<typename Type>
        ZY_INLINE void Serialize(Text, Ref<Type> Value)
            requires IsEnum<Type>
        {
            using Underlying = Underlying<Type>;

            Underlying Number = static_cast<Underlying>(Value);
            Serialize(Text::Empty(), Number);

            if constexpr (IsReader)
            {
                Value = static_cast<Type>(Number);
            }
        }

        /// \brief Serializes an integral or floating-point value.
        ///
        /// \param Value The value to read into or write from.
        template<typename Type>
        ZY_INLINE void Serialize(Text, Ref<Type> Value)
            requires IsNumeric<Type>
        {
            if constexpr (IsReader)
            {
                Value = mArchive.template Read<Type>();
            }
            else
            {
                mArchive.template Write<Type>(Value);
            }
        }

        /// \brief Serializes an object value by copying its raw bytes.
        ///
        /// \param Value The value to read into or write from.
        template<typename Type>
        ZY_INLINE void Serialize(Text, Ref<Type> Value)
            requires (!IsEnum<Type> && !IsNumeric<Type> && !IsContainer<Type>)
        {
            if      constexpr (IsSerializable<Type, Archive>)
            {
                Value.Serialize(* this);
            }
            else if constexpr (IsTriviallyCopyable<Type>)
            {
                SerializeBlock(AddressOf(Value), 1);
            }
            else
            {
                static_assert(false, "Type must implement Serializable interface or be trivially copyable");
            }
        }

        /// \brief Serializes a contiguous container by writing its size followed by its elements.
        ///
        /// \param Value The container to read into or write from.
        template<typename Type>
        ZY_INLINE void Serialize(Text, Ref<Type> Value)
            requires IsContiguousOf<Type, typename Type::Element>
        {
            if constexpr (IsResizable<Type>)
            {
                if constexpr (IsReader)
                {
                    const UInt32 Size = mArchive.template ReadUInt<UInt32>();
                    Value.Advance(Size);
                }
                else
                {
                    mArchive.template WriteUInt<UInt32>(Value.GetSize());
                }
            }

            if constexpr (IsTriviallyCopyable<typename Type::Element>)
            {
                SerializePtr(Value.GetData(), Value.GetSize());
            }
            else
            {
                for (Ref<typename Type::Element> Element: Value)
                {
                    Serialize(Text::Empty(), Element);
                }
            }
        }

        /// \brief Serializes a text string by reading or writing its contents.
        ///
        /// \param Value The string to read into or write from.
        template<typename Type>
        ZY_INLINE void Serialize(Text, Ref<Type> Value)
            requires IsContiguousOf<Type, Char>
        {
            if constexpr (IsReader)
            {
                Value = mArchive.ReadText();
            }
            else
            {
                mArchive.WriteText(Value);
            }
        }

    private:

        /// \brief Serializes a contiguous block of trivially copyable elements as raw bytes.
        ///
        /// \param Data  The pointer to the first element.
        /// \param Count The number of elements.
        template<typename Type>
        ZY_INLINE void SerializeBlock(Ptr<Type> Data, UInt Count)
        {
            const UInt Size = Count * sizeof(Type);

            if constexpr (IsReader)
            {
                const ConstPtr<Type> Source = mArchive.template Read<ConstPtr<Type> >(Size);
                Blit(Data, Size, Source);
            }
            else
            {
                mArchive.Write(Data, Size);
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Object> mArchive;
    };
}