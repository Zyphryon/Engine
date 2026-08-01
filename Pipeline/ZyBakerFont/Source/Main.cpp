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

namespace Pipeline::Baker::Font
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Str Derive(Text Source, Text Extension)
    {
        // A separator at index zero belongs to a dotfile, not an extension, and `Slice(0, 0)` takes everything.
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

        LOG_I("Usage: ZyBakerFont <source> [destination] [options]");
        LOG_I("");
        LOG_I("Bakes one typeface into the engine's native '.{0}' format. The destination defaults to the", Exporter::kOutput);
        LOG_I("source path with its extension replaced.");
        LOG_I("");
        LOG_I("Sources : {0}", Types);
        LOG_I("");
        LOG_I("Options :");
        LOG_I("  --size <pixels>      Em size the fields are generated at              (default: 40)");
        LOG_I("  --range <texels>     Width of the distance band                       (default: 20)");
        LOG_I("  --angle <radians>    Join angle past which an edge counts as a corner (default: 3)");
        LOG_I("  --charset <spans>    Codepoints to bake                               (default: ascii)");
        LOG_I("  --face <index>       Typeface to read out of a collection             (default: 0)");
        LOG_I("  --padding <texels>   Gap left between neighbouring glyphs             (default: 1)");
        LOG_I("  --limit <texels>     Largest atlas page before another one opens      (default: 8192)");
        LOG_I("  --underline <em>     Total vertical space an underline occupies       (default: 1.2)");
        LOG_I("");
        LOG_I("The charset is a comma-separated list of spans, where each entry is a preset ('ascii', 'latin1'");
        LOG_I("or 'punctuation'), a single codepoint, or a 'first-last' span. Numbers are decimal unless they");
        LOG_I("carry a '0x' prefix. Spans may overlap, and a codepoint the typeface lacks is simply skipped.");
        LOG_I("");
        LOG_I("  --charset latin1,punctuation      --charset 32-126,0x4E00-0x4EFF      --charset 65,66,0x43");
        LOG_I("");
        LOG_I("Glyphs that do not fit one page of '--limit' texels spill onto further pages, each becoming its");
        LOG_I("own texture the engine draws from; a glyph too large for an entire page is an error instead.");
        LOG_I("");
        LOG_I("The range is what every outline, glow and blur effect draws from: an effect can never reach");
        LOG_I("further than half the range from the edge before the field saturates and the effect clips. It");
        LOG_I("is also pure padding around each glyph, so raising it grows the atlas quadratically. Raise the");
        LOG_I("size instead when the glyphs themselves look soft, since that is what resolves fine detail.");
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

        const Str  Derived     = Derive(Source, Exporter::kOutput);
        const Text Destination = (Operands.GetSize() > 1) ? Operands[1] : Text(Derived);

        return Instance.Bake(Source, Destination, Settings) ? 0 : 1;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int main(int Count, char * Arguments[])
{
    const SInt32 Result = Pipeline::Baker::Font::Run(static_cast<UInt>(Count), Arguments);

    Log::Flush();
    return Result;
}