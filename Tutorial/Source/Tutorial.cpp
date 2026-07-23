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
        // Mount the embedded resource archive (Canvas techniques and the default font live under Embedded://).
        ZyRegisterEmbedded(* GetService<Content::Service>());

        // The canvas registers the font loader and its techniques on construction; the renderer drives passes.
        mCanvas   = Unique<Render::Canvas>::Create(* this);
        mRenderer = Unique<Render::Renderer>::Create(* this);

        // A single display pass: clear to a dark background, then flush the canvas into it.
        Ref<CanvasPass>               Stage = mRenderer->AddPass<CanvasPass>(* mCanvas);
        Render::Pass::ColorAttachment Background;
        Background.Target = nullptr;                                 // the display surface
        Background.Load   = Graphic::Action::Clear;
        Background.Tint   = Color(0.055f, 0.06f, 0.09f, 1.0f);
        Stage.AddColor(Background);

        // Request the embedded font. The load is asynchronous, so text is gated on IsFontReady().
        mFont = GetService<Content::Service>()->Load<Render::Font>("Embedded://Font/Roboto.fnt");

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
            mRenderer->Resize(Width, Height);
        }

        mElapsed += Delta;
        mFps      = (Delta > 0.0) ? static_cast<Real32>(1.0 / Delta) : 0.0f;

        // Screen-space orthographic projection (top-left origin, Y increasing downward).
        mCamera.SetOrthographic(0, static_cast<Real32>(Width), 0, static_cast<Real32>(Height), -1.0f, 1.0f);
        mCamera.Compute();

        // Record this frame's 2D content, then submit through the renderer.
        mCanvas->Begin();

        if (GetService<Content::Service>()->GetPending() <= 0)
        {
            Compose(static_cast<Real32>(mElapsed));
        }

        ConstRetainer<Graphic::Service> Graphics = GetService<Graphic::Service>();
        Graphic::Transient<Matrix4x4>   Uniforms = Graphics->AllocateTransientUniforms<Matrix4x4>(1);
        Uniforms[0] = mCamera.GetViewProjection();

        mRenderer->Run(Uniforms.GetStream());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Host::Compose(Real32 Time)
    {
        const Real32 Width = static_cast<Real32>(mWidth);
        const Real32 Pulse = 0.5f + 0.5f * static_cast<Real32>(std::sin(Time * 2.0f));

        const IntColor8 Ink     = IntColor8(235, 238, 245, 255);
        const IntColor8 Muted   = IntColor8(150, 160, 180, 255);
        const IntColor8 Accent  = IntColor8( 90, 170, 255, 255);
        const IntColor8 Panel   = IntColor8(255, 255, 255,  14);
        const IntColor8 Outline = IntColor8(255, 255, 255,  32);

        mCanvas->DrawText(mFont, 46.0f, Vector2(), "ZYPHRYON",
            Matrix3x2::FromTranslation(Vector2(48.0f, 40.0f)), 0.95f, Ink,
            Render::FontEffect::Outline(0.14f, 0.10f, Color(0.02f, 0.03f, 0.06f, 1.0f)));

        mCanvas->DrawText(mFont, 17.0f, Vector2(3.0f, 0.0f), "2D CANVAS SHOWCASE",
            Matrix3x2::FromTranslation(Vector2(50.0f, 92.0f)), 0.95f, Accent,
            Render::FontEffect::Bold(0.03f));

        const Real32 RuleWidth = 60.0f + 200.0f * Pulse;
        mCanvas->DrawLine(Line(Vector2(50.0f, 114.0f), Vector2(50.0f + RuleWidth, 114.0f)), 0.90f, Accent, 3.0f);

        constexpr UInt32 Count  = 5;
        const     Real32 Margin = 48.0f;
        const     Real32 Gap    = 20.0f;
        const     Real32 CardW  = (Width - 2.0f * Margin - (Count - 1) * Gap) / Count;
        const     Real32 CardH  = 190.0f;
        const     Real32 CardY  = 152.0f;

        static constexpr Text kLabels[Count] = { "Circle", "Ring", "Rectangle", "Rounded", "Lines" };

        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            const Real32 X  = Margin + Index * (CardW + Gap);
            const Real32 CX = X + CardW * 0.5f;
            const Real32 CY = CardY + 78.0f;
            const Real32 R  = 44.0f;

            mCanvas->DrawRoundedRect(Rect(X, CardY, X + CardW, CardY + CardH), 0.60f, Panel, 14.0f);
            mCanvas->DrawStrokeRect (Rect(X, CardY, X + CardW, CardY + CardH), 0.61f, Outline, 1.0f);

            switch (Index)
            {
            case 0:
                mCanvas->DrawCircle(Circle(CX, CY, R * (0.82f + 0.18f * Pulse)), 0.70f, IntColor8(90, 170, 255, 255));
                break;
            case 1:
                mCanvas->DrawRing(Circle(CX, CY, R), 0.70f, IntColor8(255, 150, 90, 255), 0.22f);
                break;
            case 2:
                mCanvas->DrawRect(Rect(CX - R, CY - R, CX + R, CY + R), 0.70f, IntColor8(120, 205, 140, 255));
                break;
            case 3:
                mCanvas->DrawRoundedRect(Rect(CX - R, CY - R, CX + R, CY + R), 0.70f, IntColor8(198, 150, 255, 255), 18.0f);
                break;
            case 4:
                for (UInt32 Spoke = 0; Spoke < 8; ++Spoke)
                {
                    const Real32 Angle = Time + Spoke * 0.7853982f; // 8 spokes, 45 degrees apart.
                    const Vector2 End(CX + std::cos(Angle) * R, CY + std::sin(Angle) * R);
                    mCanvas->DrawLine(Line(Vector2(CX, CY), End), 0.70f, IntColor8(240, 220, 120, 255), 2.5f);
                }
                break;
            default:
                break;
            }

            DrawTextCentered(kLabels[Index], CX, CardY + CardH - 30.0f, 15.0f, Muted);
        }

        ComposeOverlay();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Host::ComposeOverlay()
    {
        ConstRetainer<Platform::Service> PlatformService = GetService<Platform::Service>();
        ConstRetainer<Input::Service>    InputService    = GetService<Input::Service>();

        Ref<Platform::Window>       Window  = PlatformService->GetWindow();
        ConstRef<Platform::Monitor> Monitor = PlatformService->GetDisplay().GetMonitor(Text());

        const Real32 PanelW = 344.0f;
        const Real32 PanelH = 196.0f;
        const Real32 PanelX = 48.0f;
        const Real32 PanelY = static_cast<Real32>(mHeight) - PanelH - 44.0f;

        mCanvas->DrawRoundedRect(Rect(PanelX, PanelY, PanelX + PanelW, PanelY + PanelH), 0.40f, IntColor8(255, 255, 255, 16), 12.0f);
        mCanvas->DrawStrokeRect (Rect(PanelX, PanelY, PanelX + PanelW, PanelY + PanelH), 0.41f, IntColor8(255, 255, 255, 34), 1.0f);

        DrawTextCentered("DIAGNOSTICS", PanelX + PanelW * 0.5f, PanelY + 14.0f, 14.0f,
            IntColor8(90, 170, 255, 255), Render::FontEffect::Bold(0.02f));

        const IntColor8 Value = IntColor8(228, 232, 240, 255);
        const Real32    LabelX = PanelX + 22.0f;
        Real32          Y      = PanelY + 48.0f;

        String<160> Line;
        const auto   Row = [&](ConstRef<String<160>> Content)
        {
            mCanvas->DrawText(mFont, 15.0f, Vector2(), Content, Matrix3x2::FromTranslation(Vector2(LabelX, Y)), 0.35f, Value, Render::FontEffect());
            Y += 25.0f;
        };

        Line.Format<"FPS       {0}">(static_cast<UInt32>(mFps + 0.5f));
        Row(Line);
        Line.Format<"Window    {0} x {1}   {2}">(Window.GetWidth(), Window.GetHeight(), Window.IsFocused() ? "focused" : "idle");
        Row(Line);
        Line.Format<"Monitor   {0}">(Monitor.GetName());
        Row(Line);
        Line.Format<"Display   {0} x {1} @ {2} Hz">(Monitor.GetWidth(), Monitor.GetHeight(), Monitor.GetFrequency());
        Row(Line);
        Line.Format<"Mouse     {0}, {1}">(static_cast<SInt32>(InputService->GetMouseX()), static_cast<SInt32>(InputService->GetMouseY()));
        Row(Line);
        Line.Format<"[Space]   {0}">(InputService->IsKeyHeld(Input::Key::Space) ? "held"_Text : "released"_Text);
        Row(Line);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Host::DrawTextCentered(Text Content, Real32 CenterX, Real32 Y, Real32 Size, IntColor8 Tint, ConstRef<Render::FontEffect> Effect)
    {
        const Rect   Bounds = mFont->Enclose(Content, Size);
        const Real32 Width  = Bounds.GetMaximumX() - Bounds.GetMinimumX();

        mCanvas->DrawText(mFont, Size, Vector2(), Content,
            Matrix3x2::FromTranslation(Vector2(CenterX - Width * 0.5f, Y)), 0.55f, Tint, Effect);
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
