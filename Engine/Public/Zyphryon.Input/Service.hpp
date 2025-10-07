// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
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

        /// \brief Event triggered when a character is typed (text input).
        MulticastDelegate<Bool(UInt32)>                         OnKeyType;

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

        /// \brief Updates all input devices and dispatches events to registered listeners.
        ///
        /// This method is called once per frame by the engine and is responsible for
        /// polling devices via \ref Poller and updating mouse and keyboard states.
        ///
        /// \param Time The time step data for the current frame.
        void OnTick(ConstRef<Time> Time) override;

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

        /// \brief Gets the current mouse position.
        ///
        /// \return The mouse position in window or screen coordinates.
        ZYPHRYON_INLINE Vector2 GetMousePosition() const
        {
            return mMouse.GetPosition();
        }

        /// \brief Gets the mouse scroll delta since the last frame.
        ///
        /// \return The mouse scroll delta vector.
        ZYPHRYON_INLINE Vector2 GetMouseScroll() const
        {
            return mMouse.GetScroll();
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

        /// \brief Internal event handler that dispatches events to the appropriate delegates.
        ///
        /// \param Event The event to handle.
        /// \return `true` if the event was handled and should not propagate further, otherwise
        ZYPHRYON_INLINE Bool Handle(ConstRef<Event> Event)
        {
            switch (Event.Kind)
            {
            case Event::Type::KeyType:
                return OnKeyType.Propagate(Event.KeyType.Codepoint);
            case Event::Type::KeyUp:
                return OnKeyUp.Propagate(Event.KeyAction.Key);
            case Event::Type::KeyDown:
                return OnKeyDown.Propagate(Event.KeyAction.Key);
            case Event::Type::MouseUp:
                return OnMouseUp.Propagate(Event.MouseAction.Button);
            case Event::Type::MouseDown:
                return OnMouseDown.Propagate(Event.MouseAction.Button);
            case Event::Type::MouseMove:
                return OnMouseMove.Propagate(Event.MouseAxis.X, Event.MouseAxis.Y, Event.MouseAxis.DeltaX, Event.MouseAxis.DeltaY);
            case Event::Type::MouseScroll:
                return OnMouseScroll.Propagate(Event.MouseAxis.DeltaX, Event.MouseAxis.DeltaY);
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

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Poller   mPoller;
        Mouse    mMouse;
        Keyboard mKeyboard;
    };
}