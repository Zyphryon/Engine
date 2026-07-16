// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a color with red, green, blue, and alpha channels.
    template<typename Type>
    class AnyColor final
    {
    public:

        /// \brief Constructs a color with all channels initialized to 0 (transparent black).
        ZY_INLINE constexpr AnyColor()
            : mComponents { 0 }
        {
        }

        /// \brief Constructs a color with specified components.
        ///
        /// \param Red   The red component value.
        /// \param Green The green component value.
        /// \param Blue  The blue component value.
        /// \param Alpha The alpha component value.
        ZY_INLINE constexpr AnyColor(Type Red, Type Green, Type Blue, Type Alpha = Limit())
            : mComponents { Red, Green, Blue, Alpha }
        {
        }

        /// \brief Gets the red component.
        ///
        /// \return The red component value.
        ZY_INLINE constexpr Type GetRed() const
        {
            return mComponents[0];
        }

        /// \brief Gets the green component.
        ///
        /// \return The green component value.
        ZY_INLINE constexpr Type GetGreen() const
        {
            return mComponents[1];
        }

        /// \brief Gets the blue component.
        ///
        /// \return The blue component value.
        ZY_INLINE constexpr Type GetBlue() const
        {
            return mComponents[2];
        }

        /// \brief Gets the alpha component.
        ///
        /// \return The alpha component value.
        ZY_INLINE constexpr Type GetAlpha() const
        {
            return mComponents[3];
        }

        /// \brief Calculates the perceived brightness of the color.
        ///
        /// \return The normalized luminance value in [0, 1].
        ZY_INLINE constexpr Real32 GetLuminance() const
        {
            const Real32 R = static_cast<Real32>(GetRed())   / static_cast<Real32>(Limit());
            const Real32 G = static_cast<Real32>(GetGreen()) / static_cast<Real32>(Limit());
            const Real32 B = static_cast<Real32>(GetBlue())  / static_cast<Real32>(Limit());
            return R * 0.2126f + G * 0.7152f + B * 0.0722f;
        }

        /// \brief Checks if the color is opaque (alpha channel is at maximum value).
        ///
        /// \return `true` if the color is opaque, `false` otherwise.
        ZY_INLINE constexpr Bool IsOpaque() const
        {
            return GetAlpha() == Limit();
        }

        /// \brief Checks if the color is transparent (alpha channel is below maximum value).
        ///
        /// \return `true` if the color is transparent, `false` otherwise.
        ZY_INLINE constexpr Bool IsTransparent() const
        {
            return GetAlpha() < Limit();
        }

        /// \brief Packs color components into a 32-bit RGBA format.
        ///
        /// \return A 32-bit unsigned integer representing the packed RGBA value.
        ZY_INLINE constexpr UInt32 ToRGBA8() const
        {
            const UInt32 R = static_cast<UInt32>(Scale<1ULL << 8>(mComponents[0]));
            const UInt32 G = static_cast<UInt32>(Scale<1ULL << 8>(mComponents[1]));
            const UInt32 B = static_cast<UInt32>(Scale<1ULL << 8>(mComponents[2]));
            const UInt32 A = static_cast<UInt32>(Scale<1ULL << 8>(mComponents[3]));
            return (A << 24) | (B  << 16) | (G << 8) | R;
        }

        /// \brief Converts a floating-point color to an 8-bit integer color.
        ///
        /// \return The converted color in 8-bit integer format.
        ZY_INLINE constexpr AnyColor<UInt8> ToColor8() const
            requires(IsReal<Type>)
        {
            return AnyColor(
                static_cast<UInt8>(Scale<1ULL << 8>(mComponents[0])),
                static_cast<UInt8>(Scale<1ULL << 8>(mComponents[1])),
                static_cast<UInt8>(Scale<1ULL << 8>(mComponents[2])),
                static_cast<UInt8>(Scale<1ULL << 8>(mComponents[3])));
        }

        /// \brief Returns the RGB channels premultiplied by an intensity scalar.
        ///
        /// \param Intensity The intensity scalar to multiply the RGB channels by.
        /// \param Alpha     The alpha value to use for the resulting color.
        /// \return A new color with RGB multiplied by intensity and alpha preserved.
        ZY_INLINE constexpr AnyColor WithIntensity(Type Intensity, Type Alpha) const
            requires(IsReal<Type>)
        {
            return AnyColor(mComponents[0] * Intensity,
                            mComponents[1] * Intensity,
                            mComponents[2] * Intensity,
                            Alpha);
        }

        /// \brief Adds another color to this color (component-wise RGBA).
        ///
        /// \param Other The color to add.
        /// \return A new color that is the sum of this color and the input color.
        ZY_INLINE constexpr AnyColor operator+(AnyColor Other) const
        {
            return AnyColor(mComponents[0] + Other.mComponents[0],
                            mComponents[1] + Other.mComponents[1],
                            mComponents[2] + Other.mComponents[2],
                            mComponents[3] + Other.mComponents[3]);
        }

        /// \brief Adds a scalar to all channels of this color.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new color with the scalar added to all channels.
        ZY_INLINE constexpr AnyColor operator+(Type Scalar) const
        {
            return AnyColor(mComponents[0] + Scalar,
                            mComponents[1] + Scalar,
                            mComponents[2] + Scalar,
                            mComponents[3] + Scalar);
        }

        /// \brief Subtracts another color from this color (component-wise RGBA).
        ///
        /// \param Other The color to subtract.
        /// \return A new color that is the difference of the two colors.
        ZY_INLINE constexpr AnyColor operator-(AnyColor Other) const
        {
            return AnyColor(mComponents[0] - Other.mComponents[0],
                            mComponents[1] - Other.mComponents[1],
                            mComponents[2] - Other.mComponents[2],
                            mComponents[3] - Other.mComponents[3]);
        }

        /// \brief Subtracts a scalar from all channels of this color.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new color with the scalar subtracted from all channels.
        ZY_INLINE constexpr AnyColor operator-(Type Scalar) const
        {
            return AnyColor(mComponents[0] - Scalar,
                            mComponents[1] - Scalar,
                            mComponents[2] - Scalar,
                            mComponents[3] - Scalar);
        }

        /// \brief Multiplies this color by another color (component-wise RGBA).
        ///
        /// \param Other The color to multiply by.
        /// \return A new color that is the product of the two colors.
        ZY_INLINE constexpr AnyColor operator*(AnyColor Other) const
        {
            return AnyColor(mComponents[0] * Other.mComponents[0],
                            mComponents[1] * Other.mComponents[1],
                            mComponents[2] * Other.mComponents[2],
                            mComponents[3] * Other.mComponents[3]);
        }

        /// \brief Multiplies all channels of this color by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new color with all channels multiplied by the scalar.
        ZY_INLINE constexpr AnyColor operator*(Type Scalar) const
        {
            return AnyColor(mComponents[0] * Scalar,
                            mComponents[1] * Scalar,
                            mComponents[2] * Scalar,
                            mComponents[3] * Scalar);
        }

        /// \brief Divides this color by another color (component-wise RGBA).
        ///
        /// \param Other The color to divide by.
        /// \return A new color that is the quotient of the two colors.
        ZY_INLINE constexpr AnyColor operator/(AnyColor Other) const
        {
            ZY_ASSERT(!::IsAlmostZero(Other.mComponents[0]), "Division by zero (Red)");
            ZY_ASSERT(!::IsAlmostZero(Other.mComponents[1]), "Division by zero (Green)");
            ZY_ASSERT(!::IsAlmostZero(Other.mComponents[2]), "Division by zero (Blue)");
            ZY_ASSERT(!::IsAlmostZero(Other.mComponents[3]), "Division by zero (Alpha)");

            return AnyColor(mComponents[0] / Other.mComponents[0],
                            mComponents[1] / Other.mComponents[1],
                            mComponents[2] / Other.mComponents[2],
                            mComponents[3] / Other.mComponents[3]);
        }

        /// \brief Divides all channels of this color by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new color with all channels divided by the scalar.
        ZY_INLINE constexpr AnyColor operator/(Type Scalar) const
        {
            ZY_ASSERT(!::IsAlmostZero(Scalar), "Division by zero");

            return AnyColor(mComponents[0] / Scalar,
                            mComponents[1] / Scalar,
                            mComponents[2] / Scalar,
                            mComponents[3] / Scalar);
        }

        /// \brief Adds another color to the current color (component-wise RGBA).
        ///
        /// \param Other The color to add.
        /// \return A reference to the updated color.
        ZY_INLINE constexpr Ref<AnyColor> operator+=(AnyColor Other)
        {
            mComponents[0] += Other.mComponents[0];
            mComponents[1] += Other.mComponents[1];
            mComponents[2] += Other.mComponents[2];
            mComponents[3] += Other.mComponents[3];
            return (* this);
        }

        /// \brief Adds a scalar value to all channels of the color.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A reference to the updated color.
        ZY_INLINE constexpr Ref<AnyColor> operator+=(Type Scalar)
        {
            mComponents[0] += Scalar;
            mComponents[1] += Scalar;
            mComponents[2] += Scalar;
            mComponents[3] += Scalar;
            return (* this);
        }

        /// \brief Subtracts another color from the current color (component-wise RGBA).
        ///
        /// \param Other The color to subtract.
        /// \return A reference to the updated color.
        ZY_INLINE constexpr Ref<AnyColor> operator-=(AnyColor Other)
        {
            mComponents[0] -= Other.mComponents[0];
            mComponents[1] -= Other.mComponents[1];
            mComponents[2] -= Other.mComponents[2];
            mComponents[3] -= Other.mComponents[3];
            return (* this);
        }

        /// \brief Subtracts a scalar value from all channels of the color.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A reference to the updated color.
        ZY_INLINE constexpr Ref<AnyColor> operator-=(Type Scalar)
        {
            mComponents[0] -= Scalar;
            mComponents[1] -= Scalar;
            mComponents[2] -= Scalar;
            mComponents[3] -= Scalar;
            return (* this);
        }

        /// \brief Multiplies all channels of the color by another color (component-wise RGBA).
        ///
        /// \param Other The color to multiply.
        /// \return A reference to the updated color.
        ZY_INLINE constexpr Ref<AnyColor> operator*=(AnyColor Other)
        {
            mComponents[0] *= Other.mComponents[0];
            mComponents[1] *= Other.mComponents[1];
            mComponents[2] *= Other.mComponents[2];
            mComponents[3] *= Other.mComponents[3];
            return (* this);
        }

        /// \brief Multiplies all channels of the color by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated color.
        ZY_INLINE constexpr Ref<AnyColor> operator*=(Type Scalar)
        {
            mComponents[0] *= Scalar;
            mComponents[1] *= Scalar;
            mComponents[2] *= Scalar;
            mComponents[3] *= Scalar;
            return (* this);
        }

        /// \brief Divides all channels of the color by another color (component-wise RGBA).
        ///
        /// \param Other The color to divide by.
        /// \return A reference to the updated color.
        ZY_INLINE constexpr Ref<AnyColor> operator/=(AnyColor Other)
        {
            ZY_ASSERT(!::IsAlmostZero(Other.mComponents[0]), "Division by zero (Red)");
            ZY_ASSERT(!::IsAlmostZero(Other.mComponents[1]), "Division by zero (Green)");
            ZY_ASSERT(!::IsAlmostZero(Other.mComponents[2]), "Division by zero (Blue)");
            ZY_ASSERT(!::IsAlmostZero(Other.mComponents[3]), "Division by zero (Alpha)");

            mComponents[0] /= Other.mComponents[0];
            mComponents[1] /= Other.mComponents[1];
            mComponents[2] /= Other.mComponents[2];
            mComponents[3] /= Other.mComponents[3];
            return (* this);
        }

        /// \brief Divides all channels of the color by a scalar value.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A reference to the updated color.
        ZY_INLINE constexpr Ref<AnyColor> operator/=(Type Scalar)
        {
            ZY_ASSERT(!::IsAlmostZero(Scalar), "Division by zero");

            mComponents[0] /= Scalar;
            mComponents[1] /= Scalar;
            mComponents[2] /= Scalar;
            mComponents[3] /= Scalar;
            return (* this);
        }

        /// \brief Checks if this color is equal to another color.
        ///
        /// \param Other The color to compare to.
        /// \return `true` if all channels are equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(AnyColor Other) const
        {
            return mComponents[0] == Other.mComponents[0] &&
                   mComponents[1] == Other.mComponents[1] &&
                   mComponents[2] == Other.mComponents[2] &&
                   mComponents[3] == Other.mComponents[3];
        }

        /// \brief Checks if this color is not equal to another color.
        ///
        /// \param Other The color to compare to.
        /// \return `true` if any channel differs, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(AnyColor Other) const
        {
            return !(* this == Other);
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<9> kPattern("({0}, {1}, {2}, {3})");
            Format::Processor<Output>::Format(Buffer, kPattern, GetRed(), GetGreen(), GetBlue(), GetAlpha());
        }

    private:

        /// \brief Returns the maximum channel value for the given primitive type.
        ///
        /// \return `1.0` for floating-point types or the maximum representable value for integral types.
        template<typename Primitive = Type>
        ZY_INLINE static constexpr Primitive Limit()
        {
            if constexpr(IsReal<Primitive>)
            {
                return 1.0f;
            }
            else
            {
                return kMaximum<Primitive>;
            }
        }

        /// \brief Scales a channel value into the target numeric range defined by \p Value.
        ///
        /// \param Channel The channel value to scale.
        /// \return The scaled channel value clamped to [0, Limit] then multiplied by \p Value for real types,
        ///         or the unmodified channel for integral types.
        template<UInt Value>
        ZY_INLINE static constexpr auto Scale(Type Channel)
        {
            if constexpr (IsReal<Type>)
            {
                return Clamp(Channel, Type(0), Limit()) * Value;
            }
            else
            {
                return Channel;
            }
        }

    public:

        /// \brief Constructs a color from a hexadecimal string in the format "#RRGGBB" or "#RRGGBBAA".
        ///
        /// \param Hexadecimal The hexadecimal color string to parse.
        /// \return The parsed color, or transparent black if the format is invalid.
        ZY_INLINE static constexpr AnyColor FromHexadecimal(Text Hexadecimal)
        {
            UInt Cursor = 0;
            StrConsume(Hexadecimal, Cursor, '#');

            const UInt8 R = StrExtractNumber<16, UInt8>(Hexadecimal, Cursor);
            const UInt8 G = StrExtractNumber<16, UInt8>(Hexadecimal, Cursor);
            const UInt8 B = StrExtractNumber<16, UInt8>(Hexadecimal, Cursor);
            const UInt8 A = (Hexadecimal.GetSize() >= 8) ? StrExtractNumber<16, UInt8>(Hexadecimal, Cursor) : Limit<UInt8>();

            if constexpr (IsReal<Type>)
            {
                constexpr Real32 kInvLimit = 1.0f / Limit<UInt8>();
                return AnyColor(R * kInvLimit, G * kInvLimit, B * kInvLimit, A * kInvLimit);
            }
            else
            {
                return AnyColor(static_cast<Type>(R), static_cast<Type>(G), static_cast<Type>(B), static_cast<Type>(A));
            }
        }

        /// \brief Converts a color from 8-bit RGBA format to 32-bit floating-point format.
        ///
        /// \param Color The color in 8-bit RGBA format to convert.
        /// \return The converted color in 32-bit floating-point format, with each channel normalized to the range [0, 1].
        ZY_INLINE static constexpr AnyColor<Real32> FromColor8(AnyColor<UInt8> Color)
        {
            constexpr Real32 kInvLimit = 1.0f / Limit<UInt8>();

            const Real32 R = static_cast<Real32>(Color.GetRed())   * kInvLimit;
            const Real32 G = static_cast<Real32>(Color.GetGreen()) * kInvLimit;
            const Real32 B = static_cast<Real32>(Color.GetBlue())  * kInvLimit;
            const Real32 A = static_cast<Real32>(Color.GetAlpha()) * kInvLimit;
            return AnyColor(R, G, B, A);
        }

        /// \brief Constructs a fully saturated color from a hue value.
        ///
        /// \param Hue   The hue in turns, where one full revolution of the color wheel spans [0, 1).
        /// \param Alpha The alpha component in the range [0, 255].
        /// \return The color corresponding to the given hue.
        ZY_INLINE static constexpr AnyColor FromHue(Real32 Hue, UInt8 Alpha = 255)
        {
            const Real32 H = (Hue - Floor(Hue)) * 6.0f;
            const Real32 X = 1.0f - Abs(Mod(H, 2.0f) - 1.0f);

            Real32 R = 0.0f, G = 0.0f, B = 0.0f;

            switch (static_cast<UInt32>(H))
            {
            case 0:
                R = 1.0f;
                G = X;
                break;
            case 1:
                R = X;
                G = 1.0f;
                break;
            case 2:
                G = 1.0f;
                B = X;
                break;
            case 3:
                G = X;
                B = 1.0f;
                break;
            case 4:
                R = X;
                B = 1.0f;
                break;
            default:
                R = 1.0f;
                B = X;
                break;
            }

            if constexpr (IsReal<Type>)
            {
                constexpr Real32 kInvLimit = 1.0f / Limit<UInt8>();

                return AnyColor(static_cast<Type>(R),
                                static_cast<Type>(G),
                                static_cast<Type>(B),
                                static_cast<Type>(Alpha * kInvLimit));
            }
            else
            {
                constexpr Real32 kLimit = static_cast<Real32>(Limit<Type>());

                return AnyColor(static_cast<Type>(R * kLimit),
                                static_cast<Type>(G * kLimit),
                                static_cast<Type>(B * kLimit),
                                static_cast<Type>(Alpha));
            }
        }

        /// \brief Returns a fully transparent black color.
        ///
        /// \return A color representing transparent black.
        ZY_INLINE static constexpr AnyColor Transparent()
        {
            return AnyColor(Type(0), Type(0), Type(0), Type(0));
        }

        /// \brief Returns an opaque black color.
        ///
        /// \return A color representing opaque black.
        ZY_INLINE static constexpr AnyColor Black()
        {
            return AnyColor(Type(0), Type(0), Type(0), Limit());
        }

        /// \brief Returns an opaque white color.
        ///
        /// \return A color representing opaque white.
        ZY_INLINE static constexpr AnyColor White()
        {
            return AnyColor(Limit(), Limit(), Limit(), Limit());
        }

        /// \brief Returns a white color with the specified opacity.
        ///
        /// \param Opacity The alpha value for the white color, where 0 is fully transparent.
        /// \return A color representing white with the specified opacity.
        ZY_INLINE static constexpr AnyColor White(Type Opacity)
        {
            return AnyColor(Limit(), Limit(), Limit(), Opacity);
        }

        /// \brief Returns an opaque 50% gray color.
        ///
        /// \return A color representing opaque gray.
        ZY_INLINE static constexpr AnyColor Gray()
        {
            return AnyColor(Limit() / Type(2), Limit() / Type(2), Limit() / Type(2), Limit());
        }

        /// \brief Returns an opaque red color.
        ///
        /// \return A color with maximum red, zero green and blue, and full alpha.
        ZY_INLINE static constexpr AnyColor Red()
        {
            return AnyColor(Limit(), Type(0), Type(0), Limit());
        }

        /// \brief Returns an opaque green color.
        ///
        /// \return A color with maximum green, zero red and blue, and full alpha.
        ZY_INLINE static constexpr AnyColor Green()
        {
            return AnyColor(Type(0), Limit(), Type(0), Limit());
        }

        /// \brief Returns an opaque blue color.
        ///
        /// \return A color with maximum blue, zero red and green, and full alpha.
        ZY_INLINE static constexpr AnyColor Blue()
        {
            return AnyColor(Type(0), Type(0), Limit(), Limit());
        }

        /// \brief Returns an opaque yellow color.
        ///
        /// \return A color with maximum red and green, zero blue, and full alpha.
        ZY_INLINE static constexpr AnyColor Yellow()
        {
            return AnyColor(Limit(), Limit(), Type(0), Limit());
        }

        /// \brief Returns an opaque cyan color.
        ///
        /// \return A color with maximum green and blue, zero red, and full alpha.
        ZY_INLINE static constexpr AnyColor Cyan()
        {
            return AnyColor(Type(0), Limit(), Limit(), Limit());
        }

        /// \brief Returns an opaque pink color.
        ///
        /// \return A color with maximum red and blue, zero green, and full alpha.
        ZY_INLINE static constexpr AnyColor Pink()
        {
            return AnyColor(Limit(), Type(0), Limit(), Limit());
        }

        /// \brief Returns the component-wise minimum of two colors.
        ///
        /// \param First  The first color.
        /// \param Second The second color.
        /// \return A color with the component-wise minimum values.
        ZY_INLINE static constexpr AnyColor Min(AnyColor First, AnyColor Second)
        {
            return AnyColor(::Min(First.GetRed(),   Second.GetRed()),
                            ::Min(First.GetGreen(), Second.GetGreen()),
                            ::Min(First.GetBlue(),  Second.GetBlue()),
                            ::Min(First.GetAlpha(), Second.GetAlpha()));
        }

        /// \brief Returns the component-wise maximum of two colors.
        ///
        /// \param First  The first color.
        /// \param Second The second color.
        /// \return A color with the component-wise maximum values.
        ZY_INLINE static constexpr AnyColor Max(AnyColor First, AnyColor Second)
        {
            return AnyColor(::Max(First.GetRed(),   Second.GetRed()),
                            ::Max(First.GetGreen(), Second.GetGreen()),
                            ::Max(First.GetBlue(),  Second.GetBlue()),
                            ::Max(First.GetAlpha(), Second.GetAlpha()));
        }

        /// \brief Clamps each channel of a color between a minimum and maximum value.
        ///
        /// \param Color   The input color.
        /// \param Minimum The minimum allowed channel value.
        /// \param Maximum The maximum allowed channel value.
        /// \return A color with the component values clamped.
        ZY_INLINE static constexpr AnyColor Clamp(AnyColor Color, Type Minimum, Type Maximum)
        {
            return AnyColor(::Clamp(Color.GetRed(),   Minimum, Maximum),
                            ::Clamp(Color.GetGreen(), Minimum, Maximum),
                            ::Clamp(Color.GetBlue(),  Minimum, Maximum),
                            ::Clamp(Color.GetAlpha(), Minimum, Maximum));
        }

        /// \brief Clamps all channels of a color to the valid range [0, Limit].
        ///
        /// \param Color The input color.
        /// \return A color with all channels saturated to the legal range.
        ZY_INLINE static constexpr AnyColor Saturate(AnyColor Color)
        {
            return Clamp(Color, Type(0), Limit());
        }

        /// \brief Multiplies the RGB channels of two colors component-wise.
        ///
        /// \param First  The first color.
        /// \param Second The second color.
        /// \return A color with modulated RGB and the alpha of \p First.
        ZY_INLINE static constexpr AnyColor Modulate(AnyColor First, AnyColor Second)
            requires(IsReal<Type>)
        {
            const Type Red   = (First.GetRed() * Second.GetRed());
            const Type Green = (First.GetGreen() * Second.GetGreen());
            const Type Blue  = (First.GetBlue() * Second.GetBlue());
            return AnyColor(Red, Green, Blue, First.GetAlpha());
        }

        /// \brief Returns the inverted color (1 - channel).
        ///
        /// \param Color The input color.
        /// \return A color where each RGB channel is inverted. Alpha is preserved.
        ZY_INLINE static constexpr AnyColor Invert(AnyColor Color)
        {
            return AnyColor(Limit() - Color.GetRed(), Limit() - Color.GetGreen(), Limit() - Color.GetBlue(), Color.GetAlpha());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Type , 4> mComponents;
    };

    /// \brief Represents a color with 32-bit floating-point channels in the normalized range [0, 1].
    using Color     = AnyColor<Real32>;

    /// \brief Represents a color with 8-bit unsigned integer channels in the range [0, 255].
    using IntColor8 = AnyColor<UInt8>;
}