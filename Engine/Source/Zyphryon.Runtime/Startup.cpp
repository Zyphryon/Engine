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

#include "Startup.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Runtime
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Startup::Startup()
        : mWindowTitle           { "Undefined" },
          mWindowWidth           { 1024 },
          mWindowHeight          { 768 },
          mWindowFullscreen      { false },
          mWindowBorderless      { false },
          mAudioPauseOnFocusLost { false },
          mGraphicsTearless      { true  },
          mGraphicsColorFormat   { Graphic::TextureFormat::Unspecified },
          mGraphicsDepthFormat   { Graphic::TextureFormat::Unspecified }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Startup::Load(JsonObject Root)
    {
        if (const JsonObject Section = Root.GetObject("Window"); Section.IsValid())
        {
            SetWindowMonitor(Section.GetString("monitor", GetWindowMonitor()));
            SetWindowWidth(Section.GetNumber<UInt32>("width", GetWindowWidth()));
            SetWindowHeight(Section.GetNumber<UInt32>("height", GetWindowHeight()));
            SetWindowFullscreen(Section.GetBool("fullscreen", IsWindowFullscreen()));
        }

        if (const JsonObject Section = Root.GetObject("Graphic"); Section.IsValid())
        {
            SetGraphicsAdapter(Section.GetString("adapter", GetGraphicsDevice()));
            SetGraphicsTearless(Section.GetBool("tearless", IsGraphicsTearless()));
        }

        if (const JsonObject Section = Root.GetObject("Audio"); Section.IsValid())
        {
            SetAudioAdapter(Section.GetString("adapter", GetAudioAdapter()));
            SetAudioPauseOnFocusLost(Section.GetBool("pause", IsAudioPauseOnFocusLost()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Startup::Save(JsonObject Root) const
    {
        JsonObject Window = Root.SetObject("Window");
        Window.SetString("monitor", GetWindowMonitor());
        Window.SetNumber("width", GetWindowWidth());
        Window.SetNumber("height", GetWindowHeight());
        Window.SetBool("fullscreen", IsWindowFullscreen());

        JsonObject Graphic = Root.SetObject("Graphic");
        Graphic.SetString("adapter", GetGraphicsDevice());
        Graphic.SetBool("tearless", IsGraphicsTearless());

        JsonObject Audio = Root.SetObject("Audio");
        Audio.SetString("adapter", GetAudioAdapter());
        Audio.SetBool("pause", IsAudioPauseOnFocusLost());
    }
}