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

#include "Zyphryon.Network/Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Represents a queue of bytes written at the back and taken from the front.
    class Pipe final
    {
    public:

        /// The bytes a length prefix occupies at the front of each frame.
        static constexpr UInt kHeader = sizeof(UInt32);

    public:

        /// \brief Specifies what the front of a pipe holds.
        enum class Front : UInt8
        {
            Bare,    ///< Not even a length has arrived yet.
            Partial, ///< A length arrived, with the frame it counts still short.
            Whole,   ///< A frame has entirely arrived, ready to be taken.
            Broken,  ///< A length arrived that no frame may be, which nothing after can be trusted past.
        };

    public:

        /// \brief Constructs an empty pipe.
        ZY_INLINE Pipe()
            : mReader { 0 },
              mWriter { 0 },
              mMarker { 0 }
        {
        }

        /// \brief Checks whether nothing is waiting to be taken.
        ///
        /// \return `true` if nothing is waiting, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mReader == mWriter;
        }

        /// \brief Checks whether a whole frame is waiting at the front.
        ///
        /// \return `true` if the frame at the front is entirely here and no larger than one may be, `false` otherwise.
        ZY_INLINE Bool IsWhole() const
        {
            return Inspect() == Front::Whole;
        }

        /// \brief Checks whether the front of the pipe announced a frame larger than one may be.
        ///
        /// \return `true` if the writer announced a frame that can never arrive, `false` otherwise.
        ZY_INLINE Bool IsBroken() const
        {
            return Inspect() == Front::Broken;
        }

        /// \brief Gets the bytes waiting to be taken.
        ///
        /// \return A pointer to the first byte not yet taken.
        ZY_INLINE ConstPtr<Byte> GetData() const
        {
            return mData.GetData() + mReader;
        }

        /// \brief Gets how much is waiting to be taken.
        ///
        /// \return The number of bytes not yet taken.
        ZY_INLINE UInt GetSize() const
        {
            return mWriter - mReader;
        }

        /// \brief Gets how much room the pipe is holding, taken or not.
        ///
        /// \return The number of bytes the pipe has room for.
        ZY_INLINE UInt GetCapacity() const
        {
            return mData.GetSize();
        }

        /// \brief Makes room at the back and hands it out to be written into.
        ///
        /// \note Paired with \ref Commit, which says how much of the room was actually written.
        ///
        /// \param Count The number of bytes to make room for.
        /// \return The room that was made.
        ZY_INLINE Span<Byte> Stage(UInt Count)
        {
            Reserve(Count);

            return Span(mData.GetData() + mWriter, Count);
        }

        /// \brief Takes as written the front of the room \ref Stage handed out, releasing the rest of it.
        ///
        /// \param Count The number of bytes written, which may not exceed the room asked for.
        ZY_INLINE void Commit(UInt Count)
        {
            mWriter += Count;
            mMarker  = Max(mMarker, GetSize());
        }

        /// \brief Appends bytes to the back.
        ///
        /// \param Data The bytes to append.
        ZY_INLINE void Append(ConstSpan<Byte> Data)
        {
            const Span<Byte> Room = Stage(Data.GetSize());

            Blit(Room.GetData(), Data.GetSize(), Data.GetData());
            Commit(Data.GetSize());
        }

        /// \brief Writes the length that opens a frame, leaving the bytes it counts to be appended after it.
        ///
        /// \param Length The number of bytes the frame holds behind its length.
        ZY_INLINE void Frame(UInt32 Length)
        {
            const Span<Byte> Room = Stage(kHeader);

            Room[0] = static_cast<Byte>(Length);
            Room[1] = static_cast<Byte>(Length >>  8);
            Room[2] = static_cast<Byte>(Length >> 16);
            Room[3] = static_cast<Byte>(Length >> 24);

            Commit(kHeader);
        }

        /// \brief Reads the length prefix waiting at the front, without taking anything.
        ///
        /// \return The number of bytes the frame at the front says it holds behind its length.
        ZY_INLINE UInt32 Peek() const
        {
            ZY_ASSERT(GetSize() >= kHeader, "No length is waiting at the front");

            const ConstPtr<Byte> Data = GetData();

            return static_cast<UInt32>(Data[0])
                 | static_cast<UInt32>(Data[1]) <<  8
                 | static_cast<UInt32>(Data[2]) << 16
                 | static_cast<UInt32>(Data[3]) << 24;
        }

        /// \brief Takes the whole frame waiting at the front, stepping past it.
        ///
        /// \return The payload of the frame taken, behind its length no longer.
        ZY_INLINE ConstSpan<Byte> Take()
        {
            ZY_ASSERT(IsWhole(), "No whole frame is waiting at the front");

            const ConstSpan Payload(GetData() + kHeader, Peek());

            Consume(kHeader + Payload.GetSize());
            return Payload;
        }

        /// \brief Drops bytes from the front.
        ///
        /// \note The bytes dropped are not disturbed until something further is written.
        ///
        /// \param Count The number of bytes to drop.
        ZY_INLINE void Consume(UInt Count)
        {
            ZY_ASSERT(Count <= GetSize(), "More was taken than was waiting");

            mReader += Count;

            if (mReader == mWriter)
            {
                mReader = 0;
                mWriter = 0;
            }
        }

        /// \brief Trades contents with another pipe.
        ///
        /// \param Other The pipe to trade with.
        ZY_INLINE void Swap(Ref<Pipe> Other)
        {
            Base::Swap(mData,   Other.mData);
            Base::Swap(mReader, Other.mReader);
            Base::Swap(mWriter, Other.mWriter);
            Base::Swap(mMarker, Other.mMarker);
        }

        /// \brief Drops everything waiting, keeping the room it was written into.
        ZY_INLINE void Clear()
        {
            mReader = 0;
            mWriter = 0;
            mMarker = 0;
        }

        /// \brief Gives back room the pipe has stopped needing.
        ZY_INLINE void Compact()
        {
            const UInt Left = GetSize();

            if (mReader > 0)
            {
                if (Left > 0)
                {
                    Blit(mData.GetData(), Left, mData.GetData() + mReader);
                }
                mReader = 0;
                mWriter = Left;
            }

            if (const UInt Room = Max(mMarker, Left, static_cast<UInt>(kMaxRead)); mData.GetSize() > Room)
            {
                mData.Remove(Room, mData.GetSize() - Room);
                mData.Shrink();
            }
            mMarker = Left;
        }

    private:

        /// \brief Gets how far along the frame at the front of the pipe is.
        ///
        /// \return How much of the frame at the front has arrived, or \ref Front::Broken when its length says it never will.
        ZY_INLINE Front Inspect() const
        {
            if (GetSize() < kHeader)
            {
                return Front::Bare;
            }

            const UInt32 Length = Peek();

            if (Length > kMaxFrame)
            {
                return Front::Broken;
            }
            return (GetSize() >= kHeader + Length) ? Front::Whole : Front::Partial;
        }

        /// \brief Ensures the back has room for the given number of bytes.
        ///
        /// \param Count The number of bytes the back must have room for.
        ZY_INLINE void Reserve(UInt Count)
        {
            if (mWriter + Count <= mData.GetSize())
            {
                return;
            }

            if (const UInt Left = GetSize(); mReader > 0)
            {
                if (Left > 0)
                {
                    Blit(mData.GetData(), Left, mData.GetData() + mReader);
                }
                mReader = 0;
                mWriter = Left;
            }

            if (const UInt Required = mWriter + Count; Required > mData.GetSize())
            {
                mData.Advance(Required - mData.GetSize());
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Byte> mData;
        UInt           mReader;
        UInt           mWriter;
        UInt           mMarker;
    };
}