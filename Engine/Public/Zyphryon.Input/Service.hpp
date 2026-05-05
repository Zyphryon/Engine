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

#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "Poller.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Input
{
    /// \brief Provides high-level management of the input subsystem.
    class Service final : public AbstractService<Service>
    {
    public:

        /// \brief Event triggered when a key is pressed down.
        MulticastDelegate<Bool(Key)>                            OnKeyDown;

        /// \brief Event triggered when a key is released.
        MulticastDelegate<Bool(Key)>                            OnKeyUp;

        /// \brief Event triggered when a text input is received.
        MulticastDelegate<Bool(ConstStr8)>                      OnKeyType;

        /// \brief Event triggered when a mouse button is pressed down.
        MulticastDelegate<Bool(Button)>                         OnMouseDown;

        /// \brief Event triggered when a mouse button is released.
        MulticastDelegate<Bool(Button)>                         OnMouseUp;

        /// \brief Event triggered when the mouse is moved.
        MulticastDelegate<Bool(Real32, Real32, Real32, Real32)> OnMouseMove;

        /// \brief Event triggered when the mouse wheel is scrolled.
        MulticastDelegate<Bool(Real32, Real32)>                 OnMouseScroll;

        /// \brief Event triggered when the window gains or loses focus.
        MulticastDelegate<Bool(Bool)>                           OnWindowFocus;

        /// \brief Event triggered when the window is resized.
        MulticastDelegate<Bool(UInt32, UInt32)>                 OnWindowResize;

        /// \brief Event triggered when the window is requested to close or exit.
        MulticastDelegate<Bool()>                               OnWindowExit;

    public:

        /// \brief Constructs the input service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \copydoc Service::OnTick(Time)
        void OnTick(Time Time) override;

        /// \brief Dispatches events to the appropriate delegates.
        ///
        /// \param Event The event to handle.
        /// \return `true` if the event was handled and should not propagate further, otherwise
        ZYPHRYON_INLINE Bool Invoke(ConstRef<Event> Event) const
        {
            switch (Event.Kind)
            {
            case Event::Type::KeyType:
                return OnKeyType.Propagate(Event.KeyType.Text);
            case Event::Type::KeyUp:
                return OnKeyUp.Propagate(Event.KeyAction.Key);
            case Event::Type::KeyDown:
                return OnKeyDown.Propagate(Event.KeyAction.Key);
            case Event::Type::MouseUp:
                return OnMouseUp.Propagate(Event.MouseAction.Button);
            case Event::Type::MouseDown:
                return OnMouseDown.Propagate(Event.MouseAction.Button);
            case Event::Type::MouseMove:
                return OnMouseMove.Propagate(
                    Event.MouseAxis.X,
                    Event.MouseAxis.Y,
                    Event.MouseAxis.DeltaX,
                    Event.MouseAxis.DeltaY);
            case Event::Type::MouseScroll:
                return OnMouseScroll.Propagate(Event.MouseScroll.DeltaX, Event.MouseScroll.DeltaY);
            case Event::Type::WindowFocus:
                return OnWindowFocus.Propagate(Event.WindowFocus.State);
            case Event::Type::WindowResize:
                return OnWindowResize.Propagate(Event.WindowResize.Width, Event.WindowResize.Height);
            case Event::Type::WindowExit:
                return OnWindowExit.Propagate();
            default:
                return false;
            }
        }

        /// \brief Checks if a key was pressed in the current frame.
        ///
        /// \param Key The key to check.
        /// \return `true` if the key transitioned from released to pressed this frame, otherwise `false`.
        ZYPHRYON_INLINE Bool IsKeyPressed(Key Key) const
        {
            return mKeyboard.IsKeyPressed(Key);
        }

        /// \brief Checks if a key is currently held down.
        ///
        /// \param Key The key to check.
        /// \return `true` if the key is currently pressed, otherwise `false`.
        ZYPHRYON_INLINE Bool IsKeyHeld(Key Key) const
        {
            return mKeyboard.IsKeyHeld(Key);
        }

        /// \brief Checks if a key was released in the current frame.
        ///
        /// \param Key The key to check.
        /// \return `true` if the key transitioned from pressed to released this frame, otherwise `false`.
        ZYPHRYON_INLINE Bool IsKeyReleased(Key Key) const
        {
            return mKeyboard.IsKeyReleased(Key);
        }

        /// \brief Gets the current X position of the mouse cursor.
        ///
        /// \return The X coordinate of the mouse cursor.
        ZYPHRYON_INLINE Real32 GetMouseX() const
        {
            return mMouse.GetX();
        }

        /// \brief Gets the current Y position of the mouse cursor.
        ///
        /// \return The Y coordinate of the mouse cursor.
        ZYPHRYON_INLINE Real32 GetMouseY() const
        {
            return mMouse.GetY();
        }

        /// \brief Gets the current scroll offset along the X axis.
        ///
        /// \return The scroll offset in the X direction.
        ZYPHRYON_INLINE Real32 GetMouseScrollX() const
        {
            return mMouse.GetScrollX();
        }

        /// \brief Gets the current scroll offset along the Y axis.
        ///
        /// \return The scroll offset in the Y direction.
        ZYPHRYON_INLINE Real32 GetMouseScrollY() const
        {
            return mMouse.GetScrollY();
        }

        /// \brief Checks if a mouse button was pressed in the current frame.
        ///
        /// \param Button The mouse button to check.
        /// \return `true` if the button transitioned from released to pressed this frame, otherwise `false`.
        ZYPHRYON_INLINE Bool IsMouseButtonPressed(Button Button) const
        {
            return mMouse.IsButtonPressed(Button);
        }

        /// \brief Checks if a mouse button is currently held down.
        ///
        /// \param Button The mouse button to check.
        /// \return `true` if the button is currently pressed, otherwise `false`.
        ZYPHRYON_INLINE Bool IsMouseButtonHeld(Button Button) const
        {
            return mMouse.IsButtonHeld(Button);
        }

        /// \brief Checks if a mouse button was released in the current frame.
        ///
        /// \param Button The mouse button to check.
        /// \return `true` if the button transitioned from pressed to released this frame, otherwise `false`.
        ZYPHRYON_INLINE Bool IsMouseButtonReleased(Button Button) const
        {
            return mMouse.IsButtonReleased(Button);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Poller   mPoller;
        Mouse    mMouse;
        Keyboard mKeyboard;
    };
}