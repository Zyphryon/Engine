// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Application.hpp"
#include <Zyphryon.Content/Mount/DiskMount.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Example
{
    bool RENDER_NEW = false;

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Application::OnInitialize()
    {
        auto Content = GetService<Content::Service>();
        Content->AddMount("Resources", Tracker<Content::DiskMount>::Create("Resources"));

        AppRenderer = NewUniquePtr<Graphic::Render2D>(* this);

        AppCamera.SetOrthographic(0.0f, GetDevice().GetWidth(), 0.0f, GetDevice().GetHeight(), -1.0f, 1.0f);
        AppCamera.Compute();

        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Application::OnTick(ConstRef<Time> Time)
    {
        ZYPHRYON_PROFILE;

        ConstTracker<Graphic::Service> Graphics = GetService<Graphic::Service>();

        static Real32 Thickness = 1.0f;
        Thickness += (Time.GetDelta() * 1);

        Graphic::Viewport Viewport(0, 0, GetDevice().GetWidth(), GetDevice().GetHeight());
        Graphics->Prepare(Graphic::kDisplay, Viewport, Graphic::Clear::All, Color::Black(), 1, 0);
        {
            if (AppRenderer->IsReady())
            {
                AppRenderer->SetGlobalParameters(ConstSpan<Matrix4x4>(&AppCamera.GetViewProjection(), 1));

                AppRenderer->DrawLine(Line({100, 100}, {200, 100}), 0.0f, Color::Red().ToRGBA8(), Thickness);
                AppRenderer->DrawStrokeRect(Rect({100, 200, 200, 300}), 0.3f, Color::Green().ToRGBA8(), 4.0f);
                AppRenderer->DrawRect(Rect({100, 350, 200, 450}), 0.2f, Color::Blue().ToRGBA8());

                AppRenderer->DrawCircle(Circle({600, 600}, 128), 0.3f, Color::White().ToRGBA8());
                AppRenderer->DrawRing(Circle({200, 200}, 64), 0.4f, Color::Red().ToRGBA8(), 1);
                AppRenderer->DrawRing(Circle({400, 400}, 32), 0.5f, Color::Yellow().ToRGBA8(), 2);
                AppRenderer->DrawRing(Circle({300, 300}, 16), 0.6f, Color::Blue().ToRGBA8(), 3);
                AppRenderer->DrawLine(Line({0, 0}, {800, 800}), 0.1f, Color::Green().ToRGBA8(), Thickness);

                AppRenderer->Flush();
            }
        }
        Graphics->Commit(Graphic::kDisplay);
        Graphics->Finish(false);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Application::OnTeardown()
    {
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   MAIN   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int main([[maybe_unused]] int Argc, [[maybe_unused]] Ptr<Char> Argv[])
{
    Engine::Properties Properties;
    Properties.SetWindowTitle("Zyphryon Example");
    Properties.SetWindowWidth(1024);
    Properties.SetWindowHeight(768);
    Properties.SetWindowFullscreen(false);
    Properties.SetWindowBorderless(false);
    Properties.SetVideoDriver("D3D11");

    // Initialize 'Zyphryon Engine' and enter main loop
    Tracker<Example::Application> Application = Tracker<Example::Application>::Create();
    Application->Initialize(Service::Host::Mode::Client, Properties);
    Application->Run();

    return 0;
}