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

#include "D3D11Device.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    /// \brief Represents a Direct3D 11 buffer.
    class Buffer final
    {
    public:

        /// \brief Creates the buffer and fills it with its initial contents.
        ///
        /// \param Device   The device the buffer is created on.
        /// \param Storage  The update frequency the buffer is created for.
        /// \param Usage    The bindings the buffer is drawn from.
        /// \param Capacity The size of the buffer, in bytes.
        /// \param Data     The initial contents, or empty to leave the buffer undefined.
        void Create(ConstRef<Device> Device, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data);

        /// \brief Writes a range of the buffer.
        ///
        /// \param Device The device whose context records the write.
        /// \param Offset The byte offset the write starts at.
        /// \param Data   The bytes to write.
        void Update(ConstRef<Device> Device, UInt32 Offset, ConstSpan<Byte> Data);

        /// \brief Releases the buffer and returns to the empty state.
        void Delete();

        /// \brief Maps the whole buffer for writing, discarding its previous contents.
        ///
        /// \param Device The device whose context maps the buffer.
        /// \return The address the buffer is written through, or `nullptr` when mapping failed.
        Ptr<Byte> Map(ConstRef<Device> Device);

        /// \brief Ends the write started by \ref Map.
        ///
        /// \param Device The device whose context unmaps the buffer.
        void Unmap(ConstRef<Device> Device);

        /// \brief Gets the buffer resource.
        ///
        /// \return The buffer, or `nullptr` when none was created.
        ZY_INLINE Ptr<ID3D11Buffer> GetHandle() const
        {
            return mHandle.Get();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ComPtr<ID3D11Buffer> mHandle;
    };
}