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

#include "OGLCompiler.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Text ReadWord(Text Content, Ref<UInt> Cursor)
    {
        const UInt Start = Cursor;
        StrSkip(Content, Cursor, StrIsIdentifier);
        return Content.Slice(Start, Cursor - Start);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Text Trim(Text Value)
    {
        UInt Start = 0;
        UInt End   = Value.GetSize();

        StrSkip(Value, Start, StrIsSpace);

        while (End > Start && StrIsSpace(Value[End - 1]))
        {
            --End;
        }
        return Value.Slice(Start, End - Start);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool SkipComment(Text Code, Ref<UInt> Cursor)
    {
        if (StrConsume(Code, Cursor, "//"))
        {
            while (Cursor < Code.GetSize() && Code[Cursor] != '\n')
            {
                ++Cursor;
            }
            return true;
        }

        if (StrConsume(Code, Cursor, "/*"))
        {
            const SInt End = StrFind(Code.Slice(Cursor), "*/");
            Cursor = (End < 0) ? Code.GetSize() : Cursor + static_cast<UInt>(End) + 2;
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Text ReadDirective(Text Code, Ref<UInt> Cursor)
    {
        const UInt Start = Cursor;

        while (Cursor < Code.GetSize() && Code[Cursor] != '\n')
        {
            ++Cursor;
        }
        return Code.Slice(Start, Cursor - Start);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool IsExtension(Text Directive)
    {
        UInt Cursor = 1;

        StrSkip(Directive, Cursor, StrIsSpace);
        return ReadWord(Directive, Cursor) == "extension";
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void SkipSpace(Text Code, Ref<UInt> Cursor)
    {
        while (Cursor < Code.GetSize())
        {
            if (StrIsSpace(Code[Cursor]))
            {
                ++Cursor;
            }
            else if (!SkipComment(Code, Cursor))
            {
                break;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ReadBinding(Text Qualifier, Ref<GLint> Point)
    {
        UInt Cursor = 0;

        if (ReadWord(Qualifier, Cursor) != "binding")
        {
            return false;
        }

        StrSkip(Qualifier, Cursor, StrIsSpace);

        if (!StrConsume(Qualifier, Cursor, '='))
        {
            return false;
        }

        StrSkip(Qualifier, Cursor, StrIsSpace);

        if (Cursor >= Qualifier.GetSize() || !StrIsDigit(Qualifier[Cursor]))
        {
            return false;
        }

        Point = StrExtractNumber<10, GLint>(Qualifier, Cursor);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool Record(Text Code, UInt Cursor, GLint Point, Ref<Sequence<Compiler::Binding>> Bindings)
    {
        SkipSpace(Code, Cursor);

        if (ReadWord(Code, Cursor) != "uniform")
        {
            return false;
        }

        SkipSpace(Code, Cursor);

        Text Type = ReadWord(Code, Cursor);

        // A precision qualifier may stand before a sampler's type, and is no part of its name.
        if (Type == "lowp" || Type == "mediump" || Type == "highp")
        {
            SkipSpace(Code, Cursor);
            Type = ReadWord(Code, Cursor);
        }

        // A block is known by the name after `uniform`, a sampler by the one after its type.
        const Bool Block = !StrContains(Type, "sampler");
        Text       Name  = Type;

        if (!Block)
        {
            SkipSpace(Code, Cursor);
            Name = ReadWord(Code, Cursor);
        }

        if (Name.IsEmpty())
        {
            return false;
        }

        // Both stages may declare the same resource, and both then mean the one slot.
        const Bool Declared = Bindings.Contains([& Name](ConstRef<Compiler::Binding> Entry)
        {
            return Entry.Name == Name;
        });

        if (!Declared)
        {
            Bindings.Append(Name, Point, Block);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Str Rewrite(Text Code, Bool Unbind, Ref<Str> Extensions, Ref<Sequence<Compiler::Binding>> Bindings)
    {
        Str  Result;
        UInt Copied = 0;
        UInt Cursor = 0;

        while (Cursor < Code.GetSize())
        {
            // What a comment says is never a declaration, though it may well mention one.
            if (SkipComment(Code, Cursor))
            {
                continue;
            }

            if (Code[Cursor] == '#')
            {
                const UInt Start     = Cursor;
                const Text Directive = ReadDirective(Code, Cursor);

                // An extension must come before any declaration, so it moves above the defaults the preamble declares.
                if (IsExtension(Directive))
                {
                    Extensions.Append(Directive);
                    Extensions.Append('\n');

                    Result.Append(Code.Slice(Copied, Start - Copied));
                    Copied = Cursor;
                }
                continue;
            }

            if (!Unbind || !StrIsIdentifier(Code[Cursor]))
            {
                ++Cursor;
                continue;
            }

            const UInt Start = Cursor;

            if (ReadWord(Code, Cursor) != "layout")
            {
                continue;
            }

            UInt Open = Cursor;
            SkipSpace(Code, Open);

            if (!StrConsume(Code, Open, '('))
            {
                continue;
            }

            const SInt Length = StrFind(Code.Slice(Open), ')');

            if (Length < 0)
            {
                break;
            }

            const UInt Close = Open + static_cast<UInt>(Length);
            GLint      Point = -1;
            Str        Kept;

            for (UInt Item = Open; Item <= Close;)
            {
                const SInt Comma = StrFind(Code.Slice(Item, Close - Item), ',');
                const UInt End   = (Comma < 0) ? Close : Item + static_cast<UInt>(Comma);
                const Text Entry = Trim(Code.Slice(Item, End - Item));

                if (!Entry.IsEmpty() && !ReadBinding(Entry, Point))
                {
                    if (!Kept.IsEmpty())
                    {
                        Kept.Append(", ");
                    }
                    Kept.Append(Entry);
                }
                Item = End + 1;
            }

            Cursor = Close + 1;

            // Anything but a uniform block or a sampler keeps its qualifiers untouched.
            if (Point < 0 || !Record(Code, Cursor, Point, Bindings))
            {
                continue;
            }

            Result.Append(Code.Slice(Copied, Start - Copied));

            // An empty `layout()` is not valid, so a list that held the binding alone goes with it.
            if (!Kept.IsEmpty())
            {
                Result.Append("layout(");
                Result.Append(Kept);
                Result.Append(')');
            }

            // Line breaks inside the list stay, so the compiler's line numbers still match the source.
            Result.Append('\n', StrCount(Code.Slice(Start, Cursor - Start), '\n'));
            Copied = Cursor;
        }

        Result.Append(Code.Slice(Copied));
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Compiler::Compiler(ConstRef<Device> Device)
        : mDevice { Device }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Compiler::Initialize()
    {
        const UInt16 Language = mDevice.GetLanguage();

        if (mDevice.GetProfile() == Profile::Desktop)
        {
            // 4.1 and newer get their own dialect, so a Level3 context has gather in core; 4.0 stays on 3.30.
            const UInt32 Version = (Language >= 410) ? Min(Language - Language % 10u, 460u) : 330u;

            mVersion.Format<"#version {0} core\n">(Version);

            if (mDevice.Has(Feature::ShaderBinding) && Version < 420)
            {
                mVersion.Append("#extension GL_ARB_shading_language_420pack : require\n");
            }
        }
        else
        {
            const UInt32 Version = (Language >= 320) ? 320u : (Language >= 310) ? 310u : 300u;

            mVersion.Format<"#version {0} es\n">(Version);

            // ES leaves most opaque types without a default precision, and every one used must then name it.
            mDefaults.Append("precision highp float;\n");
            mDefaults.Append("precision highp int;\n");
            mDefaults.Append("precision highp sampler2D;\n");
            mDefaults.Append("precision highp sampler3D;\n");
            mDefaults.Append("precision highp samplerCube;\n");
            mDefaults.Append("precision highp sampler2DShadow;\n");
            mDefaults.Append("precision highp samplerCubeShadow;\n");
            mDefaults.Append("precision highp sampler2DArray;\n");
            mDefaults.Append("precision highp sampler2DArrayShadow;\n");
            mDefaults.Append("precision highp isampler2D;\n");
            mDefaults.Append("precision highp isampler3D;\n");
            mDefaults.Append("precision highp isamplerCube;\n");
            mDefaults.Append("precision highp isampler2DArray;\n");
            mDefaults.Append("precision highp usampler2D;\n");
            mDefaults.Append("precision highp usampler3D;\n");
            mDefaults.Append("precision highp usamplerCube;\n");
            mDefaults.Append("precision highp usampler2DArray;\n");
        }

        // The engine lays every uniform block out as HLSL packs a constant buffer, which std140 matches.
        mDefaults.Append("layout(std140) uniform;\n");

        if (mDevice.Has(Feature::ParallelCompile))
        {
            glMaxShaderCompilerThreads(0xFFFFFFFF);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Compiler::Acquire(ConstRef<Program> Program)
    {
        const UInt64 Key = Fingerprint(Program);

        // A program switch is what a draw costs most under OpenGL, so pipelines that differ only in their states share one.
        if (const Ptr<Linkage> Shared = mPrograms.Find(Key))
        {
            ++Shared->Users;
            return Key;
        }

        Linkage Entry;
        Entry.Handle = Compile(Program, Entry.Build);
        Entry.Users  = 1;

        if (Entry.Handle == 0)
        {
            return 0;
        }

        mPrograms.Assign(Key, Move(Entry));
        return Key;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLuint Compiler::Settle(UInt64 Key)
    {
        Ref<Linkage> Entry = * mPrograms.Find(Key);

        // A program left without stages has settled, whether its link landed or not.
        if (Entry.Build.Vertex == 0)
        {
            return Entry.Handle;
        }

        if (!IsReady(Entry.Handle))
        {
            return 0;
        }

        if (!Finish(Entry.Handle, Entry.Build))
        {
            glDeleteProgram(Entry.Handle);

            Entry.Handle = 0;
        }

        // The stages went with the link, so nothing of the build is left to own.
        Entry.Build = Build();
        return Entry.Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Compiler::Release(UInt64 Key)
    {
        Ref<Linkage> Entry = * mPrograms.Find(Key);

        if (--Entry.Users > 0)
        {
            return;
        }

        // A program deleted before its link landed still owns its stages.
        if (Entry.Handle)
        {
            glDeleteShader(Entry.Build.Vertex);
            glDeleteShader(Entry.Build.Fragment);
            glDeleteProgram(Entry.Handle);
        }
        mPrograms.Erase(Key);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLuint Compiler::Compile(ConstRef<Program> Program, Ref<Build> Build) const
    {
        Build.Vertex   = Compile(
            ShaderStage::Vertex,
            Program.Modules[ZyEnum::Cast(ShaderStage::Vertex)],
            Program.Macros,
            Build);
        Build.Fragment = Compile(
            ShaderStage::Fragment,
            Program.Modules[ZyEnum::Cast(ShaderStage::Fragment)],
            Program.Macros,
            Build);

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
        return Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Compiler::IsReady(GLuint Handle) const
    {
        GLint Completed = GL_TRUE;

        if (mDevice.Has(Feature::ParallelCompile))
        {
            glGetProgramiv(Handle, GL_COMPLETION_STATUS_KHR, AddressOf(Completed));
        }
        return Completed == GL_TRUE;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Compiler::Finish(GLuint Handle, Ref<Build> Build) const
    {
        GLint Linked = GL_FALSE;
        glGetProgramiv(Handle, GL_LINK_STATUS, AddressOf(Linked));

        if (Linked == GL_FALSE)
        {
            LOG_E("Graphics: Failed to link program: {0}", Text(ReadLog(Handle, true)));

            // A stage that failed to compile surfaces here, since nothing asked it earlier.
            Report("Vertex", Build.Vertex);
            Report("Fragment", Build.Fragment);
        }

        glDetachShader(Handle, Build.Vertex);
        glDetachShader(Handle, Build.Fragment);
        glDeleteShader(Build.Vertex);
        glDeleteShader(Build.Fragment);

        if (Linked == GL_FALSE)
        {
            return false;
        }

        if (!Build.Bindings.IsEmpty())
        {
            Bind(Handle, Build.Bindings);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GLuint Compiler::Compile(ShaderStage Stage, ConstRef<Blob> Source, ConstRef<Sequence<Macro>> Macros, Ref<Build> Build) const
    {
        // Extensions are hoisted out of the source, and binding qualifiers taken out where the context cannot keep them.
        Str        Extensions;
        const Text Code(Source.GetData<Char>(), Source.GetSize());
        const Str  Body = Rewrite(Code, !mDevice.Has(Feature::ShaderBinding), Extensions, Build.Bindings);

        Str Preamble = mVersion;
        Preamble.Append(Extensions);
        Preamble.Append(mDefaults);

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
            Preamble.Append(Macro.Name);

            if (!Macro.Value.IsEmpty())
            {
                Preamble.Append(' ');
                Preamble.Append(Macro.Value);
            }
            Preamble.Append('\n');
        }

        // A compiler's errors then count the lines of the source rather than of what was put before it.
        Preamble.Append("#line 1\n");

        const Array Sources(Preamble.GetData(),                     Body.GetData(),                     "\n");
        const Array Lengths(static_cast<GLint>(Preamble.GetSize()), static_cast<GLint>(Body.GetSize()), 1);

        const GLuint Handle = glCreateShader(Convert(Stage));
        glShaderSource(Handle, 3, Sources.GetData(), Lengths.GetData());
        glCompileShader(Handle);
        return Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Compiler::Bind(GLuint Handle, ConstRef<Sequence<Binding>> Bindings) const
    {
        const Bool Uniform = mDevice.Has(Feature::ProgramUniform);

        // Without program uniforms a sampler's unit is set on the program in use, which the next draw replaces anyway.
        if (!Uniform)
        {
            glUseProgram(Handle);
        }

        for (ConstRef<Binding> Entry : Bindings)
        {
            if (Entry.Block)
            {
                if (const GLuint Index = glGetUniformBlockIndex(Handle, Entry.Name.GetData()); Index != GL_INVALID_INDEX)
                {
                    glUniformBlockBinding(Handle, Index, static_cast<GLuint>(Entry.Point));
                }
            }
            else if (const GLint Location = glGetUniformLocation(Handle, Entry.Name.GetData()); Location >= 0)
            {
                if (Uniform)
                {
                    glProgramUniform1i(Handle, Location, Entry.Point);
                }
                else
                {
                    glUniform1i(Location, Entry.Point);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Str Compiler::ReadLog(GLuint Handle, Bool Program) const
    {
        GLint Length = 0;

        if (Program)
        {
            glGetProgramiv(Handle, GL_INFO_LOG_LENGTH, AddressOf(Length));
        }
        else
        {
            glGetShaderiv(Handle, GL_INFO_LOG_LENGTH, AddressOf(Length));
        }

        Str Log;

        // The length counts the terminator, so a log of one character is empty.
        if (Length > 1)
        {
            GLsizei Written = 0;

            Log.Append('\0', static_cast<UInt>(Length));

            if (Program)
            {
                glGetProgramInfoLog(Handle, Length, AddressOf(Written), Log.GetData());
            }
            else
            {
                glGetShaderInfoLog(Handle, Length, AddressOf(Written), Log.GetData());
            }

            Log.Truncate(static_cast<UInt>(Max(Written, 0)));
        }
        return Log;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Compiler::Report(Text Stage, GLuint Shader) const
    {
        if (const Str Log = ReadLog(Shader, false); !Log.IsEmpty())
        {
            LOG_E("Graphics: {0} shader: {1}", Stage, Text(Log));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Compiler::Fingerprint(ConstRef<Program> Program) const
    {
        UInt64 Key = 0;

        for (ConstRef<Blob> Module : Program.Modules)
        {
            Key = HashCombine(Key, Text(Module.GetData<Char>(), Module.GetSize()));
        }

        for (ConstRef<Macro> Macro : Program.Macros)
        {
            Key = HashCombine(Key, Macro.Name, Macro.Value);
        }

        // Zero tells the caller that nothing was built, so no program is ever filed under it.
        return (Key != 0 ? Key : 1);
    }
}