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

#include <Zyphryon.Engine/Kernel.hpp>
#include <Zyphryon.Render/Renderer.hpp>
#include <Zyphryon.Render/Canvas/Canvas.hpp>
#include <Zyphryon.Render/Typography/Font.hpp>
#include <Zyphryon.Graphic/Camera.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Application
{
    /// \brief A 2D Canvas showcase: geometry, MSDF text, and a live diagnostics overlay.
    class Host final : public Engine::Kernel
    {
    protected:

        /// \see Engine::Kernel::OnInitialize()
        Bool OnInitialize() override;

        /// \see Engine::Kernel::OnTick(Real64)
        void OnTick(Real64 Delta) override;

        /// \see Engine::Kernel::OnTerminate()
        void OnTerminate() override;

    private:

        class CanvasPass final : public Render::Pass
        {
        public:

            ZY_INLINE explicit CanvasPass(Ref<Render::Canvas> Canvas)
                : mCanvas { Canvas }
            {
            }

            /// \see Render::Pass::Run(Ref<Render::Encoder>)
            void Run(Ref<Render::Encoder> Encoder) override
            {
                mCanvas.Flush(Encoder);
            }

        private:

            Ref<Render::Canvas> mCanvas;
        };

        /// \brief Records the whole frame (header, shape gallery, and overlay) into the canvas.
        void Compose(Real32 Time);

        /// \brief Records the live window / monitor / input diagnostics panel.
        void ComposeOverlay();

        /// \brief Draws a horizontally-centered line of text, measuring its width through Font::Enclose.
        void DrawTextCentered(Text Content, Real32 CenterX, Real32 Y, Real32 Size, IntColor8 Tint,
            ConstRef<Render::FontEffect> Effect = Render::FontEffect());

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Unique<Render::Canvas>   mCanvas;
        Unique<Render::Renderer> mRenderer;
        Retainer<Render::Font>   mFont;
        Graphic::Camera          mCamera;
        Real64                   mElapsed = 0.0;
        Real32                   mFps     = 0.0f;
        UInt16                   mWidth   = 0;
        UInt16                   mHeight  = 0;
    };
}
