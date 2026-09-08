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

#include "GLES3Compiler.hpp"

#if defined(ZY_PLATFORM_WEB)
#include <emscripten/html5.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool GLES3FindWord(Text Content, Text Word, Ref<UInt> Cursor)
    {
        while (Cursor < Content.GetSize())
        {
            const SInt Relative = StrFind(Content.Slice(Cursor), Word);
            if (Relative < 0)
            {
                return false;
            }

            const UInt Start = Cursor + static_cast<UInt>(Relative);
            const UInt End   = Start + Word.GetSize();
            Cursor = End;

            // Accept only whole-word matches (both edges on non-identifier boundaries).
            const Bool Before = (Start == 0)               || !StrIsIdentifier(Content[Start - 1]);
            const Bool After  = (End == Content.GetSize()) || !StrIsIdentifier(Content[End]);
            if (Before && After)
            {
                return true;
            }
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Text GLES3ReadWord(Text Content, Ref<UInt> Cursor)
    {
        const UInt Start = Cursor;
        StrSkip(Content, Cursor, [](Char Character) { return StrIsIdentifier(Character); });
        return (Cursor > Start) ? Text(Content.GetData() + Start, Cursor - Start) : Text();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Str GLES3StripBinding(ConstRef<Blob> Source)
    {
        const Text Code(Source.GetData<Char>(), Source.GetSize());

        const auto IsInlineSpace = [](Char Character) { return Character == ' ' || Character == '\t'; };

        Str  Result;
        UInt Copied = 0;
        UInt Cursor = 0;

        while (GLES3FindWord(Code, "binding", Cursor))
        {
            const UInt Word = Cursor - 7;

            // A binding qualifier reads `binding [ws] = [ws] digits`; anything else keeps the word in place.
            UInt End = Cursor;
            StrSkip(Code, End, IsInlineSpace);
            if (!StrConsume(Code, End, '='))
            {
                continue;
            }
            StrSkip(Code, End, IsInlineSpace);
            StrSkip(Code, End, [](Char Character) { return StrIsDigit(Character); });

            // Extend the removed span over one adjacent separator: a leading ", " or a trailing ",".
            UInt RemoveStart = Word;
            UInt Back        = Word;
            while (Back > Copied && IsInlineSpace(Code[Back - 1]))
            {
                --Back;
            }

            UInt Forward = End;
            while (Forward < Code.GetSize() && IsInlineSpace(Code[Forward]))
            {
                ++Forward;
            }

            if (Back > Copied && Code[Back - 1] == '(' && Forward < Code.GetSize() && Code[Forward] == ')')
            {
                // Nothing else stood in the list, and an empty `layout()` is not valid, so the whole one goes.
                UInt Head = Back - 1;
                while (Head > Copied && IsInlineSpace(Code[Head - 1]))
                {
                    --Head;
                }
                while (Head > Copied && StrIsIdentifier(Code[Head - 1]))
                {
                    --Head;
                }

                RemoveStart = Head;
                End         = Forward + 1;

                while (End < Code.GetSize() && IsInlineSpace(Code[End]))
                {
                    ++End;
                }
            }
            else if (Back > Copied && Code[Back - 1] == ',')
            {
                RemoveStart = Back - 1;
            }
            else if (Forward < Code.GetSize() && Code[Forward] == ',')
            {
                End = Forward + 1;
            }

            Result.Append(Text(Code.GetData() + Copied, RemoveStart - Copied));
            Copied = End;
            Cursor = End;
        }

        Result.Append(Text(Code.GetData() + Copied, Code.GetSize() - Copied));
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Compiler::Initialize()
    {
#if defined(ZY_PLATFORM_WEB)
        mParallel = emscripten_webgl_enable_extension(
            emscripten_webgl_get_current_context(), "KHR_parallel_shader_compile") == EM_TRUE;
#else
        mParallel = (GLAD_GL_KHR_parallel_shader_compile != 0);

        if (mParallel)
        {
            glMaxShaderCompilerThreadsKHR(0xFFFFFFFF);
        }
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLuint GLES3Compiler::Compile(ConstRef<Program> Program, Ref<GLES3Build> Build) const
    {
        Build.Vertex   = Compile(ShaderStage::Vertex,   Program.Modules[Enum::Cast(ShaderStage::Vertex)],   Program.Macros);
        Build.Fragment = Compile(ShaderStage::Fragment, Program.Modules[Enum::Cast(ShaderStage::Fragment)], Program.Macros);

        if (Build.Vertex == 0 || Build.Fragment == 0)
        {
            glDeleteShader(Build.Vertex);
            glDeleteShader(Build.Fragment);
            return 0;
        }

        const GLuint Handle = glCreateProgram();

        glAttachShader(Handle, Build.Vertex);
        glAttachShader(Handle, Build.Fragment);
        glLinkProgram(Handle);

        for (ConstRef<Blob> Module : Program.Modules)
        {
            if (Module.GetSize() > 0)
            {
                Parse(Module, Build.Blocks);
                Sample(Module, Build.Samplers);
            }
        }
        return Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool GLES3Compiler::IsReady(GLuint Handle) const
    {
        GLint Completed = GL_TRUE;

        if (mParallel)
        {
            glGetProgramiv(Handle, GL_COMPLETION_STATUS_KHR, AddressOf(Completed));
        }
        return Completed == GL_TRUE;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool GLES3Compiler::Finish(GLuint Handle, Ref<GLES3Build> Build) const
    {
        GLint Linked = GL_FALSE;
        glGetProgramiv(Handle, GL_LINK_STATUS, AddressOf(Linked));

        if (Linked == GL_FALSE)
        {
            GLchar  Message[1024] { };
            GLsizei Length = 0;

            glGetProgramInfoLog(Handle, sizeof(Message), AddressOf(Length), Message);
            LOG_E("GLES3Compiler: Failed to link program: {0}", Text(Message, Length));

            // A stage that failed to compile surfaces here, since nothing asked it earlier.
            for (const GLuint Shader : { Build.Vertex, Build.Fragment })
            {
                glGetShaderInfoLog(Shader, sizeof(Message), AddressOf(Length), Message);

                if (Length > 0)
                {
                    LOG_E("GLES3Compiler: {0}", Text(Message, Length));
                }
            }
        }

        glDetachShader(Handle, Build.Vertex);
        glDetachShader(Handle, Build.Fragment);
        glDeleteShader(Build.Vertex);
        glDeleteShader(Build.Fragment);

        if (Linked == GL_FALSE)
        {
            return false;
        }

        // Bind each uniform block to its declared binding point.
        for (ConstRef<GLES3Binding> Block : Build.Blocks)
        {
            const GLuint Index = glGetUniformBlockIndex(Handle, Block.Name.GetData());

            if (Index != GL_INVALID_INDEX)
            {
                glUniformBlockBinding(Handle, Index, Block.Point);
            }
        }

        glUseProgram(Handle);

        for (ConstRef<GLES3Binding> Sampler : Build.Samplers)
        {
            if (const GLint Location = glGetUniformLocation(Handle, Sampler.Name.GetData()); Location >= 0)
            {
                glUniform1i(Location, Sampler.Point);
            }
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLuint GLES3Compiler::Compile(ShaderStage Stage, ConstRef<Blob> Source, ConstRef<Sequence<Macro>> Macros) const
    {
        Str Preamble;

#if defined(ZY_PLATFORM_WEB)

        Preamble.Append("#version 300 es\n");
        Preamble.Append("precision highp float;\n");
        Preamble.Append("precision highp int;\n");
        Preamble.Append("precision highp sampler2D;\n");
        Preamble.Append("precision highp sampler2DArray;\n");
        Preamble.Append("precision highp sampler2DShadow;\n");

#else

        Preamble.Append("#version 330 core\n");
        Preamble.Append("#extension GL_ARB_shading_language_420pack : enable\n");

#endif

        if (Stage == ShaderStage::Vertex)
        {
            Preamble.Append("#define VERTEX_SHADER 1\n");
        }
        else
        {
            Preamble.Append("#define FRAGMENT_SHADER 1\n");
        }

        for (ConstRef<Macro> Macro : Macros)
        {
            Preamble.Append("#define ");
            Preamble.Append(Text(Macro.Name.GetData(), Macro.Name.GetSize()));

            if (!Macro.Value.IsEmpty())
            {
                Preamble.Append(' ');
                Preamble.Append(Text(Macro.Value.GetData(), Macro.Value.GetSize()));
            }
            Preamble.Append('\n');
        }

#if defined(ZY_PLATFORM_WEB)

        const Str  Body = GLES3StripBinding(Source);

#else

        const Text Body(Source.GetData<Char>(), Source.GetSize());

#endif

        const ConstPtr<GLchar> Sources[] = { Preamble.GetData(),                     Body.GetData(),                     "\n" };
        const GLint            Lengths[] = { static_cast<GLint>(Preamble.GetSize()), static_cast<GLint>(Body.GetSize()), 1    };

        const GLuint Handle = glCreateShader(GLES3Convert(Stage));
        glShaderSource(Handle, 3, Sources, Lengths);
        glCompileShader(Handle);
        return Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Compiler::Sample(ConstRef<Blob> Source, Ref<Sequence<GLES3Binding>> Samplers) const
    {
        UInt Cursor = 0;

        const Text Code(Source.GetData<Char>(), Source.GetSize());

        while (GLES3FindWord(Code, "layout", Cursor))
        {
            StrSkipWhitespace(Code, Cursor);

            if (!StrConsume(Code, Cursor, '('))
            {
                continue;
            }

            // Isolate the parenthesized qualifier list.
            const SInt CloseRelative = StrFind(Code.Slice(Cursor), ')');
            if (CloseRelative <= 0)
            {
                continue;
            }

            const Text Qualifiers(Code.GetData() + Cursor, static_cast<UInt>(CloseRelative));
            Cursor += static_cast<UInt>(CloseRelative) + 1;             // Skip past ')'.

            // Recover the binding point declared inside the qualifier list, if any: `binding = N`.
            UInt Inner = 0;

            if (!GLES3FindWord(Qualifiers, "binding", Inner))
            {
                continue;
            }

            StrSkipWhitespace(Qualifiers, Inner);

            if (!StrConsume(Qualifiers, Inner, '='))
            {
                continue;
            }

            StrSkipWhitespace(Qualifiers, Inner);

            if (Inner >= Qualifiers.GetSize() || !StrIsDigit(Qualifiers[Inner]))
            {
                continue;
            }
            const GLint Point = StrExtractNumber<10, GLint>(Qualifiers, Inner);

            StrSkipWhitespace(Code, Cursor);

            if (!StrEqualCase(GLES3ReadWord(Code, Cursor), "uniform"))
            {
                continue;
            }

            StrSkipWhitespace(Code, Cursor);

            if (!StrContains(GLES3ReadWord(Code, Cursor), "sampler"))
            {
                continue;
            }

            StrSkipWhitespace(Code, Cursor);

            const Text Name = GLES3ReadWord(Code, Cursor);

            if (Name.IsEmpty())
            {
                continue;
            }

            // Both stages may name the same sampler, and both then mean the one texture.
            Bool Declared = false;

            for (ConstRef<GLES3Binding> Sampler : Samplers)
            {
                Declared = Declared || StrEqualCase(Text(Sampler.Name.GetData(), Sampler.Name.GetSize()), Name);
            }

            if (!Declared)
            {
                Samplers.Append(Name, Point);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Compiler::Parse(ConstRef<Blob> Source, Ref<Sequence<GLES3Binding>> Blocks) const
    {
        UInt Cursor = 0;

        const Text Code(Source.GetData<Char>(), Source.GetSize());

        while (GLES3FindWord(Code, "layout", Cursor))
        {
            StrSkipWhitespace(Code, Cursor);

            if (!StrConsume(Code, Cursor, '('))
            {
                continue;
            }

            // Isolate the parenthesized qualifier list.
            const SInt CloseRelative = StrFind(Code.Slice(Cursor), ')');
            if (CloseRelative <= 0)
            {
                continue;
            }

            const Text Qualifiers(Code.GetData() + Cursor, static_cast<UInt>(CloseRelative));
            Cursor += static_cast<UInt>(CloseRelative) + 1;             // Skip past ')'.

            // Recover the binding point declared inside the qualifier list, if any: `binding = N`.
            UInt Inner = 0;

            if (!GLES3FindWord(Qualifiers, "binding", Inner))
            {
                continue;
            }

            StrSkipWhitespace(Qualifiers, Inner);

            if (!StrConsume(Qualifiers, Inner, '='))
            {
                continue;
            }

            StrSkipWhitespace(Qualifiers, Inner);

            if (Inner >= Qualifiers.GetSize() || !StrIsDigit(Qualifiers[Inner]))
            {
                continue;
            }
            const GLint Point = StrExtractNumber<10, GLint>(Qualifiers, Inner);

            StrSkipWhitespace(Code, Cursor);

            if (!StrEqualCase(GLES3ReadWord(Code, Cursor), "uniform"))
            {
                continue;
            }

            StrSkipWhitespace(Code, Cursor);

            if (const Text Name = GLES3ReadWord(Code, Cursor); !Name.IsEmpty() && !StrContains(Name, "sampler"))
            {
                Blocks.Append(Name, Point);
            }
        }
    }
}