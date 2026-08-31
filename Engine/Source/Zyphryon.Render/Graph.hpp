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

#include "Blueprint.hpp"
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief One realization of a \ref Blueprint: a texture per target and a handle per pass, at one size.
    ///
    /// \note A view is a graph, so drawing the same scene from another camera costs just another set of buffers.
    class Graph final
    {
    public:

        /// \brief Constructs a graph that realizes the given blueprint.
        ///
        /// \param Host      The service host that provides the graphic service.
        /// \param Blueprint The blueprint naming what the graph draws.
        Graph(Ref<Engine::Subsystem::Host> Host, Ref<Blueprint> Blueprint);

        /// \brief Destroys every texture and handle the graph holds.
        ~Graph();

        /// \brief Recreates every texture and pass handle at the specified output size.
        ///
        /// \param Width  The frame's output width, in pixels.
        /// \param Height The frame's output height, in pixels.
        void Resize(UInt16 Width, UInt16 Height);

        /// \brief Executes every active pass in order and submits the frame.
        ///
        /// \param Frame The pre-packed frame uniform stream.
        void Run(Graphic::Stream Frame);

        /// \brief Gets the texture realized for one of the blueprint's targets.
        ///
        /// \param Slot The slot naming the target, as \ref Blueprint::AddTarget returned it.
        /// \return The texture object, valid until the next resize.
        ZY_INLINE Graphic::Object GetTexture(UInt32 Slot) const
        {
            return mSlots[Slot].Texture;
        }

        /// \brief Gets the width one of the blueprint's targets came out at, in pixels.
        ///
        /// \param Slot The slot naming the target.
        /// \return The width the target was realized at.
        ZY_INLINE UInt16 GetWidth(UInt32 Slot) const
        {
            return mSlots[Slot].Width;
        }

        /// \brief Gets the height one of the blueprint's targets came out at, in pixels.
        ///
        /// \param Slot The slot naming the target.
        /// \return The height the target was realized at.
        ZY_INLINE UInt16 GetHeight(UInt32 Slot) const
        {
            return mSlots[Slot].Height;
        }

        /// \brief Gets the output width the graph was last resized to, in pixels.
        ///
        /// \return The width every full-scale target tracks.
        ZY_INLINE UInt16 GetWidth() const
        {
            return mWidth;
        }

        /// \brief Gets the output height the graph was last resized to, in pixels.
        ///
        /// \return The height every full-scale target tracks.
        ZY_INLINE UInt16 GetHeight() const
        {
            return mHeight;
        }

    private:

        /// \brief One target the graph realized, and the size it came out at.
        struct Slot final
        {
            /// TODO_DOC
            Graphic::Object Texture;

            /// TODO_DOC
            UInt16          Width;

            /// TODO_DOC
            UInt16          Height;
        };

        /// \brief One pass the graph baked, and the surface it draws into.
        struct Step final
        {
            /// The pass handle, borrowed from the step before it when the pass draws inline.
            Graphic::Object   Handle = 0;

            /// Whether the handle belongs to the step before it, which this pass appends its draws to.
            Bool              Inline = false;

            /// The viewport covering the target the pass draws into.
            Graphic::Viewport Viewport;
        };

        /// \brief Destroys every texture and handle, leaving the graph unrealized.
        void Release();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Graphic::Service> mService;
        Ref<Blueprint>             mBlueprint;
        Encoder                    mEncoder;
        Sequence<Slot>             mSlots;
        Sequence<Step>             mSteps;
        UInt16                     mWidth;
        UInt16                     mHeight;
    };
}