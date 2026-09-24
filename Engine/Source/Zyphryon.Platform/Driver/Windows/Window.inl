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

#include <windows.h>
#include <windowsx.h>
#include <shellscalingapi.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyPlatform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static ZyInput::Key ConvertVirtualKey(UInt32 Key, LPARAM Flags)
    {
        const UINT Scancode = (Flags >> 16) & 0xFF;
        const Bool Extended = (HIWORD(Flags) & KF_EXTENDED) != 0;

        switch (Key)
        {
        case 'A':
            return ZyInput::Key::A;
        case 'B':
            return ZyInput::Key::B;
        case 'C':
            return ZyInput::Key::C;
        case 'D':
            return ZyInput::Key::D;
        case 'E':
            return ZyInput::Key::E;
        case 'F':
            return ZyInput::Key::F;
        case 'G':
            return ZyInput::Key::G;
        case 'H':
            return ZyInput::Key::H;
        case 'I':
            return ZyInput::Key::I;
        case 'J':
            return ZyInput::Key::J;
        case 'K':
            return ZyInput::Key::K;
        case 'L':
            return ZyInput::Key::L;
        case 'M':
            return ZyInput::Key::M;
        case 'N':
            return ZyInput::Key::N;
        case 'O':
            return ZyInput::Key::O;
        case 'P':
            return ZyInput::Key::P;
        case 'Q':
            return ZyInput::Key::Q;
        case 'R':
            return ZyInput::Key::R;
        case 'S':
            return ZyInput::Key::S;
        case 'T':
            return ZyInput::Key::T;
        case 'U':
            return ZyInput::Key::U;
        case 'V':
            return ZyInput::Key::V;
        case 'W':
            return ZyInput::Key::W;
        case 'X':
            return ZyInput::Key::X;
        case 'Y':
            return ZyInput::Key::Y;
        case 'Z':
            return ZyInput::Key::Z;
        case '0':
            return ZyInput::Key::Alpha0;
        case '1':
            return ZyInput::Key::Alpha1;
        case '2':
            return ZyInput::Key::Alpha2;
        case '3':
            return ZyInput::Key::Alpha3;
        case '4':
            return ZyInput::Key::Alpha4;
        case '5':
            return ZyInput::Key::Alpha5;
        case '6':
            return ZyInput::Key::Alpha6;
        case '7':
            return ZyInput::Key::Alpha7;
        case '8':
            return ZyInput::Key::Alpha8;
        case '9':
            return ZyInput::Key::Alpha9;
        case VK_F1:
            return ZyInput::Key::F1;
        case VK_F2:
            return ZyInput::Key::F2;
        case VK_F3:
            return ZyInput::Key::F3;
        case VK_F4:
            return ZyInput::Key::F4;
        case VK_F5:
            return ZyInput::Key::F5;
        case VK_F6:
            return ZyInput::Key::F6;
        case VK_F7:
            return ZyInput::Key::F7;
        case VK_F8:
            return ZyInput::Key::F8;
        case VK_F9:
            return ZyInput::Key::F9;
        case VK_F10:
            return ZyInput::Key::F10;
        case VK_F11:
            return ZyInput::Key::F11;
        case VK_F12:
            return ZyInput::Key::F12;
        case VK_LEFT:
            return ZyInput::Key::Left;
        case VK_RIGHT:
            return ZyInput::Key::Right;
        case VK_UP:
            return ZyInput::Key::Up;
        case VK_DOWN:
            return ZyInput::Key::Down;
        case VK_HOME:
            return ZyInput::Key::Home;
        case VK_END:
            return ZyInput::Key::End;
        case VK_PRIOR:
            return ZyInput::Key::PageUp;
        case VK_NEXT:
            return ZyInput::Key::PageDown;
        case VK_INSERT:
            return ZyInput::Key::Insert;
        case VK_DELETE:
            return ZyInput::Key::Delete;
        case VK_SPACE:
            return ZyInput::Key::Space;
        case VK_ESCAPE:
            return ZyInput::Key::Escape;
        case VK_RETURN:
            return ZyInput::Key::Enter;
        case VK_TAB:
            return ZyInput::Key::Tab;
        case VK_BACK:
            return ZyInput::Key::Backspace;
        case VK_CAPITAL:
            return ZyInput::Key::Capital;
        case VK_SCROLL:
            return ZyInput::Key::Scroll;
        case VK_NUMLOCK:
            return ZyInput::Key::NumLock;
        case VK_SNAPSHOT:
            return ZyInput::Key::Print;
        case VK_PAUSE:
            return ZyInput::Key::Pause;
        case VK_SHIFT:
            if (::MapVirtualKeyW(Scancode, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT)
            {
                return ZyInput::Key::RightShift;
            }
            else
            {
                return ZyInput::Key::LeftShift;
            }
        case VK_CONTROL:
            return Extended ? ZyInput::Key::RightCtrl : ZyInput::Key::LeftCtrl;
        case VK_MENU:
            return Extended ? ZyInput::Key::RightAlt : ZyInput::Key::LeftAlt;
        case VK_LWIN:
            return ZyInput::Key::LeftSuper;
        case VK_RWIN:
            return ZyInput::Key::RightSuper;
        case VK_APPS:
            return ZyInput::Key::Menu;
        case VK_OEM_1:
            return ZyInput::Key::Semicolon;
        case VK_OEM_2:
            return ZyInput::Key::Slash;
        case VK_OEM_3:
            return ZyInput::Key::Grave;
        case VK_OEM_4:
            return ZyInput::Key::LeftBracket;
        case VK_OEM_5:
            return ZyInput::Key::Backslash;
        case VK_OEM_6:
            return ZyInput::Key::RightBracket;
        case VK_OEM_7:
            return ZyInput::Key::Apostrophe;
        case VK_OEM_COMMA:
            return ZyInput::Key::Comma;
        case VK_OEM_MINUS:
            return ZyInput::Key::Minus;
        case VK_OEM_PERIOD:
            return ZyInput::Key::Period;
        case VK_OEM_PLUS:
            return ZyInput::Key::Equal;
        case VK_NUMPAD0:
            return ZyInput::Key::Keypad0;
        case VK_NUMPAD1:
            return ZyInput::Key::Keypad1;
        case VK_NUMPAD2:
            return ZyInput::Key::Keypad2;
        case VK_NUMPAD3:
            return ZyInput::Key::Keypad3;
        case VK_NUMPAD4:
            return ZyInput::Key::Keypad4;
        case VK_NUMPAD5:
            return ZyInput::Key::Keypad5;
        case VK_NUMPAD6:
            return ZyInput::Key::Keypad6;
        case VK_NUMPAD7:
            return ZyInput::Key::Keypad7;
        case VK_NUMPAD8:
            return ZyInput::Key::Keypad8;
        case VK_NUMPAD9:
            return ZyInput::Key::Keypad9;
        case VK_DECIMAL:
            return ZyInput::Key::Decimal;
        case VK_DIVIDE:
            return ZyInput::Key::Divide;
        case VK_MULTIPLY:
            return ZyInput::Key::Multiply;
        case VK_SUBTRACT:
            return ZyInput::Key::Subtract;
        case VK_ADD:
            return ZyInput::Key::Add;
        default:
            return ZyInput::Key::Unknown;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static DWORD ConvertStyle(Bool Fullscreen, Bool Borderless)
    {
        DWORD Style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

        if (Fullscreen)
        {
            Style |= WS_POPUP;
        }
        else
        {
            Style |= WS_SYSMENU | WS_MINIMIZEBOX;

            if (Borderless)
            {
                Style |= WS_POPUP;
            }
            else
            {
                Style |= WS_CAPTION;
                Style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
            }
        }
        return Style;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Window::Backend
    {
        /// The native Win32 window handle (HWND) associated with the window.
        HWND  Handle    = nullptr;

        /// The cursor position seen by the last motion message, which is also where the cursor returns when unlocked.
        POINT Cursor    = { };

        /// The window area (position and size) before entering fullscreen.
        RECT  Region    = { };

        /// The high half of a surrogate pair, held until its low half arrives in the next `WM_CHAR`.
        WCHAR Surrogate = L'\0';

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        void ApplyCursorLock(Bool Lock)
        {
            if (Lock)
            {
                RECT Rect;
                ::GetClientRect(Handle, AddressOf(Rect));
                ::ClientToScreen(Handle, reinterpret_cast<Ptr<POINT>>(AddressOf(Rect.left)));
                ::ClientToScreen(Handle, reinterpret_cast<Ptr<POINT>>(AddressOf(Rect.right)));
                ::ClipCursor(AddressOf(Rect));
                ::ShowCursor(FALSE);
            }
            else
            {
                POINT Point = Cursor;
                ::ClientToScreen(Handle, AddressOf(Point));
                ::SetCursorPos(Point.x, Point.y);

                ::ClipCursor(nullptr);
                ::ShowCursor(TRUE);
            }
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZY_INLINE LRESULT OnWindowMessage(Ptr<Window> Window, UINT Message, WPARAM Arg0, LPARAM Arg1)
        {
            Ref<Dispatcher> Dispatcher = Window->mDispatcher;
            
            switch (Message)
            {
            case WM_DISPLAYCHANGE:
            {
                Dispatcher.Notify(ZyPlatform::Dispatcher::Notification::Monitor);
                break;
            }
            case WM_DEVICECHANGE:
            {
                switch (Arg0)
                {
                case 0x8000: // DBT_DEVICEARRIVAL
                case 0x8004: // DBT_DEVICEREMOVECOMPLETE
                    Dispatcher.Notify(ZyPlatform::Dispatcher::Notification::Monitor);
                    break;
                default:
                    break;
                }
                return TRUE;
            }
            case WM_WINDOWPOSCHANGED:
            {
                const ConstPtr<WINDOWPOS> Parameter = reinterpret_cast<ConstPtr<WINDOWPOS>>(Arg1);

                Window->mX      = Parameter->x;
                Window->mY      = Parameter->y;
                return ::DefWindowProcW(Handle, Message, Arg0, Arg1);
            }
            case WM_SETFOCUS:
            {
                Window->mStates = SetBit(Window->mStates, State::Focused);
                Dispatcher.QueueWindowFocus(true);

                if (Window->IsCursorLocked())
                {
                    ApplyCursorLock(true);
                }
                break;
            }
            case WM_KILLFOCUS:
            {
                Window->mStates = ClearBit(Window->mStates, State::Focused);
                Dispatcher.QueueWindowFocus(false);

                if (Window->IsCursorLocked())
                {
                    ApplyCursorLock(false);
                }
                break;
            }
            case WM_SIZE:
            {
                Window->mWidth  = LOWORD(Arg1);
                Window->mHeight = HIWORD(Arg1);

                if (Arg0 == SIZE_MINIMIZED)
                {
                    Window->mStates = ClearBit(Window->mStates, State::Visible);
                }
                else if (Arg0 == SIZE_MAXIMIZED || Arg0 == SIZE_RESTORED)
                {
                    Window->mStates = SetBit(Window->mStates, State::Visible);

                    if (!HasBit(Window->mStates, State::Held))
                    {
                        Dispatcher.QueueWindowResize(Window->mWidth, Window->mHeight);
                    }
                }
                break;
            }
            case WM_EXITSIZEMOVE:
            {
                if (HasBit(Window->mStates, State::Held))
                {
                    Window->mStates = ClearBit(Window->mStates, State::Held);

                    Dispatcher.QueueWindowResize(Window->mWidth, Window->mHeight);
                }
                break;
            }
            case WM_SYSCOMMAND:
            {
                if ((Arg0 & 0xFFF0) == SC_SIZE)
                {
                    Window->mStates = SetBit(Window->mStates, State::Held);
                }
                return ::DefWindowProcW(Handle, Message, Arg0, Arg1);
            }
            case WM_SHOWWINDOW:
            {
                Window->mStates = SetOrClearBit(Window->mStates, State::Visible, static_cast<BOOL>(Arg0) == TRUE);
                break;
            }
            case WM_DPICHANGED:
            {
                const auto [Left, Top, Right, Bottom] = *reinterpret_cast<ConstPtr<RECT>>(Arg1);
                ::SetWindowPos(Handle, nullptr, Left, Top, Right - Left, Bottom - Top, SWP_NOZORDER | SWP_NOACTIVATE);
                break;
            }
            case WM_CLOSE:
            {
                Dispatcher.QueueWindowExit();
                break;
            }
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            {
                if ((HIWORD(Arg1) & KF_REPEAT) == 0)
                {
                    const ZyInput::Key Key = ConvertVirtualKey(static_cast<UInt32>(Arg0), Arg1);

                    if (Key != ZyInput::Key::Unknown)
                    {
                        Dispatcher.QueueKeyDown(Key);
                    }
                }
                return ::DefWindowProcW(Handle, Message, Arg0, Arg1);
            }
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                const ZyInput::Key Key = ConvertVirtualKey(static_cast<UInt32>(Arg0), Arg1);

                if (Key != ZyInput::Key::Unknown)
                {
                    Dispatcher.QueueKeyUp(Key);
                }
                return ::DefWindowProcW(Handle, Message, Arg0, Arg1);
            }
            case WM_LBUTTONDOWN:
            {
                Dispatcher.QueueMouseButtonDown(ZyInput::Button::Left);
                break;
            }
            case WM_LBUTTONUP:
            {
                Dispatcher.QueueMouseButtonUp(ZyInput::Button::Left);
                break;
            }
            case WM_MBUTTONDOWN:
            {
                Dispatcher.QueueMouseButtonDown(ZyInput::Button::Middle);
                break;
            }
            case WM_MBUTTONUP:
            {
                Dispatcher.QueueMouseButtonUp(ZyInput::Button::Middle);
                break;
            }
            case WM_RBUTTONDOWN:
            {
                Dispatcher.QueueMouseButtonDown(ZyInput::Button::Right);
                break;
            }
            case WM_RBUTTONUP:
            {
                Dispatcher.QueueMouseButtonUp(ZyInput::Button::Right);
                break;
            }
            case WM_XBUTTONDOWN:
            {
                const ZyInput::Button Button = (GET_XBUTTON_WPARAM(Arg0) == XBUTTON1)
                    ? ZyInput::Button::Back
                    : ZyInput::Button::Forward;
                Dispatcher.QueueMouseButtonDown(Button);
                break;
            }
            case WM_XBUTTONUP:
            {
                const ZyInput::Button Button = (GET_XBUTTON_WPARAM(Arg0) == XBUTTON1)
                    ? ZyInput::Button::Back
                    : ZyInput::Button::Forward;
                Dispatcher.QueueMouseButtonUp(Button);
                break;
            }
            case WM_MOUSEWHEEL:
            {
                const Real32 Delta = static_cast<Real32>(GET_WHEEL_DELTA_WPARAM(Arg0)) / WHEEL_DELTA;
                Dispatcher.QueueMouseScroll(0.0f, Delta);
                break;
            }
            case WM_MOUSEHWHEEL:
            {
                const Real32 Delta = static_cast<Real32>(GET_WHEEL_DELTA_WPARAM(Arg0)) / WHEEL_DELTA;
                Dispatcher.QueueMouseScroll(Delta, 0.0f);
                break;
            }
            case WM_ERASEBKGND:
            {
                return TRUE;
            }
            case WM_MOUSEMOVE:
            {
                if (!Window->IsCursorLocked())
                {
                    const UInt32 X = GET_X_LPARAM(Arg1);
                    const UInt32 Y = GET_Y_LPARAM(Arg1);
                    const SInt32 DeltaX = X - Cursor.x;
                    const SInt32 DeltaY = Y - Cursor.y;

                    Cursor.x = X;
                    Cursor.y = Y;
                    Dispatcher.QueueMouseMove(X, Y, DeltaX, DeltaY);
                }
                break;
            }
            case WM_INPUT:
            {
                UINT Size = sizeof(RAWINPUT);

                RAWINPUT Data;
                ::GetRawInputData(reinterpret_cast<HRAWINPUT>(Arg1), RID_INPUT, AddressOf(Data), AddressOf(Size), sizeof(RAWINPUTHEADER));

                if (Data.header.dwType == RIM_TYPEMOUSE && Window->IsCursorLocked())
                {
                    const Real32 DeltaX = static_cast<Real32>(Data.data.mouse.lLastX);
                    const Real32 DeltaY = static_cast<Real32>(Data.data.mouse.lLastY);

                    if (DeltaX != 0 || DeltaY != 0)
                    {
                        Dispatcher.QueueMouseMove(0.0f, 0.0f, DeltaX, DeltaY);
                    }
                }
                break;
            }
            case WM_CHAR:
            {
                if (IS_HIGH_SURROGATE(Arg0))
                {
                    Surrogate = static_cast<WCHAR>(Arg0);
                }
                else
                {
                    const WCHAR High = Exchange(Surrogate, 0);

                    Str16 Buffer;

                    if (IS_LOW_SURROGATE(Arg0) && High)
                    {
                        Buffer.AppendCodepoint(((High - 0xD800) << 10) + (Arg0 - 0xDC00) + 0x10000);
                    }
                    else if (Arg0 >= 0x20 || Arg0 == '\t' || Arg0 == '\r' || Arg0 == '\n')
                    {
                        Buffer.AppendCodepoint(Arg0);
                    }
                    else
                    {
                        break;
                    }
                    Dispatcher.QueueKeyType(Buffer);
                }
                break;
            }
            default:
                return ::DefWindowProcW(Handle, Message, Arg0, Arg1);
            }
            return 0;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static LRESULT CALLBACK OnWindowDispatcher(HWND Handle, UINT Message, WPARAM Arg0, LPARAM Arg1)
        {
            Ptr<Window> Instance = nullptr;

            if (Message == WM_NCCREATE)
            {
                const ConstPtr<CREATESTRUCTW> lpMessage = reinterpret_cast<ConstPtr<CREATESTRUCTW>>(Arg1);

                Instance = static_cast<Ptr<Window>>(lpMessage->lpCreateParams);
                Instance->mBackend->Handle = Handle;

                SetWindowLongPtrW(Handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Instance));
            }
            else
            {
                Instance = reinterpret_cast<Ptr<Window>>(GetWindowLongPtrW(Handle, GWLP_USERDATA));
            }
            if (Instance)
            {
                return Instance->mBackend->OnWindowMessage(Instance, Message, Arg0, Arg1);
            }
            return ::DefWindowProcW(Handle, Message, Arg0, Arg1);
        }
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Window::~Window()
    {
        if (!mBackend)
        {
            return;
        }

        if (mBackend->Handle)
        {
            ::DestroyWindow(mBackend->Handle);
        }
        UnregisterClassW(L"ZyWindowClass", ::GetModuleHandle(nullptr));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Window::Handle Window::GetHandle() const
    {
        return mBackend->Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::Poll()
    {
        MSG Event = { };

        while (::PeekMessageW(AddressOf(Event), nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(AddressOf(Event));
            ::DispatchMessageW(AddressOf(Event));

            if (Event.message == WM_QUIT)
            {
                break;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetTitle(Text Title)
    {
        mTitle = Title;

        Sequence<Wide, MAX_PATH> InTitle = StrConvertUTF16<MAX_PATH>(mTitle);
        ::SetWindowTextW(mBackend->Handle, InTitle.GetData());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetPosition(SInt32 X, SInt32 Y)
    {
        ::SetWindowPos(mBackend->Handle, nullptr, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetVisible(Bool Visible)
    {
        ::ShowWindow(mBackend->Handle, Visible ? SW_SHOW : SW_HIDE);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetFocus()
    {
        ::SetForegroundWindow(mBackend->Handle);
        ::SetFocus(mBackend->Handle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetFullscreen(Bool Fullscreen)
    {
        if (Fullscreen == IsFullscreen())
        {
            return;
        }

        if (Fullscreen)
        {
            ::GetWindowRect(mBackend->Handle, AddressOf(mBackend->Region));

            const HMONITOR Monitor     = ::MonitorFromWindow(mBackend->Handle, MONITOR_DEFAULTTONEAREST);
            MONITORINFO    MonitorInfo = { .cbSize = sizeof(MONITORINFO) };
            ::GetMonitorInfoW(Monitor, AddressOf(MonitorInfo));

            ::SetWindowLongPtrW(mBackend->Handle, GWL_STYLE, ConvertStyle(true, true));
            ::SetWindowLongPtrW(mBackend->Handle, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);

            ::SetWindowPos(
                mBackend->Handle,
                HWND_TOPMOST,
                MonitorInfo.rcMonitor.left,
                MonitorInfo.rcMonitor.top,
                MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOCOPYBITS);
        }
        else
        {
            ::SetWindowLongPtrW(mBackend->Handle, GWL_STYLE, ConvertStyle(false, IsBorderless()));
            ::SetWindowLongPtrW(mBackend->Handle, GWL_EXSTYLE, WS_EX_APPWINDOW);

            ::SetWindowPos(
                mBackend->Handle,
                HWND_NOTOPMOST,
                mBackend->Region.left,
                mBackend->Region.top,
                mBackend->Region.right - mBackend->Region.left,
                mBackend->Region.bottom - mBackend->Region.top,
                SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOCOPYBITS);
        }

        mStates = SetOrClearBit(mStates, State::Fullscreen, Fullscreen);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetSize(UInt32 Width, UInt32 Height)
    {
        RECT Rect = {
            .left   = 0,
            .top    = 0,
            .right  = static_cast<LONG>(Width),
            .bottom = static_cast<LONG>(Height)
        };

        const DWORD  Style = ::GetWindowLongW(mBackend->Handle, GWL_STYLE);
        const UInt32 Dpi   = ::GetDpiForWindow(mBackend->Handle);

        ::AdjustWindowRectExForDpi(AddressOf(Rect), Style, FALSE, 0, Dpi);
        ::SetWindowPos(mBackend->Handle, nullptr, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_NOMOVE | SWP_NOZORDER);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetCursorPosition(UInt32 X, UInt32 Y)
    {
        POINT Point = { .x = static_cast<LONG>(X), .y = static_cast<LONG>(Y) };
        ::ClientToScreen(mBackend->Handle, AddressOf(Point));
        ::SetCursorPos(Point.x, Point.y);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetCursorLock(Bool Lock)
    {
        ZY_ASSERT(Lock != IsCursorLocked(), "The cursor is already in the requested lock state");

        // Focus changes apply and release the lock themselves, so an unfocused window only records it for later.
        if (IsFocused())
        {
            mBackend->ApplyCursorLock(Lock);
        }

        const RAWINPUTDEVICE Device {
            .usUsagePage = 0x01,
            .usUsage     = 0x02,
            .dwFlags     = Lock ? 0u : RIDEV_REMOVE,
            .hwndTarget  = Lock ? mBackend->Handle : nullptr
        };
        ::RegisterRawInputDevices(AddressOf(Device), 1, sizeof(RAWINPUTDEVICE));

        mStates = SetOrClearBit(mStates, State::Locked, Lock);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Window::Initialize(Text Title, SInt32 X, SInt32 Y, UInt32 Width, UInt32 Height, Bool Borderless, Bool Fullscreen)
    {
        WNDCLASSEXW Win32Class   = { };
        Win32Class.cbSize        = sizeof(WNDCLASSEXW);
        Win32Class.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        Win32Class.lpfnWndProc   = Backend::OnWindowDispatcher;
        Win32Class.hInstance     = ::GetModuleHandleW(nullptr);
        Win32Class.hCursor       = ::LoadCursorW(nullptr, IDC_ARROW);
        Win32Class.hbrBackground = nullptr;
        Win32Class.lpszClassName = L"ZyWindowClass";
        ::RegisterClassExW(AddressOf(Win32Class));

        Sequence<Wide, MAX_PATH> InTitle = StrConvertUTF16<MAX_PATH>(Title);

        mBackend = Unique<Backend>::Create();

        // The frame is sized at the DPI of the monitor the window opens on.
        const HMONITOR Monitor = ::MonitorFromPoint(POINT { .x = X, .y = Y }, MONITOR_DEFAULTTONEAREST);
        UINT           DpiX    = USER_DEFAULT_SCREEN_DPI;
        UINT           DpiY    = USER_DEFAULT_SCREEN_DPI;
        ::GetDpiForMonitor(Monitor, MDT_EFFECTIVE_DPI, AddressOf(DpiX), AddressOf(DpiY));

        // Leaving fullscreen restores the windowed frame, so it is worked out even when the window opens fullscreen.
        RECT Frame {
            .left   = 0,
            .top    = 0,
            .right  = static_cast<LONG>(Width),
            .bottom = static_cast<LONG>(Height)
        };
        ::AdjustWindowRectExForDpi(AddressOf(Frame), ConvertStyle(false, Borderless), FALSE, WS_EX_APPWINDOW, DpiX);

        mBackend->Region = {
            .left   = X,
            .top    = Y,
            .right  = X + (Frame.right - Frame.left),
            .bottom = Y + (Frame.bottom - Frame.top)
        };

        RECT Bounds = mBackend->Region;

        if (Fullscreen)
        {
            MONITORINFO MonitorInfo {
                .cbSize = sizeof(MONITORINFO) 
            };
            ::GetMonitorInfoW(Monitor, AddressOf(MonitorInfo));

            Bounds = MonitorInfo.rcMonitor;
        }

        mTitle  = Title;
        mWidth  = Fullscreen ? static_cast<UInt32>(Bounds.right - Bounds.left) : Width;
        mHeight = Fullscreen ? static_cast<UInt32>(Bounds.bottom - Bounds.top) : Height;
        mStates = SetOrClearBit(mStates, State::Borderless, Borderless);
        mStates = SetOrClearBit(mStates, State::Fullscreen, Fullscreen);

        mBackend->Handle = ::CreateWindowExW(
            WS_EX_APPWINDOW | (Fullscreen ? WS_EX_TOPMOST : 0),
            L"ZyWindowClass",
            InTitle.GetData(),
            ConvertStyle(Fullscreen, Borderless),
            Bounds.left,
            Bounds.top,
            Bounds.right - Bounds.left,
            Bounds.bottom - Bounds.top,
            nullptr,
            nullptr,
            Win32Class.hInstance,
            this);

        return (mBackend->Handle != nullptr);
    }
}