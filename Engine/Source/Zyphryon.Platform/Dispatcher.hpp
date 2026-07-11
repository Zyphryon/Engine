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

#include "Zyphryon.Input/Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Platform
{
    /// \brief Collects and queues platform input events for processing by the input service.
    class Dispatcher final
    {
    public:

        /// \brief Represents the types of notifications from the system that requires attention.
        enum class Notification : UInt8
        {
            None,       ///< No notifications.
            Monitor,    ///< Monitor-related notifications.
        };
        ZY_DEFINE_BITWISE_FRIEND_ENUM(Notification)

        /// \brief The maximum number of input events stored in a single frame.
        static constexpr UInt32 kMaxInputEvents = 32;

        /// \brief The maximum size of text input captured in a single frame.
        static constexpr UInt32 kMaxInputText   = 256;

    public:

        /// \brief Resets the dispatcher queues.
        ZY_INLINE void Reset()
        {
            mNotifications = Notification::None;

            mInputEvents.Clear();
            mInputText.Clear();
        }

        /// \brief Notify that a specific notification has occurred.
        ///
        /// \param Notification The notification type to notify.
        ZY_INLINE void Notify(Notification Notification)
        {
            mNotifications = SetBit(mNotifications, Notification);
        }

        /// \brief Checks if a specific notification has been notified.
        ///
        /// \param Notification The notification type to check.
        /// \return `true` if the notification has been notified, `false` otherwise.
        ZY_INLINE Bool IsNotified(Notification Notification) const
        {
            return HasBit(mNotifications, Notification);
        }

        /// \brief Queues a text input event.
        ///
        /// \param Content The text content entered by the user.
        ZY_INLINE void QueueKeyType(Text Content)
        {
            const UInt Offset = mInputText.GetSize();
            mInputText.Append(Content);

            Ref<Input::Event> Event = mInputEvents.Append(Input::Event::Type::KeyType);
            Event.KeyType.Content = Text(mInputText.GetData() + Offset, Content.GetSize());
        }

        /// \brief Queues a key release event.
        ///
        /// \param Key The physical key that was released.
        ZY_INLINE void QueueKeyUp(Input::Key Key)
        {
            Ref<Input::Event> Event = mInputEvents.Append(Input::Event::Type::KeyUp);
            Event.KeyAction.Key = Key;
        }

        /// \brief Queues a key press event.
        ///
        /// \param Key The physical key that was pressed.
        ZY_INLINE void QueueKeyDown(Input::Key Key)
        {
            Ref<Input::Event> Event = mInputEvents.Append(Input::Event::Type::KeyDown);
            Event.KeyAction.Key = Key;
        }

        /// \brief Queues a mouse movement event.
        ///
        /// \param X         The absolute X position of the mouse cursor.
        /// \param Y         The absolute Y position of the mouse cursor.
        /// \param RelativeX The relative X movement since the last event.
        /// \param RelativeY The relative Y movement since the last event.
        ZY_INLINE void QueueMouseMove(Real32 X, Real32 Y, Real32 RelativeX, Real32 RelativeY)
        {
            Ref<Input::Event> Event = mInputEvents.Append(Input::Event::Type::MouseMove);
            Event.MouseAxis.X      = X;
            Event.MouseAxis.Y      = Y;
            Event.MouseAxis.DeltaX = RelativeX;
            Event.MouseAxis.DeltaY = RelativeY;
        }

        /// \brief Queues a mouse scroll event.
        ///
        /// \param DeltaX The horizontal scroll delta.
        /// \param DeltaY The vertical scroll delta.
        ZY_INLINE void QueueMouseScroll(Real32 DeltaX, Real32 DeltaY)
        {
            Ref<Input::Event> Event = mInputEvents.Append(Input::Event::Type::MouseScroll);
            Event.MouseScroll.DeltaX = DeltaX;
            Event.MouseScroll.DeltaY = DeltaY;
        }

        /// \brief Queues a mouse button release event.
        ///
        /// \param Button The mouse button that was released.
        ZY_INLINE void QueueMouseButtonUp(Input::Button Button)
        {
            Ref<Input::Event> Event = mInputEvents.Append(Input::Event::Type::MouseUp);
            Event.MouseAction.Button = Button;
        }

        /// \brief Queues a mouse button press event.
        ///
        /// \param Button The mouse button that was pressed.
        ZY_INLINE void QueueMouseButtonDown(Input::Button Button)
        {
            Ref<Input::Event> Event = mInputEvents.Append(Input::Event::Type::MouseDown);
            Event.MouseAction.Button = Button;
        }

        /// \brief Queues a window focus event.
        ///
        /// \param Focused The focus state (true if gained, false if lost).
        ZY_INLINE void QueueWindowFocus(Bool Focused)
        {
            Ref<Input::Event> Event = mInputEvents.Append(Input::Event::Type::WindowFocus);
            Event.WindowFocus.State = Focused;
        }

        /// \brief Queues a window resize event.
        ///
        /// \param Width  The new window width in pixels.
        /// \param Height The new window height in pixels.
        ZY_INLINE void QueueWindowResize(UInt16 Width, UInt16 Height)
        {
            Ref<Input::Event> Event = mInputEvents.Append(Input::Event::Type::WindowResize);
            Event.WindowResize.Width  = Width;
            Event.WindowResize.Height = Height;
        }

        /// \brief Queues a window close request event.
        ZY_INLINE void QueueWindowExit()
        {
            mInputEvents.Append(Input::Event::Type::WindowExit);
        }

        /// \brief Gets the queued input events.
        ///
        /// \return The span of collected events from the current frame.
        ZY_INLINE ConstSpan<Input::Event> GetInputEvents() const
        {
            return mInputEvents;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Notification                            mNotifications;
        Sequence<Input::Event, kMaxInputEvents> mInputEvents;
        String<kMaxInputText>                   mInputText;
    };
}