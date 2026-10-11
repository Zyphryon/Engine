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

inline namespace ZyBase
{
    /// \brief Represents a lightweight command journal for recording and executing deferred operations.
    class Journal final
    {
    public:

        /// \brief Constructs an empty journal with zero capacity.
        ZY_INLINE Journal() = default;

        /// \brief Constructs a journal with pre-allocated buffer capacity.
        ///
        /// \param Capacity The initial buffer capacity in bytes.
        ZY_INLINE Journal(UInt Capacity)
            : mBuffer { Capacity }
        {
        }

        /// \brief Gets the current size of recorded commands in bytes.
        ///
        /// \return The total byte size of all recorded commands including metadata.
        ZY_INLINE UInt GetSize() const
        {
            return mBuffer.GetSize();
        }

        /// \brief Gets the current buffer capacity in bytes.
        ///
        /// \return The allocated buffer capacity.
        ZY_INLINE UInt GetCapacity() const
        {
            return mBuffer.GetCapacity();
        }

        /// \brief Checks whether the journal holds no recorded commands.
        ///
        /// \return `true` if no commands are recorded, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mBuffer.IsEmpty();
        }

        /// \brief Reserves buffer capacity without recording commands.
        ///
        /// \param Capacity The desired buffer capacity in bytes.
        ZY_INLINE void Reserve(UInt Capacity)
        {
            mBuffer.Reserve(Capacity);
        }

        /// \brief Trims the buffer down to the bytes recorded, leaving it untouched when nothing is recorded.
        ZY_INLINE void Shrink()
        {
            mBuffer.Shrink();
        }

        /// \brief Records a callable object for deferred execution.
        ///
        /// \param Callback The callable object to record.
        template<typename Callable>
        ZY_INLINE void Record(AnyRef<Callable> Callback)
        {
            using Payload = StripAll<Callable>;

            static constexpr UInt32    Stride   = AlignPowTwo(sizeof(Payload), alignof(OnExecute));
            static constexpr OnExecute Function = [](Ref<Ptr<Byte>> Cursor)
            {
                Ref<Payload> Command = * Launder(reinterpret_cast<Ptr<Payload>>(Cursor));
                Command();

                Destruct(Command);

                Cursor += Stride;
            };

            const Ptr<Byte> Allocation = Allocate(sizeof(OnExecute), alignof(OnExecute));
            Blit(Allocation, sizeof(OnExecute), AddressOf(Function));

            Ptr<Byte> Memory = Allocate(Stride, alignof(Payload));
            Construct<Payload>(reinterpret_cast<Ptr<Payload>>(Memory), Forward<Callable>(Callback));
        }

        /// \brief Records a deferred method call on a target object.
        ///
        /// \tparam Method     The member function to invoke.
        /// \param  Target     The object instance on which to invoke the method.
        /// \param  Parameters The arguments to forward to the method when executed.
        template<auto Method, typename Object, typename... Arguments>
        ZY_INLINE void Record(Ptr<Object> Target, AnyRef<Arguments>... Parameters)
        {
            Record([Target, ...Parameters = Forward<Arguments>(Parameters)]() mutable
            {
                (Target->*Method)(Parameters...);
            });
        }

        /// \brief Executes all recorded commands in FIFO order and clears the journal.
        ZY_INLINE void Run()
        {
            for (Ptr<Byte> Cursor = mBuffer.GetData(), End = Cursor + mBuffer.GetSize(); Cursor < End;)
            {
                const OnExecute Function = (* reinterpret_cast<Ptr<OnExecute>>(Cursor));
                Cursor += sizeof(OnExecute);

                Function(Cursor);
            }
            mBuffer.Clear();
        }

    private:

        /// \brief Function pointer type for command execution trampolines.
        using OnExecute = void(*)(Ref<Ptr<Byte>>);

        /// \brief Allocates aligned memory from the journal buffer.
        ///
        /// \param Size      The number of bytes to allocate.
        /// \param Alignment The required alignment boundary in bytes.
        /// \return The pointer to the allocated memory region.
        ZY_INLINE Ptr<Byte> Allocate(UInt32 Size, UInt32 Alignment)
        {
            const UInt32 Offset = AlignPowTwo(static_cast<UInt32>(mBuffer.GetSize()), Alignment);
            const UInt32 Length = Offset + Size;

            mBuffer.Advance(Length - mBuffer.GetSize());
            return mBuffer.GetData() + Offset;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Byte> mBuffer;
    };
}