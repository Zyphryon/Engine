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

#include "OGLDevice.hpp"
#include "OGLCommon.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Represents an OpenGL buffer and the writes the driver makes into it.
    class Buffer final
    {
    public:

        /// \brief Constructs an empty buffer that owns no OpenGL object until \ref Create.
        Buffer();

        /// \brief Creates the buffer object and fills it with its initial contents.
        ///
        /// \param Device   The device whose uniform alignment a uniform buffer's size is rounded up to.
        /// \param Storage  The update frequency the buffer is hinted with.
        /// \param Usage    The bindings the buffer is drawn from.
        /// \param Capacity The size of the buffer, in bytes.
        /// \param Data     The initial contents, or empty to leave the buffer undefined.
        void Create(ConstRef<Device> Device, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data);

        /// \brief Writes a range of the buffer.
        ///
        /// \param Offset The byte offset the write starts at.
        /// \param Data   The bytes to write.
        void Update(UInt32 Offset, ConstSpan<Byte> Data);

        /// \brief Deletes the buffer object and returns to the empty state.
        void Delete();

        /// \brief Maps a range of the buffer for writing, discarding its previous contents.
        ///
        /// \note A dynamic buffer first waits for the GPU to finish the copy its last \ref Unmap issued.
        ///
        /// \param Offset The byte offset the range starts at.
        /// \param Size   The size of the range, in bytes.
        /// \return The address the range is written through, or `nullptr` where mapping is unsupported.
        Ptr<Byte> Map(UInt32 Offset, UInt32 Size);

        /// \brief Ends the write started by \ref Map.
        ///
        /// \param Size The number of bytes written from the start of the mapped range.
        void Unmap(UInt32 Size);

        /// \brief Gets the buffer object.
        ///
        /// \return The OpenGL name of the buffer, `0` when none was created.
        ZY_INLINE GLuint GetHandle() const
        {
            return mHandle;
        }

        /// \brief Gets the target the buffer is bound to when drawn from.
        ///
        /// \return The binding target the buffer's usage maps to.
        ZY_INLINE GLenum GetTarget() const
        {
            return mTarget;
        }

    private:

        /// The longest a single wait on a fence lasts before it is retried, in nanoseconds.
        static constexpr GLuint64 kTimeout = 1'000'000'000;

        /// \brief Blocks until the GPU has finished the copy the last unmap made out of the staging memory.
        void Wait() const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        GLuint    mHandle;
        GLuint    mStaging;
        GLenum    mTarget;
        Ptr<Byte> mMemory;
        GLsync    mFence;
        UInt32    mOffset;
    };
}