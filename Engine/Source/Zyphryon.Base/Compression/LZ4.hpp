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

#include "Zyphryon.Base/Container/Span.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief The lowest search effort \ref LZ4Encode accepts.
    inline constexpr UInt32 kLZ4LevelMin = 1;

    /// \brief The highest search effort \ref LZ4Encode accepts; beyond this the ratio stops moving.
    inline constexpr UInt32 kLZ4LevelMax = 12;

    /// \brief Computes the worst-case compressed size for an input of the given length.
    ///
    /// \param Size The uncompressed input size, in bytes.
    /// \return The maximum number of bytes `LZ4Encode` may produce.
    ZY_INLINE constexpr UInt32 LZ4Bound(UInt32 Size)
    {
        return Size + (Size / 255) + 16;
    }

    /// \brief Encodes a buffer into a single LZ4 block.
    ///
    /// \note The encoder performs no bounds checks; sizing `Destination` below `LZ4Bound` is undefined behaviour.
    ///
    /// \param Source      The uncompressed input bytes.
    /// \param Destination The output buffer receiving the compressed bytes.
    /// \param Capacity    The capacity of the output buffer, in bytes (must be at least `LZ4Bound(Source size)`).
    /// \return The number of compressed bytes written.
    UInt32 LZ4Encode(ConstSpan<Byte> Source, Ptr<Byte> Destination, UInt32 Capacity);

    /// \brief Encodes a buffer into a single LZ4 block, searching much harder for matches.
    ///
    /// \note The encoder performs no bounds checks; sizing `Destination` below `LZ4Bound` is undefined behaviour.
    ///
    /// \param Source      The uncompressed input bytes.
    /// \param Destination The output buffer receiving the compressed bytes.
    /// \param Capacity    The capacity of the output buffer, in bytes (must be at least `LZ4Bound(Source size)`).
    /// \param Level       How far to walk each chain, clamped to [`kLZ4LevelMin`, `kLZ4LevelMax`].
    /// \return The number of compressed bytes written.
    UInt32 LZ4Encode(ConstSpan<Byte> Source, Ptr<Byte> Destination, UInt32 Capacity, UInt32 Level);

    /// \brief Decodes a single LZ4 block into the destination buffer.
    ///
    /// \param Source      The compressed input bytes.
    /// \param Destination The output buffer receiving the decompressed bytes.
    /// \param Capacity    The capacity of the output buffer, in bytes.
    /// \return The number of bytes written, or `0` on malformed input or overflow.
    UInt32 LZ4Decode(ConstSpan<Byte> Source, Ptr<Byte> Destination, UInt32 Capacity);
}