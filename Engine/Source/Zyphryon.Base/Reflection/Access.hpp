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

namespace Reflection
{
    class Field;

    /// \brief Describes how a type presents itself to reflection: the name it answers to, and how it is shown.
    struct Presentation final
    {
        /// The name the type is registered under.
        Text Name;

        /// Whether the type stands in for the one part it holds, rather than being stepped into.
        Bool Flat = false;
    };

    /// \brief Concept satisfied when the given type has been self described.
    template<typename Type>
    concept IsSelfDescribed = requires { Type::OnDescribe(); };

    /// \brief Concept satisfied when the given type has named itself to reflection.
    template<typename Type>
    concept IsSelfClassified = requires { Type::OnClassify(); };

    /// \brief Customization point that lists the fields of a type.
    ///
    /// \code
    /// // A plain type: every member is read and written through whatever accessor it already has.
    /// ZY_REFLECT(Timer,
    ///     Reflection::Field::Property<&Timer::mElapsed>("Elapsed").Between(0.0f, 60.0f),
    ///     Reflection::Field::Property<&Timer::IsPaused, &Timer::SetPaused>("Paused"));
    ///
    /// // A type holding another described type, which a walk steps into rather than flattening.
    /// ZY_REFLECT(Profile,
    ///     Reflection::Field::Property<&Profile::GetName, &Profile::SetName>("Name"),
    ///     Reflection::Field::Nested<&Profile::GetTimer, &Profile::SetTimer>("Timer"));
    ///
    /// // A value no schema can express, handed on under the tag its own module gave it.
    /// ZY_REFLECT(Palette,
    ///     Reflection::Field::Custom<&Palette::mGradient>("Gradient", "Palette.Gradient"));
    /// \endcode
    /// \brief Concept satisfied when a type lays out its own fields rather than having them laid out for it.
    template<typename Type>
    struct Describe
    {
    };

    /// \brief Takes the fields a type laid out for itself in `OnDescribe`.
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

    /// \brief Customization point that names the kind a type is edited as.
    ///
    /// \code
    /// template<>
    /// struct Reflection::Classify<Timestamp>
    /// {
    ///     static constexpr Reflection::Kind kValue = Reflection::Kind::Foreign;
    ///     static constexpr Char             kTag[] = "Chrono.Timestamp";
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

    /// \brief Takes the tag a type named itself with in `OnClassify`.
    template<IsSelfClassified Type>
    struct Classify<Type>
    {
        /// The kind the type is edited as, which is its parts when it has any and its own bytes otherwise.
        static constexpr Kind kValue = IsDescribed<Type> ? Kind::Structure : Kind::Foreign;

        /// The tag whoever edits the value matches on.
        static constexpr Text kTag   = Type::OnClassify().Name;
    };

    /// \brief Names a view of characters as text, which is what a getter over owned storage hands back.
    template<>
    struct Classify<Text>
    {
        /// The kind a view of characters is edited as.
        static constexpr Kind kValue = Kind::Text;
    };

    /// \brief Describes what a run of elements holds, laid down once per type of run and never moved.
    struct Listing final
    {
        /// The fields of one element, empty unless the elements are described.
        Span<const Field> Fields;

        /// The number of elements the run holds.
        UInt32            Count;

        /// The distance in bytes from one element to the next.
        UInt32            Stride;

        /// The kind one element carries.
        Kind              Content;
    };

    namespace Detail
    {
        /// \brief Decomposes a member pointer into the type it reaches through and the value it carries.
        template<typename Type>
        struct Accessor;

        /// \brief Decomposes a pointer to a data member, which is its own getter and setter.
        template<typename Class, typename Type>
        struct Accessor<Type Class::*> final
        {
            /// Whether the member is data, and so needs no call to reach it.
            static constexpr Bool kIsData = true;

            /// The type the member is reached through.
            using Owner  = Class;

            /// The type as the member spells it, references and qualifiers and all.
            using Result = Type;

            /// The type with its reference and qualifiers taken off.
            using Value  = StripAll<Type>;
        };

        /// \brief Decomposes a pointer to a constant getter.
        template<typename Class, typename Type>
        struct Accessor<Type (Class::*)() const> final
        {
            static constexpr Bool kIsData = false;

            using Owner  = Class;
            using Result = Type;
            using Value  = StripAll<Type>;
        };

        /// \brief Decomposes a pointer to a constant getter that promises not to throw.
        template<typename Class, typename Type>
        struct Accessor<Type (Class::*)() const noexcept> final
        {
            static constexpr Bool kIsData = false;

            using Owner  = Class;
            using Result = Type;
            using Value  = StripAll<Type>;
        };

