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

#include "Zyphryon.Base/Container/Sequence.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a run of values keyed by the moment each is worn, read by blending the pair either side of one.
    ///
    /// \tparam Type     The value the run carries, blended by its own `Lerp` where it has one and by arithmetic otherwise.
    /// \tparam Capacity The number of stops held inline, or zero to hold them on the heap.
    template<typename Type, UInt Capacity = 0>
    class Ramp final
    {
    public:

        /// \brief Represents one value the run passes through, and the moment it is worn at.
        struct Stop final
        {
            /// The moment the value is worn at.
            Real32 Time = 0.0f;

            /// The value worn at that moment.
            Type   Value     { };

            /// \brief Serializes the state of the object to or from the specified archive.
            ///
            /// \param Archive The archive to serialize the object with.
            template<typename Serializer>
            ZY_INLINE void Serialize(Serializer Archive)
            {
                Archive.Serialize(Time);
                Archive.Serialize(Value);
            }
        };

        /// \brief The stops the run passes through, in the order it passes through them.
        using Stops = Sequence<Stop, Capacity>;

    public:

        /// \brief Constructs a run holding no stops, which reads as the fallback it is given.
        ZY_INLINE Ramp() = default;

        /// \brief Sets the stops the run passes through.
        ///
        /// \param Stops The stops to pass through, in the order they are worn.
        ZY_INLINE void SetStops(AnyRef<Stops> Stops)
        {
            mStops = Move(Stops);
        }

        /// \brief Gets the stops the run passes through.
        ///
        /// \return The stops, in the order they are worn.
        ZY_INLINE ConstRef<Stops> GetStops() const
        {
            return mStops;
        }

        /// \brief Adds one stop to the end of the run.
        ///
        /// \param Time  The moment the value is worn at.
        /// \param Value The value worn at that moment.
        ZY_INLINE void Append(Real32 Time, AnyRef<Type> Value)
        {
            mStops.Append(Time, Move(Value));
        }

        /// \brief Removes one stop from the run.
        ///
        /// \param Index The position of the stop to remove.
        ZY_INLINE void Remove(UInt Index)
        {
            mStops.Remove(Index);
        }

        /// \brief Gets one stop of the run.
        ///
        /// \param Index The position of the stop.
        /// \return The stop at that position.
        ZY_INLINE Ref<Stop> operator[](UInt Index)
        {
            return mStops[Index];
        }

        /// \brief Gets one stop of the run.
        ///
        /// \param Index The position of the stop.
        /// \return The stop at that position.
        ZY_INLINE ConstRef<Stop> operator[](UInt Index) const
        {
            return mStops[Index];
        }

        /// \brief Gets how many stops the run passes through.
        ///
        /// \return The count of stops.
        ZY_INLINE UInt GetSize() const
        {
            return mStops.GetSize();
        }

        /// \brief Checks whether the run holds no stops.
        ///
        /// \return `true` when the run holds nothing, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mStops.IsEmpty();
        }

        /// \brief Checks whether the run holds every stop it has room for.
        ///
        /// \return `true` when no further stop fits, otherwise `false`.
        ZY_INLINE Bool IsFull() const
        {
            if constexpr (Capacity > 0)
            {
                return mStops.IsFull();
            }
            else
            {
                return false;
            }
        }

        /// \brief Gets the value worn at a moment, blended between the stops either side of it.
        ///
        /// \param Time     The moment to read the run at.
        /// \param Fallback The value to read when the run holds no stops at all.
        /// \return The value worn at that moment, held flat before the first stop and after the last.
        ZY_INLINE Type Sample(Real32 Time, Type Fallback = Type()) const
        {
            if (mStops.IsEmpty())
            {
                return Fallback;
            }

            for (UInt Index = 1; Index < mStops.GetSize(); ++Index)
            {
                ConstRef<Stop> Before = mStops[Index - 1];
                ConstRef<Stop> After  = mStops[Index];

                if (Time <= After.Time)
                {
                    const Real32 Span   = After.Time - Before.Time;
                    const Real32 Amount = Span > 0.0f ? Clamp((Time - Before.Time) / Span, 0.0f, 1.0f) : 0.0f;

                    return Blend(Before.Value, After.Value, Amount);
                }
            }
            return mStops.GetBack().Value;
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mStops);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        /// \brief Blends two values of the run, by whichever means the value itself provides.
        ///
        /// \param First  The value blended from.
        /// \param Second The value blended toward.
        /// \param Amount The share of the way from the first to the second.
        /// \return The value that share of the way along.
        ZY_INLINE static Type Blend(ConstRef<Type> First, ConstRef<Type> Second, Real32 Amount)
        {
            if constexpr (requires { Type::Lerp(First, Second, Amount); })
            {
                return Type::Lerp(First, Second, Amount);
            }
            else
            {
                return First + (Second - First) * Amount;
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Stops mStops;
    };
}