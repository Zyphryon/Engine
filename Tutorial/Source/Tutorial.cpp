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

#include "Tutorial.hpp"
#include <Zyphryon.Graphic/Service.hpp>
#include <Zyphryon.Platform/Service.hpp>
#include <Zyphryon.Input/Service.hpp>
#include <Zyphryon.Content/Mount/Disk.hpp>
#include <ZyTutorial.Modules.hpp>           // ${TARGET}.Modules.hpp  (Generated from CMake)
#include <ZyTutorial.Embedded.hpp>          // ${TARGET}.Embedded.hpp (Generated from CMake)

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Application
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Host::OnInitialize()
    {
        Filesystem::Path Root = Filesystem::GetRootFolder();
        Root.Append("/Resources");

        ConstRetainer<Content::Service> Content = GetService<Content::Service>();
        Content->AddMount("Resources", Retainer<Content::Disk>::Create(Root));
        ZyRegisterEmbedded(* Content);

        mGraph = Unique<Render::Graph>::Create(* this);

        Ref<Host::Pass> Scene = mGraph->AddPass<Host::Pass>();

        Render::Pass::ColorAttachment Background;
        Background.Target = nullptr;
        Background.Load   = Graphic::Action::Clear;
        Background.Tint   = Color(0.055f, 0.06f, 0.09f, 1.0f);
        Scene.AddColor(Background);

        LOG_D("Host::OnInitialize");
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Host::OnTick(Real64 Delta)
    {
        Ref<Platform::Window> Window = GetService<Platform::Service>()->GetWindow();

        const UInt16 Width  = static_cast<UInt16>(Window.GetWidth());
        const UInt16 Height = static_cast<UInt16>(Window.GetHeight());

        // Re-fit the managed targets and pass viewports whenever the window size changes.
        if (Width != mWidth || Height != mHeight)
        {
            mWidth  = Width;
            mHeight = Height;
            mGraph->Resize(Width, Height);
        }

        // Screen-space orthographic projection (bottom-left origin, Y increasing downward).
        mCamera.SetOrthographic(0, Width, 0, Height, -1.0f, 1.0f);
        mCamera.Compute();

        ConstRetainer<Graphic::Service> Graphics = GetService<Graphic::Service>();
        Graphic::Transient<Matrix4x4>   Uniforms = Graphics->AllocateTransientUniforms<Matrix4x4>(1);
        Uniforms[0] = mCamera.GetViewProjection();

        mGraph->Run(Uniforms.GetStream());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Host::OnTerminate()
    {
        LOG_D("Host::OnTerminate");
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   MAIN  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int main()
{
    Engine::Config Configuration;
    Configuration.SetWindowWidth(1280);
    Configuration.SetWindowHeight(720);
    Configuration.SetWindowFullscreen(false);
    Configuration.SetWindowBorderless(false);
    Configuration.SetWindowTitle("Zyphryon - Canvas Showcase");
    Configuration.SetGraphicsColorFormat(Graphic::TextureFormat::Unspecified);
    Configuration.SetGraphicsDepthFormat(Graphic::TextureFormat::D24S8UIntNorm);

#ifdef ZY_PLATFORM_WINDOWS
    Configuration.SetGraphicsDriver("D3D11");
#else
    Configuration.SetGraphicsDriver("GLES3");
#endif

    Application::Host Host;
    Host.Run(Move(Configuration), ZyRegisterModules());

    return 0;
}
