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

#include "Zyphryon.Base/Bit.hpp"
#include "Zyphryon.Base/Concept.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Represents a slot paired with the epoch that slot was handed out at.
    ///
    /// \tparam Capacity  The number of slots the key must be able to name.
    /// \tparam EpochBits The number of bits given to the epoch, or zero to carry none.
    template<UInt Capacity, UInt EpochBits = 16>
    class Key final
    {
    public:

        /// \brief The number of bits given to the slot, counting the zero that names nothing.
        static constexpr UInt kSlotBits  = CountSignificantBits(Capacity);

        /// \brief The number of bits given to the epoch.
        static constexpr UInt kEpochBits = EpochBits;

        /// \brief The number of bits the whole key occupies.
        static constexpr UInt kBits      = kSlotBits + kEpochBits;

        /// \brief The integer the key is packed into, the smallest one that holds every field.
        using Value = Integer<(1ull << (kBits - 1)) | ((1ull << (kBits - 1)) - 1)>;

        /// \brief The integer a slot is counted in.
        using Slot  = Integer<Capacity>;

        /// \brief The integer an epoch is counted in.
        using Epoch = Integer<(1ull << kEpochBits) - 1>;

    public:

        /// \brief Constructs a key that names nothing.
        ZY_INLINE constexpr Key()
            : mValue { 0 }
        {
        }

        /// \brief Constructs a key naming one slot at one epoch.
        ///
        /// \param Slot  The slot, counted from one, or zero to name nothing.
        /// \param Epoch How many times that slot had been handed out when the key was minted.
        ZY_INLINE constexpr Key(Slot Slot, Epoch Epoch)
            : mValue { static_cast<Value>(static_cast<Value>(Slot) | (static_cast<Value>(Epoch) << kSlotBits)) }
        {
        }

        /// \brief Constructs a key naming one slot, for a key that carries no epoch.
        ///
        /// \param Slot The slot, counted from one, or zero to name nothing.
        ZY_INLINE constexpr Key(Slot Slot)
            requires (kEpochBits == 0)
            : mValue { static_cast<Value>(Slot) }
        {
        }

        /// \brief Checks whether the key names a slot at all.
        ///
        /// \return `true` if the key names a slot, otherwise `false`.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return GetSlot() != 0;
        }

        /// \brief Gets the slot the key names.
        ///
        /// \return The slot, counted from one, or zero when the key names nothing.
        ZY_INLINE constexpr Slot GetSlot() const
        {
            if constexpr (kEpochBits == 0)
            {
                return static_cast<Slot>(mValue);
            }
            else
            {
                return static_cast<Slot>(mValue & ((static_cast<Value>(1) << kSlotBits) - 1));
            }
        }

        /// \brief Gets the epoch the slot was handed out at.
        ///
        /// \return The epoch, or zero when the key carries none.
        ZY_INLINE constexpr Epoch GetEpoch() const
        {
            if constexpr (kEpochBits == 0)
            {
                return 0;
            }
            else
            {
                return static_cast<Epoch>(mValue >> kSlotBits);
            }
        }

        /// \brief Gets the key packed into one integer, for a caller that has to store it as such.
        ///
        /// \return The packed value.
        ZY_INLINE constexpr Value GetValue() const
        {
            return mValue;
        }

        /// \brief Rebuilds the key as one naming a slot in a pool of another capacity.
        ///
        /// \tparam Other The capacity of the pool the rebuilt key names a slot in.
        /// \return The same slot and epoch, named against \p Other.
        template<UInt Other>
        ZY_INLINE constexpr Key<Other, EpochBits> Rebind() const
        {
            using Target = Key<Other, EpochBits>;

            return Target(static_cast<Target::Slot>(GetSlot()), GetEpoch());
        }

        /// \brief Compares this key against another for equality.
        ///
        /// \param Other The key to compare against.
        /// \return `true` if both name the same slot at the same epoch, otherwise `false`.
        ZY_INLINE constexpr Bool operator==(ConstRef<Key> Other) const
        {
            return mValue == Other.mValue;
        }

        /// \brief Converts the key to its slot, for a key that carries no epoch and is therefore only a slot.
        ///
        /// \return The slot the key names.
        ZY_INLINE constexpr operator Slot() const
            requires (kEpochBits == 0)
        {
            return GetSlot();
        }

        /// \brief Checks whether the key names a slot at all.
        ///
        /// \return `true` if the key names a slot, otherwise `false`.
        ZY_INLINE constexpr explicit operator Bool() const
        {
            return IsValid();
        }

    private:

        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-
        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-

        Value mValue;
    };
}