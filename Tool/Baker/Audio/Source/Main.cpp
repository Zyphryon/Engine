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

namespace Tool::Baker::Audio
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

    static void Usage()
    {
        Str Types;

        for (const Text Extension : Importer::kTypes)
        {
            if (!Types.IsEmpty())
            {
                Types.Append(", ");
            }
            Types.Append(Extension);
        }

        LOG_I("Usage: ZyBakerAudio <source> [destination] [options]");
        LOG_I("");
        LOG_I("Bakes one source sound into the engine's native '.{0}' format. The destination defaults to the", Exporter::kOutput);
        LOG_I("source path with its extension replaced.");
        LOG_I("");
        LOG_I("Sources : {0}", Types);
        LOG_I("");
        LOG_I("Options :");
        LOG_I("  --adpcm           Store IMA ADPCM rather than 16-bit samples          (default: off)");
        LOG_I("  --bitrate=<rate>  Rate '--opus' targets, in bits per second           (default: 96000)");
        LOG_I("  --compressed      LZ4-compress the payload when it shrinks the output (default: on)");
        LOG_I("  --opus            Store Opus packets rather than 16-bit samples       (default: off)");
        LOG_I("");
        LOG_I("Every toggle may be negated with its '--no-' spelling, as in '--no-compressed'.");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static SInt32 Run(UInt Count, ConstPtr<ConstPtr<Char>> Arguments)
    {
        constexpr Baker Instance;

        Environment Parsed;
        Parsed.Parse(Count, Arguments);

        const ConstSpan<Text> Operands = Parsed.GetOperands();

        if (Operands.IsEmpty() || Parsed.Contains("help"))
        {
            Usage();

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
    const SInt32 Result = Tool::Baker::Audio::Run(static_cast<UInt>(Count), Arguments);

    Log::Flush();
    return Result;
}
