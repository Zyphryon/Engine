// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Base/Primitive.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Engine
{
    /// \brief Represents the configuration parameters used to initialize the engine.
    class Properties final
    {
    public:

        /// \brief Constructs a new set of engine properties with default values.
        Properties();

        /// \brief Sets the window system handle.
        ///
        /// \param Handle Native platform-specific window handle.
        ZYPHRYON_INLINE void SetWindowHandle(UInt Handle)
        {
            mWindowHandle = Handle;
        }

        /// \brief Returns the native window handle.
        ///
        /// \return Native platform-specific window handle.
        ZYPHRYON_INLINE UInt GetWindowHandle() const
        {
            return mWindowHandle;
        }

        /// \brief Sets the window title string.
        ///
        /// \param Title Window title as UTF-8 text.
        ZYPHRYON_INLINE void SetWindowTitle(ConstStr8 Title)
        {
            mWindowTitle = Title;
        }

        /// \brief Returns the current window title.
        ///
        /// \return Window title as UTF-8 text.
        ZYPHRYON_INLINE ConstStr8 GetWindowTitle() const
        {
            return mWindowTitle;
        }

        /// \brief Sets the width of the window in pixels.
        ///
        /// \param Width New window width in pixels.
        ZYPHRYON_INLINE void SetWindowWidth(UInt16 Width)
        {
            mWindowWidth = Width;
        }

        /// \brief Returns the window width.
        ///
        /// \return Width of the window in pixels.
        ZYPHRYON_INLINE UInt16 GetWindowWidth() const
        {
            return mWindowWidth;
        }

        /// \brief Sets the height of the window in pixels.
        ///
        /// \param Height New window height in pixels.
        ZYPHRYON_INLINE void SetWindowHeight(UInt16 Height)
        {
            mWindowHeight = Height;
        }

        /// \brief Returns the window height.
        ///
        /// \return Height of the window in pixels.
        ZYPHRYON_INLINE UInt16 GetWindowHeight() const
        {
            return mWindowHeight;
        }

        /// \brief Sets the number of samples used for window multisampling.
        ///
        /// \param Samples New multisample count for the window.
        ZYPHRYON_INLINE void SetWindowSamples(UInt8 Samples)
        {
            mWindowSamples = Samples;
        }

        /// \brief Retrieves the current multisample count of the window.
        ///
        /// \return Number of samples used for window multisampling.
        ZYPHRYON_INLINE UInt8 GetWindowSamples() const
        {
            return mWindowSamples;
        }

        /// \brief Enables or disables fullscreen mode.
        ///
        /// \param Fullscreen `true` to enable fullscreen, `false` to disable.
        ZYPHRYON_INLINE void SetWindowFullscreen(Bool Fullscreen)
        {
            mWindowFullscreen = Fullscreen;
        }

        /// \brief Checks if fullscreen mode is enabled.
        ///
        /// \return `true` if fullscreen is enabled, `false` otherwise.
        ZYPHRYON_INLINE Bool IsWindowFullscreen() const
        {
            return mWindowFullscreen;
        }

        /// \brief Enables or disables borderless window mode.
        ///
        /// \param Borderless `true` to enable borderless mode, `false` to disable.
        ZYPHRYON_INLINE void SetWindowBorderless(Bool Borderless)
        {
            mWindowBorderless = Borderless;
        }

        /// \brief Checks if borderless window mode is enabled.
        ///
        /// \return `true` if borderless mode is enabled, `false` otherwise.
        ZYPHRYON_INLINE Bool IsWindowBorderless() const
        {
            return mWindowBorderless;
        }

        /// \brief Sets the preferred audio output device.
        ///
        /// \param Device Identifier or name of the audio device.
        ZYPHRYON_INLINE void SetAudioDevice(ConstStr8 Device)
        {
            mAudioDevice = Device;
        }

        /// \brief Returns the selected audio device.
        ///
        /// \return Identifier or name of the audio device.
        ZYPHRYON_INLINE ConstStr8 GetAudioDevice() const
        {
            return mAudioDevice;
        }

        /// \brief Enables or disables pausing audio when the window loses focus.
        ///
        /// \param Activate `true` to pause audio on focus lost, `false` to ignore.
        ZYPHRYON_INLINE void SetAudioPauseOnFocusLost(Bool Activate)
        {
            mAudioPauseOnFocusLost = Activate;
        }

        /// \brief Returns whether audio is paused when the window loses focus.
        ///
        /// \return `true` if audio pauses on focus lost, `false` otherwise.
        ZYPHRYON_INLINE Bool GetAudioPauseOnFocusLost() const
        {
            return mAudioPauseOnFocusLost;
        }

        /// \brief Sets the name of the graphics API driver.
        ///
        /// \param Driver Driver name or backend (e.g., "Direct3D11", "OpenGL").
        ZYPHRYON_INLINE void SetVideoDriver(ConstStr8 Driver)
        {
            mVideoDriver = Driver;
        }

        /// \brief Returns the active graphics driver.
        ///
        /// \return Driver name (e.g., "Direct3D11", "OpenGL").
        ZYPHRYON_INLINE ConstStr8 GetVideoDriver() const
        {
            return mVideoDriver;
        }

        /// \brief Sets the GPU or video device to be used.
        ///
        /// \param Device Identifier of the video device.
        ZYPHRYON_INLINE void SetVideoDevice(ConstStr8 Device)
        {
            mVideoDevice = Device;
        }

        /// \brief Returns the selected video device.
        ///
        /// \return Identifier of the video device.
        ZYPHRYON_INLINE ConstStr8 GetVideoDevice() const
        {
            return mVideoDevice;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt   mWindowHandle;
        Str8   mWindowTitle;
        UInt16 mWindowWidth;
        UInt16 mWindowHeight;
        UInt8  mWindowSamples;
        Bool   mWindowFullscreen;
        Bool   mWindowBorderless;
        Str8   mAudioDevice;
        Bool   mAudioPauseOnFocusLost;
        Str8   mVideoDriver;
        Str8   mVideoDevice;
    };
}
