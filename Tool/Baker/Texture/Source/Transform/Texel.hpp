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

#include <Zyphryon.Graphic/Metadata.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Texture
{
    /// \brief The storage kind a single texel channel takes.
    enum class Component : UInt8
    {
        UInt8,      ///< 8-bit unsigned normalized.
        SInt8,      ///< 8-bit signed normalized.
        UInt16,     ///< 16-bit unsigned normalized.
        SInt16,     ///< 16-bit signed normalized.
        Half,       ///< 16-bit floating-point.
        Real32,     ///< 32-bit floating-point.
    };

    /// \brief The colour-space step a conversion has to take.
    enum class Gamma : UInt8
    {
        None,       ///< Both ends share a space, so no transfer applies.
        Linear,     ///< The source is sRGB-encoded and the target is linear.
        sRGB,       ///< The source is linear and the target is sRGB-encoded.
    };

    /// \brief The number of steps an eight-bit channel can take, which is what makes sRGB tabulatable.
    static constexpr UInt32 kSteps       = 256;

    /// \brief The resolution of the encode grid, which only has to get within a byte of the right answer.
    static constexpr UInt32 kGrid        = 4096;

    /// \brief The widest interleaved layout handled, and the size of the scratch a texel unpacks into.
    static constexpr UInt32 kMaxChannels = 4;

    /// \brief Selects the type a \ref Component is stored as.
    ///
    /// \tparam Value The storage kind to map.
    /// \return A value of the type the kind is stored as.
    template<Component Value>
    constexpr auto Storage()
    {
        if constexpr (Value == Component::UInt8)
        {
            return Base::UInt8();
        }
        else if constexpr (Value == Component::SInt8)
        {
            return Base::SInt8();
        }
        else if constexpr (Value == Component::UInt16)
        {
            return Base::UInt16();
        }
        else if constexpr (Value == Component::SInt16)
        {
            return Base::SInt16();
        }
        else if constexpr (Value == Component::Half)
        {
            return Base::Half();
        }
        else
        {
            return Base::Real32();
        }
    }

    /// \brief The type a \ref Component is stored as.
    template<Component Value>
    using StorageOf = decltype(Storage<Value>());

    /// \brief Resolves the storage kind a format's channels take.
    ///
    /// \param Format The format description to classify.
    /// \return The matching storage kind.
    ZY_INLINE Component GetComponent(ConstRef<Graphic::TextureMetadata> Format)
    {
        const UInt32 Bits = Format.BitsPerComponent();

        if (Format.IsFloat)
        {
            return (Bits == 16) ? Component::Half : Component::Real32;
        }
        if (Format.IsSigned)
        {
            return (Bits == 16) ? Component::SInt16 : Component::SInt8;
        }
        return (Bits == 16) ? Component::UInt16 : Component::UInt8;
    }

    /// \brief Resolves the colour-space step between two ends of a conversion.
    ///
    /// \param Source `true` when the source is sRGB-encoded.
    /// \param Target `true` when the target is sRGB-encoded.
    /// \return The step to apply.
    ZY_INLINE Gamma GetGamma(Bool Source, Bool Target)
    {
        if (Source == Target)
        {
            return Gamma::None;
        }
        return Source ? Gamma::Linear : Gamma::sRGB;
    }

    /// \brief The sRGB transfer, tabulated in both directions.
    struct Curve final
    {
        /// The linear value of each encoded byte.
        Array<Real32, kSteps>     Linear;

        /// An approximate encoded byte, indexed by the square root of a linear value.
        Array<Byte, kGrid>        Hint;

        /// The lowest linear value that encodes to each byte above zero.
        Array<Real32, kSteps - 1> Boundary;

        /// \brief Samples the engine's transfer into every table.
        Curve()
        {
            for (UInt32 Step = 0; Step < kSteps; ++Step)
            {
                const Real32 Value = static_cast<Real32>(Step) / 255.0f;

                Linear[Step] = Color(Value, Value, Value, 1.0f).ToLinear().GetRed();
            }

            for (UInt32 Step = 0; Step < kGrid; ++Step)
            {
                const Real32 Root = (static_cast<Real32>(Step) + 0.5f) / static_cast<Real32>(kGrid - 1);

                Hint[Step] = static_cast<Byte>(Sample(Min(Root * Root, 1.0f)));
            }

            for (UInt32 Step = 1; Step < kSteps; ++Step)
            {
                Boundary[Step - 1] = Threshold(Step);
            }
        }

    private:

        /// \brief Encodes a linear value through the engine's transfer and rounds it to the nearest byte.
        ///
        /// \param Value The linear value, clamped into unit range once encoded.
        /// \return The encoded step, in the range `[0, 255]`.
        static UInt32 Sample(Real32 Value)
        {
            const Real32 Encoded = Color(Value, Value, Value, 1.0f).ToSRGB().GetRed();
            return static_cast<UInt32>(Clamp(Encoded, 0.0f, 1.0f) * 255.0f + 0.5f);
        }

        /// \brief Finds the lowest linear value that encodes to a given step.
        ///
        /// Non-negative floats order the same way as their bit patterns, so the search bisects the pattern
        /// rather than the value and lands exactly on the boundary instead of near it.
        ///
        /// \param Level The encoded step to find the boundary of, in the range `[1, 255]`.
        /// \return The lowest linear value \ref Sample maps to that step.
        static Real32 Threshold(UInt32 Level)
        {
            UInt32 Low  = 0x00000000;   // 0.0f
            UInt32 High = 0x3F800000;   // 1.0f

            while (Low < High)
            {
                const UInt32 Middle = Low + (High - Low) / 2;

                Real32 Value;
                Blit(AddressOf(Value), sizeof(Value), AddressOf(Middle));

                if (Sample(Value) >= Level)
                {
                    High = Middle;
                }
                else
                {
                    Low = Middle + 1;
                }
            }

            Real32 Result;
            Blit(AddressOf(Result), sizeof(Result), AddressOf(Low));

            return Result;
        }
    };
    inline const Curve kCurve;

    /// \brief Encodes a linear value as an sRGB byte.
    ///
    /// \param Value The linear value, clamped into unit range.
    /// \return The encoded byte.
    ZY_INLINE Byte EncodeGamma(Real32 Value)
    {
        const Real32 Unit = Clamp(Value, 0.0f, 1.0f);

        UInt32 Step = kCurve.Hint[static_cast<UInt32>(Sqrt(Unit) * (kGrid - 1))];

        while (Step < kSteps - 1 && Unit >= kCurve.Boundary[Step])
        {
            ++Step;
        }
        while (Step > 0 && Unit < kCurve.Boundary[Step - 1])
        {
            --Step;
        }
        return static_cast<Byte>(Step);
    }

    /// \brief Decodes one channel as a normalized or floating-point value.
    ///
    /// \param Source The address of the channel.
    /// \return The decoded value.
    template<typename Type>
    ZY_INLINE Real32 Decode(ConstPtr<Byte> Source)
    {
        if constexpr (IsAnyOf<Type, Half>)
        {
            UInt16 Bits;
            Blit(AddressOf(Bits), sizeof(Bits), Source);

            return Half::FromBits(Bits);
        }
        else if constexpr (IsAnyOf<Type, Real32>)
        {
            Real32 Result;
            Blit(AddressOf(Result), sizeof(Result), Source);

            return Result;
        }
        else if constexpr (IsAnyOf<Type, UInt16>)
        {
            UInt16 Bits;
            Blit(AddressOf(Bits), sizeof(Bits), Source);

            return static_cast<Real32>(Bits) / 65535.0f;
        }
        else if constexpr (IsAnyOf<Type, SInt16>)
        {
            SInt16 Bits;
            Blit(AddressOf(Bits), sizeof(Bits), Source);

            return Max(static_cast<Real32>(Bits) / 32767.0f, -1.0f);
        }
        else if constexpr (IsAnyOf<Type, SInt8>)
        {
            return Max(static_cast<Real32>(static_cast<SInt8>(* Source)) / 127.0f, -1.0f);
        }
        else
        {
            return static_cast<Real32>(* Source) / 255.0f;
        }
    }

    /// \brief Encodes one channel from a normalized or floating-point value.
    ///
    /// \param Target The address of the channel.
    /// \param Value  The value to write.
    template<typename Type>
    ZY_INLINE void Encode(Ptr<Byte> Target, Real32 Value)
    {
        if constexpr (IsAnyOf<Type, Half>)
        {
            const UInt16 Bits = Half(Value).GetBits();

            Blit(Target, sizeof(Bits), AddressOf(Bits));
        }
        else if constexpr (IsAnyOf<Type, Real32>)
        {
            Blit(Target, sizeof(Value), AddressOf(Value));
        }
        else if constexpr (IsAnyOf<Type, UInt16>)
        {
            const UInt16 Bits = static_cast<UInt16>(Clamp(Value, 0.0f, 1.0f) * 65535.0f + 0.5f);

            Blit(Target, sizeof(Bits), AddressOf(Bits));
        }
        else if constexpr (IsAnyOf<Type, SInt16>)
        {
            const Real32 Clamped = Clamp(Value, -1.0f, 1.0f);
            const SInt16 Bits    = static_cast<SInt16>(Clamped * 32767.0f + (Clamped >= 0.0f ? 0.5f : -0.5f));

            Blit(Target, sizeof(Bits), AddressOf(Bits));
        }
        else if constexpr (IsAnyOf<Type, SInt8>)
        {
            const Real32 Clamped = Clamp(Value, -1.0f, 1.0f);

            * Target = static_cast<Byte>(static_cast<SInt8>(Clamped * 127.0f + (Clamped >= 0.0f ? 0.5f : -0.5f)));
        }
        else
        {
            * Target = static_cast<UInt8>(Clamp(Value, 0.0f, 1.0f) * 255.0f + 0.5f);
        }
    }

    /// \brief Reads one texel as a linear-space colour.
    ///
    /// \param Source The address of the level.
    /// \param Index  The texel index within the level.
    /// \return The texel, in linear space.
    template<typename Type, UInt32 Channels, Bool SRGB>
    ZY_INLINE Color Fetch(ConstPtr<Byte> Source, UInt32 Index)
    {
        const ConstPtr<Byte> Address = Source + static_cast<UInt64>(Index) * Channels * sizeof(Type);

        Array Components(0.0f, 0.0f, 0.0f, 1.0f);

        for (UInt32 Channel = 0; Channel < Channels; ++Channel)
        {
            if constexpr (SRGB && IsAnyOf<Type, UInt8>)
            {
                Components[Channel] = (Channel < 3) ? kCurve.Linear[Address[Channel]] : Decode<Type>(Address + Channel);
            }
            else
            {
                Components[Channel] = Decode<Type>(Address + Channel * sizeof(Type));
            }
        }

        const Color Result(Components[0], Components[1], Components[2], Components[3]);

        if constexpr (SRGB && !IsAnyOf<Type, UInt8>)
        {
            return Result.ToLinear();
        }
        return Result;
    }

    /// \brief Writes one texel from a linear-space colour.
    ///
    /// \param Target The address of the level.
    /// \param Index  The texel index within the level.
    /// \param Value  The texel to write, in linear space.
    template<typename Type, UInt32 Channels, Bool SRGB>
    ZY_INLINE void Store(Ptr<Byte> Target, UInt32 Index, ConstRef<Color> Value)
    {
        const Ptr<Byte> Address = Target + static_cast<UInt64>(Index) * Channels * sizeof(Type);

        Color Result = Value;

        // Any width other than eight bits has no grid to reach for, so it evaluates the curve.
        if constexpr (SRGB && !IsAnyOf<Type, UInt8>)
        {
            Result = Result.ToSRGB();
        }

        Array Components(Result.GetRed(), Result.GetGreen(), Result.GetBlue(), Result.GetAlpha());

        for (UInt32 Channel = 0; Channel < Channels; ++Channel)
        {
            // Alpha is never gamma-encoded, so it keeps the plain path.
            if constexpr (SRGB && IsAnyOf<Type, UInt8>)
            {
                if (Channel < 3)
                {
                    Address[Channel] = EncodeGamma(Components[Channel]);
                }
                else
                {
                    Encode<Type>(Address + Channel, Components[Channel]);
                }
            }
            else
            {
                Encode<Type>(Address + Channel * sizeof(Type), Components[Channel]);
            }
        }
    }
}