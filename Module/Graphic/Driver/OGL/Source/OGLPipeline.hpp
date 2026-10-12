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

#include "OGLCompiler.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Describes how a pipeline blends into its color attachments.
    struct Blend final
    {
        /// `true` when blending is enabled.
        Bool   Enable        = false;

        /// The source factor of the color blend.
        GLenum SrcColor      = GL_ONE;

        /// The destination factor of the color blend.
        GLenum DstColor      = GL_ZERO;

        /// The equation of the color blend.
        GLenum EquationColor = GL_FUNC_ADD;

        /// The source factor of the alpha blend.
        GLenum SrcAlpha      = GL_ONE;

        /// The destination factor of the alpha blend.
        GLenum DstAlpha      = GL_ZERO;

        /// The equation of the alpha blend.
        GLenum EquationAlpha = GL_FUNC_ADD;

        /// The color channels written, one bit per channel.
        UInt8  Mask          = 0x0F;

        /// \brief Checks whether two blend states are the same.
        ///
        /// \param Other The blend state to compare against.
        /// \return `true` if every field matches, otherwise `false`.
        ZY_INLINE constexpr Bool operator==(ConstRef<Blend> Other) const = default;
    };

    /// \brief Describes how a pipeline tests and writes depth.
    struct Depth final
    {
        /// `true` when the depth test is enabled.
        Bool   Enable = true;

        /// `true` when depth writes are enabled.
        Bool   Write  = true;

        /// The comparison of the depth test.
        GLenum Test   = GL_LEQUAL;

        /// `true` when depth is clipped rather than clamped.
        Bool   Clip   = true;

        /// The constant depth bias.
        Real32 Bias   = 0.0f;

        /// The depth bias scaled by the polygon's slope.
        Real32 Slope  = 0.0f;

        /// \brief Checks whether two depth states are the same.
        ///
        /// \param Other The depth state to compare against.
        /// \return `true` if every field matches, otherwise `false`.
        ZY_INLINE constexpr Bool operator==(ConstRef<Depth> Other) const = default;
    };

    /// \brief Describes how a pipeline tests and writes stencil.
    struct Stencil final
    {
        /// \brief Describes the stencil test and actions of one face.
        struct Face final
        {
            /// The comparison of the stencil test.
            GLenum Test      = GL_ALWAYS;

            /// The operation when the stencil test fails.
            GLenum Fail      = GL_KEEP;

            /// The operation when the stencil test passes and the depth test fails.
            GLenum DepthFail = GL_KEEP;

            /// The operation when both tests pass.
            GLenum DepthPass = GL_KEEP;

            /// \brief Checks whether two faces test and act the same.
            ///
            /// \param Other The face to compare against.
            /// \return `true` if every field matches, otherwise `false`.
            ZY_INLINE constexpr Bool operator==(ConstRef<Face> Other) const = default;
        };

        /// `true` when the stencil test is enabled.
        Bool   Enable = false;

        /// The mask applied to stencil values before they are compared.
        GLuint Read   = 0xFF;

        /// The mask applied to stencil values before they are written.
        GLuint Write  = 0xFF;

        /// The test and actions of front faces.
        Face   Front;

        /// The test and actions of back faces.
        Face   Back;

        /// \brief Checks whether two stencil states are the same.
        ///
        /// \param Other The stencil state to compare against.
        /// \return `true` if every field matches, otherwise `false`.
        ZY_INLINE constexpr Bool operator==(ConstRef<Stencil> Other) const = default;
    };

    /// \brief Describes how a pipeline rasterizes.
    struct Raster final
    {
        /// `true` when face culling is enabled.
        Bool   Cull    = true;

        /// The face that is culled.
        GLenum Face    = GL_BACK;

        /// `true` when the scissor test is enabled.
        Bool   Scissor = false;

        /// The polygon mode, which only desktop OpenGL applies.
        GLenum Fill    = GL_FILL;

        /// \brief Checks whether two rasterizer states are the same.
        ///
        /// \param Other The rasterizer state to compare against.
        /// \return `true` if every field matches, otherwise `false`.
        ZY_INLINE constexpr Bool operator==(ConstRef<Raster> Other) const = default;
    };

    /// \brief Represents an OpenGL pipeline, its program plus fixed-function state.
    class Pipeline final
    {
    public:

        /// \brief Constructs an empty pipeline that holds no program until \ref Create.
        Pipeline();

        /// \brief Issues the compile and link of the program and resolves the fixed-function state it draws with.
        ///
        /// \note The link is left to run; the first draw that wants the program settles it.
        ///
        /// \param Device    The device of the context, which gates depth clamping and wireframe.
        /// \param Compiler  The compiler that builds the program, or shares one built from the same modules.
        /// \param Program   The shader modules and preprocessor macros of the program.
        /// \param Signature The vertex attributes the program reads.
        /// \param States    The fixed-function state the draws run with.
        /// \return `true` when the program was issued, `false` when it could not be.
        Bool Create(ConstRef<Device> Device, Ref<Compiler> Compiler, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States);

        /// \brief Brings the program into use once its link has landed, applying its bindings.
        ///
        /// \param Compiler The compiler that issued the link.
        /// \return `true` when the pipeline can be drawn with, `false` while it links or once it failed.
        Bool Settle(Ref<Compiler> Compiler);

        /// \brief Lets go of the program, which the compiler deletes once no pipeline draws with it, and returns to the empty state.
        ///
        /// \param Compiler The compiler that issued the link.
        void Delete(Ref<Compiler> Compiler);

        /// \brief Checks whether the pipeline can be drawn with.
        ///
        /// \return `true` when the program has linked and its bindings are applied, otherwise `false`.
        ZY_INLINE Bool IsReady() const
        {
            return mHandle != 0;
        }

        /// \brief Gets the program.
        ///
        /// \return The program, `0` until its link has landed or once it failed.
        ZY_INLINE GLuint GetHandle() const
        {
            return mHandle;
        }

        /// \brief Gets the blend state, with the channels written.
        ///
        /// \return The blend state.
        ZY_INLINE ConstRef<Blend> GetBlend() const
        {
            return mBlend;
        }

        /// \brief Gets the depth state.
        ///
        /// \return The depth state.
        ZY_INLINE ConstRef<Depth> GetDepth() const
        {
            return mDepth;
        }

        /// \brief Gets the stencil state.
        ///
        /// \return The stencil state.
        ZY_INLINE ConstRef<Stencil> GetStencil() const
        {
            return mStencil;
        }

        /// \brief Gets the rasterizer state.
        ///
        /// \return The rasterizer state.
        ZY_INLINE ConstRef<Raster> GetRaster() const
        {
            return mRaster;
        }

        /// \brief Gets the primitive topology.
        ///
        /// \return The topology the pipeline's draws assemble.
        ZY_INLINE GLenum GetTopology() const
        {
            return mTopology;
        }

        /// \brief Gets the vertex attributes the program reads.
        ///
        /// \return The vertex attributes.
        ZY_INLINE ConstRef<Attributes> GetLayout() const
        {
            return mLayout;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        GLuint     mHandle;
        UInt64     mKey;
        Blend      mBlend;
        Depth      mDepth;
        Stencil    mStencil;
        Raster     mRaster;
        GLenum     mTopology;
        Attributes mLayout;
    };
}