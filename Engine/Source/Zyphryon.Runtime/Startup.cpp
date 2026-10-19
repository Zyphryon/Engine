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

namespace ZyRuntime
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Startup::Startup()
        : mHeadless { false }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Startup::Load(JsonObject Root)
    {
        if (const JsonObject Section = Root.GetObject("Window"); Section.IsValid())
        {
            mWindow.Monitor    = Section.GetString("monitor", mWindow.Monitor);
            mWindow.Width      = Section.GetNumber<UInt32>("width", mWindow.Width);
            mWindow.Height     = Section.GetNumber<UInt32>("height", mWindow.Height);
            mWindow.Fullscreen = Section.GetBool("fullscreen", mWindow.Fullscreen);
        }

        if (const JsonObject Section = Root.GetObject("Graphic"); Section.IsValid())
        {
            mGraphic.Driver   = Section.GetString("driver", mGraphic.Driver);
            mGraphic.Adapter  = Section.GetString("adapter", mGraphic.Adapter);
            mGraphic.Tearless = Section.GetBool("tearless", mGraphic.Tearless);
        }

        if (const JsonObject Section = Root.GetObject("Audio"); Section.IsValid())
        {
            mAudio.Adapter = Section.GetString("adapter", mAudio.Adapter);
            mAudio.Pause   = Section.GetBool("pause", mAudio.Pause);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Startup::Save(JsonObject Root) const
    {
        JsonObject Window = Root.SetObject("Window");
        Window.SetString("monitor", mWindow.Monitor);
        Window.SetNumber("width", mWindow.Width);
        Window.SetNumber("height", mWindow.Height);
        Window.SetBool("fullscreen", mWindow.Fullscreen);

        JsonObject Graphic = Root.SetObject("Graphic");
        Graphic.SetString("driver", mGraphic.Driver);
        Graphic.SetString("adapter", mGraphic.Adapter);
        Graphic.SetBool("tearless", mGraphic.Tearless);

        JsonObject Audio = Root.SetObject("Audio");
        Audio.SetString("adapter", mAudio.Adapter);
        Audio.SetBool("pause", mAudio.Pause);
    }
}