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

#include "OGLDevice.hpp"
#include "OGLApi.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Text ReadString(GLenum Name)
    {
        const ConstPtr<GLubyte> Value = glGetString(Name);
        return Value ? StrConvert(reinterpret_cast<ConstPtr<Char>>(Value)) : Text();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt32 ReadLimit(GLenum Name)
    {
        GLint Value = 0;
        glGetIntegerv(Name, AddressOf(Value));
        return static_cast<UInt32>(Max(Value, 0));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt64 HashName(Text Name)
    {
        // Emscripten lists every WebGL name twice, once with the prefix, so both forms fold into one.
        return (StrStartsWith(Name, "GL_") ? Name.Slice(3) : Name).Hash();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Profile ParseProfile(Text Version)
    {
        // ES reports "OpenGL ES 3.2 ...", and WebGL "OpenGL ES 3.0 (WebGL 2.0 ...)".
        if (StrStartsWith(Version, "OpenGL ES "))
        {
            return StrContains(Version, "WebGL") ? Profile::Web : Profile::Embedded;
        }
        return Profile::Desktop;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt16 ParseLanguage(Text Version)
    {
        // "4.60 NVIDIA", "OpenGL ES GLSL ES 3.20" and "WebGL GLSL ES 3.00 (...)" all lead with their first number.
        UInt Cursor = 0;

        while (Cursor < Version.GetSize() && !StrIsDigit(Version[Cursor]))
        {
            ++Cursor;
        }

        if (Cursor == Version.GetSize())
        {
            return 0;
        }

        const UInt16 Major = StrExtractNumber<10, UInt16>(Version, Cursor);
        UInt16       Minor = 0;

        if (StrConsume(Version, Cursor, '.'))
        {
            // Only the first two digits count, so "4.6" and "4.60" both read as sixty.
            for (UInt Digit = 0; Digit < 2; ++Digit, ++Cursor)
            {
                const Bool Valid = Cursor < Version.GetSize() && StrIsDigit(Version[Cursor]);
                Minor = static_cast<UInt16>(Minor * 10 + (Valid ? Version[Cursor] - '0' : 0));
            }
        }
        return static_cast<UInt16>(Major * 100 + Minor);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool IsBaseline(Profile Profile, UInt8 Version)
    {
        switch (Profile)
        {
        case Profile::Desktop:
            return Version >= 33;
        case Profile::Embedded:
            return Version >= 30;
        default:
            // WebGL 2 is OpenGL ES 3.0.
            return true;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Device::Device()
        : mProfile  { Profile::Desktop },
          mVersion  { 0 },
          mLanguage { 0 }
    {
        mGrants.Fill(kNoAlias);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Device::Grant(Text Name)
    {
        mExtensions.Append(HashName(Name));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Device::Initialize()
    {
        mProfile  = ParseProfile(ReadString(GL_VERSION));
        mVersion  = static_cast<UInt8>(Min(ReadLimit(GL_MAJOR_VERSION) * 10 + Min(ReadLimit(GL_MINOR_VERSION), 9u), 0xFFu));
        mLanguage = ParseLanguage(ReadString(GL_SHADING_LANGUAGE_VERSION));

        for (UInt32 Index = 0, Count = ReadLimit(GL_NUM_EXTENSIONS); Index < Count; ++Index)
        {
            if (const ConstPtr<GLubyte> Name = glGetStringi(GL_EXTENSIONS, Index))
            {
                mExtensions.Append(HashName(StrConvert(reinterpret_cast<ConstPtr<Char>>(Name))));
            }
        }

        if (!IsBaseline(mProfile, mVersion))
        {
            LOG_E("Graphics: The context ({0} {1}) is older than OpenGL 3.3 and OpenGL ES 3.0",
                ZyEnum::GetName(mProfile),
                mVersion);
            return false;
        }

        for (UInt32 Index = 0; Index < kFeatures; ++Index)
        {
            const Feature Value = static_cast<Feature>(Index);

            if (IsCore(Value))
            {
                mFeatures.Set(Index);
                continue;
            }

            for (UInt32 Candidate = 0; Candidate < sizeof(kAliases) / sizeof(kAliases[0]); ++Candidate)
            {
                if (kAliases[Candidate].Feature == Value && HasExtension(kAliases[Candidate].Name))
                {
                    mFeatures.Set(Index);
                    mGrants[Index] = static_cast<UInt8>(Candidate);
                    break;
                }
            }
        }

        // The sRGB forms are named by extensions that say nothing of S3TC itself.
        if (!Has(Feature::S3TC))
        {
            Revoke(Feature::S3TCsRGB);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Device::HasExtension(Text Name) const
    {
        return mExtensions.Contains(HashName(Name));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Device::IsCore(Feature Value) const
    {
        ConstRef<Version> Entry = kVersions[ZyEnum::Cast(Value)];

        switch (mProfile)
        {
        case Profile::Desktop:
            return Entry.Desktop  != 0 && mVersion >= Entry.Desktop;
        case Profile::Embedded:
            return Entry.Embedded != 0 && mVersion >= Entry.Embedded;
        default:
            return Entry.Web;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Device::Revoke(Feature Value)
    {
        mFeatures.Reset(ZyEnum::Cast(Value));
        mGrants[ZyEnum::Cast(Value)] = kNoAlias;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Device::Describe()
    {
        static constexpr Text kBackends[] = { "OpenGL", "OpenGLES", "WebGL" };

        mDescription.Backend  = kBackends[ZyEnum::Cast(mProfile)];
        mDescription.Language = ShaderLanguage::GLSL;

        // Compute and storage buffers arrive with desktop 4.3 and ES 3.1; WebGL 2 never reaches either.
        const Bool Extended = (mProfile == Profile::Desktop  && mVersion >= 43)
                           || (mProfile == Profile::Embedded && mVersion >= 31);
        mDescription.Tier = Extended ? Tier::Level3 : Tier::Level2;

        Ref<Capabilities> Limits = mDescription.Capabilities;
        Limits.SupportsFormatS3TC    = Has(Feature::S3TC);
        Limits.SupportsFormatRGTC    = Has(Feature::RGTC);
        Limits.SupportsFormatBPTC    = Has(Feature::BPTC);
        Limits.SupportsFormatETC2    = Has(Feature::ETC2);
        Limits.SupportsBaseVertex    = Has(Feature::BaseVertex);
        Limits.MaxTextureDimension   = ReadLimit(GL_MAX_TEXTURE_SIZE);
        Limits.MaxTextureLayers      = Min(ReadLimit(GL_MAX_ARRAY_TEXTURE_LAYERS), 0xFFFFu);
        Limits.MaxTextureMipmaps     = kMaxMipmaps;
        Limits.MaxTextureSlots       = Min(ReadLimit(GL_MAX_TEXTURE_IMAGE_UNITS), Command::kMaxTextures);
        Limits.MaxRenderTargets      = Min(ReadLimit(GL_MAX_COLOR_ATTACHMENTS), kMaxAttachments);
        Limits.MaxVertexAttributes   = Min(ReadLimit(GL_MAX_VERTEX_ATTRIBS), kMaxAttributes);
        Limits.MaxVertexStreams      = Command::kMaxVertices;
        Limits.UniformBlockAlignment = Min(ReadLimit(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT), 0xFFFFu);
        Limits.UniformBlockCapacity  = ReadLimit(GL_MAX_UNIFORM_BLOCK_SIZE);
        Limits.MaxAnisotropy         = 0;

        if (Has(Feature::Anisotropy))
        {
            GLfloat Anisotropy = 1.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, AddressOf(Anisotropy));

            Limits.MaxAnisotropy = static_cast<UInt8>(Clamp(Anisotropy, 1.0f, 16.0f));
        }

        Ref<Adapter> Adapter = mDescription.Adapters.Append();
        Adapter.Description  = ReadString(GL_RENDERER);
        Adapter.Memory       = 0;
    }
}