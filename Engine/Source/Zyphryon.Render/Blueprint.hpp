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

#include "Pass.hpp"
#include "Target.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Declares what a frame draws: the targets it needs, and the passes that fill them, in order.
    ///
    /// \note A scene drawn from several cameras keeps a single set of passes and the state each of them carries.
    class Blueprint final
    {
        friend class Graph;

    public:

        /// \brief Declares a target and returns the slot that names it.
        ///
        /// \param Description The target's format and sizing policy.
        /// \return The slot every attachment and every graph addresses the target by.
        ZY_INLINE UInt32 AddTarget(ConstRef<Target> Description)
        {
            mTargets.Append(Description);

            return mTargets.GetSize() - 1;
        }

        /// \brief Gets the targets the pipeline declares.
        ///
        /// \return A read-only reference to the target list, in slot order.
        ZY_INLINE ConstSpan<Target> GetTargets() const
        {
            return mTargets;
        }

        /// \brief Restates the fraction of the output a graph realizes one target at.
        ///
        /// \param Slot   The slot the target was declared under.
        /// \param Sizing The fraction of the output to realize it at.
        ZY_INLINE void ResizeTarget(UInt32 Slot, Target::Scale Sizing)
        {
            mTargets[Slot].Sizing = Sizing;
        }

        /// \brief Restates the explicit size a graph realizes one target at, independent of the output.
        ///
        /// \param Slot   The slot the target was declared under.
        /// \param Width  The width to realize it at, in pixels.
        /// \param Height The height to realize it at, in pixels.
        ZY_INLINE void ResizeTarget(UInt32 Slot, UInt16 Width, UInt16 Height)
        {
            Ref<Target> Description = mTargets[Slot];
            Description.Sizing = Target::Scale::Fixed;
            Description.Width  = Width;
            Description.Height = Height;
        }

        /// \brief Creates a pass of the given type, appends it to the execution order, and returns it.
        ///
        /// \param Parameters The arguments forwarded to the pass's constructor.
        /// \return A reference to the newly created pass.
        template<typename Type, typename... Arguments>
        ZY_INLINE Ref<Type> AddPass(AnyRef<Arguments>... Parameters)
        {
            return static_cast<Ref<Type>>(* mPasses.Append(Unique<Type>::Create(Forward<Arguments>(Parameters)...)));
        }

        /// \brief Gets a pass by its position in the execution order.
        ///
        /// \param Index The zero-based index of the pass, in execution order.
        /// \return A reference to the requested pass.
        template<typename Type = Pass>
        ZY_INLINE Ref<Type> GetPass(UInt32 Index)
        {
            return static_cast<Ref<Type>>(* mPasses[Index]);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Target>       mTargets;
        Sequence<Unique<Pass>> mPasses;
    };
}