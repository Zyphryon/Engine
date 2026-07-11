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

#include "Zyphryon.Base/Utility.hpp"
#include "Zyphryon.Base/Scalar.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A type-safe union that can hold one value of any type in \p Types at a time.
    template<typename... Types>
    class Variant final
    {
    public:

        /// \brief Constructs an empty variant holding no value.
        ZY_INLINE Variant()
            : mSlot { kEmpty }
        {
        }

        /// \brief Constructs a variant holding the given value of type \p Type.
        ///
        /// \param Value The value to store in the variant.
        template<typename Type>
        ZY_INLINE Variant(AnyRef<Type> Value)
            requires IsAnyOf<StripAll<Type>, Types...>
            : mSlot { kEmpty }
        {
            Emplace<StripAll<Type>>(Forward<Type>(Value));
        }

        /// \brief Constructs a variant by copying from another variant.
        ///
        /// \param Other The variant to copy from.
        ZY_INLINE Variant(ConstRef<Variant> Other)
            : mSlot { kEmpty }
        {
            if (!Other.IsEmpty())
            {
                Other.Visit([this]<typename T0>(AnyRef<T0> Value)
                {
                    Emplace<StripAll<T0>>(Value);
                });
            }
        }

        /// \brief Constructs a variant by transferring ownership from another variant.
        ///
        /// \param Other The variant to move from.
        ZY_INLINE Variant(AnyRef<Variant> Other)
            : mSlot { kEmpty }
        {
            if (!Other.IsEmpty())
            {
                Other.Visit([this]<typename T0>(AnyRef<T0> Value)
                {
                    Emplace<StripAll<T0>>(Move(Value));
                });
                Other.Reset();
            }
        }

        /// \brief Destroys the active value and releases the variant.
        ZY_INLINE ~Variant()
        {
            Reset();
        }

        /// \brief Checks whether the variant is currently empty.
        ///
        /// \return `true` if the variant holds no value, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mSlot == kEmpty;
        }

        /// \brief Checks whether the variant currently holds a value of type \p Type.
        ///
        /// \return `true` if the variant holds \p Type, `false` otherwise.
        template<typename Type>
        ZY_INLINE Bool IsHolding() const
            requires IsAnyOf<Type, Types...>
        {
            return mSlot == SlotOf<Type>();
        }

        /// \brief Gets a reference to the active value as type \p Type.
        ///
        /// \return A reference to the value.
        template<typename Type>
        ZY_INLINE Ref<Type> Get()
            requires IsAnyOf<Type, Types...>
        {
            ZY_ASSERT(IsHolding<Type>(), "Variant does not hold the requested type");

            return * Launder(reinterpret_cast<Ptr<Type>>(mData));
        }

        /// \brief Gets a constant reference to the active value as type \p Type.
        ///
        /// \return A constant reference to the value.
        template<typename Type>
        ZY_INLINE ConstRef<Type> Get() const
            requires IsAnyOf<Type, Types...>
        {
            ZY_ASSERT(IsHolding<Type>(), "Variant does not hold the requested type");

            return * Launder(reinterpret_cast<ConstPtr<Type>>(mData));
        }

        /// \brief Gets a pointer to the active value if it is of type \p Type.
        ///
        /// \return A pointer to the value, or \c nullptr if the variant does not hold \p Type.
        template<typename Type>
        ZY_INLINE Ptr<Type> TryGet()
            requires IsAnyOf<Type, Types...>
        {
            return IsHolding<Type>() ? Launder(reinterpret_cast<Ptr<Type>>(mData)) : nullptr;
        }

        /// \brief Gets a constant pointer to the active value if it is of type \p Type.
        ///
        /// \return A constant pointer to the value, or \c nullptr if the variant does not hold \p Type.
        template<typename Type>
        ZY_INLINE ConstPtr<Type> TryGet() const
            requires IsAnyOf<Type, Types...>
        {
            return IsHolding<Type>() ? Launder(reinterpret_cast<ConstPtr<Type>>(mData)) : nullptr;
        }

        /// \brief Gets the zero-based slot of the active type.
        ///
        /// \return The slot of the current type.
        ZY_INLINE UInt8 GetSlot() const
        {
            return mSlot;
        }

        /// \brief Constructs a value of type \p Type in-place, replacing the current value.
        ///
        /// \param Parameters The constructor arguments for \p Type.
        /// \return A reference to the newly constructed value.
        template<typename Type, typename... Arguments>
        ZY_INLINE Ref<Type> Emplace(AnyRef<Arguments>... Parameters)
            requires IsAnyOf<Type, Types...>
        {
            Reset();

            Construct<Type>(reinterpret_cast<Ptr<Type>>(mData), Forward<Arguments>(Parameters)...);
            mSlot = SlotOf<Type>();

            return Get<Type>();
        }

        /// \brief Destroys the active value and resets the variant to empty.
        ZY_INLINE void Reset()
        {
            if (mSlot != kEmpty)
            {
                Visit([]<typename T0>(AnyRef<T0> Value)
                {
                    Destruct<StripAll<T0>>(Value);
                });
                mSlot = kEmpty;
            }
        }

        /// \brief Invokes a callable with the active value.
        ///
        /// \param Callback The callable to invoke with the active value.
        /// \return The result of invoking \p Callback with the active value.
        template<typename Callable>
        ZY_INLINE decltype(auto) Visit(AnyRef<Callable> Callback)
        {
            ZY_ASSERT(!IsEmpty(), "Cannot visit an empty variant");

            return VisitImpl(mSlot, Forward<Callable>(Callback), * this);
        }

        /// \brief Invokes a callable with the active value.
        ///
        /// \param Callback The callable to invoke with the active value.
        /// \return The result of invoking \p Callback with the active value.
        template<typename Callable>
        ZY_INLINE decltype(auto) Visit(AnyRef<Callable> Callback) const
        {
            ZY_ASSERT(!IsEmpty(), "Cannot visit an empty variant");

            return VisitImpl(mSlot, Forward<Callable>(Callback), * this);
        }

        /// \brief Assigns a new value to the variant, replacing the current one.
        ///
        /// \param Value The value to store.
        /// \return A reference to this variant.
        template<typename Type>
        ZY_INLINE Ref<Variant> operator=(AnyRef<Type> Value)
            requires IsAnyOf<StripAll<Type>, Types...>
        {
            Emplace<StripAll<Type>>(Forward<Type>(Value));
            return (* this);
        }

        /// \brief Assigns from another variant by copying its active value.
        ///
        /// \param Other The variant to copy from.
        /// \return A reference to this variant.
        ZY_INLINE Ref<Variant> operator=(ConstRef<Variant> Other)
        {
            if (this != AddressOf(Other))
            {
                Reset();

                if (!Other.IsEmpty())
                {
                    Other.Visit([this]<typename T0>(AnyRef<T0> Value)
                    {
                        Emplace<StripAll<T0>>(Value);
                    });
                }
            }
            return (* this);
        }

        /// \brief Assigns from another variant by moving its active value.
        ///
        /// \param Other The variant to move from.
        /// \return A reference to this variant.
        ZY_INLINE Ref<Variant> operator=(AnyRef<Variant> Other)
        {
            if (this != AddressOf(Other))
            {
                Reset();

                if (!Other.IsEmpty())
                {
                    Other.Visit([this]<typename T0>(AnyRef<T0> Value)
                    {
                        Emplace<StripAll<T0>>(Move(Value));
                    });
                    Other.Reset();
                }
            }
            return (* this);
        }

    private:

        /// \brief The slot used to represent an empty variant.
        static constexpr UInt8 kEmpty = static_cast<UInt8>(-1);

        /// \brief Finds the zero-based slot of \p Type in the parameter pack \p Types.
        ///
        /// \return The the zero-based slot of \p Type in \p Types, or `kEmpty` if \p Type is not found.
        template<typename Type>
        ZY_INLINE static constexpr UInt8 SlotOf()
        {
            UInt8 Slot = 0;
            return ((IsAnyOf<Type, Types> || (++Slot, false)) || ...), Slot < sizeof...(Types) ? Slot : kEmpty;
        }

        /// \brief Invokes a callable with the active variant value based on the runtime slot.
        ///
        /// \param Index    The runtime slot of the active type.
        /// \param Callback The callable to invoke with the active value.
        /// \param Value    The variant whose active value will be passed to \p Callback.
        /// \return The result of invoking \p Callback with the active value.
        template<UInt8 Element = 0, typename Callable, typename Type>
        ZY_INLINE static decltype(auto) VisitImpl(UInt8 Index, AnyRef<Callable> Callback, AnyRef<Type> Value)
        {
            if constexpr (Element < sizeof...(Types))
            {
                if (Element == Index)
                {
                    return Callback(Value.template Get<typename Identify<Element, Types...>::Type>());
                }
                else
                {
                    return VisitImpl<Element + 1, Callable>(Index, Forward<Callable>(Callback), Value);
                }
            }
        }

    public:

        /// \brief Gets the byte size of the type at the given zero-based slot.
        ///
        /// \param Index The zero-based slot to query.
        /// \return The size, in bytes, of the type occupying \p Index.
        static constexpr UInt16 GetSize(UInt8 Index)
        {
            ZY_ASSERT(Index < sizeof...(Types), "Index out of range");

            static constexpr Array kSizes = Array(static_cast<UInt16>(sizeof(Types))...);
            return kSizes[Index];
        }

        /// \brief Gets the required alignment of the type at the given zero-based slot.
        ///
        /// \param Index The zero-based slot to query.
        /// \return The alignment, in bytes, of the type occupying \p Index.
        static constexpr UInt16 GetAlignment(UInt8 Index)
        {
            ZY_ASSERT(Index < sizeof...(Types), "Index out of range");

            static constexpr Array kAlignments = Array(static_cast<UInt16>(alignof(Types))...);
            return kAlignments[Index];
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        alignas(Max(alignof(Types)...)) Byte mData[Max(sizeof(Types)...)];
        Integer<sizeof...(Types)>            mSlot;
    };
}