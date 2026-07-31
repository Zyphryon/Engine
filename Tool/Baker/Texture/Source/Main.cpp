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

#include "Main.hpp"
#include "Baker.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Texture
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Str Derive(Text Source, Text Extension)
    {
        const SInt Separator = StrFindLast(Source, '.');

        Str Result(Separator > 0 ? Source.Slice(0, Separator) : Source);
        Result.Append('.');
        Result.Append(Extension);
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void Usage(ConstRef<Baker> Instance)
    {
        ConstRef<Baker::Registry> Registry = Instance.GetRegistry();

        Str Types;

        // The registry holds one entry per accepted extension, so listing it needs no visit to the importers.
        for (UInt Index = 0; Index < Registry.GetSize(); ++Index)
        {
            if (!Types.IsEmpty())
            {
                Types.Append(", ");
            }
            Types.Append(Registry.GetData()[Index].First);
        }

        LOG_I("Usage: ZyBakerTexture <source> [destination] [options]");
        LOG_I("");
        LOG_I("Bakes one source image into the engine's native '.{0}' format. The destination defaults to the", Exporter::kOutput);
        LOG_I("source path with its extension replaced.");
        LOG_I("");
        LOG_I("Sources : {0}", Types);
        LOG_I("");
        LOG_I("Options :");
        LOG_I("  --format <name>   Any texture format name, such as R8UIntNorm or RGBA16Float (default: inferred)");
        LOG_I("  --mipmaps         Generate a full mip chain down to 1x1                      (default: off)");
        LOG_I("  --compressed      LZ4-compress the payload when it shrinks the output        (default: on)");
        LOG_I("  --linear          Treat the source as linear-encoded colour                  (default: on)");
        LOG_I("  --cube            Cut the source into the six faces of a cube map            (default: off)");
        LOG_I("");
        LOG_I("Every switch may be negated with its '--no-' spelling, as in '--no-mipmaps'.");
        LOG_I("");
        LOG_I("With '--cube' the source is one image holding all six faces, and its arrangement is recovered");
        LOG_I("from the proportions: 6x1 or 1x6 strips, a 4x3 or 3x4 cross, or a 3x2 or 2x3 grid.");
        LOG_I("");
        LOG_I("Pass '--no-linear' for colour art authored in sRGB, such as albedo. Mips are then filtered in");
        LOG_I("linear space and the result is stored as an sRGB format, so the GPU decodes it on sample.");
        LOG_I("");
        LOG_I("Without '--format', the source's own depth and channel count are preserved: 8-bit stays 8-bit,");
        LOG_I("16-bit stays 16-bit, floating-point stays floating-point, and a grayscale source stays single");
        LOG_I("channel.");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static SInt32 Run(UInt Count, ConstPtr<ConstPtr<Char>> Arguments)
    {
        const Baker Instance;

        Environment Parsed;
        Parsed.Parse(Count, Arguments);

        const ConstSpan<Text> Operands = Parsed.GetOperands();

        if (Operands.IsEmpty() || Parsed.Contains("help"))
        {
            Usage(Instance);

            return Operands.IsEmpty() ? 1 : 0;
        }

        const Text    Source   = Operands[0];
        const Profile Settings = Profile::From(Parsed);

        // An unrecognized `--format` resolves to `Unspecified`, which is indistinguishable from omitting the
        // switch, so reject it rather than letting a typo silently fall back to inference.
        if (Parsed.Contains("format") && Settings.Format == Graphic::TextureFormat::Unspecified)
        {
            LOG_E("Texture: '{0}' is not a recognized format name", Parsed.GetText("format", Text::Empty()));
            return 1;
        }

        const Str  Derived     = Derive(Source, Exporter::kOutput);
        const Text Destination = (Operands.GetSize() > 1) ? Operands[1] : Text(Derived);

        return Instance.Bake(Source, Destination, Settings) ? 0 : 1;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int main(int Count, char * Arguments[])
{
    const SInt32 Result = Tool::Baker::Texture::Run(static_cast<UInt>(Count), Arguments);

    Log::Flush();
    return Result;
}