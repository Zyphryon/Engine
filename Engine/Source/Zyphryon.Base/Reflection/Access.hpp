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

#include "Value.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReflection
{
    class Field;

    /// \brief Describes how a type presents itself to reflection: the name it answers to, and how it is shown.
    struct Presentation final
    {
        /// The name the type is registered under.
        Text Name;

        /// `true` when the type stands in for the one part it holds, rather than being stepped into.
        Bool Flat = false;
    };

    /// \brief Concept satisfied when the given type has been self-described.
    template<typename Type>
    concept IsSelfDescribed = requires { Type::OnDescribe(); };

    /// \brief Concept satisfied when the given type has named itself to reflection.
    template<typename Type>
    concept IsSelfClassified = requires { Type::OnClassify(); };

    /// \brief Provides the customization point that lists the fields of a type.
    template<typename Type>
    struct Describe
    {
    };

    /// \brief Provides the fields a type laid out for itself in `OnDescribe`.
    template<IsSelfDescribed Type>
    struct Describe<Type>
    {
        /// The fields of the type, in the order it declared them.
        static constexpr auto kFields = Type::OnDescribe();
    };

    /// \brief Concept satisfied when a type asks to be shown as the one part it holds.
    template<typename Type>
    concept IsFlat = IsSelfClassified<Type> && Type::OnClassify().Flat;

    /// \brief Concept satisfied when the given type has been described.
    template<typename Type>
    concept IsDescribed = requires { Describe<Type>::kFields; };

    /// \brief Provides the customization point that names the kind a type is edited as.
    ///
    /// \code
    /// template<>
    /// struct ZyReflection::Classify<Timestamp>
    /// {
    ///     static constexpr ZyReflection::Kind kValue = ZyReflection::Kind::Foreign;
    ///     static constexpr Char               kTag[] = "Chrono.Timestamp";
    /// };
    /// \endcode
    template<typename Type>
    struct Classify
    {
        /// The kind the type is edited as, or `Kind::None` when nothing knows how to edit it.
        static constexpr Kind kValue =
              IsAnyOf<Type, Bool>        ? Kind::Boolean
            : IsEnum<Type>               ? Kind::Enumeration
            : IsIntegral<Type>           ? Kind::Integer
            : IsReal<Type>               ? Kind::Real
            : IsContiguousOf<Type, Char> ? Kind::Text
            : IsDescribed<Type>          ? Kind::Structure
            : Kind::None;
    };

    /// \brief Provides the tag a type named itself with in `OnClassify`.
    template<IsSelfClassified Type>
    struct Classify<Type>
    {
        /// The kind the type is edited as, which is its parts when it has any and its own bytes otherwise.
        static constexpr Kind kValue = IsDescribed<Type> ? Kind::Structure : Kind::Foreign;

        /// The tag whoever edits the value matches on.
        static constexpr Text kTag   = Type::OnClassify().Name;
    };

    /// \brief Provides the kind of a view of characters, which is what a getter over owned storage hands back.
    template<>
    struct Classify<Text>
    {
        /// The kind a view of characters is edited as.
        static constexpr Kind kValue = Kind::Text;
    };

    /// \brief Describes what a run of elements holds, laid down once per type of run and never moved.
    struct Listing final
    {
        /// \brief Signature of the handler that counts the elements a run holds now.
        using Counter = UInt (*)(ConstPtr<void> Run);

        /// \brief Signature of the handler that finds where the first element of a run lies.
        using Finder  = Ptr<void> (*)(Ptr<void> Run);

        /// \brief Signature of the handler that makes an element at a position and hands back its address.
        using Maker   = Ptr<void> (*)(Ptr<void> Run, UInt Index);

        /// \brief Signature of the handler that takes the element at a position out of a run.
        using Taker   = void (*)(Ptr<void> Run, UInt Index);

        /// The fields of one element, empty unless the elements are described.
        ConstSpan<Field> Fields;

        /// The most elements the run holds, its length when fixed and zero when it grows without bound.
        UInt32           Count;

        /// The distance in bytes from one element to the next.
        UInt32           Stride;

        /// The kind one element carries.
        Kind             Content;

        /// The handler counting the elements the run holds now.
        Counter          Size;

        /// The handler finding the first element, from which the rest lie a stride apart.
        Finder           Data;

        /// The handler making an element at a position, or `nullptr` when the run never grows.
        Maker            Insert;

        /// The handler taking the element at a position out, or `nullptr` when the run never shrinks.
        Taker            Remove;
    };

    namespace Detail
    {
        /// \brief Describes a member pointer into the type it reaches through and the value it carries.
        template<typename Type>
        struct Accessor;

        /// \brief Holds the members every accessor shares.
        template<typename Class, typename Type, Bool Data>
        struct Member
        {
            /// `true` when the member is data, so reaching it needs no call.
            static constexpr Bool kIsData = Data;

            /// The type the member is reached through.
            using Owner  = Class;

            /// The type as the member spells it, references and qualifiers and all.
            using Result = Type;

            /// The type with its reference and qualifiers taken off.
            using Value  = StripAll<Type>;
        };

        /// \brief Describes a pointer to a data member, which is its own getter and setter.
        template<typename Class, typename Type>
        struct Accessor<Type Class::*> final : Member<Class, Type, true>
        {
        };

        /// \brief Describes a pointer to a constant getter.
        template<typename Class, typename Type>
        struct Accessor<Type (Class::*)() const> final : Member<Class, Type, false>
        {
        };

        /// \brief Describes a pointer to a constant getter that promises not to throw.
        template<typename Class, typename Type>
        struct Accessor<Type (Class::*)() const noexcept> final : Member<Class, Type, false>
        {
        };

        /// \brief Describes a pointer to a mutable getter.
        template<typename Class, typename Type>
        struct Accessor<Type (Class::*)()> final : Member<Class, Type, false>
        {
        };

        /// \brief Describes a pointer to a mutable getter that promises not to throw.
        template<typename Class, typename Type>
        struct Accessor<Type (Class::*)() noexcept> final : Member<Class, Type, false>
        {
        };

        /// \brief Describes a pointer to a setter, whatever it hands back.
        template<typename Class, typename Return, typename Type>
        struct Accessor<Return (Class::*)(Type)> final : Member<Class, Type, false>
        {
        };

        /// \brief Describes a pointer to a setter that promises not to throw.
        template<typename Class, typename Return, typename Type>
        struct Accessor<Return (Class::*)(Type) noexcept> final : Member<Class, Type, false>
        {
        };

        /// \brief Provides a type with its reference and constness taken off, and its extent kept.
        template<typename Type>
        struct Bare final
        {
            /// The type as it stands.
            using Result = Type;
        };

        /// \brief Provides the type with its constness taken off.
        template<typename Type>
        struct Bare<const Type> final
        {
            using Result = typename Bare<Type>::Result;
        };

        /// \brief Provides the type with its reference taken off.
        template<typename Type>
        struct Bare<Type &> final
        {
            using Result = typename Bare<Type>::Result;
        };

        /// \brief Describes a run of elements into what it holds and how many of them there are.
        template<typename Type>
        struct Sequenced;

        /// \brief Holds what every run of a fixed length shares, whose first element lies where the run does.
        template<UInt Count>
        struct Fixed
        {
            /// The number of elements.
            static constexpr UInt kCount = Count;

            /// The handler making an element, which is none since the run never grows.
            static constexpr Listing::Maker Insert = nullptr;

            /// The handler taking an element out, which is none since the run never shrinks.
            static constexpr Listing::Taker Remove = nullptr;

            /// \brief Counts the elements of the run, which never change.
            ///
            /// \return The length of the run.
            ZY_INLINE static UInt Size(ConstPtr<void>)
            {
                return Count;
            }

            /// \brief Finds the first element of the run.
            ///
            /// \param Run The address of the run.
            /// \return The address of the run itself.
            ZY_INLINE static Ptr<void> Data(Ptr<void> Run)
            {
                return Run;
            }
        };

        /// \brief Describes a plain array.
        template<typename Type, UInt Count>
        struct Sequenced<Type[Count]> final : Fixed<Count>
        {
            /// The type of one element.
            using Element = Type;
        };

        /// \brief Describes the fixed container, which is a plain array wearing a name.
        template<typename Type, UInt Count>
        struct Sequenced<Array<Type, Count>> final : Fixed<Count>
        {
            using Element = Type;
        };

        /// \brief Describes the growing container, whose elements lie wherever it keeps them.
        template<typename Type, UInt Capacity>
        struct Sequenced<Sequence<Type, Capacity>> final
        {
            /// The type of one element.
            using Element = Type;

            /// The most elements the container holds, or zero when it grows without bound.
            static constexpr UInt kCount = Capacity;

            /// \brief Counts the elements the container holds now.
            ///
            /// \param Run The address of the container.
            /// \return The number of elements.
            ZY_INLINE static UInt Size(ConstPtr<void> Run)
            {
                return static_cast<ConstPtr<Sequence<Type, Capacity>>>(Run)->GetSize();
            }

            /// \brief Finds the first element the container keeps.
            ///
            /// \param Run The address of the container.
            /// \return The address of the first element.
            ZY_INLINE static Ptr<void> Data(Ptr<void> Run)
            {
                return static_cast<Ptr<Sequence<Type, Capacity>>>(Run)->GetData();
            }

            /// \brief Makes an element at a position, built as its type builds by default.
            ///
            /// \param Run   The address of the container, which has room for one more.
            /// \param Index The position the element takes, up to the number of elements.
            /// \return The address of the new element.
            ZY_INLINE static Ptr<void> Insert(Ptr<void> Run, UInt Index)
            {
                const Ptr<Sequence<Type, Capacity>> Container = static_cast<Ptr<Sequence<Type, Capacity>>>(Run);

                Container->Insert(Index);
                return Container->GetData() + Index;
            }

            /// \brief Takes the element at a position out of the container.
            ///
            /// \param Run   The address of the container.
            /// \param Index The position of the element, below the number of elements.
            ZY_INLINE static void Remove(Ptr<void> Run, UInt Index)
            {
                static_cast<Ptr<Sequence<Type, Capacity>>>(Run)->Remove(Index);
            }
        };

        /// \brief Concept satisfied when a type is a run of elements of one type.
        template<typename Type>
        concept IsSequenced = requires { typename Sequenced<Type>::Element; };

        /// \brief `true` when a value handed back this way names storage that outlives the call.
        template<typename Type>
        inline constexpr Bool kIsBorrowed = false;

        /// \brief Marks a reference as borrowed.
        template<typename Type>
        inline constexpr Bool kIsBorrowed<Type &> = true;

        /// \brief Marks a constant reference as borrowed.
        template<typename Type>
        inline constexpr Bool kIsBorrowed<const Type &> = true;

        /// \brief Fails the build when a borrowed kind is read out of something that dies with the call.
        ///
        /// \note A prvalue `Text` is a view and cannot own; an lvalue reference names storage inside the instance.
        template<auto Handle, Kind Tag>
        ZY_INLINE constexpr void Borrow()
        {
            using Access = Accessor<decltype(Handle)>;

            if constexpr (!Access::kIsData)
            {
                if constexpr (Tag == Kind::Text)
                {
                    static_assert(IsAnyOf<typename Access::Result, Text> || kIsBorrowed<typename Access::Result>,
                        "A text field must hand back a view or a reference, never a string by value");
                }
                else if constexpr (Tag == Kind::Structure || Tag == Kind::Opaque || Tag == Kind::List)
                {
                    static_assert(kIsBorrowed<typename Access::Result>,
                        "A value reached in place must be a reference, or the walk would edit a temporary");
                }
            }
        }

        /// \brief Concept satisfied when a type gave itself a name.
        template<typename Type>
        concept IsNamed = requires { Classify<Type>::kTag; };

        /// \brief Gets the tag the module owning a type gave it, for whoever knows how to edit it.
        ///
        /// \return The tag of the type, empty when it named none.
        template<typename Type>
        ZY_INLINE constexpr Text TagOf()
        {
            if constexpr (IsNamed<Type>)
            {
                return Text(Classify<Type>::kTag);
            }
            else
            {
                return Text::Empty();
            }
        }

        /// \brief Converts a value of the type of the field into the payload its kind travels as.
        ///
        /// \param Content The value as the type spells it.
        /// \return The value as the kind carries it.
        template<typename Payload, typename Type>
        ZY_INLINE Payload Export(ConstRef<Type> Content)
        {
            if constexpr (requires { Classify<Type>::Read(Content); })
            {
                return Classify<Type>::Read(Content);
            }
            else if constexpr (IsEnum<Type>)
            {
                return static_cast<Payload>(ZyEnum::Cast(Content));
            }
            else
            {
                return static_cast<Payload>(Content);
            }
        }

        /// \brief Converts the payload a kind travels as back into the type of the field.
        ///
        /// \param Content The value as the kind carries it.
        /// \return The value as the type spells it.
        template<typename Type, typename Payload>
        ZY_INLINE Type Import(Payload Content)
        {
            if constexpr (requires { Classify<Type>::Write(Content); })
            {
                return Classify<Type>::Write(Content);
            }
            else if constexpr (IsCastable<Payload, Type>)
            {
                return static_cast<Type>(Content);
            }
            else
            {
                return Type(Content);
            }
        }

        /// \brief Reads a value out of a type and wraps it as the kind it is edited as.
        ///
        /// \param Content The value as the type spells it.
        /// \return The value wrapped for whoever edits it.
        template<Kind Tag, typename Type>
        ZY_INLINE Value Encode(ConstRef<Type> Content)
        {
            if constexpr (Tag == Kind::Boolean)
            {
                return Value::FromBoolean(Export<Bool>(Content));
            }
            else if constexpr (Tag == Kind::Real)
            {
                return Value::FromReal(Export<Real64>(Content));
            }
            else if constexpr (Tag == Kind::Text)
            {
                return Value::FromText(Export<Text>(Content));
            }
            else if constexpr (Tag == Kind::Integer || Tag == Kind::Enumeration)
            {
                return Value::FromInteger(Tag, Export<SInt64>(Content));
            }
            else
            {
                return Value::FromForeign(Content);
            }
        }

        /// \brief Unwraps an edited value back into the type it is written into.
        ///
        /// \param Content The value that was edited.
        /// \return The value as the type spells it.
        template<Kind Tag, typename Type>
        ZY_INLINE Type Decode(ConstRef<Value> Content)
        {
            if constexpr (Tag == Kind::Boolean)
            {
                return Import<Type>(Content.GetBoolean());
            }
            else if constexpr (Tag == Kind::Real)
            {
                return Import<Type>(Content.GetReal());
            }
            else if constexpr (Tag == Kind::Text)
            {
                return Import<Type>(Content.GetText());
            }
            else if constexpr (Tag == Kind::Integer || Tag == Kind::Enumeration)
            {
                return Import<Type>(Content.GetInteger());
            }
            else if constexpr (Tag == Kind::Structure || Tag == Kind::Opaque)
            {
                return * static_cast<ConstPtr<Type>>(Content.GetAddress());
            }
            else
            {
                return Content.template Get<Type>();
            }
        }

        /// \brief Builds the named values of an enum, in the order it declares them.
        ///
        /// \return The options of the enum, each borrowing its name from the compile-time name pool of the enum.
        template<IsEnum Type>
        constexpr auto BuildOptions()
        {
            static_assert(ZyEnum::Count<Type>() > 0, "An enumerated field needs an enum with at least one enumerator");

            constexpr auto Values = ZyEnum::GetValues<Type>();

            Array<Option, ZyEnum::Count<Type>()> Result;

            for (UInt Index = 0; Index < Values.GetSize(); ++Index)
            {
                Result[Index] = Option(ZyEnum::GetName(Values[Index]), static_cast<SInt64>(ZyEnum::Cast(Values[Index])));
            }
            return Result;
        }

        /// \brief The named values of the given enum, laid down once per enum and never moved.
        template<IsEnum Type>
        inline constexpr auto kOptions = BuildOptions<Type>();

        /// \brief Checks whether every field a type declares is reached through that same type.
        ///
        /// \return `true` when every field belongs to the type, otherwise `false`.
        template<typename Type>
        constexpr Bool Matches()
        {
            ConstRef<decltype(Describe<Type>::kFields)> Fields = Describe<Type>::kFields;

            for (UInt Index = 0; Index < Fields.GetSize(); ++Index)
            {
                if (!Fields[Index].template Owns<Type>())
                {
                    return false;
                }
            }
            return true;
        }

        /// \brief Builds what a run of elements holds, out of the type of the run.
        ///
        /// \return The capacity, stride, kind, fields and handlers of the run's elements.
        template<IsSequenced Type>
        constexpr Listing BuildListing()
        {
            using Run     = Sequenced<Type>;
            using Element = typename Run::Element;

            constexpr Kind Tag = Classify<Element>::kValue;

            static_assert(Tag != Kind::None, "The elements have no kind");
            static_assert(Tag != Kind::List, "A run of runs is not a layout, so it stays a custom field");

            const UInt32 Count  = static_cast<UInt32>(Run::kCount);
            const UInt32 Stride = static_cast<UInt32>(sizeof(Element));

            ConstSpan<Field> Fields;

            if constexpr (Tag == Kind::Structure)
            {
                Fields = ConstSpan<Field>(Describe<Element>::kFields);
            }
            return Listing(Fields, Count, Stride, Tag, Run::Size, Run::Data, Run::Insert, Run::Remove);
        }

        /// \brief What a run of elements of the given type holds, laid down once and never moved.
        template<IsSequenced Type>
        inline constexpr Listing kListing = BuildListing<Type>();
    }
}