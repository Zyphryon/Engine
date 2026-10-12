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

#include "OGLDevice.hpp"
#include "OGLCommon.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Represents the compiler that builds the engine's GLSL into OpenGL programs for the OpenGL driver.
    class Compiler final
    {
    public:

        /// \brief Represents a uniform block or sampler the source binds, which the context could not bind itself.
        struct Binding final
        {
            /// The name of the uniform block or sampler.
            Str   Name;

            /// The binding point of a uniform block, or the texture unit of a sampler.
            GLint Point = 0;

            /// `true` for a uniform block, `false` for a sampler.
            Bool  Block = false;
        };

        /// \brief Holds what a program still owes once its link is issued.
        struct Build final
        {
            /// The vertex shader, kept so its log can be read if the link fails.
            GLuint            Vertex   = 0;

            /// The fragment shader, kept so its log can be read if the link fails.
            GLuint            Fragment = 0;

            /// The bindings taken out of the source, applied once the link has landed.
            Sequence<Binding> Bindings;
        };

        /// \brief Holds a program and the pipelines that draw with it.
        struct Linkage final
        {
            /// The OpenGL program, `0` once its link has failed.
            GLuint Handle = 0;

            /// What the program still owes until its link has landed, empty once it has settled.
            Build  Build;

            /// The number of pipelines that draw with the program.
            UInt32 Users  = 0;
        };

    public:

        /// \brief Constructs a compiler that writes no dialect until \ref Initialize reads the device.
        ///
        /// \param Device The device the programs are built for, read once its context is current.
        explicit Compiler(ConstRef<Device> Device);

        /// \brief Picks the dialect the context compiles, and lets it compile on its own threads where it offers to.
        void Initialize();

        /// \brief Issues the build of a program, or shares the one already built from the same modules and macros.
        ///
        /// \param Program The engine program describing the shader modules and preprocessor macros.
        /// \return The key the program is shared under, or `0` when no shader could be created.
        UInt64 Acquire(ConstRef<Program> Program);

        /// \brief Checks whether a program has linked, finishing it the first time it has, without waiting for it.
        ///
        /// \param Key The key \ref Acquire handed back.
        /// \return The OpenGL program once linked, or `0` while it links or once its link failed.
        GLuint Settle(UInt64 Key);

        /// \brief Lets go of a program, deleting it once no pipeline draws with it.
        ///
        /// \param Key The key \ref Acquire handed back.
        void Release(UInt64 Key);

    private:

        /// \brief Compiles and issues the link of a program without waiting on either.
        ///
        /// \param Program The engine program describing the shader modules and preprocessor macros.
        /// \param Build   Receives what \ref Finish needs once the link has completed.
        /// \return The OpenGL program name, or `0` when no shader could be created.
        GLuint Compile(ConstRef<Program> Program, Ref<Build> Build) const;

        /// \brief Checks whether a program's link has finished, without waiting for it.
        ///
        /// \note Without parallel shader compilation the answer is always `true`, and \ref Finish then waits.
        ///
        /// \param Handle The program whose link was issued by \ref Compile.
        /// \return `true` when \ref Finish can be called without stalling, otherwise `false`.
        Bool IsReady(GLuint Handle) const;

        /// \brief Reads the outcome of a link, applies the program's bindings and releases its stages.
        ///
        /// \param Handle The program whose link was issued by \ref Compile.
        /// \param Build  The build \ref Compile handed back for it.
        /// \return `true` when the program linked and is ready to draw with, otherwise `false`.
        Bool Finish(GLuint Handle, Ref<Build> Build) const;

        /// \brief Compiles a single shader stage from GLSL source.
        ///
        /// \param Stage  The shader stage to compile.
        /// \param Source The GLSL source module for the stage.
        /// \param Macros The preprocessor macros to inject before the source.
        /// \param Build  Receives the bindings taken out of the source, when the context cannot keep them.
        /// \return The shader name, or `0` when it could not be created.
        GLuint Compile(ShaderStage Stage, ConstRef<Blob> Source, ConstRef<Sequence<Macro>> Macros, Ref<Build> Build) const;

        /// \brief Points each binding taken out of the source at the slot it declared.
        ///
        /// \param Handle   The linked program.
        /// \param Bindings The bindings taken out of its stages.
        void Bind(GLuint Handle, ConstRef<Sequence<Binding>> Bindings) const;

        /// \brief Reads the info log of a shader or a program.
        ///
        /// \param Handle  The shader or program read.
        /// \param Program `true` when the handle names a program, `false` for a shader.
        /// \return The log, empty when the driver wrote none.
        Str ReadLog(GLuint Handle, Bool Program) const;

        /// \brief Logs what a stage's compile reported, when it reported anything.
        ///
        /// \param Stage  The name of the stage, for the log line.
        /// \param Shader The shader whose log is read.
        void Report(Text Stage, GLuint Shader) const;

        /// \brief Gets the key a program is shared under, hashed from its modules and macros.
        ///
        /// \param Program The program hashed.
        /// \return The key, never `0`.
        UInt64 Fingerprint(ConstRef<Program> Program) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstRef<Device>       mDevice;
        Str                    mVersion;
        Str                    mDefaults;
        Table<UInt64, Linkage> mPrograms;
    };
}