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
            if (Back > Copied && Code[Back - 1] == ',')
            {
                RemoveStart = Back - 1;
            }
            else
            {
                UInt Forward = End;
                while (Forward < Code.GetSize() && IsInlineSpace(Code[Forward]))
                {
                    ++Forward;
                }
                if (Forward < Code.GetSize() && Code[Forward] == ',')
                {
                    End = Forward + 1;
                }
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

    static Bool GLES3IsSampler(GLenum Type)
    {
        switch (Type)
        {
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
            return true;
        default:
            return false;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLuint GLES3Compiler::Compile(ConstRef<Program> Program) const
    {
        const GLuint Vertex   = Compile(ShaderStage::Vertex,   Program.Modules[Enum::Cast(ShaderStage::Vertex)],   Program.Macros);
        const GLuint Fragment = Compile(ShaderStage::Fragment, Program.Modules[Enum::Cast(ShaderStage::Fragment)], Program.Macros);

        if (Vertex == 0 || Fragment == 0)
        {
            if (Vertex)
            {
                glDeleteShader(Vertex);
            }
            if (Fragment)
            {
                glDeleteShader(Fragment);
            }
            return 0;
        }

        const GLuint Handle = glCreateProgram();
        glAttachShader(Handle, Vertex);
        glAttachShader(Handle, Fragment);
        glLinkProgram(Handle);

        glDetachShader(Handle, Vertex);
        glDetachShader(Handle, Fragment);

        glDeleteShader(Vertex);
        glDeleteShader(Fragment);

        GLint Linked = GL_FALSE;
        glGetProgramiv(Handle, GL_LINK_STATUS, AddressOf(Linked));

        if (Linked == GL_FALSE)
        {
            GLchar  Message[1024] { };
            GLsizei Length = 0;
            glGetProgramInfoLog(Handle, sizeof(Message), AddressOf(Length), Message);

            LOG_E("GLES3Compiler: Failed to link program: {0}", Text(Message, Length));
            glDeleteProgram(Handle);
            return 0;
        }

        Reflect(Handle, Program);
        return Handle;
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

        const ConstPtr<GLchar> Sources[] = { Preamble.GetData(),                     Body.GetData()                     };
        const GLint            Lengths[] = { static_cast<GLint>(Preamble.GetSize()), static_cast<GLint>(Body.GetSize()) };

        const GLuint Handle = glCreateShader(GLES3Convert(Stage));
        glShaderSource(Handle, 2, Sources, Lengths);
        glCompileShader(Handle);

        GLint Compiled = GL_FALSE;
        glGetShaderiv(Handle, GL_COMPILE_STATUS, AddressOf(Compiled));

        if (Compiled == GL_FALSE)
        {
            GLchar  Message[1024] { };
            GLsizei Length = 0;
            glGetShaderInfoLog(Handle, sizeof(Message), AddressOf(Length), Message);

            LOG_E("GLES3Compiler: Failed to compile {0} shader: {1}", Enum::GetName(Stage), Text(Message, Length));
            glDeleteShader(Handle);
            return 0;
        }
        return Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GLES3Compiler::Reflect(GLuint Handle, ConstRef<Program> Program) const
    {
        Sequence<GLES3Binding> Blocks;

        for (ConstRef<Blob> Module : Program.Modules)
        {
            if (Module.GetSize() > 0)
            {
                Parse(Module, Blocks);
            }
        }

        // Bind each uniform block to its declared binding point.
        for (ConstRef<GLES3Binding> Block : Blocks)
        {
            const GLuint Index = glGetUniformBlockIndex(Handle, Block.Name.GetData());

            if (Index != GL_INVALID_INDEX)
            {
                glUniformBlockBinding(Handle, Index, Block.Point);
            }
        }

        glUseProgram(Handle);

        // Assign each sampler the next sequential texture unit in enumeration order.
        GLint Count = 0;
        glGetProgramiv(Handle, GL_ACTIVE_UNIFORMS, AddressOf(Count));

        GLint Unit = 0;
        for (GLint Index = 0; Index < Count; ++Index)
        {
            GLchar  Name[128] { };
            GLsizei Length = 0;
            GLint   Size   = 0;
            GLenum  Type   = 0;
            glGetActiveUniform(Handle, Index, sizeof(Name), AddressOf(Length), AddressOf(Size), AddressOf(Type), Name);

            if (!GLES3IsSampler(Type))
            {
                continue;
            }

            if (const GLint Location = glGetUniformLocation(Handle, Name); Location >= 0)
            {
                glUniform1i(Location, Unit++);
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
