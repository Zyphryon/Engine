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
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Executes an ordered set of render passes for a frame.
    class Renderer final
    {
    public:

        /// \brief Constructs a renderer bound to the host's graphic service.
        ///
        /// \param Host The service host that provides the graphic service.
        Renderer(Ref<Engine::Subsystem::Host> Host);

        /// \brief Creates a pass of the given type, appends it to the execution order, and returns it.
        ///
        /// \param Parameters The arguments forwarded to the pass's constructor.
        /// \return A reference to the newly created pass.
        template<typename Type, typename... Arguments>
        ZY_INLINE Ref<Type> AddPass(AnyRef<Arguments>... Parameters)
        {
            return static_cast<Ref<Type>>(* mPasses.Append(Unique<Type>::Create(Forward<Arguments>(Parameters)...)));
        }

        /// \brief Creates a renderer-managed target.
        ///
        /// \param Description The target's format and sizing policy.
        /// \return A reference to the newly created target.
        ZY_INLINE Ref<Target> AddTarget(ConstRef<Target::Description> Description)
        {
            return * mTargets.Append(Unique<Target>::Create(Description));
        }

        /// \brief Resizes every managed target and rebuilds each pass handle from its declared attachments.
        ///
        /// \param Width  The frame's output width, in pixels.
        /// \param Height The frame's output height, in pixels.
        void Resize(UInt16 Width, UInt16 Height);

        /// \brief Executes every registered pass in order and submits the frame.
        ///
        /// \param Global The pre-packed frame-global uniform stream.
        void Run(ConstRef<Graphic::Stream> Global);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Graphic::Service> mService;
        Encoder                    mEncoder;
        Sequence<Unique<Target>>   mTargets;
        Sequence<Unique<Pass>>     mPasses;
    };
}
