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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    /// \brief Represents the compiler that builds the engine's GLSL into OpenGL programs for the GLES3 driver.
    class GLES3Compiler final
    {
    public:

        /// \brief Represents a GLSL binding name (a uniform block or sampler) paired with its binding point.
        struct GLES3Binding final
        {
            /// The name of the binding.
            Str    Name;

            /// The binding point.
            GLint  Point = 0;
        };

        /// \brief Holds what a program still owes once its link is issued.
        struct GLES3Build final
        {
            /// The vertex shader, kept so its log can be read if the link fails.
            GLuint                 Vertex   = 0;

            /// The fragment shader, kept so its log can be read if the link fails.
            GLuint                 Fragment = 0;

            /// The uniform blocks declared in the source, with the binding point each asked for.
            Sequence<GLES3Binding> Blocks;

            /// The samplers declared in the source, with the texture unit each asked for.
            Sequence<GLES3Binding> Samplers;
        };

    public:

        /// \brief Lets the driver compile on its own threads where it offers to, and remembers whether it does.
        void Initialize();

        /// \brief Compiles and issues the link of a program without waiting on either.
        ///
        /// \param Program The engine program describing the shader modules and preprocessor macros.
        /// \param Build   Receives what \ref Finish needs once the link has completed.
        /// \return The OpenGL program name, or `0` when no shader could be created.
        GLuint Compile(ConstRef<Program> Program, Ref<GLES3Build> Build) const;

        /// \brief Checks whether a program's link has finished, without waiting for it.
        ///
        /// \note Without `KHR_parallel_shader_compile` the answer is always `true`, and \ref Finish then waits.
        ///
        /// \param Handle The program whose link was issued by \ref Compile.
        /// \return `true` when \ref Finish can be called without stalling, otherwise `false`.
        Bool IsReady(GLuint Handle) const;

        /// \brief Reads the outcome of a link, applies the program's bindings and releases its stages.
        ///
        /// \param Handle The program whose link was issued by \ref Compile.
        /// \param Build  The build \ref Compile handed back for it.
        /// \return `true` when the program linked and is ready to draw with, otherwise `false`.
        Bool Finish(GLuint Handle, Ref<GLES3Build> Build) const;

    private:

        /// \brief Compiles a single shader stage from GLSL source.
        ///
        /// \param Stage  The shader stage to compile.
        /// \param Source The GLSL source module for the stage.
        /// \param Macros The preprocessor macros to inject before the source.
        /// \return The shader name, or `0` when it could not be created.
        GLuint Compile(ShaderStage Stage, ConstRef<Blob> Source, ConstRef<Sequence<Macro>> Macros) const;

        /// \brief Scans GLSL source for the uniform blocks and samplers it binds explicitly.
        ///
        /// \param Source The GLSL source module to scan.
        /// \param Build  Receives each uniform block found, and each sampler not declared before.
        void Parse(ConstRef<Blob> Source, Ref<GLES3Build> Build) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bool mParallel = false;
    };
}