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

#include "Reflection.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Specifies the traits a component type carries, which may be several taken together.
    enum class Trait : UInt8
    {
        None         = 0,       ///< The type carries no trait at all.
        Serializable = 1 << 0,  ///< The type travels in saves, and must know how to write itself unless it is a tag.
        Inheritable  = 1 << 1,  ///< The type is read by instances from their archetype rather than copied onto each.
        Local        = 1 << 2,  ///< The type never reaches an instance made from an archetype.
        Singleton    = 1 << 3,  ///< The type has its one instance held by the world, which queries read from there.
        Loaded       = 1 << 4,  ///< The type has its lifecycle code loaded, which a discarded module's type has not.
        Keeps        = 1 << 5,  ///< The type keeps the value a removal took away, which readers of removals are handed.
        Server       = 1 << 6,  ///< The type belongs only to the side that decides for the world.
        Client       = 1 << 7,  ///< The type belongs only to the side that shows the world to its player.
    };
    ZY_DEFINE_BITWISE_ENUM(Trait)

    /// \brief Represents one component type, as the storage, saves and tools know it.
    struct Metatype final
    {
        /// The name saves know the type by, the one the compiler gives it unless it was declared under another.
        Text                 Name;

        /// The default constructor over a run of instances, or `nullptr` where zeroed bytes are a valid instance.
        void (* Construct)(Ptr<void> Destination, UInt Count);

        /// The destructor over a run of instances, or `nullptr` where there is nothing to destroy.
        void (* Destruct)(Ptr<void> Target, UInt Count);

        /// The move into uninitialized storage that also ends the source, or `nullptr` where copying the bytes does.
        void (* Relocate)(Ptr<void> Destination, Ptr<void> Source, UInt Count);

        /// The copy into uninitialized storage, or `nullptr` where copying the bytes does.
        void (* Copy)(Ptr<void> Destination, ConstPtr<void> Source, UInt Count);

        /// The writing of one instance into a save through its own serialization, or `nullptr` when it has none.
        void (* Save)(Ref<Writer> Output, ConstPtr<void> Source);

        /// The reading of one instance out of a save, over one already built, or `nullptr` when it has none.
        void (* Load)(Ref<Reader> Input, Ptr<void> Target);

        /// The components given alongside whenever this one is, each at most once.
        Sequence<UInt32>     Implies;

        /// The description of how a tool shows the component, with an empty label while nothing described it.
        Description          Presentation;

        /// The width and alignment of one instance, and the fields a tool reads and writes once reflected.
        ZyReflection::Schema Layout;

        /// The traits the type carries.
        Trait                Traits;

        /// \brief Constructs a type with no name, no lifecycle and no trait but the one saying its code is loaded.
        ZY_INLINE Metatype()
            : Construct { nullptr },
              Destruct  { nullptr },
              Relocate  { nullptr },
              Copy      { nullptr },
              Save      { nullptr },
              Load      { nullptr },
              Traits    { Trait::Loaded }
        {
        }

        /// \brief Gets the size of one instance.
        ///
        /// \return The size in bytes, zero for a tag.
        ZY_INLINE UInt GetSize() const
        {
            return Layout.GetWidth();
        }

        /// \brief Gets the alignment one instance needs.
        ///
        /// \return The alignment in bytes.
        ZY_INLINE UInt GetAlignment() const
        {
            return Max<UInt>(Layout.GetAlignment(), 1);
        }

        /// \brief Checks whether the type carries no data, and so takes no column at all.
        ///
        /// \return `true` for a tag, `false` otherwise.
        ZY_INLINE Bool IsTag() const
        {
            return Layout.GetWidth() == 0;
        }

        /// \brief Gives the type a trait or takes it away.
        ///
        /// \param Which   The trait, or several taken together.
        /// \param Enabled `true` to give it, `false` to take it away.
        ZY_INLINE void Set(Trait Which, Bool Enabled)
        {
            Traits = SetOrClearBit(Traits, Which, Enabled);
        }

        /// \brief Checks whether the type carries a trait.
        ///
        /// \param Which The trait, or several taken together.
        /// \return `true` if the type carries every trait given, `false` otherwise.
        ZY_INLINE Bool Has(Trait Which) const
        {
            return HasBit(Traits, Which);
        }

        /// \brief Builds the description of a type the storage moves instances of.
        ///
        /// \tparam Type The type to describe, empty for a tag.
        /// \return The layout and lifecycle of \p Type.
        template<typename Type>
        static Metatype Of()
        {
            Metatype Info;
            Info.Name = Qualify<Type>();

            if constexpr (!IsEmpty<Type>)
            {
                static_assert( sizeof(Type) <= 0xFFFF
                           && alignof(Type) <= 0xFF, "A component must fit a 16-bit size and an 8-bit alignment");

                Info.Layout = ZyReflection::Schema(sizeof(Type), alignof(Type));

                // Saves reuse the type's own serialization.
                if constexpr ((IsSerializable<Type, Archive<Writer>>
                            && IsSerializable<Type, Archive<Reader>>) || IsTriviallyCopyable<Type>)
                {
                    Info.Save = Lifecycle<Type>::Save;
                    Info.Load = Lifecycle<Type>::Load;
                }

                if constexpr (!IsTriviallyConstructible<Type>)
                {
                    Info.Construct = Lifecycle<Type>::Construct;
                }

                if constexpr (!IsTriviallyDestructible<Type>)
                {
                    Info.Destruct  = Lifecycle<Type>::Destruct;
                }

                if constexpr (!IsTriviallyCopyable<Type>)
                {
                    Info.Copy      = Lifecycle<Type>::Copy;
                    Info.Relocate  = Lifecycle<Type>::Relocate;
                }
            }
            return Info;
        }

        /// \brief Gets the description columns of an unresolved type are built from, a run of bytes kept as saved.
        ///
        /// \return The description, whose save writes the bytes verbatim and whose load takes all it is handed.
        static ConstRef<Metatype> Uninitialized();

        /// \brief Gets the fully qualified name of a type as the compiler spells it, which needs no definition of it.
        ///
        /// \tparam Type The type, which may be only declared.
        /// \return The name, namespace included, without the keyword the compiler opens a class name with.
        template<typename Type>
        static constexpr Text Qualify()
        {
#if defined(ZY_COMPILER_MSVC)
            constexpr Text Signature = __FUNCSIG__;
            constexpr Text Prologue  = "Qualify<";
            constexpr Text Epilogue  = ">(void)";
#else
            constexpr Text Signature = __PRETTY_FUNCTION__;
            constexpr Text Prologue  = "Type = ";
            constexpr Text Epilogue  = "]";
#endif
            constexpr Text Qualified = StrBefore(StrAfter(Signature, Prologue), Epilogue);

            if      constexpr (StrStartsWith(Qualified, "struct "))
            {
                return StrAfter(Qualified, "struct ");
            }
            else if constexpr (StrStartsWith(Qualified, "class "))
            {
                return StrAfter(Qualified, "class ");
            }
            else
            {
                return Qualified;
            }
        }

    private:

        /// \brief Represents the lifecycle of one component type, as the storage runs it over raw bytes.
        ///
        /// \tparam Type The type the bytes hold.
        template<typename Type>
        struct Lifecycle final
        {
            /// \brief Builds a run of default instances.
            ///
            /// \param Destination The first byte of the run.
            /// \param Count       The number of instances.
            static void Construct(Ptr<void> Destination, UInt Count)
            {
                for (UInt Index = 0; Index < Count; ++Index)
                {
                    ::Construct(static_cast<Ptr<Type>>(Destination) + Index);
                }
            }

            /// \brief Destroys a run of instances.
            ///
            /// \param Target The first byte of the run.
            /// \param Count  The number of instances.
            static void Destruct(Ptr<void> Target, UInt Count)
            {
                for (UInt Index = 0; Index < Count; ++Index)
                {
                    ::Destruct(static_cast<Ptr<Type>>(Target)[Index]);
                }
            }

            /// \brief Copies a run of instances into uninitialized storage.
            ///
            /// \param Destination The first byte of the storage.
            /// \param Source      The first byte of the instances copied.
            /// \param Count       The number of instances.
            static void Copy(Ptr<void> Destination, ConstPtr<void> Source, UInt Count)
            {
                if constexpr (IsCopyConstructible<Type>)
                {
                    for (UInt Index = 0; Index < Count; ++Index)
                    {
                        ::Construct(
                            static_cast<Ptr<Type>>(Destination) + Index,
                            static_cast<ConstPtr<Type>>(Source)[Index]);
                    }
                }
                else
                {
                    ZY_ASSERT(false, "A component that cannot be copied cannot sit on an archetype");
                }
            }

            /// \brief Moves a run of instances into uninitialized storage, ending the originals.
            ///
            /// \param Destination The first byte of the storage.
            /// \param Source      The first byte of the instances moved.
            /// \param Count       The number of instances.
            static void Relocate(Ptr<void> Destination, Ptr<void> Source, UInt Count)
            {
                for (UInt Index = 0; Index < Count; ++Index)
                {
                    ::Relocate(static_cast<Ptr<Type>>(Destination)[Index], Move(static_cast<Ptr<Type>>(Source)[Index]));
                }
            }

            /// \brief Writes one instance into a save, through the type's own serialization.
            ///
            /// \param Output The save to write into.
            /// \param Source The instance to write.
            static void Save(Ref<Writer> Output, ConstPtr<void> Source)
            {
                Archive(Output).Serialize(* const_cast<Ptr<Type>>(static_cast<ConstPtr<Type>>(Source)));
            }

            /// \brief Reads one instance out of a save, over one already built.
            ///
            /// \param Input  The save to read from.
            /// \param Target The instance to read over.
            static void Load(Ref<Reader> Input, Ptr<void> Target)
            {
                Archive(Input).Serialize(* static_cast<Ptr<Type>>(Target));
            }
        };
    };
}