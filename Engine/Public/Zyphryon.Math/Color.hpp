// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Base/Log.hpp"
#include "Zyphryon.Base/Scalar.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a color with red, green, blue, and alpha channels.
    ///
    /// \tparam Type The underlying data type for color components.
    template<typename Type>
    class AnyColor final
    {
    public:

        /// \brief Constructs a color with all channels initialized to 0 (transparent black).
        ZYPHRYON_INLINE constexpr AnyColor()
            : mComponents { 0 }
        {
        }

        /// \brief Constructs a color with specified components.
        ///
        /// \param Red   The red component value.
        /// \param Green The green component value.
        /// \param Blue  The blue component value.
        /// \param Alpha The alpha component value.
        ZYPHRYON_INLINE constexpr AnyColor(Type Red, Type Green, Type Blue, Type Alpha = Limit())
            : mComponents { Red, Green, Blue, Alpha }
        {
        }

        /// \brief Retrieves the red component.
        ///
        /// \return Red component value.
        ZYPHRYON_INLINE constexpr Type GetRed() const
        {
            return mComponents[0];
        }

        /// \brief Retrieves the green component.
        ///
        /// \return Green component value.
        ZYPHRYON_INLINE constexpr Type GetGreen() const
        {
            return mComponents[1];
        }

        /// \brief Retrieves the blue component.
        ///
        /// \return Blue component value.
        ZYPHRYON_INLINE constexpr Type GetBlue() const
        {
            return mComponents[2];
        }

        /// \brief Retrieves the alpha component.
        ///
        /// \return Alpha component value.
        ZYPHRYON_INLINE constexpr Type GetAlpha() const
        {
            return mComponents[3];
        }

        /// \brief Calculates the perceived brightness of the color.
        ///
        /// \return Normalized luminance value [0, 1].
        ZYPHRYON_INLINE constexpr Real32 GetLuminance() const
        {
            const Real32 R = static_cast<Real32>(GetRed())   / static_cast<Real32>(Limit());
            const Real32 G = static_cast<Real32>(GetGreen()) / static_cast<Real32>(Limit());
            const Real32 B = static_cast<Real32>(GetBlue())  / static_cast<Real32>(Limit());
            return R * 0.2126f + G * 0.7152f + B * 0.0722f;
        }

        /// \brief Packs color components into a 32-bit RGBA format.
        ///
        /// \return A 32-bit unsigned integer representing the packed RGBA value.
        ZYPHRYON_INLINE constexpr UInt32 ToRGBA8() const
        {
            const UInt32 R = static_cast<UInt32>(Scale<UINT8_MAX>(mComponents[0]));
            const UInt32 G = static_cast<UInt32>(Scale<UINT8_MAX>(mComponents[1]));
            const UInt32 B = static_cast<UInt32>(Scale<UINT8_MAX>(mComponents[2]));
            const UInt32 A = static_cast<UInt32>(Scale<UINT8_MAX>(mComponents[3]));

            if constexpr (ZYPHRYON_LITTLE_ENDIAN)
            {
                return (A << 24) | (B  << 16) | (G << 8) | R;
            }
            else
            {
                return (R << 24) | (G  << 16) | (B << 8) | A;
            }
        }

        /// \brief Adds another color to this color (component-wise RGBA).
        ///
        /// \param Color The color to add.
        /// \return A new color that is the sum of this color and the input color.
        ZYPHRYON_INLINE constexpr AnyColor operator+(ConstRef<AnyColor> Color) const
        {
            return AnyColor(mComponents[0] + Color.mComponents[0],
                            mComponents[1] + Color.mComponents[1],
                            mComponents[2] + Color.mComponents[2],
                            mComponents[3] + Color.mComponents[3]);
        }

        /// \brief Adds a scalar to all channels of this color.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new color with the scalar added to all channels.
        ZYPHRYON_INLINE constexpr AnyColor operator+(Type Scalar) const
        {
            return AnyColor(mComponents[0] + Scalar,
                            mComponents[1] + Scalar,
                            mComponents[2] + Scalar,
                            mComponents[3] + Scalar);
        }

        /// \brief Subtracts another color from this color (component-wise RGBA).
        ///
        /// \param Color The color to subtract.
        /// \return A new color that is the difference of the two colors.
        ZYPHRYON_INLINE constexpr AnyColor operator-(ConstRef<AnyColor> Color) const
        {
            return AnyColor(mComponents[0] - Color.mComponents[0],
                            mComponents[1] - Color.mComponents[1],
                            mComponents[2] - Color.mComponents[2],
                            mComponents[3] - Color.mComponents[3]);
        }

        /// \brief Subtracts a scalar from all channels of this color.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new color with the scalar subtracted from all channels.
        ZYPHRYON_INLINE constexpr AnyColor operator-(Type Scalar) const
        {
            return AnyColor(mComponents[0] - Scalar,
                            mComponents[1] - Scalar,
                            mComponents[2] - Scalar,
                            mComponents[3] - Scalar);
        }

        /// \brief Multiplies this color by another color (component-wise RGBA).
        ///
        /// \param Color The color to multiply by.
        /// \return A new color that is the product of the two colors.
        ZYPHRYON_INLINE constexpr AnyColor operator*(ConstRef<AnyColor> Color) const
        {
            return AnyColor(mComponents[0] * Color.mComponents[0],
                            mComponents[1] * Color.mComponents[1],
                            mComponents[2] * Color.mComponents[2],
                            mComponents[3] * Color.mComponents[3]);
        }

        /// \brief Multiplies all channels of this color by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new color with all channels multiplied by the scalar.
        ZYPHRYON_INLINE constexpr AnyColor operator*(Type Scalar) const
        {
            return AnyColor(mComponents[0] * Scalar,
                            mComponents[1] * Scalar,
                            mComponents[2] * Scalar,
                            mComponents[3] * Scalar);
        }

        /// \brief Divides this color by another color (component-wise RGBA).
        ///
        /// \param Color The color to divide by.
        /// \return A new color that is the quotient of the two colors.
        ZYPHRYON_INLINE constexpr AnyColor operator/(ConstRef<AnyColor> Color) const
        {
            LOG_ASSERT(!Base::IsAlmostZero(Color.mComponents[0]), "Division by zero (Red)");
            LOG_ASSERT(!Base::IsAlmostZero(Color.mComponents[1]), "Division by zero (Green)");
            LOG_ASSERT(!Base::IsAlmostZero(Color.mComponents[2]), "Division by zero (Blue)");
            LOG_ASSERT(!Base::IsAlmostZero(Color.mComponents[3]), "Division by zero (Alpha)");

            return AnyColor(mComponents[0] / Color.mComponents[0],
                            mComponents[1] / Color.mComponents[1],
                            mComponents[2] / Color.mComponents[2],
                            mComponents[3] / Color.mComponents[3]);
        }

        /// \brief Divides all channels of this color by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new color with all channels divided by the scalar.
        ZYPHRYON_INLINE constexpr AnyColor operator/(Type Scalar) const
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");

            return AnyColor(mComponents[0] / Scalar,
                            mComponents[1] / Scalar,
                            mComponents[2] / Scalar,
                            mComponents[3] / Scalar);
        }

        /// \brief Adds another color to the current color (component-wise RGBA).
        ///
        /// \param Color The color to add.
        /// \return A reference to the updated color.
        ZYPHRYON_INLINE constexpr Ref<AnyColor> operator+=(ConstRef<AnyColor> Color)
        {
            mComponents[0] += Color.mComponents[0];
            mComponents[1] += Color.mComponents[1];
            mComponents[2] += Color.mComponents[2];
            mComponents[3] += Color.mComponents[3];
            return (* this);
        }

        /// \brief Adds a scalar value to all channels of the color.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A reference to the updated color.
        ZYPHRYON_INLINE constexpr Ref<AnyColor> operator+=(Type Scalar)
        {
            mComponents[0] += Scalar;
            mComponents[1] += Scalar;
            mComponents[2] += Scalar;
            mComponents[3] += Scalar;
            return (* this);
        }

        /// \brief Subtracts another color from the current color (component-wise RGBA).
        ///
        /// \param Color The color to subtract.
        /// \return A reference to the updated color.
        ZYPHRYON_INLINE constexpr Ref<AnyColor> operator-=(ConstRef<AnyColor> Color)
        {
            mComponents[0] -= Color.mComponents[0];
            mComponents[1] -= Color.mComponents[1];
            mComponents[2] -= Color.mComponents[2];
            mComponents[3] -= Color.mComponents[3];
            return (* this);
        }

        /// \brief Subtracts a scalar value from all channels of the color.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A reference to the updated color.
        ZYPHRYON_INLINE constexpr Ref<AnyColor> operator-=(Type Scalar)
        {
            mComponents[0] -= Scalar;
            mComponents[1] -= Scalar;
            mComponents[2] -= Scalar;
            mComponents[3] -= Scalar;
            return (* this);
        }

        /// \brief Multiplies all channels of the color by another color (component-wise RGBA).
        ///
        /// \param Color The color to multiply.
        /// \return A reference to the updated color.
        ZYPHRYON_INLINE constexpr Ref<AnyColor> operator*=(ConstRef<AnyColor> Color)
        {
            mComponents[0] *= Color.mComponents[0];
            mComponents[1] *= Color.mComponents[1];
            mComponents[2] *= Color.mComponents[2];
            mComponents[3] *= Color.mComponents[3];
            return (* this);
        }

        /// \brief Multiplies all channels of the color by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated color.
        ZYPHRYON_INLINE constexpr Ref<AnyColor> operator*=(Type Scalar)
        {
            mComponents[0] *= Scalar;
            mComponents[1] *= Scalar;
            mComponents[2] *= Scalar;
            mComponents[3] *= Scalar;
            return (* this);
        }

        /// \brief Divides all channels of the color by another color (component-wise RGBA).
        ///
        /// \param Color The color to divide by.
        /// \return A reference to the updated color.
        ZYPHRYON_INLINE constexpr Ref<AnyColor> operator/=(ConstRef<AnyColor> Color)
        {
            LOG_ASSERT(!Base::IsAlmostZero(Color.mComponents[0]), "Division by zero (Red)");
            LOG_ASSERT(!Base::IsAlmostZero(Color.mComponents[1]), "Division by zero (Green)");
            LOG_ASSERT(!Base::IsAlmostZero(Color.mComponents[2]), "Division by zero (Blue)");
            LOG_ASSERT(!Base::IsAlmostZero(Color.mComponents[3]), "Division by zero (Alpha)");

            mComponents[0] /= Color.mComponents[0];
            mComponents[1] /= Color.mComponents[1];
            mComponents[2] /= Color.mComponents[2];
            mComponents[3] /= Color.mComponents[3];
            return (* this);
        }

        /// \brief Divides all channels of the color by a scalar value.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A reference to the updated color.
        ZYPHRYON_INLINE constexpr Ref<AnyColor> operator/=(Type Scalar)
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");

            mComponents[0] /= Scalar;
            mComponents[1] /= Scalar;
            mComponents[2] /= Scalar;
            mComponents[3] /= Scalar;
            return (* this);
        }

        /// \brief Checks if this color is equal to another color.
        ///
        /// \param Color The color to compare to.
        /// \return `true` if all channels are equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(ConstRef<AnyColor> Color) const
        {
            return mComponents[0] == Color.mComponents[0] &&
                   mComponents[1] == Color.mComponents[1] &&
                   mComponents[2] == Color.mComponents[2] &&
                   mComponents[3] == Color.mComponents[3];
        }

        /// \brief Checks if this color is not equal to another color.
        ///
        /// \param Other The color to compare to.
        /// \return `true` if any channel differs, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(ConstRef<AnyColor> Other) const
        {
            return !(* this == Other);
        }

        /// \brief Computes a hash value for the object.
        ///
        /// \return A hash value uniquely representing the current state of the object.
        ZYPHRYON_INLINE constexpr UInt64 Hash() const
        {
            return HashCombine(this);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        /// 
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeArray(mComponents);
        }

    private:

        /// \brief Returns the maximum channel value for this color type.
        ///
        /// \return The maximum channel value.
        template<typename Primitive = Type>
        ZYPHRYON_INLINE constexpr static Primitive Limit()
        {
            if constexpr(IsReal<Primitive>)
            {
                return 1.0f;
            }
            else
            {
                return std::numeric_limits<Primitive>::max();
            }
        }

        /// \brief Scales a color channel value to the full range of the target type.
        ///
        /// \param Channel The channel value to scale.
        /// \return The scaled channel value in the range of \p Other.
        template<UInt Value>
        ZYPHRYON_INLINE constexpr static auto Scale(Type Channel)
        {
            if constexpr (IsReal<Type>)
            {
                return Base::Clamp(Channel, Type(0), Limit()) * Value;
            }
            else
            {
                return Channel;
            }
        }

    public:

        /// \brief Returns a fully transparent black color.
        ///
        /// \return A color representing transparent black.
        ZYPHRYON_INLINE constexpr static AnyColor Transparent()
        {
            return AnyColor(Type(0), Type(0), Type(0), Type(0));
        }

        /// \brief Returns an opaque black color.
        ///
        /// \return A color representing opaque black.
        ZYPHRYON_INLINE constexpr static AnyColor Black()
        {
            return AnyColor(Type(0), Type(0), Type(0), Limit());
        }

        /// \brief Returns an opaque white color.
        ///
        /// \return A color representing opaque white.
        ZYPHRYON_INLINE constexpr static AnyColor White()
        {
            return AnyColor(Limit(), Limit(), Limit(), Limit());
        }

        /// \brief Returns an opaque 50% gray color.
        ///
        /// \return A color representing opaque gray.
        ZYPHRYON_INLINE constexpr static AnyColor Gray()
        {
            return AnyColor(Limit() / Type(2), Limit() / Type(2), Limit() / Type(2), Limit());
        }

        /// \brief Returns an opaque red color.
        ///
        /// \return A color with maximum red, zero green and blue, and full alpha.
        ZYPHRYON_INLINE static constexpr AnyColor Red()
        {
            return AnyColor(Limit(), 0, 0, Limit());
        }

        /// \brief Returns an opaque green color.
        ///
        /// \return A color with maximum green, zero red and blue, and full alpha.
        ZYPHRYON_INLINE static constexpr AnyColor Green()
        {
            return AnyColor(0, Limit(), 0, Limit());
        }

        /// \brief Returns an opaque blue color.
        ///
        /// \return A color with maximum blue, zero red and green, and full alpha.
        ZYPHRYON_INLINE static constexpr AnyColor Blue()
        {
            return AnyColor(0, 0, Limit(), Limit());
        }

        /// \brief Returns an opaque yellow color.
        ///
        /// \return A color with maximum red and green, zero blue, and full alpha.
        ZYPHRYON_INLINE static constexpr AnyColor Yellow()
        {
            return AnyColor(Limit(), Limit(), 0, Limit());
        }

        /// \brief Returns the component-wise minimum of two colors.
        ///
        /// \param First  The first color.
        /// \param Second The second color.
        /// \return A color with the component-wise maximum values.
        ZYPHRYON_INLINE constexpr static AnyColor Min(ConstRef<AnyColor> First, ConstRef<AnyColor> Second)
        {
            return AnyColor(Base::Min(First.GetRed(),   Second.GetRed()),
                            Base::Min(First.GetGreen(), Second.GetGreen()),
                            Base::Min(First.GetBlue(),  Second.GetBlue()),
                            Base::Min(First.GetAlpha(), Second.GetAlpha()));
        }

        /// \brief Returns the component-wise maximum of two colors.
        ///
        /// \param First  The first color.
        /// \param Second The second color.
        /// \return A color with the component-wise maximum values.
        ZYPHRYON_INLINE constexpr static AnyColor Max(ConstRef<AnyColor> First, ConstRef<AnyColor> Second)
        {
            return AnyColor(Base::Max(First.GetRed(),   Second.GetRed()),
                            Base::Max(First.GetGreen(), Second.GetGreen()),
                            Base::Max(First.GetBlue(),  Second.GetBlue()),
                            Base::Max(First.GetAlpha(), Second.GetAlpha()));
        }

        /// \brief Clamps each channel of a color between a minimum and maximum value.
        ///
        /// \param Color   The input color.
        /// \param Minimum Minimum allowed channel value.
        /// \param Maximum Maximum allowed channel value.
        /// \return A color with the component values clamped.
        ZYPHRYON_INLINE constexpr static AnyColor Clamp(ConstRef<AnyColor> Color, Type Minimum, Type Maximum)
        {
            return AnyColor(Base::Clamp(Color.GetRed(),   Minimum, Maximum),
                            Base::Clamp(Color.GetGreen(), Minimum, Maximum),
                            Base::Clamp(Color.GetBlue(),  Minimum, Maximum),
                            Base::Clamp(Color.GetAlpha(), Minimum, Maximum));
        }

        /// \brief Clamps all channels of a color to the valid range [0, Limit].
        ///
        /// \param Color The input color.
        /// \return A color with all channels saturated to the legal range.
        ZYPHRYON_INLINE constexpr static AnyColor Saturate(ConstRef<AnyColor> Color)
        {
            return Clamp(Color, Type(0), Limit());
        }

        /// \brief Multiplies the RGB channels of two colors component-wise.
        ///
        /// \param First  The first color.
        /// \param Second The second color.
        /// \return A color with modulated RGB and the alpha of \p First.
        ZYPHRYON_INLINE constexpr static AnyColor Modulate(ConstRef<AnyColor> First, ConstRef<AnyColor> Second)
        {
            return AnyColor(First.GetRed()   * Second.GetRed(),
                            First.GetGreen() * Second.GetGreen(),
                            First.GetBlue()  * Second.GetBlue(),
                            First.GetAlpha());
        }

        /// \brief Returns the inverted color (1 - channel).
        ///
        /// \param Color The input color.
        /// \return A color where each RGB channel is inverted. Alpha is preserved.
        ZYPHRYON_INLINE constexpr static AnyColor Invert(ConstRef<AnyColor> Color)
        {
            return AnyColor(Limit() - Color.GetRed(), Limit() - Color.GetGreen(), Limit() - Color.GetBlue(), Color.GetAlpha());
        }

        /// \brief Linearly interpolates between two colors.
        /// 
        /// \param Start       The starting color.
        /// \param End         The ending color.
        /// \param Percentage The interpolation percentage (range between 0 and 1).
        /// \return A color interpolated between the start and end colors.
        ZYPHRYON_INLINE constexpr static AnyColor Lerp(ConstRef<AnyColor> Start, ConstRef<AnyColor> End, Real32 Percentage)
        {
            const Type Red   = Base::Lerp(Start.GetRed(),   End.GetRed(),   Percentage);
            const Type Green = Base::Lerp(Start.GetGreen(), End.GetGreen(), Percentage);
            const Type Blue  = Base::Lerp(Start.GetBlue(),  End.GetBlue(),  Percentage);
            const Type Alpha = Base::Lerp(Start.GetAlpha(), End.GetAlpha(), Percentage);
            return AnyColor(Red, Green, Blue, Alpha);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Type, 4> mComponents;
    };

    /// \brief Represents a color with 32-bit floating-point channels in the normalized range [0,1].
    using Color     = AnyColor<Real32>;

    /// \brief Represents a color with 8-bit unsigned integer channels in the range [0,255].
    using IntColor8 = AnyColor<UInt8>;
}