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

#include "Array.hpp"
#include "Sequence.hpp"
#include "Span.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Represents a fixed grid of cells addressed by column and row, with the walks a field of them invites.
    template<typename Type, UInt32 Width, UInt32 Height>
    class Raster final
    {
        static_assert(Width > 0 && Height > 0, "Raster must hold at least one cell");

    public:

        /// \brief The number of cells across.
        static constexpr UInt32 kWidth  = Width;

        /// \brief The number of cells down.
        static constexpr UInt32 kHeight = Height;

        /// \brief The number of cells in all.
        static constexpr UInt32 kCount  = Width * Height;

        /// \brief Specifies which cells count as beside a cell.
        enum class Neighbourhood : UInt8
        {
            Cross,   ///< The four cells sharing an edge.
            Square,  ///< The eight cells sharing an edge or a corner.
        };

    public:

        /// \brief Constructs a raster with every cell value-initialized.
        ZY_INLINE constexpr Raster()
            : mCells { }
        {
        }

        /// \brief Gets a cell by column and row.
        ///
        /// \param X The column, which must lie within the raster.
        /// \param Y The row, which must lie within the raster.
        /// \return A reference to the cell.
        ZY_INLINE constexpr Ref<Type> At(UInt32 X, UInt32 Y)
        {
            return mCells[GetIndex(X, Y)];
        }

        /// \brief Gets a cell by column and row.
        ///
        /// \param X The column, which must lie within the raster.
        /// \param Y The row, which must lie within the raster.
        /// \return A reference to the cell.
        ZY_INLINE constexpr ConstRef<Type> At(UInt32 X, UInt32 Y) const
        {
            return mCells[GetIndex(X, Y)];
        }

        /// \brief Gets the cell nearest a column and row, which is the cell itself when they lie within the raster
        ///        and the cell at the edge in their direction when they do not.
        ///
        /// \param X The column, which may lie outside the raster.
        /// \param Y The row, which may lie outside the raster.
        /// \return A reference to the cell.
        ZY_INLINE constexpr ConstRef<Type> Nearest(SInt32 X, SInt32 Y) const
        {
            const UInt32 NearX = static_cast<UInt32>(Clamp(X, 0, static_cast<SInt32>(Width) - 1));
            const UInt32 NearY = static_cast<UInt32>(Clamp(Y, 0, static_cast<SInt32>(Height) - 1));

            return mCells[GetIndex(NearX, NearY)];
        }

        /// \brief Gets one row of cells.
        ///
        /// \param Y The row, which must lie within the raster.
        /// \return A span over the row's cells, from the first column to the last.
        ZY_INLINE constexpr Span<Type> GetRow(UInt32 Y)
        {
            return Span<Type>(mCells.GetData() + Y * Width, Width);
        }

        /// \brief Gets one row of cells.
        ///
        /// \param Y The row, which must lie within the raster.
        /// \return A span over the row's cells, from the first column to the last.
        ZY_INLINE constexpr ConstSpan<Type> GetRow(UInt32 Y) const
        {
            return ConstSpan<Type>(mCells.GetData() + Y * Width, Width);
        }

        /// \brief Gets a pointer to the contiguous cell storage.
        ///
        /// \return A pointer to the first cell.
        ZY_INLINE constexpr Ptr<Type> GetData()
        {
            return mCells.GetData();
        }

        /// \brief Gets a pointer to the contiguous cell storage.
        ///
        /// \return A pointer to the first cell.
        ZY_INLINE constexpr ConstPtr<Type> GetData() const
        {
            return mCells.GetData();
        }

        /// \brief Sets every cell to one value.
        ///
        /// \param Value The value to set.
        ZY_INLINE constexpr void Fill(ConstRef<Type> Value)
        {
            mCells.Fill(Value);
        }

        /// \brief Copies a rectangle of cells out of another raster, which may be of any size.
        ///
        /// \param Source  The raster to copy from.
        /// \param SourceX The column of the rectangle's first cell in the source.
        /// \param SourceY The row of the rectangle's first cell in the source.
        /// \param TargetX The column the rectangle's first cell lands on here.
        /// \param TargetY The row the rectangle's first cell lands on here.
        /// \param Columns The number of cells the rectangle spans across, which must fit both rasters.
        /// \param Rows    The number of cells the rectangle spans down, which must fit both rasters.
        template<typename Other>
        ZY_INLINE constexpr void Blit(
            ConstRef<Other> Source,
            UInt32          SourceX,
            UInt32          SourceY,
            UInt32          TargetX,
            UInt32          TargetY,
            UInt32          Columns,
            UInt32          Rows)
        {
            ZY_ASSERT(SourceX + Columns <= Other::kWidth
                   && SourceY + Rows <= Other::kHeight, "Blit reads past the source");
            ZY_ASSERT(TargetX + Columns <= Width
                   && TargetY + Rows <= Height, "Blit writes past the raster");

            for (UInt32 Row = 0; Row < Rows; ++Row)
            {
                const ConstSpan<Type> From = Source.GetRow(SourceY + Row).Slice(SourceX, Columns);
                const Span<Type>      To   = GetRow(TargetY + Row).Slice(TargetX, Columns);

                // A row is contiguous, so cells that copy as bytes go across in one move.
                if constexpr (IsTriviallyCopyable<Type>)
                {
                    ZyBase::Blit(To.GetData(), Columns * sizeof(Type), From.GetData());
                }
                else
                {
                    for (UInt32 Column = 0; Column < Columns; ++Column)
                    {
                        To[Column] = From[Column];
                    }
                }
            }
        }

        /// \brief Visits every cell in turn, row by row.
        ///
        /// \param Callback The callback invoked with each cell's index and a reference to it.
        template<typename Function>
        ZY_INLINE constexpr void ForEach(AnyRef<Function> Callback)
        {
            for (UInt32 Index = 0; Index < kCount; ++Index)
            {
                Callback(Index, mCells[Index]);
            }
        }

        /// \brief Visits every cell beside one, skipping those that would fall outside the raster.
        ///
        /// \param Index    The cell, counted row by row.
        /// \param Beside   The cells that count as beside it.
        /// \param Callback The callback invoked with each neighbour's index.
        template<typename Function>
        ZY_INLINE constexpr void ForEachNeighbour(UInt32 Index, Neighbourhood Beside, AnyRef<Function> Callback) const
        {
            const SInt32 X     = static_cast<SInt32>(GetX(Index));
            const SInt32 Y     = static_cast<SInt32>(GetY(Index));
            const UInt32 Count = (Beside == Neighbourhood::Cross) ? 4u : 8u;

            for (UInt32 Step = 0; Step < Count; ++Step)
            {
                const SInt32 NearX = X + kSteps[Step][0];
                const SInt32 NearY = Y + kSteps[Step][1];

                if (Contains(NearX, NearY))
                {
                    Callback(GetIndex(static_cast<UInt32>(NearX), static_cast<UInt32>(NearY)));
                }
            }
        }

        /// \brief Grows a region outward from a seed over every cell a step is admitted into, visiting each once.
        ///
        /// \param Seed    The cell to grow from, counted row by row.
        /// \param Beside  The cells that count as beside one another.
        /// \param Scratch The room the growth spreads through, borrowed so that a flood allocates nothing.
        /// \param Admit   The callback asked whether a step may be taken, with the cell stepped from and the cell to.
        /// \param Visit   The callback invoked with each cell the region grows over, the seed included.
        /// \return The number of cells the region grew over.
        template<typename FAdmit, typename FVisit>
        UInt32 Flood(
            UInt32                Seed,
            Neighbourhood         Beside,
            Ref<Sequence<UInt32>> Scratch,
            AnyRef<FAdmit>        Admit,
            AnyRef<FVisit>        Visit) const
        {
            UInt32 Grown = 1;

            Scratch.Clear();
            Scratch.Append(Seed);

            Visit(Seed);

            while (!Scratch.IsEmpty())
            {
                const UInt32 Cursor = Scratch.GetBack();
                Scratch.RemoveLast();

                ForEachNeighbour(Cursor, Beside, [&](UInt32 Next)
                {
                    if (Admit(Cursor, Next))
                    {
                        Visit(Next);
                        Scratch.Append(Next);

                        ++Grown;
                    }
                });
            }
            return Grown;
        }

        /// \brief Measures how far every cell lies from the nearest cell a predicate holds for, in cells either way.
        ///
        /// \param Result The raster the distance of every cell is written into, zero where the predicate holds.
        /// \param Solid  The callback asked whether a cell is one the distance is measured to, given the cell.
        template<typename Distance, typename FSolid>
        void Measure(Ref<Raster<Distance, Width, Height>> Result, AnyRef<FSolid> Solid) const
        {
            constexpr UInt32 kFarthest = static_cast<UInt32>(kMaximum<Distance>);

            const auto Chamfer = [&](SInt32 X, SInt32 Y, SInt32 Toward)
            {
                UInt32 Nearest = kFarthest;

                const SInt32 Behind[4][2] = {
                    { X + Toward, Y     },
                    { X, Y + Toward     },
                    { X - 1, Y + Toward },
                    { X + 1, Y + Toward }
                };

                for (UInt32 Each = 0; Each < 4; ++Each)
                {
                    if (Contains(Behind[Each][0], Behind[Each][1]))
                    {
                        const UInt32 Near = Result[GetIndex(Behind[Each][0], Behind[Each][1])];

                        Nearest = Min(Nearest, Near + 1u);
                    }
                }
                return static_cast<Distance>(Min(Nearest, kFarthest));
            };

            for (SInt32 Y = 0; Y < static_cast<SInt32>(Height); ++Y)
            {
                for (SInt32 X = 0; X < static_cast<SInt32>(Width); ++X)
                {
                    const UInt32 Index = GetIndex(X, Y);

                    Result[Index] = Solid(mCells[Index]) ? Distance(0) : Chamfer(X, Y, -1);
                }
            }

            for (SInt32 Y = static_cast<SInt32>(Height) - 1; Y >= 0; --Y)
            {
                for (SInt32 X = static_cast<SInt32>(Width) - 1; X >= 0; --X)
                {
                    const UInt32 Index = GetIndex(X, Y);

                    if (!Solid(mCells[Index]))
                    {
                        Result[Index] = Min(Result[Index], Chamfer(X, Y, 1));
                    }
                }
            }
        }

        /// \brief Gets a cell by index.
        ///
        /// \param Index The index, counted row by row.
        /// \return A reference to the cell.
        ZY_INLINE constexpr Ref<Type> operator[](UInt32 Index)
        {
            return mCells[Index];
        }

        /// \brief Gets a cell by index.
        ///
        /// \param Index The index, counted row by row.
        /// \return A reference to the cell.
        ZY_INLINE constexpr ConstRef<Type> operator[](UInt32 Index) const
        {
            return mCells[Index];
        }

    public:

        /// \brief Gets the column of the cell an index names.
        ///
        /// \param Index The index, counted row by row.
        /// \return The column.
        ZY_INLINE static constexpr UInt32 GetX(UInt32 Index)
        {
            return Index % Width;
        }

        /// \brief Gets the row of the cell an index names.
        ///
        /// \param Index The index, counted row by row.
        /// \return The row.
        ZY_INLINE static constexpr UInt32 GetY(UInt32 Index)
        {
            return Index / Width;
        }

        /// \brief Flattens a column and row into the index of the cell they name.
        ///
        /// \param X The column, which must lie within the raster.
        /// \param Y The row, which must lie within the raster.
        /// \return The index, counted row by row.
        ZY_INLINE static constexpr UInt32 GetIndex(UInt32 X, UInt32 Y)
        {
            return Y * Width + X;
        }

        /// \brief Checks whether a column and row name a cell of the raster.
        ///
        /// \param X The column.
        /// \param Y The row.
        /// \return `true` when both lie within the raster, otherwise `false`.
        ZY_INLINE static constexpr Bool Contains(SInt32 X, SInt32 Y)
        {
            return X >= 0 && X < static_cast<SInt32>(Width) && Y >= 0 && Y < static_cast<SInt32>(Height);
        }

    private:

        /// \brief The steps to the eight cells beside any one, the four sharing an edge first.
        static constexpr SInt32 kSteps[8][2] = {
            { -1,  0 }, {  1,  0 }, {  0, -1 }, {  0,  1 },
            { -1, -1 }, {  1, -1 }, { -1,  1 }, {  1,  1 } };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Type, kCount> mCells;
    };
}