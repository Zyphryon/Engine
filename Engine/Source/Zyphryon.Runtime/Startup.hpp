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

namespace ZyRuntime
{
    /// \brief Holds the parameters the engine is brought up with, before any service exists.
    class ZY_API Startup final
    {
    public:

        /// \brief Describes the window the engine opens.
        struct Window final
        {
            /// The name of the monitor to open the window on, or empty for the primary one.
            Str    Monitor;

            /// The title, as UTF-8 text.
            Str    Title      = "Undefined";

            /// The width in pixels, before the monitor's scale is applied.
            UInt32 Width      = 1024;

            /// The height in pixels, before the monitor's scale is applied.
            UInt32 Height     = 768;

            /// `true` to cover the whole monitor, `false` to open a window.
            Bool   Fullscreen = false;

            /// `true` to open the window without a frame, `false` to open it framed.
            Bool   Borderless = false;
        };

        /// \brief Describes the device the engine plays sound through.
        struct Audio final
        {
            /// The name of the output device, or empty for the system's default.
            Str  Adapter;

            /// `true` to pause the sound while the window is out of focus, `false` to keep it playing.
            Bool Pause = false;
        };

        /// \brief Describes the device the engine draws with.
        struct Graphic final
        {
            /// The name of the driver to draw with, or empty for the preferred one.
            Str16                    Driver;

            /// The name of the adapter to draw on, or empty for the default one.
            Str                      Adapter;

            /// `true` when presentation waits for the vertical blank, `false` to present as soon as a frame is ready.
            Bool                     Tearless    = true;

            /// The color format of the swapchain, or `Unspecified` to pick one from the monitor.
            ZyGraphic::TextureFormat ColorFormat = ZyGraphic::TextureFormat::Unspecified;

            /// The format of the depth buffer, or `Unspecified` for none.
            ZyGraphic::TextureFormat DepthFormat = ZyGraphic::TextureFormat::Unspecified;
        };

    public:

        /// \brief Constructs a new set of engine configuration parameters with default values.
        Startup();

        /// \brief Reads the parameters a user is allowed to change from a document.
        ///
        /// \param Root The object the parameters were written under.
        void Load(JsonObject Root);

        /// \brief Writes the parameters a user is allowed to change into a document.
        ///
        /// \param Root The object to write the parameters under.
        void Save(JsonObject Root) const;

        /// \brief Sets whether the engine starts with no window, input, graphics, audio or rendering.
        ///
        /// \param Headless `true` to start without them, as a server does, `false` to start with them.
        ZY_INLINE void SetHeadless(Bool Headless)
        {
            mHeadless = Headless;
        }

        /// \brief Checks whether the engine starts with no window, input, graphics, audio or rendering.
        ///
        /// \return `true` if it starts without them, `false` otherwise.
        ZY_INLINE Bool IsHeadless() const
        {
            return mHeadless;
        }

        /// \brief Sets the window the engine opens.
        ///
        /// \param Window The window to open.
        ZY_INLINE void SetWindow(ConstRef<Window> Window)
        {
            mWindow = Window;
        }

        /// \brief Gets the window the engine opens.
        ///
        /// \return The window to open.
        ZY_INLINE ConstRef<Window> GetWindow() const
        {
            return mWindow;
        }

        /// \brief Sets the device the engine plays sound through.
        ///
        /// \param Audio The device to play through.
        ZY_INLINE void SetAudio(ConstRef<Audio> Audio)
        {
            mAudio = Audio;
        }

        /// \brief Gets the device the engine plays sound through.
        ///
        /// \return The device to play through.
        ZY_INLINE ConstRef<Audio> GetAudio() const
        {
            return mAudio;
        }

        /// \brief Sets the device the engine draws with.
        ///
        /// \param Graphic The device to draw with.
        ZY_INLINE void SetGraphic(ConstRef<Graphic> Graphic)
        {
            mGraphic = Graphic;
        }

        /// \brief Gets the device the engine draws with.
        ///
        /// \return The device to draw with.
        ZY_INLINE ConstRef<Graphic> GetGraphic() const
        {
            return mGraphic;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bool    mHeadless;
        Window  mWindow;
        Audio   mAudio;
        Graphic mGraphic;
    };
}