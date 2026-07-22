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

namespace Graphic
{
    /// \brief Compiles and links the engine's shared GLSL into OpenGL programs for the GLES3 driver.
    class GLES3Compiler final
    {
    public:

        /// \brief Compiles and links a program from the given source modules.
        ///
        /// \param Program The engine program describing the shader modules and preprocessor macros.
        /// \return The linked OpenGL program name, or `0` on failure.
        GLuint Compile(ConstRef<Program> Program) const;

    private:

        /// \brief Pairs a GLSL binding name (a uniform block or sampler) with its assigned binding point.
        struct GLES3Binding final
        {
            /// The name of the binding.
            Str    Name;

            /// The binding point.
            GLint  Point = 0;
        };

        /// \brief Compiles a single shader stage from GLSL source.
        ///
        /// \param Stage  The shader stage to compile.
        /// \param Source The GLSL source module for the stage.
        /// \param Macros The preprocessor macros to inject before the source.
        /// \return The compiled shader name, or `0` on failure.
        GLuint Compile(ShaderStage Stage, ConstRef<Blob> Source, ConstRef<Sequence<Macro>> Macros) const;

        /// \brief Binds the uniform-block bindings and sampler texture units of a freshly linked program.
        ///
        /// \param Handle  The linked program to introspect and configure.
        /// \param Program The engine program providing the shader source to parse.
        void Reflect(GLuint Handle, ConstRef<Program> Program) const;

        /// \brief Parses the GLSL source to extract uniform-block bindings.
        ///
        /// \param Source The GLSL source to parse.
        /// \param Blocks The sequence to populate with extracted bindings.
        void Parse(ConstRef<Blob> Source, Ref<Sequence<GLES3Binding>> Blocks) const;
    };
}
