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
    /// \brief Represents an angle in radians and provides methods for conversion to degrees.
    class Angle final
    {
    public:

        /// \brief Default constructor that initializes the angle to zero radians.
        ZY_INLINE constexpr Angle()
            : mRadians { 0 }
        {
        }

        /// \brief Constructs an angle from the given radians.
        ///
        /// \param Radians The angle in radians.
        ZY_INLINE constexpr Angle(Real32 Radians)
            : mRadians { Radians }
        {
        }

        /// \brief Checks if the angle is valid (between 0 and 2π).
        ///
        /// \return `true` if the angle is valid, `false` otherwise.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return IsBetween(GetRadians(), 0.0f, kPI<Real32> * 2.0f);
        }

        /// \brief Gets the angle in radians.
        ///
        /// \return The angle in radians.
        ZY_INLINE constexpr Real32 GetRadians() const
        {
            return mRadians;
        }

        /// \brief Gets the angle in degrees.
        ///
        /// \return The angle in degrees.
        ZY_INLINE constexpr Real32 GetDegrees() const
        {
            return mRadians * (180.0f / kPI<Real32>);
        }

        /// \brief Adds two angles together.
        ///
        /// \param Other The angle to add.
        /// \return The sum of the two angles.
        ZY_INLINE constexpr Angle operator+(Angle Other) const
        {
            return Angle(mRadians + Other.mRadians);
        }

        /// \brief Subtracts one angle from another.
        ///
        /// \param Other The angle to subtract.
        /// \return The difference of the two angles.
        ZY_INLINE constexpr Angle operator-(Angle Other) const
        {
            return Angle(mRadians - Other.mRadians);
        }

        /// \brief Multiplies the angle by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return The scaled angle.
        ZY_INLINE constexpr Angle operator*(Real32 Scalar) const
        {
            return Angle(mRadians * Scalar);
        }

        /// \brief Adds another angle to the current angle.
        ///
        /// \param Other The angle to add.
        /// \return A reference to the updated angle.
        ZY_INLINE constexpr Ref<Angle> operator+=(Angle Other)
        {
            mRadians += Other.mRadians;
            return (* this);
        }

        /// \brief Subtracts another angle from the current angle.
        ///
        /// \param Other The angle to subtract.
        /// \return A reference to the updated angle.
        ZY_INLINE constexpr Ref<Angle> operator-=(Angle Other)
        {
            mRadians -= Other.mRadians;
            return (* this);
        }

        /// \brief Multiplies the angle by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated angle.
        ZY_INLINE constexpr Ref<Angle> operator*=(Real32 Scalar)
        {
            mRadians *= Scalar;
            return (* this);
        }

        /// \brief Checks if two angles are equal.
        ///
        /// \param Other The angle to compare to.
        /// \return `true` if the angles are equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(Angle Other) const
        {
            return IsAlmostEqual(mRadians, Other.mRadians);
        }

        /// \brief Checks if this angle is less than another angle.
        ///
        /// \param Other The angle to compare against.
        /// \return `true` if this angle is smaller than `Other`, otherwise `false`.
        ZY_INLINE constexpr Bool operator<(Angle Other) const
        {
            return mRadians < Other.mRadians;
        }

        /// \brief Checks if this angle is less than or equal to another angle.
        ///
        /// \param Other The angle to compare against.
        /// \return `true` if this angle is smaller than or equal to `Other`, otherwise `false`.
        ZY_INLINE constexpr Bool operator<=(Angle Other) const
        {
            return mRadians <= Other.mRadians;
        }

        /// \brief Checks if this angle is greater than another angle.
        ///
        /// \param Other The angle to compare against.
        /// \return `true` if this angle is larger than `Other`, otherwise `false`.
        ZY_INLINE constexpr Bool operator>(Angle Other) const
        {
            return mRadians > Other.mRadians;
        }

        /// \brief Checks if this angle is greater than or equal to another angle.
        ///
        /// \param Other The angle to compare against.
        /// \return `true` if this angle is larger than or equal to `Other`, otherwise `false`.
        ZY_INLINE constexpr Bool operator>=(Angle Other) const
        {
            return mRadians >= Other.mRadians;
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<2> kPattern("{0}°");
            Format::Processor<Output>::Format(Buffer, kPattern, GetDegrees());
        }

    public:

        /// \brief Creates an angle from radians.
        ///
        /// \param Radians The angle in radians.
        /// \return The angle in radians.
        ZY_INLINE static constexpr Angle FromRadians(Real32 Radians)
        {
            return Angle(Radians);
        }

        /// \brief Creates an angle from degrees.
        ///
        /// \param Degrees The angle in degrees.
        /// \return The angle in radians.
        ZY_INLINE static constexpr Angle FromDegrees(Real32 Degrees)
        {
            return Angle(Degrees * (kPI<Real32> / 180.0f));
        }

        /// \brief Creates an angle from the cosine ratio.
        ///
        /// \param Ratio The cosine ratio (adjacent/hypotenuse).
        /// \return The angle corresponding to the given cosine ratio.
        ZY_INLINE static Angle FromCosine(Real32 Ratio)
        {
            ZY_ASSERT(Ratio >= -1.0f && Ratio <= 1.0f, "Ratio out of range [-1,1]");

            return Angle(std::acos(Ratio));
        }

        /// \brief Creates an angle from the sine ratio.
        ///
        /// \param Ratio The sine ratio (opposite/hypotenuse).
        /// \return The angle corresponding to the given sine ratio.
        ZY_INLINE static Angle FromSine(Real32 Ratio)
        {
            ZY_ASSERT(Ratio >= -1.0f && Ratio <= 1.0f, "Ratio out of range [-1,1]");

            return Angle(std::asin(Ratio));
        }

        /// \brief Creates an angle from the tangent ratio.
        ///
        /// \param Ratio The tangent ratio (opposite/adjacent).
        /// \return The angle corresponding to the given tangent ratio.
        ZY_INLINE static Angle FromTangent(Real32 Ratio)
        {
            return Angle(std::atan(Ratio));
        }

        /// \brief Creates an angle from Cartesian coordinates (X, Y).
        ///
        /// \param X The X coordinate.
        /// \param Y The Y coordinate.
        /// \return The angle corresponding to the given coordinates.
        ZY_INLINE static Angle FromCartesian(Real32 X, Real32 Y)
        {
            return Angle(std::atan2(Y, X));
        }

        /// \brief Normalizes the given angle to the range [0, 2π).
        ///
        /// \param Value The angle to normalize.
        /// \return The normalized angle.
        ZY_INLINE static Angle Normalize(Angle Value)
        {
            constexpr Real32 kTwoPi = 2.0f * kPI<Real32>;

            if (const Real32 Radians = Mod(Value.GetRadians(), kTwoPi); Radians < 0.0f)
            {
                return FromRadians(Radians + kTwoPi);
            }
            else
            {
                return FromRadians(Radians);
            }
        }

        /// \brief Calculates the cosine of the given angle.
        ///
        /// \param Value The angle to calculate the cosine for.
        /// \return The cosine of the angle.
        ZY_INLINE static Real32 Cosine(Angle Value)
        {
            return std::cos(Value.GetRadians());
        }

        /// \brief Calculates the sine of the given angle.
        ///
        /// \param Value The angle to calculate the sine for.
        /// \return The sine of the angle.
        ZY_INLINE static Real32 Sine(Angle Value)
        {
            return std::sin(Value.GetRadians());
        }

        /// \brief Calculates the tangent of the given angle.
        ///
        /// \param Value The angle to calculate the tangent for.
        /// \return The tangent of the angle.
        ZY_INLINE static Real32 Tangent(Angle Value)
        {
            return std::tan(Value.GetRadians());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real32 mRadians;
    };
}