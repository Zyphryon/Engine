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

namespace Platform
{
    /// \brief Represents a physical display monitor and its properties.
    class Monitor final
    {
    public:

        /// \brief Platform-specific handle to the monitor.
        using Handle = Ptr<void>;

        /// \brief Specifies the characteristics of a monitor.
        enum class Attribute : UInt8
        {
            None      = 0,          ///< No special attributes.
            Primary   = 1 << 0,     ///< The monitor is the primary display.
            HDR       = 1 << 1,     ///< The monitor supports HDR.
        };
        ZY_DEFINE_BITWISE_FRIEND_ENUM(Attribute)

    public:

        /// \brief Constructs a monitor with the specified platform handle.
        ///
        /// \param Handle The platform-specific handle to the monitor.
        ZY_INLINE Monitor(Handle Handle)
            : mHandle     { Handle },
              mX          { 0 },
              mY          { 0 },
              mWidth      { 0 },
              mHeight     { 0 },
              mScale      { 1.0f },
              mFrequency  { 0 },
              mAttributes { Attribute::None }
        {
        }

        /// \brief Gets the platform-specific handle.
        ///
        /// \return The platform-specific handle to the monitor.
        ZY_INLINE Handle GetHandle() const
        {
            return mHandle;
        }

        /// \brief Sets the monitor name.
        ///
        /// \param Name The human-readable name to assign to the monitor.
        ZY_INLINE void SetName(Text Name)
        {
            mName = Name;
        }

        /// \brief Gets the monitor name.
        ///
        /// \return The human-readable name of the monitor.
        ZY_INLINE Text GetName() const
        {
            return mName;
        }

        /// \brief Sets the horizontal position in virtual screen space.
        ///
        /// \param X The X coordinate to assign to the monitor.
        ZY_INLINE void SetX(SInt32 X)
        {
            mX = X;
        }

        /// \brief Gets the horizontal position in virtual screen space.
        ///
        /// \return The X coordinate of the monitor.
        ZY_INLINE SInt32 GetX() const
        {
            return mX;
        }

        /// \brief Sets the vertical position in virtual screen space.
        ///
        /// \param Y The Y coordinate to assign to the monitor.
        ZY_INLINE void SetY(SInt32 Y)
        {
            mY = Y;
        }

        /// \brief Gets the vertical position in virtual screen space.
        ///
        /// \return The Y coordinate of the monitor.
        ZY_INLINE SInt32 GetY() const
        {
            return mY;
        }

        /// \brief Sets the width in pixels.
        ///
        /// \param Width The width to assign to the monitor.
        ZY_INLINE void SetWidth(UInt32 Width)
        {
            mWidth = Width;
        }

        /// \brief Gets the width in pixels.
        ///
        /// \return The width of the monitor.
        ZY_INLINE UInt32 GetWidth() const
        {
            return mWidth;
        }

        /// \brief Sets the height in pixels.
        ///
        /// \param Height The height to assign to the monitor.
        ZY_INLINE void SetHeight(UInt32 Height)
        {
            mHeight = Height;
        }

        /// \brief Gets the height in pixels.
        ///
        /// \return The height of the monitor.
        ZY_INLINE UInt32 GetHeight() const
        {
            return mHeight;
        }

        /// \brief Sets the refresh rate.
        ///
        /// \param Frequency The refresh rate in Hertz to assign to the monitor.
        ZY_INLINE void SetFrequency(UInt16 Frequency)
        {
            mFrequency = Frequency;
        }

        /// \brief Gets the refresh rate.
        ///
        /// \return The refresh rate in Hertz.
        ZY_INLINE UInt16 GetFrequency() const
        {
            return mFrequency;
        }

        /// \brief Sets the DPI scaling factor.
        ///
        /// \param Scale The scaling factor to assign to the monitor.
        ZY_INLINE void SetScale(Real32 Scale)
        {
            mScale = Scale;
        }

        /// \brief Gets the DPI scaling factor.
        ///
        /// \return The scaling factor applied to the monitor.
        ZY_INLINE Real32 GetScale() const
        {
            return mScale;
        }

        /// \brief Sets an attribute for the monitor.
        ///
        /// \param Attribute The attribute to assign to the monitor.
        ZY_INLINE void SetAttribute(Attribute Attribute)
        {
            mAttributes = SetBit(mAttributes, Attribute);
        }

        /// \brief Checks whether the monitor is the primary monitor.
        ///
        /// \return `true` if the monitor is primary, otherwise `false`.
        ZY_INLINE Bool IsPrimary() const
        {
            return HasBit(mAttributes, Attribute::Primary);
        }

        /// \brief Checks whether the monitor supports HDR.
        ///
        /// \return `true` if the monitor supports HDR, otherwise `false`.
        ZY_INLINE Bool IsHDR() const
        {
            return HasBit(mAttributes, Attribute::HDR);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle    mHandle;
        Str       mName;
        SInt32    mX;
        SInt32    mY;
        UInt32    mWidth;
        UInt32    mHeight;
        Real32    mScale;
        UInt16    mFrequency;
        Attribute mAttributes;
    };
}