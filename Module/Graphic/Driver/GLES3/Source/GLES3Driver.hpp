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

#include "GLES3Common.hpp"
#include "GLES3Compiler.hpp"
#include "GLES3Context.hpp"
#include "Zyphryon.Graphic/Driver.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Encapsulate the graphic driver implementation using OpenGL ES 3.0 / OpenGL 3.3 core.
    class GLES3Driver final : public Driver
    {
    public:

        /// \brief Releases every OpenGL object still owned by the driver.
        ~GLES3Driver() override;

        /// \see Driver::Initialize(Ptr<void>, ConstRef<Config>)
        Bool Initialize(Ptr<void> Output, ConstRef<Config> Config) override;

        /// \see Driver::Reset(UInt16, UInt16)
        void Reset(UInt16 Width, UInt16 Height) override;

        /// \see Driver::Probe(Ref<Description>)
        void Probe(Ref<Description> Output) const override;

        /// \see Driver::CreateBuffer(Object, Storage, Usage, UInt32, ConstSpan<Byte>)
        void CreateBuffer(Object ID, Storage Storage, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data) override;

        /// \see Driver::UpdateBuffer(Object, UInt32, ConstSpan<Byte>)
        void UpdateBuffer(Object ID, UInt32 Offset, ConstSpan<Byte> Data) override;

        /// \see Driver::DeleteBuffer(Object)
        void DeleteBuffer(Object ID) override;

        /// \see Driver::CopyBuffer(Object, UInt32, Object, UInt32, UInt32)
        void CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, UInt32 Size) override;

        /// \see Driver::MapBuffer(Object, UInt32, UInt32)
        Ptr<Byte> MapBuffer(Object ID, UInt32 Offset, UInt32 Size) override;

        /// \see Driver::UnmapBuffer(Object)
        void UnmapBuffer(Object ID) override;

        /// \see Driver::CreatePass(Object, ConstSpan<ColorAttachment>, DepthAttachment)
        void CreatePass(Object ID, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth) override;

        /// \see Driver::DeletePass(Object)
        void DeletePass(Object ID) override;

        /// \see Driver::CreatePipeline(Object, ConstRef<Program>, ConstRef<States>)
        void CreatePipeline(Object ID, ConstRef<Program> Program, ConstRef<States> States) override;

        /// \see Driver::DeletePipeline(Object)
        void DeletePipeline(Object ID) override;

        /// \see Driver::CreateTexture(Object, TextureLayout, TextureFormat, Storage, Usage, UInt16, UInt16, UInt8, Multisample, ConstSpan<Byte>)
        void CreateTexture(Object ID, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, ConstSpan<Byte> Data) override;

        /// \see Driver::UpdateTexture(Object, UInt8, UInt16, UInt16, UInt16, UInt16, UInt32, ConstSpan<Byte>)
        void UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data) override;

        /// \see Driver::DeleteTexture(Object)
        void DeleteTexture(Object ID) override;

        /// \see Driver::CopyTexture(Object, UInt8, UInt16, UInt16, Object, UInt8, UInt16, UInt16, UInt16, UInt16)
        void CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16 SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, UInt16 Width, UInt16 Height) override;

        /// \see Driver::Prepare(Object, ConstRef<Viewport>, ConstSpan<Color>, Real32, UInt8)
        void Prepare(Object Pass, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil) override;

        /// \see Driver::Submit(ConstSpan<Command>)
        void Submit(ConstSpan<Command> Commands) override;

        /// \see Driver::Commit(Object)
        void Commit(Object Pass) override;

    private:

        /// \brief Internal wrapper for OpenGL buffer resources.
        struct GLES3Buffer final
        {
            GLuint Object   = 0;
            GLenum Usage    = 0;
            UInt32 Capacity = 0;
        };

        /// \brief Internal wrapper for OpenGL render pass resources.
        struct GLES3Pass final
        {
            GLuint                                     Framebuffer = 0;
            GLuint                                     Resolver    = 0;
            UInt16                                     Width       = 0;
            UInt16                                     Height      = 0;
            Sequence<ColorAttachment, kMaxAttachments> Colors;
            DepthAttachment                            Depth;
        };

        /// \brief Internal wrapper for OpenGL pipeline resources (linked program plus resolved fixed-function state).
        struct GLES3Pipeline final
        {
            GLuint        Program               = 0;
            Bool          BlendEnable           = false;
            GLenum        BlendSrcColor         = GL_ONE;
            GLenum        BlendDstColor         = GL_ZERO;
            GLenum        BlendEquationColor    = GL_FUNC_ADD;
            GLenum        BlendSrcAlpha         = GL_ONE;
            GLenum        BlendDstAlpha         = GL_ZERO;
            GLenum        BlendEquationAlpha    = GL_FUNC_ADD;
            UInt8         Channel               = 0x0F;
            Bool          AlphaToCoverage       = false;
            Bool          DepthEnable           = true;
            Bool          DepthMask             = true;
            GLenum        DepthFunction         = GL_LEQUAL;
            Bool          DepthClip             = true;
            Real32        DepthBias             = 0.0f;
            Real32        DepthBiasSlope        = 0.0f;
            Bool          StencilEnable         = false;
            GLuint        StencilReadMask       = 0xFF;
            GLuint        StencilWriteMask      = 0xFF;
            GLenum        StencilFrontFunction  = GL_ALWAYS;
            GLenum        StencilFrontFail      = GL_KEEP;
            GLenum        StencilFrontDepthFail = GL_KEEP;
            GLenum        StencilFrontDepthPass = GL_KEEP;
            GLenum        StencilBackFunction   = GL_ALWAYS;
            GLenum        StencilBackFail       = GL_KEEP;
            GLenum        StencilBackDepthFail  = GL_KEEP;
            GLenum        StencilBackDepthPass  = GL_KEEP;
            Bool          CullEnable            = true;
            GLenum        CullMode              = GL_BACK;
            Bool          ScissorEnable         = false;
            GLenum        FillMode              = 0x1B02;
            GLenum        Primitive             = GL_TRIANGLES;
            Sequence<Attribute, kMaxAttributes> Attributes;
        };

        /// \brief Internal wrapper for OpenGL texture (or renderbuffer) resources.
        struct GLES3Texture final
        {
            GLuint        Object  = 0;
            GLenum        Target  = GL_TEXTURE_2D;
            TextureFormat Format  = TextureFormat::Unspecified;
            UInt16        Width   = 0;
            UInt16        Height  = 0;
            UInt8         Samples = 1;
        };

        /// \brief Internal wrapper for OpenGL pipeline snapshot.
        struct GLES3Snapshot final
        {

            UInt32         Attributes = 0;
            Ptr<GLES3Pass> Pass       = nullptr;
            GLES3Pipeline  Pipeline;
            UInt8          Stencil    = 0;
            GLuint         Vertices   = 0;
        };

    private:

        /// \brief Detects the OpenGL version, feature tier, extensions, and device limits.
        void LoadCapabilities();

        /// \brief Queries whether the current context exposes the given OpenGL extension.
        ///
        /// \param Name The extension name to look for.
        /// \return `true` when the extension is present, `false` otherwise.
        Bool HasExtension(ConstPtr<Char> Name) const;

        /// \brief Applies the fixed-function state and program described by a pipeline, emitting only the changes
        ///        relative to the state cached from the previous application.
        ///
        /// \param Pipeline The pipeline whose state should become current.
        /// \param Stencil  The stencil reference value to bind alongside the stencil state.
        void ApplyPipeline(ConstRef<GLES3Pipeline> Pipeline, UInt8 Stencil);

        /// \brief Applies the vertex stream bindings and attribute layout for a draw item.
        ///
        /// \param Pipeline        The pipeline whose input layout describes the attributes.
        /// \param Command         The draw item providing the vertex streams.
        /// \param IsPipelineDirty Whether the pipeline changed since the previous draw.
        void ApplyVertexResources(ConstRef<GLES3Pipeline> Pipeline, ConstRef<Command> Command, Bool IsPipelineDirty);

        /// \brief Applies the uniform buffer range bindings for a draw item.
        ///
        /// \param Oldest The previously submitted draw item.
        /// \param Newest The draw item being submitted.
        void ApplyUniformResources(ConstRef<Command> Oldest, ConstRef<Command> Newest) const;

        /// \brief Applies the texture and sampler bindings for a draw item.
        ///
        /// \param Oldest The previously submitted draw item.
        /// \param Newest The draw item being submitted.
        void ApplyTextureResources(ConstRef<Command> Oldest, ConstRef<Command> Newest);

        /// \brief Gets an existing sampler object matching the descriptor or lazily creates one.
        ///
        /// \param Descriptor The sampler descriptor to resolve.
        /// \return The OpenGL sampler object name.
        GLuint GetOrCreateSampler(Sampler Descriptor);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        GLES3Context                        mContext;
        GLES3Compiler                       mCompiler;
        Description                         mDescription;
        GLES3Snapshot                       mSnapshot;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        GLuint                              mGlobalReadFramebuffer = 0;
        GLuint                              mGlobalDrawFramebuffer = 0;
        GLuint                              mGlobalVAO             = 0;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<GLES3Buffer,   kMaxBuffers>   mBuffers;
        Array<GLES3Pass,     kMaxPasses>    mPasses;
        Array<GLES3Pipeline, kMaxPipelines> mPipelines;
        Array<GLES3Texture,  kMaxTextures>  mTextures;
        Table<UInt64, GLuint>               mSamplers;
    };
}
