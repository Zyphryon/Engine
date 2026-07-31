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

#include <X11/Xlib.h>
#include <X11/keysym.h>

#ifdef Bool
#undef Bool
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Platform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Input::Key ConvertKeySymbol(KeySym Symbol)
    {
        switch (Symbol)
        {
        case XK_a:
        case XK_A:
            return Input::Key::A;
        case XK_b:
        case XK_B:
            return Input::Key::B;
        case XK_c:
        case XK_C:
            return Input::Key::C;
        case XK_d:
        case XK_D:
            return Input::Key::D;
        case XK_e:
        case XK_E:
            return Input::Key::E;
        case XK_f:
        case XK_F:
            return Input::Key::F;
        case XK_g:
        case XK_G:
            return Input::Key::G;
        case XK_h:
        case XK_H:
            return Input::Key::H;
        case XK_i:
        case XK_I:
            return Input::Key::I;
        case XK_j:
        case XK_J:
            return Input::Key::J;
        case XK_k:
        case XK_K:
            return Input::Key::K;
        case XK_l:
        case XK_L:
            return Input::Key::L;
        case XK_m:
        case XK_M:
            return Input::Key::M;
        case XK_n:
        case XK_N:
            return Input::Key::N;
        case XK_o:
        case XK_O:
            return Input::Key::O;
        case XK_p:
        case XK_P:
            return Input::Key::P;
        case XK_q:
        case XK_Q:
            return Input::Key::Q;
        case XK_r:
        case XK_R:
            return Input::Key::R;
        case XK_s:
        case XK_S:
            return Input::Key::S;
        case XK_t:
        case XK_T:
            return Input::Key::T;
        case XK_u:
        case XK_U:
            return Input::Key::U;
        case XK_v:
        case XK_V:
            return Input::Key::V;
        case XK_w:
        case XK_W:
            return Input::Key::W;
        case XK_x:
        case XK_X:
            return Input::Key::X;
        case XK_y:
        case XK_Y:
            return Input::Key::Y;
        case XK_z:
        case XK_Z:
            return Input::Key::Z;
        case XK_0:
            return Input::Key::Alpha0;
        case XK_1:
            return Input::Key::Alpha1;
        case XK_2:
            return Input::Key::Alpha2;
        case XK_3:
            return Input::Key::Alpha3;
        case XK_4:
            return Input::Key::Alpha4;
        case XK_5:
            return Input::Key::Alpha5;
        case XK_6:
            return Input::Key::Alpha6;
        case XK_7:
            return Input::Key::Alpha7;
        case XK_8:
            return Input::Key::Alpha8;
        case XK_9:
            return Input::Key::Alpha9;
        case XK_F1:
            return Input::Key::F1;
        case XK_F2:
            return Input::Key::F2;
        case XK_F3:
            return Input::Key::F3;
        case XK_F4:
            return Input::Key::F4;
        case XK_F5:
            return Input::Key::F5;
        case XK_F6:
            return Input::Key::F6;
        case XK_F7:
            return Input::Key::F7;
        case XK_F8:
            return Input::Key::F8;
        case XK_F9:
            return Input::Key::F9;
        case XK_F10:
            return Input::Key::F10;
        case XK_F11:
            return Input::Key::F11;
        case XK_F12:
            return Input::Key::F12;
        case XK_Left:
            return Input::Key::Left;
        case XK_Right:
            return Input::Key::Right;
        case XK_Up:
            return Input::Key::Up;
        case XK_Down:
            return Input::Key::Down;
        case XK_Home:
            return Input::Key::Home;
        case XK_End:
            return Input::Key::End;
        case XK_Prior:
            return Input::Key::PageUp;
        case XK_Next:
            return Input::Key::PageDown;
        case XK_Insert:
            return Input::Key::Insert;
        case XK_Delete:
            return Input::Key::Delete;
        case XK_space:
            return Input::Key::Space;
        case XK_Escape:
            return Input::Key::Escape;
        case XK_Return:
            return Input::Key::Enter;
        case XK_Tab:
            return Input::Key::Tab;
        case XK_BackSpace:
            return Input::Key::Backspace;
        case XK_Caps_Lock:
            return Input::Key::Capital;
        case XK_Scroll_Lock:
            return Input::Key::Scroll;
        case XK_Num_Lock:
            return Input::Key::NumLock;
        case XK_Print:
            return Input::Key::Print;
        case XK_Pause:
            return Input::Key::Pause;
        case XK_Shift_L:
            return Input::Key::LeftShift;
        case XK_Shift_R:
            return Input::Key::RightShift;
        case XK_Control_L:
            return Input::Key::LeftCtrl;
        case XK_Control_R:
            return Input::Key::RightCtrl;
        case XK_Alt_L:
            return Input::Key::LeftAlt;
        case XK_Alt_R:
            return Input::Key::RightAlt;
        case XK_Super_L:
            return Input::Key::LeftSuper;
        case XK_Super_R:
            return Input::Key::RightSuper;
        case XK_Menu:
            return Input::Key::Menu;
        case XK_semicolon:
            return Input::Key::Semicolon;
        case XK_slash:
            return Input::Key::Slash;
        case XK_grave:
            return Input::Key::Grave;
        case XK_bracketleft:
            return Input::Key::LeftBracket;
        case XK_backslash:
            return Input::Key::Backslash;
        case XK_bracketright:
            return Input::Key::RightBracket;
        case XK_apostrophe:
            return Input::Key::Apostrophe;
        case XK_comma:
            return Input::Key::Comma;
        case XK_minus:
            return Input::Key::Minus;
        case XK_period:
            return Input::Key::Period;
        case XK_equal:
            return Input::Key::Equal;
        case XK_KP_0:
        case XK_KP_Insert:
            return Input::Key::Keypad0;
        case XK_KP_1:
        case XK_KP_End:
            return Input::Key::Keypad1;
        case XK_KP_2:
        case XK_KP_Down:
            return Input::Key::Keypad2;
        case XK_KP_3:
        case XK_KP_Next:
            return Input::Key::Keypad3;
        case XK_KP_4:
        case XK_KP_Left:
            return Input::Key::Keypad4;
        case XK_KP_5:
        case XK_KP_Begin:
            return Input::Key::Keypad5;
        case XK_KP_6:
        case XK_KP_Right:
            return Input::Key::Keypad6;
        case XK_KP_7:
        case XK_KP_Home:
            return Input::Key::Keypad7;
        case XK_KP_8:
        case XK_KP_Up:
            return Input::Key::Keypad8;
        case XK_KP_9:
        case XK_KP_Prior:
            return Input::Key::Keypad9;
        case XK_KP_Decimal:
        case XK_KP_Delete:
            return Input::Key::Decimal;
        case XK_KP_Divide:
            return Input::Key::Divide;
        case XK_KP_Multiply:
            return Input::Key::Multiply;
        case XK_KP_Subtract:
            return Input::Key::Subtract;
        case XK_KP_Add:
            return Input::Key::Add;
        default:
            return Input::Key::Unknown;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Window::Backend
    {
        /// \brief The window geometry captured before entering fullscreen, so it can be restored afterwards.
        struct Snapshot
        {
            /// The cursor position observed by the previous motion event.
            SInt32 CursorX = 0;

            /// The cursor position observed by the previous motion event.
            SInt32 CursorY = 0;
        };

        /// The connection to the X server, owned for the lifetime of the window.
        Ptr<::Display> Connection = nullptr;

        /// The native X11 window resource.
        ::Window       Handle     = None;

        /// The input method used to translate key events into text.
        XIM            Method     = nullptr;

        /// The input context bound to this window.
        XIC            Context    = nullptr;

        /// The fully transparent cursor bound while the pointer is locked.
        ::Cursor       Blank      = None;

        /// The pixmap backing \ref Blank.
        Pixmap         Stipple    = None;

        /// The `WM_DELETE_WINDOW` atom, delivered as a client message when the frame's close button is used.
        Atom           Close      = None;

        /// The most recent cursor position, used to derive relative motion.
        Snapshot       Cursor;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZY_INLINE Atom GetAtom(ConstPtr<Char> Name) const
        {
            return ::XInternAtom(Connection, Name, 0);
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZY_INLINE void ApplyState(ConstPtr<Char> Name, Bool Enable)
        {
            XEvent Message { };
            Message.type                 = ClientMessage;
            Message.xclient.window       = Handle;
            Message.xclient.message_type = GetAtom("_NET_WM_STATE");
            Message.xclient.format       = 32;
            Message.xclient.data.l[0]    = Enable ? 1 : 0;  // _NET_WM_STATE_ADD / _NET_WM_STATE_REMOVE
            Message.xclient.data.l[1]    = GetAtom(Name);
            Message.xclient.data.l[2]    = 0;
            Message.xclient.data.l[3]    = 1;

            constexpr SInt Mask = SubstructureNotifyMask | SubstructureRedirectMask;
            ::XSendEvent(Connection, ::XDefaultRootWindow(Connection), 0, Mask, AddressOf(Message));
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZY_INLINE void ApplyCursorLock(Bool Lock)
        {
            if (Lock)
            {
                constexpr UInt Mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

                ::XDefineCursor(Connection, Handle, Blank);
                ::XGrabPointer(
                    Connection, Handle, 0, Mask, GrabModeAsync, GrabModeAsync, Handle, Blank, CurrentTime);
            }
            else
            {
                ::XUngrabPointer(Connection, CurrentTime);
                ::XUndefineCursor(Connection, Handle);
            }
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        void OnWindowEvent(Ptr<Window> Window, Ref<XEvent> Event)
        {
            Ref<Dispatcher> Dispatcher = Window->mDispatcher;

            switch (Event.type)
            {
            case ConfigureNotify:
            {
                Window->mX = Event.xconfigure.x;
                Window->mY = Event.xconfigure.y;

                const UInt32 Width  = static_cast<UInt32>(Event.xconfigure.width);
                const UInt32 Height = static_cast<UInt32>(Event.xconfigure.height);

                if (Width != Window->mWidth || Height != Window->mHeight)
                {
                    Window->mWidth  = Width;
                    Window->mHeight = Height;

                    Dispatcher.QueueWindowResize(Width, Height);
                }
                break;
            }
            case MapNotify:
            {
                Window->mStates = SetBit(Window->mStates, State::Visible);
                break;
            }
            case UnmapNotify:
            {
                Window->mStates = ClearBit(Window->mStates, State::Visible);
                break;
            }
            case FocusIn:
            {
                Window->mStates = SetBit(Window->mStates, State::Focused);
                Dispatcher.QueueWindowFocus(true);

                if (Context)
                {
                    ::XSetICFocus(Context);
                }

                if (Window->IsCursorLocked())
                {
                    ApplyCursorLock(true);
                }
                break;
            }
            case FocusOut:
            {
                Window->mStates = ClearBit(Window->mStates, State::Focused);
                Dispatcher.QueueWindowFocus(false);

                if (Context)
                {
                    ::XUnsetICFocus(Context);
                }

                if (Window->IsCursorLocked())
                {
                    ApplyCursorLock(false);
                }
                break;
            }
            case ClientMessage:
            {
                if (static_cast<Atom>(Event.xclient.data.l[0]) == Close)
                {
                    Dispatcher.QueueWindowExit();
                }
                break;
            }
            case KeyPress:
            {
                const KeySym Symbol = ::XLookupKeysym(AddressOf(Event.xkey), 0);

                if (const Input::Key Key = ConvertKeySymbol(Symbol); Key != Input::Key::Unknown)
                {
                    Dispatcher.QueueKeyDown(Key);
                }

                // The input method turns the keystroke into text, which is what composed and dead keys need.
                if (Context)
                {
                    Array<Char, 32> Buffer { };
                    SInt32          Result = 0;

                    const SInt32 Length = ::Xutf8LookupString(
                        Context,
                        AddressOf(Event.xkey),
                        Buffer.GetData(),
                        Buffer.GetSize() - 1,
                        nullptr,
                        AddressOf(Result));

                    if (Length > 0 && (Result == XLookupChars || Result == XLookupBoth))
                    {
                        Dispatcher.QueueKeyType(Text(Buffer.GetData(), static_cast<UInt>(Length)));
                    }
                }
                break;
            }
            case KeyRelease:
            {
                const KeySym Symbol = ::XLookupKeysym(AddressOf(Event.xkey), 0);

                if (const Input::Key Key = ConvertKeySymbol(Symbol); Key != Input::Key::Unknown)
                {
                    Dispatcher.QueueKeyUp(Key);
                }
                break;
            }
            case ButtonPress:
            {
                switch (Event.xbutton.button)
                {
                case Button1:
                    Dispatcher.QueueMouseButtonDown(Input::Button::Left);
                    break;
                case Button2:
                    Dispatcher.QueueMouseButtonDown(Input::Button::Middle);
                    break;
                case Button3:
                    Dispatcher.QueueMouseButtonDown(Input::Button::Right);
                    break;
                case Button4:
                    Dispatcher.QueueMouseScroll(0.0f, 1.0f);
                    break;
                case Button5:
                    Dispatcher.QueueMouseScroll(0.0f, -1.0f);
                    break;
                case 6:
                    Dispatcher.QueueMouseScroll(-1.0f, 0.0f);
                    break;
                case 7:
                    Dispatcher.QueueMouseScroll(1.0f, 0.0f);
                    break;
                case 8:
                    Dispatcher.QueueMouseButtonDown(Input::Button::Back);
                    break;
                case 9:
                    Dispatcher.QueueMouseButtonDown(Input::Button::Forward);
                    break;
                default:
                    break;
                }
                break;
            }
            case ButtonRelease:
            {
                switch (Event.xbutton.button)
                {
                case Button1:
                    Dispatcher.QueueMouseButtonUp(Input::Button::Left);
                    break;
                case Button2:
                    Dispatcher.QueueMouseButtonUp(Input::Button::Middle);
                    break;
                case Button3:
                    Dispatcher.QueueMouseButtonUp(Input::Button::Right);
                    break;
                case 8:
                    Dispatcher.QueueMouseButtonUp(Input::Button::Back);
                    break;
                case 9:
                    Dispatcher.QueueMouseButtonUp(Input::Button::Forward);
                    break;
                default:
                    break;
                }
                break;
            }
            case MotionNotify:
            {
                const SInt32 X = Event.xmotion.x;
                const SInt32 Y = Event.xmotion.y;

                const SInt32 DeltaX = X - Cursor.CursorX;
                const SInt32 DeltaY = Y - Cursor.CursorY;

                Cursor.CursorX = X;
                Cursor.CursorY = Y;

                if (Window->IsCursorLocked())
                {
                    if (DeltaX != 0 || DeltaY != 0)
                    {
                        Dispatcher.QueueMouseMove(0.0f, 0.0f, DeltaX, DeltaY);
                    }
                }
                else
                {
                    Dispatcher.QueueMouseMove(X, Y, DeltaX, DeltaY);
                }
                break;
            }
            default:
                break;
            }
        }
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Window::~Window()
    {
        if (!mBackend || !mBackend->Connection)
        {
            return;
        }

        if (mBackend->Context)
        {
            ::XDestroyIC(mBackend->Context);
        }
        if (mBackend->Method)
        {
            ::XCloseIM(mBackend->Method);
        }
        if (mBackend->Blank)
        {
            ::XFreeCursor(mBackend->Connection, mBackend->Blank);
        }
        if (mBackend->Stipple)
        {
            ::XFreePixmap(mBackend->Connection, mBackend->Stipple);
        }
        if (mBackend->Handle)
        {
            ::XDestroyWindow(mBackend->Connection, mBackend->Handle);
        }
        ::XCloseDisplay(mBackend->Connection);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Window::Handle Window::GetHandle() const
    {
        return reinterpret_cast<Window::Handle>(mBackend->Handle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::Poll()
    {
        while (::XPending(mBackend->Connection) > 0)
        {
            XEvent Event;
            ::XNextEvent(mBackend->Connection, AddressOf(Event));

            if (::XFilterEvent(AddressOf(Event), None))
            {
                continue;
            }
            mBackend->OnWindowEvent(this, Event);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetTitle(Text Title)
    {
        mTitle = Title;

        const Atom Property = mBackend->GetAtom("_NET_WM_NAME");
        const Atom Encoding = mBackend->GetAtom("UTF8_STRING");

        ::XChangeProperty(
            mBackend->Connection, mBackend->Handle, Property, Encoding, 8, PropModeReplace,
            reinterpret_cast<ConstPtr<Byte>>(mTitle.GetData()), static_cast<SInt32>(mTitle.GetSize()));

        ::XStoreName(mBackend->Connection, mBackend->Handle, mTitle.GetData());
        ::XFlush(mBackend->Connection);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetPosition(SInt32 X, SInt32 Y)
    {
        ::XMoveWindow(mBackend->Connection, mBackend->Handle, X, Y);
        ::XFlush(mBackend->Connection);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetVisible(Bool Visible)
    {
        if (Visible)
        {
            ::XMapRaised(mBackend->Connection, mBackend->Handle);
        }
        else
        {
            ::XUnmapWindow(mBackend->Connection, mBackend->Handle);
        }
        ::XFlush(mBackend->Connection);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetFocus()
    {
        ::XRaiseWindow(mBackend->Connection, mBackend->Handle);
        ::XSetInputFocus(mBackend->Connection, mBackend->Handle, RevertToParent, CurrentTime);
        ::XFlush(mBackend->Connection);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetFullscreen(Bool Fullscreen)
    {
        if (Fullscreen == IsFullscreen())
        {
            return;
        }

        mBackend->ApplyState("_NET_WM_STATE_FULLSCREEN", Fullscreen);
        ::XFlush(mBackend->Connection);

        mStates = SetOrClearBit(mStates, State::Fullscreen, Fullscreen);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetSize(UInt32 Width, UInt32 Height)
    {
        ::XResizeWindow(mBackend->Connection, mBackend->Handle, Width, Height);
        ::XFlush(mBackend->Connection);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetCursorPosition(UInt32 X, UInt32 Y)
    {
        ::XWarpPointer(mBackend->Connection, 0, mBackend->Handle, 0, 0, 0, 0, X, Y);
        ::XFlush(mBackend->Connection);

        mBackend->Cursor.CursorX = X;
        mBackend->Cursor.CursorY = Y;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetCursorLock(Bool State)
    {
        mBackend->ApplyCursorLock(State);
        ::XFlush(mBackend->Connection);

        mStates = SetOrClearBit(mStates, State::Locked, State);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Window::Initialize(Text Title, SInt32 X, SInt32 Y, UInt32 Width, UInt32 Height, Bool Borderless, Bool Fullscreen)
    {
        mBackend = Unique<Backend>::Create();

        mBackend->Connection = ::XOpenDisplay(nullptr);

        if (!mBackend->Connection)
        {
            LOG_E("Platform: Failed to open a connection to the X server (is DISPLAY set?)");
            return false;
        }

        const SInt32   Screen = ::XDefaultScreen(mBackend->Connection);
        const ::Window Root   = ::XRootWindow(mBackend->Connection, Screen);

        XSetWindowAttributes Attributes { };
        Attributes.background_pixel = ::XBlackPixel(mBackend->Connection, Screen);
        Attributes.event_mask       =
            KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
            StructureNotifyMask | FocusChangeMask | ExposureMask;

        mBackend->Handle = ::XCreateWindow(
            mBackend->Connection, Root, X, Y, Width, Height, 0,
            CopyFromParent, InputOutput, CopyFromParent, CWBackPixel | CWEventMask, AddressOf(Attributes));

        if (!mBackend->Handle)
        {
            LOG_E("Platform: Failed to create the X11 window");
            return false;
        }

        // Ask for the close button to arrive as a client message instead of severing the connection.
        mBackend->Close = mBackend->GetAtom("WM_DELETE_WINDOW");
        ::XSetWMProtocols(mBackend->Connection, mBackend->Handle, AddressOf(mBackend->Close), 1);

        // A fully transparent 1x1 cursor, bound while the pointer is locked.
        XColor Color { };
        mBackend->Stipple = ::XCreatePixmap(mBackend->Connection, mBackend->Handle, 1, 1, 1);
        mBackend->Blank   = ::XCreatePixmapCursor(
            mBackend->Connection, mBackend->Stipple, mBackend->Stipple, AddressOf(Color), AddressOf(Color), 0, 0);

        // The input method turns keystrokes into text; without one, only the raw key events are reported.
        if (mBackend->Method = ::XOpenIM(mBackend->Connection, nullptr, nullptr, nullptr); mBackend->Method)
        {
            mBackend->Context = ::XCreateIC(
                mBackend->Method,
                XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                XNClientWindow, mBackend->Handle,
                nullptr);
        }

        if (Borderless)
        {
            Array<UInt, 5> Hints;
            Hints[0] = 1 << 1; // MWM_HINTS_DECORATIONS

            const Atom Property = mBackend->GetAtom("_MOTIF_WM_HINTS");
            ::XChangeProperty(
                mBackend->Connection, mBackend->Handle, Property, Property, 32, PropModeReplace,
                reinterpret_cast<ConstPtr<Byte>>(Hints.GetData()), 5);
        }

        mTitle  = Title;
        mX      = X;
        mY      = Y;
        mWidth  = Width;
        mHeight = Height;
        mStates = SetOrClearBit(mStates, State::Borderless, Borderless);

        SetTitle(Title);
        SetVisible(true);

        if (Fullscreen)
        {
            SetFullscreen(true);
        }

        ::XFlush(mBackend->Connection);
        return true;
    }
}