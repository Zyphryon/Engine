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

#include "Dispatcher.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Platform
{
    /// \brief Encapsulate a platform-specific window used as output and receive input.
    class Window final : public Retainable<Window>
    {
        friend class Service;

    public:

        /// \brief Defines the native platform handle type for the window.
        using Handle = Ptr<void>;

    public:

        /// \brief Constructs a new window.
        ///
        /// \param Dispatcher The dispatcher to handle events.
        ZY_INLINE Window(Ref<Dispatcher> Dispatcher)
            : mDispatcher { Dispatcher },
              mX          { 0 },
              mY          { 0 },
              mWidth      { 0 },
              mHeight     { 0 },
              mStates     { 0 }
        {
        }

        /// \brief Destroys the window and releases its resources.
        ~Window();

        /// \brief Gets the native platform handle for graphics API interop.
        ///
        /// \return The opaque native handle that needs to be casted to the concrete platform type at the call site.
        Handle GetHandle() const;

        /// \brief Sets the title of the window.
        ///
        /// \param Title The new window title.
        void SetTitle(Text Title);

        /// \brief Gets the current title of the window.
        ///
        /// \return The current window title.
        ZY_INLINE Text GetTitle() const
        {
            return mTitle;
        }

        /// \brief Sets the position of the window on the screen.
        ///
        /// \param X The X coordinate for the window position.
        /// \param Y The Y coordinate for the window position.
        void SetPosition(SInt32 X, SInt32 Y);

        /// \brief Gets the current X coordinate of the window position.
        ///
        /// \return The X coordinate of the window.
        ZY_INLINE SInt32 GetX() const
        {
            return mX;
        }

        /// \brief Gets the current Y coordinate of the window position.
        ///
        /// \return The Y coordinate of the window.
        ZY_INLINE SInt32 GetY() const
        {
            return mY;
        }

        /// \brief Sets the visibility of the window.
        ///
        /// \param Visible `true` to make the window visible, otherwise `false`.
        void SetVisible(Bool Visible);

        /// \brief Checks whether the window is currently visible.
        ///
        /// \return `true` if visible, otherwise `false`.
        ZY_INLINE Bool IsVisible() const
        {
            return HasBit(mStates, State::Visible);
        }

        /// \brief Brings the window to the foreground and focuses it.
        void SetFocus();

        /// \brief Checks whether the window has input focus.
        ///
        /// \return `true` if focused, otherwise `false`.
        ZY_INLINE Bool IsFocused() const
        {
            return HasBit(mStates, State::Focused);
        }

        /// \brief Sets the fullscreen state of the window.
        ///
        /// \param Fullscreen `true` to set the window to fullscreen mode, otherwise `false`.
        void SetFullscreen(Bool Fullscreen);

        /// \brief Checks whether the window is in borderless mode.
        ///
        /// \return `true` if borderless, otherwise `false`.
        ZY_INLINE Bool IsBorderless() const
        {
            return HasBit(mStates, State::Borderless);
        }

        /// \brief Checks whether the window is in fullscreen mode.
        ///
        /// \return `true` if fullscreen, otherwise `false`.
        ZY_INLINE Bool IsFullscreen() const
        {
            return HasBit(mStates, State::Fullscreen);
        }

        /// \brief Checks whether the window is in windowed mode.
        ///
        /// \return `true` if windowed, otherwise `false`.
        ZY_INLINE Bool IsWindowed() const
        {
            return !IsFullscreen();
        }

        /// \brief Sets the size of the window.
        ///
        /// \param Width  The new width of the window.
        /// \param Height The new height of the window.
        void SetSize(UInt32 Width, UInt32 Height);

        /// \brief Gets the current width of the window.
        ///
        /// \return The current window width.
        ZY_INLINE UInt32 GetWidth() const
        {
            return mWidth;
        }

        /// \brief Gets the current height of the window.
        ///
        /// \return The current window height.
        ZY_INLINE UInt32 GetHeight() const
        {
            return mHeight;
        }

        /// \brief Sets the cursor position relative to the window.
        ///
        /// \param X The new X position of the cursor.
        /// \param Y The new Y position of the cursor.
        void SetCursorPosition(UInt32 X, UInt32 Y);

        /// \brief Sets the cursor lock state.
        ///
        /// \param State `true` to lock the cursor, otherwise `false`.
        void SetCursorLock(Bool State);

        /// \brief Checks whether the cursor is locked.
        ///
        /// \return `true` if the cursor is locked, otherwise `false`.
        ZY_INLINE Bool IsCursorLocked() const
        {
            return HasBit(mStates, State::Locked);
        }

    private:

        /// \brief Describes the state of the window.
        enum class State : UInt8
        {
            Visible    = 1 << 0,   ///< Indicates that the window is visible.
            Focused    = 1 << 1,   ///< Indicates that the window is focused.
            Locked     = 1 << 2,   ///< Indicates that the window cursor is locked.
            Held       = 1 << 3,   ///< Indicates that the window is being held.
            Borderless = 1 << 4,   ///< Indicates that the window is borderless.
            Fullscreen = 1 << 5,   ///< Indicates that the window is fullscreen.
        };
        ZY_DEFINE_BITWISE_FRIEND_ENUM(State)

        /// \brief Initialize the platform-specific implementation of the window.
        ///
        /// \param Title      The title of the window.
        /// \param X          The initial X position of the window.
        /// \param Y          The initial Y position of the window.
        /// \param Width      The initial width of the window.
        /// \param Height     The initial height of the window.
        /// \param Borderless `true` to create a borderless window, otherwise `false`.
        /// \param Fullscreen `true` to create a fullscreen window, otherwise `false`.
        /// \return `true` if the window was successfully initialized, otherwise `false`.
        Bool Initialize(Text Title, SInt32 X, SInt32 Y, UInt32 Width, UInt32 Height, Bool Borderless, Bool Fullscreen);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        struct Backend;
        Unique<Backend> mBackend;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Dispatcher> mDispatcher;
        Str             mTitle;
        SInt32          mX;
        SInt32          mY;
        UInt32          mWidth;
        UInt32          mHeight;
        State           mStates;
    };
}