        /// \brief Decomposes a pointer to a mutable getter.
        template<typename Class, typename Type>
        struct Accessor<Type (Class::*)()> final
        {
            static constexpr Bool kIsData = false;

            using Owner  = Class;
            using Result = Type;
            using Value  = StripAll<Type>;
        };

        /// \brief Decomposes a pointer to a mutable getter that promises not to throw.
        template<typename Class, typename Type>
        struct Accessor<Type (Class::*)() noexcept> final
        {
            static constexpr Bool kIsData = false;

            using Owner  = Class;
            using Result = Type;
            using Value  = StripAll<Type>;
        };

        /// \brief Decomposes a pointer to a setter, whatever it hands back.
        ///
        /// \note A setter that returns its own owner, so calls may be chained, is as much a setter as one that
        ///       returns nothing, and what it hands back is no business of the field.
        template<typename Class, typename Return, typename Type>
        struct Accessor<Return (Class::*)(Type)> final
        {
            static constexpr Bool kIsData = false;

            using Owner  = Class;
            using Result = Type;
            using Value  = StripAll<Type>;
        };

        /// \brief Decomposes a pointer to a setter that promises not to throw.
        template<typename Class, typename Return, typename Type>
        struct Accessor<Return (Class::*)(Type) noexcept> final
        {
            static constexpr Bool kIsData = false;

            using Owner  = Class;
            using Result = Type;
            using Value  = StripAll<Type>;
        };

        /// \brief Names a type with its reference and constness taken off, and its extent kept.
        template<typename Type>
        struct Bare final
        {
            /// The type as it stands.
            using Result = Type;
        };

        template<typename Type>
        struct Bare<const Type> final
        {
            using Result = typename Bare<Type>::Result;
        };

        template<typename Type>
        struct Bare<Type &> final
        {
            using Result = typename Bare<Type>::Result;
        };

        /// \brief Decomposes a run of elements into what it holds and how many of them there are.
        template<typename Type>
        struct Sequenced;

        /// \brief Decomposes a plain array.
        template<typename Type, UInt Count>
        struct Sequenced<Type[Count]> final
        {
            /// The type of one element.
            using Element = Type;

            /// The number of elements.
            static constexpr UInt kCount = Count;
        };

        /// \brief Decomposes the fixed container, which is a plain array wearing a name.
        template<typename Type, UInt Count>
        struct Sequenced<Array<Type, Count>> final
        {
            using Element = Type;

            static constexpr UInt kCount = Count;
        };

        /// \brief Concept satisfied when a type is a run of elements of one type.
        template<typename Type>
        concept IsSequenced = requires { typename Sequenced<Type>::Element; };

        /// \brief Whether a value handed back this way names storage that outlives the call.
        template<typename Type>
        inline constexpr Bool kIsBorrowed = false;

        template<typename Type>
        inline constexpr Bool kIsBorrowed<Type &> = true;

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
            if constexpr (requires { Classify<Type>::kTag; })
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
                return static_cast<Payload>(Enum::Cast(Content));
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
            static_assert(Enum::Count<Type>() > 0, "An enumerated field needs an enum with at least one enumerator");

            constexpr auto Values = Enum::GetValues<Type>();

            Array<Option, Enum::Count<Type>()> Result;

            for (UInt Index = 0; Index < Values.GetSize(); ++Index)
            {
                Result[Index] = Option(Enum::GetName(Values[Index]), static_cast<SInt64>(Enum::Cast(Values[Index])));
            }
            return Result;
        }

        /// \brief The named values of the given enum, laid down once per enum and never moved.
        template<IsEnum Type>
        inline constexpr auto kOptions = BuildOptions<Type>();
        
        /// \brief Checks that every field a type declares is reached through that same type.
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
        /// \return The listing of that run.
        template<IsSequenced Type>
        constexpr Listing BuildListing()
        {
            using Element = typename Sequenced<Type>::Element;

            constexpr Kind Tag = Classify<Element>::kValue;

            static_assert(Tag != Kind::None, "The elements have no kind");
            static_assert(Tag != Kind::List, "A run of runs is not a layout, so it stays a custom field");

            const UInt32 Count  = static_cast<UInt32>(Sequenced<Type>::kCount);
            const UInt32 Stride = static_cast<UInt32>(sizeof(Element));

            if constexpr (Tag == Kind::Structure)
            {
                return Listing(Span<const Field>(Describe<Element>::kFields), Count, Stride, Tag);
            }
            else
            {
                return Listing(Span<const Field>(), Count, Stride, Tag);
            }
        }

        /// \brief What a run of elements of the given type holds, laid down once and never moved.
        template<IsSequenced Type>
        inline constexpr Listing kListing = BuildListing<Type>();
    }
}