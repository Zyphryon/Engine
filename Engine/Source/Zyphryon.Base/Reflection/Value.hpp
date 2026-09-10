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

#include "Zyphryon.Base/Container/Array.hpp"
#include "Zyphryon.Base/Container/Span.hpp"
#include "Zyphryon.Base/Pattern/Enumerator.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Reflection
{
    /// \brief Specifies what a field carries, spelled in the language rather than in any one module.
    enum class Kind : UInt8
    {
        None,        ///< The field carries nothing, which is what an unread value holds.
        Boolean,     ///< A yes or no, carried as a `Bool`.
        Integer,     ///< A whole number, carried as a `SInt64` whatever its own width.
        Real,        ///< A real number, carried as a `Real64` whatever its own width.
        Enumeration, ///< One of a fixed set of named values, carried as a `SInt64` beside its options.
        Text,        ///< A run of characters, borrowed from the instance it was read out of.
        Structure,   ///< A value with a schema of its own, reached in place by its address.
        Foreign,     ///< A value only its own module names, carried as its bytes under the tag of that module.
        List,        ///< A run of elements of one kind, reached in place by the address of the first.
        Opaque,      ///< A value nothing can carry, reached in place by its address under the tag it was given.
    };

    /// \brief Represents one named value an enumerated field chooses between.
    struct Option final
    {
        /// The name of the value, borrowed from the compile-time name pool of its own enum.
        Text   Name;

        /// The value itself, widened to the number every enumerated field travels as.
        SInt64 Content;
    };

    /// \brief Represents one value as it crosses between the type that owns it and the tool that edits it.
    class Value final
    {
    public:

        /// \brief The number of bytes a value carries inline.
        static constexpr UInt kCapacity = 16;

    public:

        /// \brief Constructs a value carrying nothing.
        ZY_INLINE Value()
            : mSize { 0 },
              mKind { Kind::None }
        {
        }

        /// \brief Checks whether the value carries nothing.
        ///
        /// \return `true` when the value carries nothing, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mKind == Kind::None;
        }

        /// \brief Gets the kind the value carries.
        ///
        /// \return The kind of the value.
        ZY_INLINE Kind GetKind() const
        {
            return mKind;
        }

        /// \brief Gets the number of bytes the value carries.
        ///
        /// \return The width of the payload, which is what the type it was written from measured.
        ZY_INLINE UInt GetSize() const
        {
            return mSize;
        }

        /// \brief Gets the value as the type it was written from.
        ///
        /// \tparam Type The type the value was written from, spelled by the caller because `Base` cannot name it.
        /// \return A copy of the value read as \p Type.
        template<typename Type>
        ZY_INLINE Type Get() const
            requires (IsTriviallyCopyable<Type> && IsDefaultConstructible<Type>)
        {
            ZY_ASSERT(sizeof(Type) == mSize, "Value read as a type of another width than it was written from");

            Type Result;
            Blit(AddressOf(Result), sizeof(Type), mData);
            return Result;
        }

        /// \brief Gets the value of a `Kind::Boolean`.
        ///
        /// \return The boolean the value carries.
        ZY_INLINE Bool GetBoolean() const
        {
            return Get<Bool>();
        }

        /// \brief Gets the value of a `Kind::Integer` or a `Kind::Enumeration`.
        ///
        /// \return The whole number the value carries.
        ZY_INLINE SInt64 GetInteger() const
        {
            return Get<SInt64>();
        }

        /// \brief Gets the value of a `Kind::Real`.
        ///
        /// \return The real number the value carries.
        ZY_INLINE Real64 GetReal() const
        {
            return Get<Real64>();
        }

        /// \brief Gets the value of a `Kind::Text`.
        ///
        /// \return The characters the value borrows from the instance it was read out of.
        ZY_INLINE Text GetText() const
        {
            return Get<Text>();
        }

        /// \brief Gets the value of a kind reached in place, which is a structure, a run or an opaque value.
        ///
        /// \return The address of the value inside the instance it was read out of.
        ZY_INLINE Ptr<void> GetAddress() const
        {
            return Get<Ptr<void>>();
        }

        /// \brief Gets the bytes of a `Kind::Foreign`, for whoever knows the type they stand for.
        ///
        /// \return The address of the payload, which lives inside the value itself.
        ZY_INLINE ConstPtr<Byte> GetBytes() const
        {
            return mData;
        }

        /// \brief Checks whether two values stand for the same thing.
        ///
        /// \param Other The value to compare to.
        /// \return `true` when both carry the same kind and the same content, otherwise `false`.
        ZY_INLINE Bool operator==(ConstRef<Value> Other) const
        {
            if (mKind != Other.mKind || mSize != Other.mSize)
            {
                return false;
            }

            switch (mKind)
            {
            case Kind::None:
                return true;
            case Kind::Boolean:
                return GetBoolean() == Other.GetBoolean();
            case Kind::Integer:
            case Kind::Enumeration:
                return GetInteger() == Other.GetInteger();
            case Kind::Real:
                return GetReal() == Other.GetReal();
            case Kind::Text:
                return GetText() == Other.GetText();
            case Kind::Structure:
            case Kind::Opaque:
            case Kind::List:
                return GetAddress() == Other.GetAddress();
            default:
                return Compare<Byte>(mData, Other.mData, mSize);
            }
        }

        /// \brief Checks whether two values stand for different things.
        ///
        /// \param Other The value to compare to.
        /// \return `true` when they differ in kind or in content, otherwise `false`.
        ZY_INLINE Bool operator!=(ConstRef<Value> Other) const
        {
            return !operator==(Other);
        }

    public:

        /// \brief Creates a value carrying a yes or no.
        ///
        /// \param Content The boolean to carry.
        /// \return A value of `Kind::Boolean`.
        ZY_INLINE static Value FromBoolean(Bool Content)
        {
            return Value(Kind::Boolean, Content);
        }

        /// \brief Creates a value carrying a whole number.
        ///
        /// \param Tag     The kind the number stands for, which is a plain number or an enumerator.
        /// \param Content The number to carry.
        /// \return A value of \p Tag.
        ZY_INLINE static Value FromInteger(Kind Tag, SInt64 Content)
        {
            ZY_ASSERT(Tag == Kind::Integer || Tag == Kind::Enumeration, "Kind is not a whole number");

            return Value(Tag, Content);
        }

        /// \brief Creates a value carrying a real number.
        ///
        /// \param Content The number to carry.
        /// \return A value of `Kind::Real`.
        ZY_INLINE static Value FromReal(Real64 Content)
        {
            return Value(Kind::Real, Content);
        }

        /// \brief Creates a value borrowing a run of characters.
        ///
        /// \param Content The characters to borrow, which must outlive the value.
        /// \return A value of `Kind::Text`.
        ZY_INLINE static Value FromText(Text Content)
        {
            return Value(Kind::Text, Content);
        }

        /// \brief Creates a value borrowing the address of a value edited where it lies.
        ///
        /// \param Tag     The kind the address stands for, described by fields of its own or only by a tag.
        /// \param Content The address of the value, which must outlive the value carrying it.
        /// \return A value of \p Tag.
        ZY_INLINE static Value FromAddress(Kind Tag, Ptr<void> Content)
        {
            ZY_ASSERT(Tag == Kind::Structure || Tag == Kind::Opaque || Tag == Kind::List,
                "Kind is not reached by address");

            return Value(Tag, Content);
        }

        /// \brief Creates a value carrying a type `Base` cannot name, as the bytes that type measures.
        ///
        /// \param  Content The value to carry.
        /// \return A value of `Kind::Foreign`.
        template<typename Type>
        ZY_INLINE static Value FromForeign(ConstRef<Type> Content)
        {
            return Value(Kind::Foreign, Content);
        }

    private:

        /// \brief Constructs a value by copying the bytes of the given content into the payload.
        ///
        /// \param Tag     The kind the bytes stand for.
        /// \param Content The value whose bytes are copied.
        template<typename Type>
        ZY_INLINE Value(Kind Tag, ConstRef<Type> Content)
            : mSize { sizeof(Type) },
              mKind { Tag }
        {
            static_assert(IsTriviallyCopyable<Type>, "A value carries bytes, so its type must be trivially copyable");
            static_assert(sizeof(Type) <= kCapacity,
                "A value wider than the payload belongs behind a schema of its own");

            Blit(mData, sizeof(Type), AddressOf(Content));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZY_ALIGN(8) Byte mData[kCapacity];
        UInt8            mSize;
        Kind             mKind;
    };
}