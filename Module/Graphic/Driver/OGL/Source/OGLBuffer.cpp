// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "OGLBuffer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static GLenum GetWriteTarget(GLenum Target)
    {
        // Writes avoid GL_ARRAY_BUFFER, which the binder points vertex attributes through.
        return (Target != GL_ELEMENT_ARRAY_BUFFER) ? GL_COPY_WRITE_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Buffer::Buffer()
        : mHandle  { 0 },
          mStaging { 0 },
          mTarget  { 0 },
          mMemory  { nullptr },
          mFence   { nullptr },
          mOffset  { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Buffer::Create(ConstRef<Device> Device, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data)
    {
        mTarget = Convert(Usage);

        const GLenum Target = GetWriteTarget(mTarget);

        glGenBuffers(1, AddressOf(mHandle));
        glBindBuffer(Target, mHandle);

        // A dynamic buffer is written through a second one mapped once, then copied, so draws never read host memory.
        if (Storage == Storage::Dynamic && Device.Has(Feature::BufferStorage))
        {
            constexpr GLbitfield Access = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

            glBufferStorage(Target, Capacity, nullptr, GL_DYNAMIC_STORAGE_BIT);

            glGenBuffers(1, AddressOf(mStaging));
            glBindBuffer(GL_COPY_READ_BUFFER, mStaging);
            glBufferStorage(GL_COPY_READ_BUFFER, Capacity, nullptr, Access | GL_CLIENT_STORAGE_BIT);
            mMemory = static_cast<Ptr<Byte>>(glMapBufferRange(GL_COPY_READ_BUFFER, 0, Capacity, Access));
        }
        else
        {
            glBufferData(Target, Capacity, nullptr, Convert(Storage));
        }

        if (!Data.IsEmpty())
        {
            glBufferSubData(Target, 0, Data.GetSizeInBytes(), Data.GetData());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Buffer::Update(UInt32 Offset, ConstSpan<Byte> Data)
    {
        const GLenum Target = GetWriteTarget(mTarget);

        glBindBuffer(Target, mHandle);
        glBufferSubData(Target, Offset, Data.GetSizeInBytes(), Data.GetData());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Buffer::Delete()
    {
        // Deleting a buffer also ends its mapping, and a name or fence of zero is ignored.
        glDeleteBuffers(1, AddressOf(mHandle));
        glDeleteBuffers(1, AddressOf(mStaging));
        glDeleteSync(mFence);

        mHandle  = 0;
        mStaging = 0;
        mTarget  = 0;
        mMemory  = nullptr;
        mFence   = nullptr;
        mOffset  = 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> Buffer::Map(UInt32 Offset, UInt32 Size)
    {
        if (mMemory)
        {
            // The last unmap's copy still reads the staging memory, so only that copy is waited for, not the frame.
            if (mFence)
            {
                Wait();
                glDeleteSync(mFence);

                mFence = nullptr;
            }
            mOffset = Offset;
            return mMemory + Offset;
        }

#if defined(ZY_PLATFORM_WEB)
        // WebGL maps nothing, so the service writes through its own memory and uploads that instead.
        return nullptr;
#else
        constexpr GLbitfield Access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

        glBindBuffer(GL_COPY_WRITE_BUFFER, mHandle);
        return static_cast<Ptr<Byte>>(glMapBufferRange(GL_COPY_WRITE_BUFFER, Offset, Size, Access));
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Buffer::Unmap(UInt32 Size)
    {
        // A persistent mapping stays open, and only the bytes written are copied to where draws read them.
        if (mMemory)
        {
            if (Size > 0)
            {
                glBindBuffer(GL_COPY_READ_BUFFER, mStaging);
                glBindBuffer(GL_COPY_WRITE_BUFFER, mHandle);
                glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, mOffset, mOffset, Size);

                mFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            }
            return;
        }

#if !defined(ZY_PLATFORM_WEB)
        glBindBuffer(GL_COPY_WRITE_BUFFER, mHandle);
        glUnmapBuffer(GL_COPY_WRITE_BUFFER);
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Buffer::Wait() const
    {
        // The first wait flushes, or a fence still queued on the CPU would never be reached.
        GLbitfield Flags = GL_SYNC_FLUSH_COMMANDS_BIT;

        while (glClientWaitSync(mFence, Flags, kTimeout) == GL_TIMEOUT_EXPIRED)
        {
            Flags = 0;
        }
    }
}