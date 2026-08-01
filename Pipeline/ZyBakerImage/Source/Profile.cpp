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

#include "Profile.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Image
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Profile::Extent Measure(ConstRef<Environment> Environment, Text Name)
    {
        const Text Value = Environment.GetText(Name, Text::Empty());

        // The switch is optional, so its absence is a division never asked for rather than a fault.
        if (Value.IsEmpty())
        {
            return Profile::Extent();
        }

        UInt         Cursor = 0;
        const UInt32 Width  = StrExtractNumber<10, UInt32>(Value, Cursor);

        // The axes are joined by an 'x', so anything else means the pair was not written as one.
        if (Cursor >= Value.GetSize() || (Value[Cursor] != 'x' && Value[Cursor] != 'X'))
        {
            LOG_E("Texture: '--{0}' expects '<width>x<height>', which '{1}' is not", Name, Value);

            return Profile::Extent();
        }
        ++Cursor;

        const UInt32 Height = StrExtractNumber<10, UInt32>(Value, Cursor);

        if (Width == 0 || Height == 0 || Width > 0xFFFF || Height > 0xFFFF)
        {
            LOG_E("Texture: '--{0}' was given '{1}', which no atlas divides by", Name, Value);

            return Profile::Extent();
        }
        return Profile::Extent(static_cast<UInt16>(Width), static_cast<UInt16>(Height));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Profile Profile::From(ConstRef<Environment> Environment)
    {
        Profile Result;
        Result.Format   = Environment.GetEnum<Graphic::TextureFormat>("format", Result.Format);
        Result.Mipmaps  = Environment.GetBool("mipmaps",    Result.Mipmaps);
        Result.Linear   = Environment.GetBool("linear",     Result.Linear);
        Result.Compress = Environment.GetBool("compressed", Result.Compress);

        // The two divisions differ in unit: a cube is counted in faces, an array in texels.
        Result.Cube     = Measure(Environment, "cube");
        Result.Slice    = Measure(Environment, "array");

        return Result;
    }
}