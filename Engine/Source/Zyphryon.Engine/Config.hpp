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

#include "Zyphryon.Graphic/Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Engine
{
    /// \brief Configuration parameters used to initialize the engine at startup.
    class Config final
    {
    public:

        /// \brief Constructs a new set of engine configuration parameters with default values.
        Config();

        /// \brief Sets the target monitor for the window.
        ///
        /// \param Name The name of the monitor to use for window placement.
        ZY_INLINE void SetWindowMonitor(Text Name)
        {
            mWindowMonitor = Name;
        }

        /// \brief Gets the target monitor for the window.
        ///
        /// \return The window monitor to use for window placement.
        ZY_INLINE Text GetWindowMonitor() const
        {
            return mWindowMonitor;
        }

        /// \brief Sets the window title string.
        ///
        /// \param Title The window title as UTF-8 text.
        ZY_INLINE void SetWindowTitle(Text Title)
        {
            mWindowTitle = Title;
        }

        /// \brief Gets the current window title.
        ///
        /// \return The window title as UTF-8 text.
        ZY_INLINE Text GetWindowTitle() const
        {
            return mWindowTitle;
        }

        /// \brief Sets the width of the window in pixels.
        ///
        /// \param Width The new window width in pixels.
        ZY_INLINE void SetWindowWidth(UInt32 Width)
        {
            mWindowWidth = Width;
        }

        /// \brief Gets the window width.
        ///
        /// \return The width of the window in pixels.
        ZY_INLINE UInt32 GetWindowWidth() const
        {
            return mWindowWidth;
        }

        /// \brief Sets the height of the window in pixels.
        ///
        /// \param Height The new window height in pixels.
        ZY_INLINE void SetWindowHeight(UInt32 Height)
        {
            mWindowHeight = Height;
        }

        /// \brief Gets the window height.
        ///
        /// \return The height of the window in pixels.
        ZY_INLINE UInt32 GetWindowHeight() const
        {
            return mWindowHeight;
        }

        /// \brief Enables or disables fullscreen mode.
        ///
        /// \param Fullscreen `true` to enable fullscreen, `false` to disable.
        ZY_INLINE void SetWindowFullscreen(Bool Fullscreen)
        {
            mWindowFullscreen = Fullscreen;
        }

        /// \brief Checks if fullscreen mode is enabled.
        ///
        /// \return `true` if fullscreen is enabled, `false` otherwise.
        ZY_INLINE Bool IsWindowFullscreen() const
        {
            return mWindowFullscreen;
        }

        /// \brief Enables or disables borderless window mode.
        ///
        /// \param Borderless `true` to enable borderless mode, `false` to disable.
        ZY_INLINE void SetWindowBorderless(Bool Borderless)
        {
            mWindowBorderless = Borderless;
        }

        /// \brief Checks if borderless window mode is enabled.
        ///
        /// \return `true` if borderless mode is enabled, `false` otherwise.
        ZY_INLINE Bool IsWindowBorderless() const
        {
            return mWindowBorderless;
        }

        /// \brief Sets the name of the audio API driver.
        ///
        /// \param Driver The name of the audio driver backend to use.
        ZY_INLINE void SetAudioDriver(Text Driver)
        {
            mAudioDriver = Driver;
        }

        /// \brief Gets the active audio driver name.
        ///
        /// \return The name of the audio driver backend.
        ZY_INLINE Text GetAudioDriver() const
        {
            return mAudioDriver;
        }

        /// \brief Sets the preferred audio output device.
        ///
        /// \param Adapter The identifier or name of the audio adapter.
        ZY_INLINE void SetAudioAdapter(Text Adapter)
        {
            mAudioAdapter = Adapter;
        }

        /// \brief Gets the selected audio adapter.
        ///
        /// \return The identifier or name of the audio adapter.
        ZY_INLINE Text GetAudioAdapter() const
        {
            return mAudioAdapter;
        }

        /// \brief Enables or disables pausing audio when the window loses focus.
        ///
        /// \param Activate `true` to pause audio on focus lost, `false` to ignore.
        ZY_INLINE void SetAudioPauseOnFocusLost(Bool Activate)
        {
            mAudioPauseOnFocusLost = Activate;
        }

        /// \brief Gets whether audio is paused when the window loses focus.
        ///
        /// \return `true` if audio pauses on focus lost, `false` otherwise.
        ZY_INLINE Bool IsAudioPauseOnFocusLost() const
        {
            return mAudioPauseOnFocusLost;
        }

        /// \brief Sets the name of the graphics API driver.
        ///
        /// \param Driver The name of the graphics driver backend to use.
        ZY_INLINE void SetGraphicsDriver(Text Driver)
        {
            mGraphicsDriver = Driver;
        }

        /// \brief Gets the active graphics driver name.
        ///
        /// \return The name of the graphics driver backend.
        ZY_INLINE Text GetGraphicsDriver() const
        {
            return mGraphicsDriver;
        }

        /// \brief Sets the graphics adapter to be used.
        ///
        /// \param Adapter The identifier of the graphics adapter.
        ZY_INLINE void SetGraphicsAdapter(Text Adapter)
        {
            mGraphicsAdapter = Adapter;
        }

        /// \brief Gets the selected graphics adapter.
        ///
        /// \return The identifier of the graphics adapter.
        ZY_INLINE Text GetGraphicsDevice() const
        {
            return mGraphicsAdapter;
        }

        /// \brief Sets the graphics color format.
        ///
        /// \param Format The desired color format for the graphics.
        ZY_INLINE void SetGraphicsColorFormat(Graphic::TextureFormat Format)
        {
            mGraphicsColorFormat = Format;
        }

        /// \brief Gets the current graphics color format.
        ///
        /// \return The current color format used by the graphics system.
        ZY_INLINE Graphic::TextureFormat GetGraphicsColorFormat() const
        {
            return mGraphicsColorFormat;
        }

        /// \brief Sets the graphics depth format.
        ///
        /// \param Format The desired depth format for the graphics.
        ZY_INLINE void SetGraphicsDepthFormat(Graphic::TextureFormat Format)
        {
            mGraphicsDepthFormat = Format;
        }

        /// \brief Gets the current graphics depth format.
        ///
        /// \return The current depth format used by the graphics system.
        ZY_INLINE Graphic::TextureFormat GetGraphicsDepthFormat() const
        {
            return mGraphicsDepthFormat;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Str                    mWindowMonitor;
        Str                    mWindowTitle;
        UInt32                 mWindowWidth;
        UInt32                 mWindowHeight;
        Bool                   mWindowFullscreen;
        Bool                   mWindowBorderless;
        Str16                  mAudioDriver;
        Str                    mAudioAdapter;
        Bool                   mAudioPauseOnFocusLost;
        Str16                  mGraphicsDriver;
        Str                    mGraphicsAdapter;
        Graphic::TextureFormat mGraphicsColorFormat;
        Graphic::TextureFormat mGraphicsDepthFormat;
    };
}
