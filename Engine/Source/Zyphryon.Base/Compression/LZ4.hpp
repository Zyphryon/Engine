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
    /// \param Source      The uncompressed input bytes.
    /// \param Destination The output buffer receiving the compressed bytes.
    /// \param Capacity    The capacity of the output buffer, in bytes (must be at least `LZ4Bound(Source size)`).
    /// \return The number of compressed bytes written.
    /// \note  The encoder performs no bounds checks; sizing `Destination` below `LZ4Bound` is undefined behaviour.
    UInt32 LZ4Encode(ConstSpan<Byte> Source, Ptr<Byte> Destination, UInt32 Capacity);

    /// \brief Decodes a single LZ4 block into the destination buffer.
    ///
    /// \param Source      The compressed input bytes.
    /// \param Destination The output buffer receiving the decompressed bytes.
    /// \param Capacity    The capacity of the output buffer, in bytes.
    /// \return The number of bytes written, or `0` on malformed input or overflow.
    UInt32 LZ4Decode(ConstSpan<Byte> Source, Ptr<Byte> Destination, UInt32 Capacity);
}
