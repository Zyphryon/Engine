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

#include "Relief.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNavigation
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Relief::Bake(ConstSpan<Vector3> Faces, ConstRef<Recipe> Recipe)
    {
        * this = Relief();

        if (Faces.GetSize() < 3 || Recipe.Spacing <= 0.0f)
        {
            return;
        }

        Vector2 Low(Faces[0].GetX(), Faces[0].GetZ());
        Vector2 High = Low;

        for (ConstRef<Vector3> Corner : Faces)
        {
            Low  = Vector2(Min(Low.GetX(), Corner.GetX()), Min(Low.GetY(), Corner.GetZ()));
            High = Vector2(Max(High.GetX(), Corner.GetX()), Max(High.GetY(), Corner.GetZ()));
        }

        // The grid starts on a whole cell, so the cells of a solid set down on the spacing fall on a world's own.
        const SInt32 Left = static_cast<SInt32>(Floor(Low.GetX() / Recipe.Spacing));
        const SInt32 Near = static_cast<SInt32>(Floor(Low.GetY() / Recipe.Spacing));

        mSpacing = Recipe.Spacing;
        mCorner  = Vector2(static_cast<Real32>(Left), static_cast<Real32>(Near)) * mSpacing;
        mWidth   = static_cast<UInt16>(static_cast<SInt32>(Ceil(High.GetX() / mSpacing)) - Left);
        mDepth   = static_cast<UInt16>(static_cast<SInt32>(Ceil(High.GetY() / mSpacing)) - Near);

        const SInt32 Wide = mWidth * kPoints;
        const SInt32 Deep = mDepth * kPoints;
        const Real32 Fine = mSpacing / kPoints;

        // Every face is laid over the points whose upright line it crosses, at the height it crosses it.
        Sequence<Hit> Hits;

        for (UInt Index = 0; Index + 2 < Faces.GetSize(); Index += 3)
        {
            const Vector3 Origin = Faces[Index];
            const Vector3 Along  = Faces[Index + 1] - Origin;
            const Vector3 Across = Faces[Index + 2] - Origin;

            // A face wound clockwise from its front is turned up when its span over the ground comes out positive.
            const Real32 Span = Along.GetX() * Across.GetZ() - Along.GetZ() * Across.GetX();

            if (Span == 0.0f)
            {
                continue;
            }

            // A face is level enough when its normal leans no further than the slope lets it rise.
            const Vector2 Lean  = Vector3::Cross(Along, Across).GetXZ();
            const Bool    Flat  = Span > 0.0f && (Recipe.Slope <= 0.0f || Lean.GetLength() <= Recipe.Slope * Span);
            const Vector2 First = (Vector2(Origin.GetX(), Origin.GetZ()) - mCorner) / Fine;
            const Vector2 Reach(Along.GetX() / Fine, Along.GetZ() / Fine);
            const Vector2 Sweep(Across.GetX() / Fine, Across.GetZ() / Fine);

            const Real32 MinimumX = First.GetX() + Min(0.0f, Min(Reach.GetX(), Sweep.GetX()));
            const Real32 MaximumX = First.GetX() + Max(0.0f, Max(Reach.GetX(), Sweep.GetX()));
            const Real32 MinimumY = First.GetY() + Min(0.0f, Min(Reach.GetY(), Sweep.GetY()));
            const Real32 MaximumY = First.GetY() + Max(0.0f, Max(Reach.GetY(), Sweep.GetY()));

            const SInt32 FromX = Max(0, static_cast<SInt32>(Floor(MinimumX)));
            const SInt32 FromY = Max(0, static_cast<SInt32>(Floor(MinimumY)));
            const SInt32 ToX   = Min(Wide, static_cast<SInt32>(Ceil(MaximumX)));
            const SInt32 ToY   = Min(Deep, static_cast<SInt32>(Ceil(MaximumY)));

            for (SInt32 Y = FromY; Y < ToY; ++Y)
            {
                for (SInt32 X = FromX; X < ToX; ++X)
                {
                    // A point is read at the middle of its square, as shares of the face's two sides from its corner.
                    const Real32 OffsetX = (static_cast<Real32>(X) + 0.5f - First.GetX()) * Fine;
                    const Real32 OffsetY = (static_cast<Real32>(Y) + 0.5f - First.GetY()) * Fine;
                    const Real32 ShareA  = (OffsetX * Across.GetZ() - OffsetY * Across.GetX()) / Span;
                    const Real32 ShareB  = (OffsetY * Along.GetX() - OffsetX * Along.GetZ()) / Span;

                    if (ShareA >= 0.0f && ShareB >= 0.0f && ShareA + ShareB <= 1.0f)
                    {
                        const Real32 Height = Origin.GetY() + ShareA * Along.GetY() + ShareB * Across.GetY();

                        Hits.Append(Hit { static_cast<UInt32>(Y * Wide + X), Height, Flat });
                    }
                }
            }
        }

        Hits.Sort([](ConstRef<Hit> Left, ConstRef<Hit> Right)
        {
            return Left.Point != Right.Point ? Left.Point < Right.Point : Left.Height < Right.Height;
        });

        // A point takes the lowest face to stand on that has its headroom clear of every face over it.
        Sequence<Real32> Points;
        Points.Fill(kNone, static_cast<UInt>(Wide) * Deep);

        for (UInt Index = 0; Index < Hits.GetSize(); ++Index)
        {
            ConstRef<Hit> Each = Hits[Index];

            if (!Each.Flat || Points[Each.Point] != kNone)
            {
                continue;
            }

            UInt Over = Index + 1;

            while (Over < Hits.GetSize() && Hits[Over].Point == Each.Point && Hits[Over].Height <= Each.Height + kSkin)
            {
                ++Over;
            }

            const Bool Capped = Over < Hits.GetSize() && Hits[Over].Point == Each.Point;

            if (!Capped || Hits[Over].Height - Each.Height >= Recipe.Headroom)
            {
                Points[Each.Point] = Each.Height;
            }
        }

        // A cell lays the middle one of the floors its points found, once a third of them found any.
        mFloors.Fill(kNone, static_cast<UInt>(mWidth) * mDepth);

        for (UInt32 Row = 0; Row < mDepth; ++Row)
        {
            for (UInt32 Column = 0; Column < mWidth; ++Column)
            {
                Sequence<Real32, kPoints * kPoints> Found;

                for (UInt32 Y = 0; Y < kPoints; ++Y)
                {
                    for (UInt32 X = 0; X < kPoints; ++X)
                    {
                        const Real32 Floor = Points[(Row * kPoints + Y) * Wide + Column * kPoints + X];

                        if (Floor != kNone)
                        {
                            Found.Append(Floor);
                        }
                    }
                }

                if (Found.GetSize() * 3 >= kPoints * kPoints)
                {
                    Found.Sort();
                    mFloors[Row * mWidth + Column] = Found[Found.GetSize() / 2];
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Relief::GetHeight(Vector2 Spot, Real32 Climb) const
    {
        if (mFloors.IsEmpty())
        {
            return kNone;
        }

        const Vector2 Cells = (Spot - mCorner) / mSpacing;
        const SInt32  X     = static_cast<SInt32>(Floor(Cells.GetX()));
        const SInt32  Z     = static_cast<SInt32>(Floor(Cells.GetY()));

        if (X < 0 || Z < 0 || X >= mWidth || Z >= mDepth)
        {
            return kNone;
        }

        const Real32 Held = mFloors[Z * mWidth + X];

        if (Held == kNone)
        {
            return kNone;
        }

        // A floor is held at the middle of its cell, so the position is eased between the middles about it.
        const Real32 FromX = Cells.GetX() - 0.5f;
        const Real32 FromZ = Cells.GetY() - 0.5f;
        const SInt32 X0    = Clamp(static_cast<SInt32>(Floor(FromX)), 0, mWidth - 1);
        const SInt32 Z0    = Clamp(static_cast<SInt32>(Floor(FromZ)), 0, mDepth - 1);
        const SInt32 X1    = Min(X0 + 1, mWidth - 1);
        const SInt32 Z1    = Min(Z0 + 1, mDepth - 1);

        // Only a floor within a climb of the cell's own eases into it, so a wall beside a floor stays a wall.
        const auto Read = [&](SInt32 CellX, SInt32 CellZ)
        {
            const Real32 Near = mFloors[CellZ * mWidth + CellX];
            return (Near != kNone && Abs(Near - Held) <= Climb) ? Near : Held;
        };

        const Real32 ShareX = Clamp(FromX - static_cast<Real32>(X0), 0.0f, 1.0f);
        const Real32 ShareZ = Clamp(FromZ - static_cast<Real32>(Z0), 0.0f, 1.0f);

        return Lerp(Lerp(Read(X0, Z0), Read(X1, Z0), ShareX), Lerp(Read(X0, Z1), Read(X1, Z1), ShareX), ShareZ);
    }
}