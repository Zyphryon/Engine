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

namespace ZyRender
{
    /// \brief Represents one realization of a \ref Blueprint: a texture per target and a handle per pass, at one size.
    ///
    /// \note A view is a graph, so drawing the same scene from another camera costs just another set of buffers.
    class ZY_API Graph final
    {
    public:

        /// \brief Constructs a graph that realizes the given blueprint.
        ///
        /// \param Host      The service host that provides the graphic service.
        /// \param Blueprint The blueprint naming what the graph draws.
        Graph(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint);

        /// \brief Destroys every texture and handle the graph holds.
        ~Graph();

        /// \brief Sets the output size the scaled targets track, which the next \ref Run realizes them at.
        ///
        /// \param Width  The frame's output width, in pixels.
        /// \param Height The frame's output height, in pixels.
        void Resize(UInt16 Width, UInt16 Height);

        /// \brief Realizes whatever target or pass changed since the last run, then executes every active pass in order.
        ///
        /// \param Frame The pre-packed frame uniform stream.
        void Run(ZyGraphic::Stream Frame);

        /// \brief Packs a value into a transient uniform block, then runs the frame with it as the frame's block.
        ///
        /// \param Block The value laid out as the techniques declare the frame's block.
        template<typename Type>
        ZY_INLINE void Run(ConstRef<Type> Block)
            requires (!IsAnyOf<Type, ZyGraphic::Stream>)
        {
            ZyGraphic::Transient<Type> Slice = mService->AllocateInFlightUniforms<Type>(1);
            Slice[0] = Block;

            Run(Slice.GetStream());
        }

        /// \brief Gets the texture realized for one of the blueprint's targets.
        ///
        /// \param Slot The slot naming the target, as \ref Blueprint::AddTarget returned it.
        /// \return The texture object, valid until a run realizes the target again, or zero before the first run.
        ZY_INLINE ZyGraphic::Object GetTexture(UInt32 Slot) const
        {
            return Slot < mSlots.GetSize() ? mSlots[Slot].Texture : 0;
        }

        /// \brief Gets the width one of the blueprint's targets came out at, in pixels.
        ///
        /// \param Slot The slot naming the target.
        /// \return The width the target was realized at, or zero before the first run.
        ZY_INLINE UInt16 GetWidth(UInt32 Slot) const
        {
            return Slot < mSlots.GetSize() ? mSlots[Slot].Width : 0;
        }

        /// \brief Gets the height one of the blueprint's targets came out at, in pixels.
        ///
        /// \param Slot The slot naming the target.
        /// \return The height the target was realized at, or zero before the first run.
        ZY_INLINE UInt16 GetHeight(UInt32 Slot) const
        {
            return Slot < mSlots.GetSize() ? mSlots[Slot].Height : 0;
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

        /// \brief Represents one target the graph realized, and the shape it came out in.
        struct Slot final
        {
            /// The texture the graph realized for the target, or zero while it holds none.
            ZyGraphic::Object        Texture = 0;

            /// The format the texture was realized with.
            ZyGraphic::TextureFormat Format  = ZyGraphic::TextureFormat::Unspecified;

            /// The width the texture came out at, in pixels.
            UInt16                   Width   = 0;

            /// The height the texture came out at, in pixels.
            UInt16                   Height  = 0;
        };

        /// \brief Represents one pass the graph baked, and the surface it draws into.
        struct Step final
        {
            /// The pass handle the step draws through.
            ZyGraphic::Object   Handle = 0;

            /// The viewport covering the target the pass draws into, left unset for the display, which tracks the output.
            ZyGraphic::Viewport Viewport;
        };

        /// \brief Realizes every target whose shape changed, and bakes the passes again when any target or pass did.
        void Reconcile();

        /// \brief Destroys every texture and handle, leaving the graph unrealized.
        void Release();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Service> mService;
        Ref<Blueprint>               mBlueprint;
        Encoder                      mEncoder;
        Sequence<Slot>               mSlots;
        Sequence<Step>               mSteps;
        UInt16                       mWidth;
        UInt16                       mHeight;
    };
}