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

#include "Access.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Reflection
{
    /// \brief Represents one editable value of a type: what it is called, what it carries, and how it is reached.
    class Field final
    {
    public:

        /// \brief Signature of the handler that reads the field out of an instance.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \return The value read, or the address of it when the field is reached in place.
        using Reader = Value (*)(Ptr<void> Instance);

        /// \brief Signature of the handler that writes the field back into an instance.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \param Input    The value to write.
        using Writer = void (*)(Ptr<void> Instance, ConstRef<Value> Input);

        /// \brief Specifies what may be done with a field besides reading it.
        enum class Trait : UInt8
        {
            Frozen    = 0b00000001, ///< The field is shown but never written, though it could be.
            Notifying = 0b00000010, ///< The owner is told after a write, so whatever watches it runs.
            Mask      = 0b00000100, ///< The whole number stands for a set of bits rather than a quantity.
            Tagged    = 0b00001000, ///< The value named itself, so the name sits where the bounds would.
        };
        ZY_DEFINE_BITWISE_FRIEND_ENUM(Trait)

    public:

        /// \brief Constructs a field describing nothing.
        ZY_INLINE constexpr Field()
            : mName       { },
              mHint       { },
              mReader     { nullptr },
              mWriter     { nullptr },
              mExtra      { .Limit = { } },
              mKind       { Kind::None },
              mTraits     { },
              mOwnerSize  { 0 },
              mOwnerAlign { 0 }
        {
        }

        /// \brief Gets the name the field is shown and looked up under.
        ///
        /// \return The name of the field.
        ZY_INLINE constexpr Text GetName() const
        {
            return mName;
        }

        /// \brief Gets the sentence shown beside the field.
        ///
        /// \return The hint of the field, empty when it has none.
        ZY_INLINE constexpr Text GetHint() const
        {
            return mHint;
        }

        /// \brief Gets the kind the field carries.
        ///
        /// \return The kind of the field.
        ZY_INLINE constexpr Kind GetKind() const
        {
            return mKind;
        }

        /// \brief Gets the tag naming who understands a value this module cannot describe.
        ///
        /// \note A run of characters carries one too, which is how a path keeps the name of what writes it
        ///       while still reading and writing as plain text.
        ///
        /// \return The tag of the field, empty unless the value was given one.
        ZY_INLINE constexpr Text GetTag() const
        {
            return HasTrait(Trait::Tagged) || mKind == Kind::Opaque ? mExtra.Label : Text::Empty();
        }

        /// \brief Gets the lowest value a number takes.
        ///
        /// \return The minimum, which equals the maximum when the field is unbounded.
        ZY_INLINE constexpr Real32 GetMinimum() const
        {
            return HasLimit() ? mExtra.Limit.Minimum : 0.0f;
        }

        /// \brief Gets the highest value a number takes.
        ///
        /// \return The maximum, which equals the minimum when the field is unbounded.
        ZY_INLINE constexpr Real32 GetMaximum() const
        {
            return HasLimit() ? mExtra.Limit.Maximum : 0.0f;
        }

        /// \brief Gets how far a number moves at a time.
        ///
        /// \return The step, or zero to leave the pace to whoever draws it.
        ZY_INLINE constexpr Real32 GetStep() const
        {
            return HasLimit() ? mExtra.Limit.Step : 0.0f;
        }

        /// \brief Gets the named values an enumerated field chooses between.
        ///
        /// \return The options of the field, empty unless it is a `Kind::Enumeration`.
        ZY_INLINE constexpr Span<const Option> GetOptions() const
        {
            return mKind == Kind::Enumeration ? mExtra.Options : Span<const Option>();
        }

        /// \brief Gets the fields of the value a nested field steps into.
        ///
        /// \return The fields of the value, empty unless the field is a `Kind::Structure`.
        ZY_INLINE constexpr Span<const Field> GetNested() const
        {
            return mKind == Kind::Structure ? mExtra.Nested : Span<const Field>();
        }

        /// \brief Gets what a run of elements holds.
        ///
        /// \return The listing of the run, or `nullptr` unless the field is a `Kind::List`.
        ZY_INLINE constexpr ConstPtr<Listing> GetListing() const
        {
            return mKind == Kind::List ? mExtra.Elements : nullptr;
        }

        /// \brief Gets the address of one element of a run.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \param Index    The position of the element.
        /// \return The address of the element, or `nullptr` when the field holds no run or the position is past it.
        ZY_INLINE Ptr<void> At(Ptr<void> Instance, UInt Index) const
        {
            const ConstPtr<Listing> Elements = GetListing();

            if (!Elements || Index >= Elements->Count)
            {
                return nullptr;
            }
            return static_cast<Ptr<Byte>>(Read(Instance).GetAddress()) + Index * Elements->Stride;
        }

        /// \brief Checks whether the field is reached through the specified type.
        ///
        /// \tparam Type The type the field is expected to reach through.
        /// \return `true` when the field reaches through that type, otherwise `false`.
        template<typename Type>
        ZY_INLINE constexpr Bool Owns() const
        {
            return sizeof(Type) == mOwnerSize && alignof(Type) == mOwnerAlign;
        }

        /// \brief Checks whether the field carries the specified trait.
        ///
        /// \param Mask The trait to look for.
        /// \return `true` when the field carries it, otherwise `false`.
        ZY_INLINE constexpr Bool HasTrait(Trait Mask) const
        {
            return (mTraits & Mask) == Mask;
        }

        /// \brief Checks whether the field was declared with no way to write it at all.
        ///
        /// \return `true` when the field has no writer, otherwise `false`.
        ZY_INLINE constexpr Bool IsReadOnly() const
        {
            return mWriter == nullptr;
        }

        /// \brief Checks whether the field could be written but must not be.
        ///
        /// \return `true` when the field has a writer and is frozen, otherwise `false`.
        ZY_INLINE constexpr Bool IsFrozen() const
        {
            return mWriter && HasTrait(Trait::Frozen);
        }

        /// \brief Checks whether the field may be written.
        ///
        /// \return `true` when the field has a writer and is not frozen, otherwise `false`.
        ZY_INLINE constexpr Bool IsWritable() const
        {
            return mWriter && !HasTrait(Trait::Frozen);
        }

        /// \brief Reads the field out of an instance.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \return The value read, which for a nested field is the address of it inside the instance.
        ZY_INLINE Value Read(Ptr<void> Instance) const
        {
            ZY_ASSERT(Instance, "A field is read out of an instance, never out of nothing");

            return mReader ? mReader(Instance) : Value();
        }

        /// \brief Writes the field back into an instance.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \param Input    The value to write.
        /// \return `true` when the field was written, `false` when it is read-only or frozen.
        ZY_INLINE Bool Write(Ptr<void> Instance, ConstRef<Value> Input) const
        {
            ZY_ASSERT(Instance, "A field is written into an instance, never into nothing");
            ZY_ASSERT(Input.GetKind() == mKind, "Value written into a field of another kind");

            if (!IsWritable())
            {
                return false;
            }
            mWriter(Instance, Input);
            return true;
        }

        /// \brief Bounds a number.
        ///
        /// \param Minimum The lowest value the field takes.
        /// \param Maximum The highest value the field takes.
        /// \return A copy of the field, bounded.
        ZY_INLINE constexpr Field Between(Real32 Minimum, Real32 Maximum) const
        {
            ZY_ASSERT(HasLimit(), "Only a number is bounded");

            Field Result = * this;
            Result.mExtra.Limit.Minimum = Minimum;
            Result.mExtra.Limit.Maximum = Maximum;
            return Result;
        }

        /// \brief Sets how far a number moves at a time.
        ///
        /// \param Step The distance one move covers.
        /// \return A copy of the field, stepped.
        ZY_INLINE constexpr Field Stepped(Real32 Step) const
        {
            ZY_ASSERT(HasLimit(), "Only a number is stepped");

            Field Result = * this;
            Result.mExtra.Limit.Step = Step;
            return Result;
        }

        /// \brief Sets the sentence shown beside the field.
        ///
        /// \param Hint The sentence to show, taken as an array so only a lasting one can be given.
        /// \return A copy of the field, hinted.
        template<UInt Count>
        ZY_INLINE constexpr Field Hinted(const Char (& Hint)[Count]) const
        {
            Field Result = * this;
            Result.mHint = Text(Hint, Count - 1);
            return Result;
        }

        /// \brief States that a whole number stands for a set of bits rather than a quantity.
        ///
        /// \return A copy of the field, masked.
        ZY_INLINE constexpr Field Masked() const
        {
            ZY_ASSERT(mKind == Kind::Integer, "Only a whole number stands for a set of bits");

            Field Result = * this;
            Result.mTraits |= Trait::Mask;
            return Result;
        }

        /// \brief Asks that the owner be told after the field is written.
        ///
        /// \return A copy of the field, notifying.
        ZY_INLINE constexpr Field Notifying() const
        {
            Field Result = * this;
            Result.mTraits |= Trait::Notifying;
            return Result;
        }

        /// \brief Forbids writing the field, though it could be written.
        ///
        /// \return A copy of the field, frozen.
        ZY_INLINE constexpr Field Frozen() const
        {
            Field Result = * this;
            Result.mTraits |= Trait::Frozen;
            return Result;
        }

    public:

        /// \brief Describes a value reached by a getter, or by a data member that is its own accessor.
        ///
        /// \tparam Getter The data member, or the getter, the value is read through.
        /// \tparam Setter The setter the value is written through, left out for a data member or a read-only value.
        /// \param  Name   The name the value is shown under, taken as an array so only a lasting one can be given.
        /// \return A field describing that value.
        template<auto Getter, auto Setter = nullptr, UInt Count>
        ZY_INLINE static constexpr Field Property(const Char (& Name)[Count])
        {
            using Access = Detail::Accessor<decltype(Getter)>;

            constexpr Bool Flat = IsFlat<typename Access::Value>;
            constexpr Kind Tag  = OnTag<typename Access::Value>();

            static_assert(Tag != Kind::None,
                "The value has no kind: specialize Reflection::Classify for its type, or Reflection::Describe it");
            static_assert(Flat || Tag != Kind::Structure,
                "A described value is declared with Field::Nested, so a walk can step into its fields");
            static_assert(!Access::kIsData || !IsAnyOf<typename Access::Value, Text>,
                "A member that is a view owns none of its characters: hold them, and hand back a view from a getter");

            if constexpr (Flat)
            {
                Detail::Borrow<Getter, Kind::Structure>();
            }
            else
            {
                Detail::Borrow<Getter, Tag>();
            }

            return Compose<Getter, Setter, Tag, Flat>(
                Text(Name, Count - 1), OnExtra<typename Access::Value, Tag>());
        }

        /// \brief Describes a value that has fields of its own, which a walk steps into.
        ///
        /// \tparam Getter The data member, or the getter, the value is reached through.
        /// \tparam Setter The setter run over the value once the walk has edited it, left out for a data member.
        /// \param  Name   The name the value is shown under, taken as an array so only a lasting one can be given.
        /// \return A field describing that value.
        template<auto Getter, auto Setter = nullptr, UInt Count>
        ZY_INLINE static constexpr Field Nested(const Char (& Name)[Count])
        {
            using Access  = Detail::Accessor<decltype(Getter)>;
            using Content = typename Access::Value;

            static_assert(IsDescribed<Content>, "A nested value needs a Reflection::Describe of its own");
            static_assert(Detail::Matches<Content>(), "The fields of the nested value belong to another type");

            Detail::Borrow<Getter, Kind::Structure>();

            const Extra Slot { .Nested = Span<const Field>(Describe<Content>::kFields) };

            return Compose<Getter, Setter, Kind::Structure>(Text(Name, Count - 1), Slot);
        }

        /// \brief Describes a value nothing can express as fields, handed on under a tag instead.
        ///
        /// \tparam Getter The data member, or the getter, the value is reached through.
        /// \tparam Setter The setter run over the value once it has been edited, left out for a data member.
        /// \param  Name   The name the value is shown under, taken as an array so only a lasting one can be given.
        /// \param  Tag    The tag naming who understands the value, taken the same way and for the same reason.
        /// \return A field describing that value.
        template<auto Getter, auto Setter = nullptr, UInt Count, UInt Length>
        ZY_INLINE static constexpr Field Custom(const Char (& Name)[Count], const Char (& Tag)[Length])
        {
            Detail::Borrow<Getter, Kind::Opaque>();

            const Extra Slot { .Label = Text(Tag, Length - 1) };

            return Compose<Getter, Setter, Kind::Opaque>(Text(Name, Count - 1), Slot);
        }

        /// \brief Describes a run of elements of one kind, which is walked by position rather than by name.
        ///
        /// \tparam Getter The data member, or the getter, the run is reached through.
        /// \param  Name   The name the run is shown under, taken as an array so only a lasting one can be given.
        /// \return A field describing that run.
        template<auto Getter, UInt Count>
        ZY_INLINE static constexpr Field List(const Char (& Name)[Count])
        {
            using Content = typename Detail::Bare<typename Detail::Accessor<decltype(Getter)>::Result>::Result;

            static_assert(Detail::IsSequenced<Content>, "A list field needs a run of elements, such as an array");

            Detail::Borrow<Getter, Kind::List>();

            const Extra Slot { .Elements = AddressOf(Detail::kListing<Content>) };

            return Compose<Getter, nullptr, Kind::List>(Text(Name, Count - 1), Slot);
        }

    private:

        /// \brief Holds whatever the kind of the field needs, which is never more than one of these at a time.
        union Extra
        {
            /// The bounds and step a number is dragged within, all zero when it is unbounded.
            struct
            {
                Real32 Minimum;
                Real32 Maximum;
                Real32 Step;
            }                  Limit;

            /// The named values an enumerated field chooses between.
            Span<const Option> Options;

            /// The fields of the value a nested field steps into.
            Span<const Field>  Nested;

            /// The tag naming who understands a value this module cannot describe.
            Text               Label;

            /// What a run of elements holds, laid down once per type it is read out of.
            ConstPtr<Listing>  Elements;
        };

        /// \brief Constructs a field from its name, its handlers and whatever its kind needs.
        ///
        /// \param Tag   The kind the field carries.
        /// \param Name  The name of the field, which outlives it.
        /// \param Read  The handler that reads the field out of an instance.
        /// \param Write The handler that writes the field back, or `nullptr` when the field is read-only.
        /// \param Slot   The bounds, options, fields or tag the kind needs.
        /// \param Traits What may be done with the field beside reading it.
        /// \param Size  The width of the type the field is reached through.
        /// \param Align The alignment of the type the field is reached through.
        ZY_INLINE constexpr Field(
            Kind Tag, Text Name, Reader Read, Writer Write, Extra Slot, Trait Traits, UInt Size, UInt Align)
            : mName       { Name },
              mHint       { },
              mReader     { Read },
              mWriter     { Write },
              mExtra      { Slot },
              mKind       { Tag },
              mTraits     { Traits },
              mOwnerSize  { static_cast<UInt16>(Size) },
              mOwnerAlign { static_cast<UInt8>(Align) }
        {
            ZY_ASSERT(Size <= 65535 && Align <= 255, "A described type is at most 65535 bytes wide");
        }

        /// \brief Checks whether the kind of the field is one that keeps bounds.
        ///
        /// \return `true` when the bounds are the live member of the union, otherwise `false`.
        ZY_INLINE constexpr Bool HasLimit() const
        {
            return (mKind == Kind::Integer || mKind == Kind::Real) && !HasTrait(Trait::Tagged);
        }

        /// \brief Gets the kind a value is edited as, which for a flat wrapper is the kind of its one part.
        ///
        /// \note Asking for both and choosing between them would demand a layout of every value that has none,
        ///       so the question is asked once, of the shape the value actually has.
        ///
        /// \return The kind the value is edited as.
        template<typename Content>
        ZY_INLINE static constexpr Kind OnTag()
        {
            if constexpr (IsFlat<Content>)
            {
                static_assert(Describe<Content>::kFields.GetSize() == 1,
                    "A value shown in place of the one part it holds must hold exactly one");

                return Describe<Content>::kFields[0].GetKind();
            }
            else
            {
                return Classify<Content>::kValue;
            }
        }

        /// \brief Builds whatever the kind needs beside its handlers.
        ///
        /// \return The options of an enumerated value, the tag of a foreign one, or an unbounded limit.
        template<typename Content, Kind Tag>
        ZY_INLINE static constexpr Extra OnExtra()
        {
            if constexpr (Tag == Kind::Enumeration)
            {
                return Extra { .Options = Span<const Option>(Detail::kOptions<Content>) };
            }
            else if constexpr (Detail::IsNamed<Content>)
            {
                return Extra { .Label = Detail::TagOf<Content>() };
            }
            else
            {
                return Extra { .Limit = { } };
            }
        }

        /// \brief Picks the read handler the kind wants, since only one of the two may be instantiated.
        ///
        /// \return The locator for a value reached in place, or the plain reader for one carried by value.
        template<auto Getter, Kind Tag, Bool Flat = false>
        ZY_INLINE static constexpr Reader OnReader()
        {
            if constexpr (Flat)
            {
                return OnReadFlat<Getter>;
            }
            else if constexpr (Tag == Kind::Structure || Tag == Kind::Opaque || Tag == Kind::List)
            {
                return OnLocate<Getter, Tag>;
            }
            else
            {
                return OnRead<Getter, Tag>;
            }
        }

        /// \brief Picks the write handler the accessor can serve, since only one of the two may be instantiated.
        ///
        /// \return The writer for the accessor, or `nullptr` when there is no way to write through it.
        template<auto Handle, Kind Tag, Bool IsSetter, Bool Flat = false>
        ZY_INLINE static constexpr Writer OnWriter()
        {
            if constexpr (Flat)
            {
                return OnWriteFlat<Handle>;
            }
            else if constexpr (Tag == Kind::List)
            {
                return nullptr;
            }
            else if constexpr (IsSetter || Detail::Accessor<decltype(Handle)>::kIsData)
            {
                return OnWrite<Handle, Tag>;
            }
            else
            {
                return nullptr;
            }
        }

        /// \brief Builds a field, generating both handlers from whichever accessors were given.
        ///
        /// \param Name The name of the field, which outlives it.
        /// \param Slot The bounds, options, fields or tag the kind needs.
        /// \return A field describing the value the accessors reach.
        template<auto Getter, auto Setter, Kind Tag, Bool Flat = false>
        ZY_INLINE static constexpr Field Compose(Text Name, Extra Slot)
        {
            using Access = Detail::Accessor<decltype(Getter)>;
            using Owner  = typename Access::Owner;

            constexpr Reader Read   = OnReader<Getter, Tag, Flat>();
            constexpr Trait  Marked = Detail::IsNamed<typename Access::Value> ? Trait::Tagged : Trait { };

            if constexpr (IsAnyOf<decltype(Setter), Null>)
            {
                return Field(
                    Tag, Name, Read, OnWriter<Getter, Tag, false, Flat>(), Slot, Marked, sizeof(Owner), alignof(Owner));
            }
            else
            {
                return Field(
                    Tag, Name, Read, OnWriter<Setter, Tag, true, Flat>(), Slot, Marked, sizeof(Owner), alignof(Owner));
            }
        }

        /// \brief Read handler generated for an accessor by the field factories.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \return The value read.
        template<auto Handle, Kind Tag>
        ZY_INLINE static Value OnRead(Ptr<void> Instance)
        {
            using Access = Detail::Accessor<decltype(Handle)>;

            const Ptr<typename Access::Owner> Object = static_cast<Ptr<typename Access::Owner>>(Instance);

            if constexpr (Access::kIsData)
            {
                return Detail::Encode<Tag>(Object->*Handle);
            }
            else
            {
                return Detail::Encode<Tag>((Object->*Handle)());
            }
        }

        /// \brief Write handler generated for an accessor by the field factories.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \param Input    The value to write.
        template<auto Handle, Kind Tag>
        ZY_INLINE static void OnWrite(Ptr<void> Instance, ConstRef<Value> Input)
        {
            using Access = Detail::Accessor<decltype(Handle)>;

            const Ptr<typename Access::Owner> Object = static_cast<Ptr<typename Access::Owner>>(Instance);

            if constexpr (Access::kIsData)
            {
                Object->*Handle = Detail::Decode<Tag, typename Access::Value>(Input);
            }
            else
            {
                (Object->*Handle)(Detail::Decode<Tag, typename Access::Value>(Input));
            }
        }

        /// \brief Finds where a value lies inside an instance, whether it is held or handed back by reference.
        ///
        /// \note The instance is mutable, so the constness of a getter is its word about the value, not the value's.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \return The address of the value inside the instance.
        template<auto Handle>
        ZY_INLINE static Ptr<void> OnAddress(Ptr<void> Instance)
        {
            using Access = Detail::Accessor<decltype(Handle)>;

            const Ptr<typename Access::Owner> Object = static_cast<Ptr<typename Access::Owner>>(Instance);

            if constexpr (Access::kIsData)
            {
                return AddressOf(Object->*Handle);
            }
            else
            {
                return const_cast<Ptr<typename Access::Value>>(AddressOf((Object->*Handle)()));
            }
        }

        /// \brief Read handler for a value shown in place of the one part it holds.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \return The value that part reads as.
        template<auto Handle>
        ZY_INLINE static Value OnReadFlat(Ptr<void> Instance)
        {
            using Content = typename Detail::Accessor<decltype(Handle)>::Value;

            return Describe<Content>::kFields[0].Read(OnAddress<Handle>(Instance));
        }

        /// \brief Write handler for a value shown in place of the one part it holds.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \param Input    The value to write into that part.
        template<auto Handle>
        ZY_INLINE static void OnWriteFlat(Ptr<void> Instance, ConstRef<Value> Input)
        {
            using Content = typename Detail::Accessor<decltype(Handle)>::Value;

            Describe<Content>::kFields[0].Write(OnAddress<Handle>(Instance), Input);
        }

        /// \brief Read handler generated for a value reached in place, which hands back where it lies.
        ///
        /// \param Instance The raw pointer to the instance the field belongs to.
        /// \return The address of the value inside the instance.
        template<auto Handle, Kind Tag>
        ZY_INLINE static Value OnLocate(Ptr<void> Instance)
        {
            return Value::FromAddress(Tag, OnAddress<Handle>(Instance));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Text   mName;
        Text   mHint;
        Reader mReader;
        Writer mWriter;
        Extra  mExtra;
        Kind   mKind;
        Trait  mTraits;
        UInt16 mOwnerSize;
        UInt8  mOwnerAlign;
    };
}