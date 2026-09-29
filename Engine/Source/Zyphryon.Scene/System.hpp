// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    class World;

    /// \brief Represents a system, which a phase runs each time the world progresses.
    class ZY_API System final
    {
    public:

        /// \brief Constructs a system that names nothing.
        ZY_INLINE constexpr System()
            : mWorld  { nullptr },
              mHandle { 0 }
        {
        }

        /// \brief Constructs a system of a world.
        ///
        /// \param Owner  The world the system runs in.
        /// \param Handle The handle the world knows it by.
        ZY_INLINE constexpr System(Ptr<World> Owner, UInt32 Handle)
            : mWorld  { Owner },
              mHandle { Handle }
        {
        }

        /// \brief Checks whether the system names one of a world.
        ///
        /// \return `true` if it does, `false` for one that names nothing.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mHandle != 0;
        }

        /// \brief Gets the handle the world knows the system by.
        ///
        /// \return The handle, or zero for none.
        ZY_INLINE constexpr UInt32 GetHandle() const
        {
            return mHandle;
        }

        /// \brief Destroys the system, which runs no more.
        void Destruct();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<World> mWorld;
        UInt32     mHandle;
    };
}