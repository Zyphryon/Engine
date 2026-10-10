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

#include "GLES3Compiler.hpp"
#include "GLES3Context.hpp"
#include "Zyphryon.Graphic/Driver.hpp"

#if defined(ZY_PROFILE_BACKEND_TRACY) && !defined(ZY_PLATFORM_WEB)
#include <tracy/TracyOpenGL.hpp>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    /// \brief Represents the graphic driver implemented over OpenGL ES 3.0 / OpenGL 3.3 core.
    class GLES3Driver final : public Driver
    {
    public:

        /// \brief Releases every OpenGL object still owned by the driver.
        ~GLES3Driver() override;

        /// \see Driver::Initialize(Ptr<void>, ConstRef<Configuration>)
        Bool Initialize(Ptr<void> Output, ConstRef<Configuration> Config) override;

        /// \see Driver::Reset(UInt16, UInt16, Bool)
        void Reset(UInt16 Width, UInt16 Height, Bool Tearless) override;

        /// \see Driver::Probe(Ref<Description>)
        void Probe(Ref<Description> Output) const override;

        /// \see Driver::CreateBuffer(Object, Storage, Usage, UInt32, ConstSpan<Byte>)
        void CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data) override;

        /// \see Driver::UpdateBuffer(Object, UInt32, ConstSpan<Byte>)
        void UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data) override;

        /// \see Driver::DeleteBuffer(Object)
        void DeleteBuffer(Object ID) override;

        /// \see Driver::MapBuffer(Object, UInt32, UInt32)
        Ptr<Byte> MapBuffer(Object ID, UInt32 Offset, UInt32 Size) override;

        /// \see Driver::UnmapBuffer(Object)
        void UnmapBuffer(Object ID) override;

        /// \see Driver::CreatePass(Object, ConstSpan<ColorAttachment>, DepthAttachment)
        void CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth) override;

        /// \see Driver::DeletePass(Object)
        void DeletePass(Object ID) override;

        /// \see Driver::CreatePipeline(Object, ConstRef<Program>, ConstRef<Signature>, ConstRef<States>)
        void CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States) override;

        /// \see Driver::DeletePipeline(Object)
        void DeletePipeline(Object ID) override;

        /// \see Driver::CreateSampler(Object, Sampler)
        void CreateSampler(Object ID, Sampler Descriptor) override;

        /// \see Driver::DeleteSampler(Object)
        void DeleteSampler(Object ID) override;

        /// \see Driver::CreateTexture(Object, TextureLayout, TextureFormat, Storage, Usage, UInt16, UInt16, UInt16, UInt8, ConstSpan<Byte>)
        void CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data) override;

        /// \see Driver::UpdateTexture(Object, UInt8, UInt16, UInt16, UInt16, UInt16, UInt16, UInt32, ConstSpan<Byte>)
        void UpdateTexture(Object ID, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data) override;

        /// \see Driver::DeleteTexture(Object)
        void DeleteTexture(Object ID) override;

        /// \see Driver::Prepare(Object, Text, ConstRef<Viewport>, ConstSpan<Color>, Real32, UInt8)
        void Prepare(Object Pass, Text Name, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil) override;

        /// \see Driver::Submit(ConstSpan<Command>)
        void Submit(ConstSpan<Command> Commands) override;

        /// \see Driver::Commit(Object)
        void Commit(Object Pass) override;

    private:

        /// \brief Represents an OpenGL buffer.
        struct GLES3Buffer final
        {
            /// The buffer object.
            GLuint Object = 0;

            /// The target the buffer is bound to when drawn from.
            GLenum Target = 0;
        };

        /// \brief Represents an OpenGL render pass.
        struct GLES3Pass final
        {
            /// The framebuffer object, `0` for the default framebuffer.
            GLuint                                     Framebuffer = 0;

            /// The height of the attachments, which flips window coordinates.
            UInt16                                     Height      = 0;

            /// The color attachments, in binding order.
            Sequence<ColorAttachment, kMaxAttachments> Colors;

            /// The depth attachment and its load and store actions.
            DepthAttachment                            Depth;
        };

        /// \brief Represents an OpenGL pipeline, its linked program plus resolved fixed-function state.
        struct GLES3Pipeline final
        {
            /// The program object, `0` when it failed to build.
            GLuint        Program               = 0;

            /// `true` once the program has linked and its bindings are applied.
            Bool          Ready                 = false;

            /// `true` when blending is enabled.
            Bool          BlendEnable           = false;

            /// The source factor of the color blend.
            GLenum        BlendSrcColor         = GL_ONE;

            /// The destination factor of the color blend.
            GLenum        BlendDstColor         = GL_ZERO;

            /// The equation of the color blend.
            GLenum        BlendEquationColor    = GL_FUNC_ADD;

            /// The source factor of the alpha blend.
            GLenum        BlendSrcAlpha         = GL_ONE;

            /// The destination factor of the alpha blend.
            GLenum        BlendDstAlpha         = GL_ZERO;

            /// The equation of the alpha blend.
            GLenum        BlendEquationAlpha    = GL_FUNC_ADD;

            /// The color channels written, one bit per channel.
            UInt8         Channel               = 0x0F;

            /// `true` when the depth test is enabled.
            Bool          DepthEnable           = true;

            /// `true` when depth writes are enabled.
            Bool          DepthMask             = true;

            /// The comparison of the depth test.
            GLenum        DepthFunction         = GL_LEQUAL;

            /// `true` when depth is clipped rather than clamped.
            Bool          DepthClip             = true;

            /// The constant depth bias.
            Real32        DepthBias             = 0.0f;

            /// The depth bias scaled by the polygon's slope.
            Real32        DepthBiasSlope        = 0.0f;

            /// `true` when the stencil test is enabled.
            Bool          StencilEnable         = false;

            /// The mask applied to stencil values before they are compared.
            GLuint        StencilReadMask       = 0xFF;

            /// The mask applied to stencil values before they are written.
            GLuint        StencilWriteMask      = 0xFF;

            /// The comparison of the front-face stencil test.
            GLenum        StencilFrontFunction  = GL_ALWAYS;

            /// The front-face operation when the stencil test fails.
            GLenum        StencilFrontFail      = GL_KEEP;

            /// The front-face operation when the stencil test passes and the depth test fails.
            GLenum        StencilFrontDepthFail = GL_KEEP;

            /// The front-face operation when both tests pass.
            GLenum        StencilFrontDepthPass = GL_KEEP;

            /// The comparison of the back-face stencil test.
            GLenum        StencilBackFunction   = GL_ALWAYS;

            /// The back-face operation when the stencil test fails.
            GLenum        StencilBackFail       = GL_KEEP;

            /// The back-face operation when the stencil test passes and the depth test fails.
            GLenum        StencilBackDepthFail  = GL_KEEP;

            /// The back-face operation when both tests pass.
            GLenum        StencilBackDepthPass  = GL_KEEP;

            /// `true` when face culling is enabled.
            Bool          CullEnable            = true;

            /// The face that is culled.
            GLenum        CullMode              = GL_BACK;

            /// `true` when the scissor test is enabled.
            Bool          ScissorEnable         = false;

            /// The polygon mode, which only desktop OpenGL applies.
            GLenum        FillMode              = kGLFill;

            /// The primitive the draws assemble.
            GLenum        Primitive             = GL_TRIANGLES;

            /// The vertex attributes the program reads.
            Attributes    Attributes;
        };

        /// \brief Represents an OpenGL texture.
        struct GLES3Texture final
        {
            /// The texture object.
            GLuint        Object = 0;

            /// The target the texture is bound to.
            GLenum        Target = GL_TEXTURE_2D;

            /// The format the texture was created with.
            TextureFormat Format = TextureFormat::Unspecified;

            /// The height of the base level.
            UInt16        Height = 0;
        };

        /// \brief Holds the context state last applied, so only changes are emitted.
        struct GLES3Snapshot final
        {
            /// The vertex attribute locations enabled, one bit per location.
            UInt32         Attributes = 0;

            /// The pass being recorded.
            Ptr<GLES3Pass> Pass       = nullptr;

            /// The pipeline state last applied.
            GLES3Pipeline  Pipeline;

            /// The stencil reference value last applied.
            UInt8          Stencil    = 0;

            /// The buffer bound to GL_ARRAY_BUFFER.
            GLuint         Vertices   = 0;
        };

        /// \brief Represents the profiler's view of the context, and the one zone a pass is timed by.
        struct GLES3Profiler final
        {
#if defined(ZY_PROFILE_BACKEND_TRACY) && !defined(ZY_PLATFORM_WEB)
            /// The storage of the zone that times the open pass.
            ZY_ALIGN(tracy::GpuCtxScope) Byte Zone[sizeof(tracy::GpuCtxScope)];

            /// `true` while a zone is open.
            Bool                              Opened = false;
#endif

            /// \brief Creates the profiler context for the current OpenGL context.
            ZY_INLINE void Initialize()
            {
#if defined(ZY_PROFILE_BACKEND_TRACY) && !defined(ZY_PLATFORM_WEB)
                TracyGpuContext;
#endif
            }

            /// \brief Opens the zone a pass is timed by.
            ///
            /// \param Name The name the zone is shown with.
            ZY_INLINE void Open(Text Name)
            {
#if defined(ZY_PROFILE_BACKEND_TRACY) && !defined(ZY_PLATFORM_WEB)
                ZY_ASSERT(!Opened, "A render pass was prepared while another was still open");

                static constexpr Char kZone[] = "GLES3Driver::Prepare";

                Construct<tracy::GpuCtxScope>(
                    reinterpret_cast<Ptr<tracy::GpuCtxScope>>(Zone),
                    static_cast<UInt32>(__LINE__),
                    __FILE__,
                    sizeof(__FILE__) - 1,
                    kZone,
                    sizeof(kZone) - 1,
                    Name.GetData(),
                    Name.GetSize(),
                    true);
                Opened = true;
#endif
            }

            /// \brief Closes the zone opened by \ref Open, if any.
            ZY_INLINE void Close()
            {
#if defined(ZY_PROFILE_BACKEND_TRACY) && !defined(ZY_PLATFORM_WEB)
                if (Opened)
                {
                    Destruct(* reinterpret_cast<Ptr<tracy::GpuCtxScope>>(Zone));
                    Opened = false;
                }
#endif
            }

            /// \brief Gathers whatever the timing queries have answered.
            ZY_INLINE void Collect()
            {
#if defined(ZY_PROFILE_BACKEND_TRACY) && !defined(ZY_PLATFORM_WEB)
                TracyGpuCollect;
#endif
            }
        };

    private:

        /// \brief Detects the OpenGL version, feature tier, extensions, and device limits.
        void LoadCapabilities();

        /// \brief Checks whether the current context exposes the given OpenGL extension.
        ///
        /// \param Name The extension name to look for.
        /// \return `true` when the extension is present, `false` otherwise.
        Bool HasExtension(Text Name) const;

        /// \brief Brings a pipeline whose link is still outstanding into use once the link has landed.
        ///
        /// \param ID The pipeline to settle.
        /// \return `true` when the pipeline can be drawn with, `false` while it links or once it failed.
        Bool Settle(Object ID);

        /// \brief Applies a pipeline's program and fixed-function state, emitting only what changed since the last one.
        ///
        /// \param Pipeline The pipeline whose state should become current.
        /// \param Stencil  The stencil reference value to bind alongside the stencil state.
        void ApplyPipeline(ConstRef<GLES3Pipeline> Pipeline, UInt8 Stencil);

        /// \brief Applies the vertex stream bindings and attribute layout for a draw item.
        ///
        /// \param Pipeline The pipeline whose input layout describes the attributes.
        /// \param Command  The draw item providing the vertex streams.
        /// \param Dirty    `true` when the pipeline changed since the previous draw.
        void ApplyVertexResources(ConstRef<GLES3Pipeline> Pipeline, ConstRef<Command> Command, Bool Dirty);

        /// \brief Applies the uniform buffer range bindings for a draw item.
        ///
        /// \param Oldest The previously submitted draw item.
        /// \param Newest The draw item being submitted.
        void ApplyUniformResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

        /// \brief Applies the texture and sampler bindings for a draw item.
        ///
        /// \param Oldest The previously submitted draw item.
        /// \param Newest The draw item being submitted.
        void ApplyTextureResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        GLES3Context                             mContext;
        GLES3Compiler                            mCompiler;
        Description                              mDescription;
        GLES3Snapshot                            mSnapshot;
        Table<Object, GLES3Compiler::GLES3Build> mCompilations;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        GLES3Profiler                            mProfiler;
        GLuint                                   mGlobalVAO = 0;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<GLES3Buffer,   kMaxBuffers   + 1>  mBuffers;
        Array<GLES3Pass,     kMaxPasses    + 1>  mPasses;
        Array<GLES3Pipeline, kMaxPipelines + 1>  mPipelines;
        Array<GLuint,        kMaxSamplers  + 1>  mSamplers;
        Array<GLES3Texture,  kMaxTextures  + 1>  mTextures;
    };
}