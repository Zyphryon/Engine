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

#include "Chunk.hpp"
#include "Zyphryon.Scene/Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Chunk::Chunk(UInt32 Index, UInt32 Base, Bool Archetype, ConstSpan<UInt32> Signature)
        : mSize      { 0 },
          mCapacity  { 0 },
          mShift     { 0 },
          mMask      { 0 },
          mStride    { sizeof(Handle) },
          mIndex     { Index },
          mBase      { Base },
          mArchetype { Archetype },
          mSignature { Signature },
          mClosure   { this }
    {
        if (!Signature.IsEmpty())
        {
            mColumnOf.Fill(kAbsent, Signature.GetBack() + 1);
            mFlags.Advance(mColumnOf.GetSize());
            mRunOf.Fill(0, mColumnOf.GetSize());
        }

        for (const UInt32 Identifier : Signature)
        {
            ConstRef<Metatype> Info   = Registry::Get().GetMetatype(Identifier);
            ConstRef<Metatype> Stored = Registry::Get().GetColumn(Identifier);

            ZY_ASSERT(Stored.GetAlignment() <= kPageAlignment, "A component is aligned wider than a page");

            // A local component and the archetype marker stay on the archetype, anything else is lent or copied.
            if (!Info.Has(Trait::Local) && Identifier != IdentifierOf<Prefab>())
            {
                mFlags[Identifier] = Info.Has(Trait::Inheritable) ? kLends : kCopies;
            }

            if (Info.Has(Trait::Serializable) && (Stored.Save || Stored.IsTag()))
            {
                mFlags[Identifier] |= kSaves;
            }

            if (Stored.IsTag())
            {
                mColumnOf[Identifier] = kTag;
            }
            else
            {
                mColumns.Append(Identifier, static_cast<UInt32>(Stored.GetSize()), 0, AddressOf(Stored));
            }
        }

        // Runs sit widest alignment first after the handles, so each one starts aligned whatever the rows of a page.
        mColumns.Sort(IsWider);

        for (UInt32 Column = 0; Column < mColumns.GetSize(); ++Column)
        {
            mStride += mColumns[Column].Size;
            mColumnOf[mColumns[Column].Identifier] = static_cast<SInt16>(Column);
        }

        // A full page holds the power of two rows closest below the bytes a page aims at, never below the first page.
        mShift = Max(CountSignificantBits(Max(kPageBytes / mStride, 1u)) - 1, CountSignificantBits(kFirstRows - 1));
        mMask  = (1u << mShift) - 1;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Chunk::~Chunk()
    {
        for (UInt32 Row = 0; Row < mSize; Row += GetRows())
        {
            for (ConstRef<Column> Target : mColumns)
            {
                Destruct(* Target.Info, GetPage(Row) + Target.Offset, Min(mSize - Row, GetRows()));
            }
        }

        for (const Ptr<Byte> Page : mPages)
        {
            Free(Page, kPageAlignment);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chunk::Rebuild()
    {
        // A chunk laid out afresh from the registry takes every row, each column whose description changed converted.
        Chunk  Fresh(mIndex, mBase, mArchetype, mSignature);
        Writer Bytes;

        for (UInt32 Row = 0; Row < mSize; ++Row)
        {
            const UInt32 Into = Fresh.Append(GetHandle(Row));

            for (UInt32 Index = 0; Index < mColumns.GetSize(); ++Index)
            {
                ConstRef<Metatype> Before = * mColumns[Index].Info;
                const SInt16       After  = Fresh.Find(mColumns[Index].Identifier);

                if (After >= 0 && Fresh.mColumns[After].Info == AddressOf(Before))
                {
                    Relocate(Before, Fresh.At(After, Into), At(Index, Row), 1);
                    continue;
                }

                Bytes.Clear();
                Before.Save(Bytes, At(Index, Row));
                Destruct(Before, At(Index, Row), 1);

                if (After >= 0)
                {
                    ConstRef<Metatype> Info = * Fresh.mColumns[After].Info;
                    Reader             Input(Bytes);

                    Construct(Info, Fresh.At(After, Into), 1);

                    // A value only reserved by name holds no bytes yet, so it keeps its default.
                    if (Info.Load && Bytes.GetSize() > 0)
                    {
                        Info.Load(Input, Fresh.At(After, Into));
                    }
                }
            }
        }

        Swap(mCapacity, Fresh.mCapacity);
        Swap(mShift,    Fresh.mShift);
        Swap(mMask,     Fresh.mMask);
        Swap(mStride,   Fresh.mStride);
        Swap(mColumns,  Fresh.mColumns);
        Swap(mColumnOf, Fresh.mColumnOf);
        Swap(mRunOf,    Fresh.mRunOf);
        Swap(mFlags,    Fresh.mFlags);
        Swap(mPages,    Fresh.mPages);

        // Its rows were moved out already, so it only gives the old pages back.
        Fresh.mSize = 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chunk::SetAddition(UInt32 Identifier, Ptr<Chunk> Target)
    {
        if (Identifier >= mAdditions.GetSize())
        {
            mAdditions.Advance(Identifier + 1 - mAdditions.GetSize());
        }
        mAdditions[Identifier] = Target;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chunk::SetRemoval(UInt32 Identifier, Ptr<Chunk> Target)
    {
        if (Identifier >= mRemovals.GetSize())
        {
            mRemovals.Advance(Identifier + 1 - mRemovals.GetSize());
        }
        mRemovals[Identifier] = Target;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Chunk::IsWider(ConstRef<Column> Left, ConstRef<Column> Right)
    {
        return Left.Info->GetAlignment() > Right.Info->GetAlignment();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chunk::Grow()
    {
        if (mCapacity >= GetRows())
        {
            mPages.Append(Allocate<Byte>(mStride * GetRows() + kPageAlignment * mColumns.GetSize(), kPageAlignment));
            mCapacity += GetRows();
            return;
        }

        // The first page doubles while it is not full, each run moving to where the larger page keeps it.
        const UInt32    Rows = mCapacity ? mCapacity * 2 : Min(kFirstRows, GetRows());
        const Ptr<Byte> Page = Allocate<Byte>(mStride * Rows + kPageAlignment * mColumns.GetSize(), kPageAlignment);
        const Ptr<Byte> Old  = mCapacity ? mPages[0] : nullptr;

        // The bytes every row takes in the runs laid out so far, the handles first.
        UInt32 Before = sizeof(Handle);

        for (UInt32 Index = 0; Index < mColumns.GetSize(); ++Index)
        {
            Ref<Column>  Target   = mColumns[Index];
            const UInt32 Previous = Target.Offset;

            // Each run starts a cache line after the rows before it end.
            Target.Offset = Rows * Before + kPageAlignment * (Index + 1);
            Before       += Target.Size;

            // A typed read steps by its type's size, so the bytes kept for an unresolved type offer it no run.
            mRunOf[Target.Identifier] = Target.Info == AddressOf(Metatype::Unitialized()) ? 0 : Target.Offset;

            if (Old)
            {
                Relocate(* Target.Info, Page + Target.Offset, Old + Previous, mSize);
            }
        }

        if (Old)
        {
            Blit(Page, sizeof(Handle) * mSize, Old);
            Free(Old, kPageAlignment);
            mPages[0] = Page;
        }
        else
        {
            mPages.Append(Page);
        }
        mCapacity = Rows;
    }
}