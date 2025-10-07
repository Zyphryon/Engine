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
#include "Zyphryon.Math/Transform.hpp"
#include "Zyphryon.Math/Geometry/Rect.hpp"
#include "Zyphryon.Scene/Builder.hpp"
#include "Zyphryon.Scene/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Example
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Application::OnInitialize()
    {
        ConstTracker<Scene::Service> Scene = GetService<Scene::Service>();
        Scene->GetComponent<Circle>();      // Register component
        Scene->GetComponent<Matrix4x4>();   // Register component

        // Create a query to fetch all circles.
        mEcsQueryCircles = Scene->CreateQuery<Scene::DSL::In<const Circle, const Matrix4x4>>("EcsQueryCircles", Scene::Cache::Auto);

        // Create some circles.
        for (UInt Count = 0; Count < 1'000; ++Count)
        {
            Scene::Entity Actor = Scene->CreateEntity();
            Actor.Emplace<Circle>(Vector2(0, 0), 1.0f);
            Actor.Set(Matrix4x4::FromTranslation(
                Vector2(
                    Random::Default().NextInteger<UInt16>(0, 1000),
                    Random::Default().NextInteger<UInt16>(0, 1000))));
        }

        // (Until Fix) -> wait for the renderer to load
        mRenderer = NewUniquePtr<Graphic::Render2D>(* this);
        while (!mRenderer->IsReady())
        {
            GetService<Content::Service>()->OnTick({});
        }

        mCamera.SetOrthographic(0.0f, GetDevice().GetWidth(), 0.0f, GetDevice().GetHeight(), -1.0f, 1.0f);
        mCamera.Compute();

        auto Phase = Scene->CreatePhase<"OnDynamicPhase">(flecs::OnUpdate);

        mTimer = Scene->CreateTimer();
        mTimer.SetInterval(1.0f / 60.0f);

        Scene->CreateSystem(
            "SystemThatRunsWithADynamicTickSource",
            Phase,
            Scene::Execution::Default,
            [&]
            {
                if (mUpward)
                {
                    mRadius += 0.01f;
                    if (mRadius >= 2.5f)
                    {
                        mUpward = false;
                    }
                }
                else
                {
                    mRadius -= 0.01f;
                    if (mRadius <= 0.0f)
                    {
                        mUpward = true;
                    }
                }
            }, Scene::DSL::Interval<1>(mTimer));


        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Application::OnTick(ConstRef<Time> Time)
    {
        ZYPHRYON_PROFILE;

        static Real32 Thickness = 0.0f;
        Thickness += (Time.GetDelta() * 1.0f);

        ConstTracker<Graphic::Service> Graphics = GetService<Graphic::Service>();

        Graphic::Viewport Viewport(0, 0, GetDevice().GetWidth(), GetDevice().GetHeight());
        Graphics->Prepare(Graphic::kDisplay, Viewport, Graphic::Clear::All, Color::Black(), 1, 0);
        {
            mRenderer->SetGlobalParameters(ConstSpan(&mCamera.GetViewProjection(), 1));

            mRenderer->DrawLine(Line({100, 100}, {200, 100}), 0.0f, Color::Red().ToRGBA8(), 1.0f);
            mRenderer->DrawStrokeRect(Rect({400, 200, 500, 300}), 0.3f, Color::Green().ToRGBA8(), 4.0f);
            mRenderer->DrawRect(Rect({100, 350, 200, 450}), 0.2f, Color::Blue().ToRGBA8());

            mRenderer->DrawCircle(Circle({600, 600}, 128), 0.3f, Color::White().ToRGBA8());
            mRenderer->DrawRing(Circle({200, 200}, 64), 0.4f, Color::Red().ToRGBA8(), 1);
            mRenderer->DrawRing(Circle({400, 400}, 32), 0.5f, Color::Yellow().ToRGBA8(), 2);
            mRenderer->DrawRing(Circle({300, 300}, 16), 0.6f, Color::Blue().ToRGBA8(), 3);
            mRenderer->DrawLine(Line({0, 0}, {800, 800}), 0.1f, Color::Green().ToRGBA8(), 1.0f);
            mRenderer->DrawRoundedRect(Rect{200, 500, 400, 700}, 0.5f, Color::White().ToRGBA8(), 0.5f);
            mRenderer->DrawRoundedRect(Rect{250, 500, 350, 700}, 0.4f, Color::Red().ToRGBA8(), 0.25f);
            mRenderer->DrawRoundedRect(Rect{275, 500, 325, 700}, 0.3f, Color::Green().ToRGBA8(), mRadius);

            mEcsQueryCircles.Run([this](ConstRef<Circle> Shape, ConstRef<Matrix4x4> Worldspace)
            {
                const Circle Transformation = Circle::Transform(Shape, Worldspace);
                mRenderer->DrawRing(Transformation, 1.0f, Color::Gray().ToRGBA8(), 0.7f);
            });

            mRenderer->Flush();
        }
        Graphics->Commit(Graphic::kDisplay);


        auto Input = GetService<Input::Service>();
        if (Input->IsKeyPressed(Input::Key::F1))
        {
            mTimer.Pause();
        }
        else if (Input->IsKeyPressed(Input::Key::F2))
        {
            mTimer.Resume();
        }

    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Application::OnTeardown()
    {
        mEcsQueryCircles.Destruct();
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
    Unique<Example::Application> Application = NewUniquePtr<Example::Application>();
    Application->Initialize(Service::Host::Mode::Client, Properties);
    Application->Run();

    return 0;
}