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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Platform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Input::Key ConvertVirtualKey(UInt32 Key)
    {
        switch (Key)
        {
        case 'A':
            return Input::Key::A;
        case 'B':
            return Input::Key::B;
        case 'C':
            return Input::Key::C;
        case 'D':
            return Input::Key::D;
        case 'E':
            return Input::Key::E;
        case 'F':
            return Input::Key::F;
        case 'G':
            return Input::Key::G;
        case 'H':
            return Input::Key::H;
        case 'I':
            return Input::Key::I;
        case 'J':
            return Input::Key::J;
        case 'K':
            return Input::Key::K;
        case 'L':
            return Input::Key::L;
        case 'M':
            return Input::Key::M;
        case 'N':
            return Input::Key::N;
        case 'O':
            return Input::Key::O;
        case 'P':
            return Input::Key::P;
        case 'Q':
            return Input::Key::Q;
        case 'R':
            return Input::Key::R;
        case 'S':
            return Input::Key::S;
        case 'T':
            return Input::Key::T;
        case 'U':
            return Input::Key::U;
        case 'V':
            return Input::Key::V;
        case 'W':
            return Input::Key::W;
        case 'X':
            return Input::Key::X;
        case 'Y':
            return Input::Key::Y;
        case 'Z':
            return Input::Key::Z;
        case '0':
            return Input::Key::Alpha0;
        case '1':
            return Input::Key::Alpha1;
        case '2':
            return Input::Key::Alpha2;
        case '3':
            return Input::Key::Alpha3;
        case '4':
            return Input::Key::Alpha4;
        case '5':
            return Input::Key::Alpha5;
        case '6':
            return Input::Key::Alpha6;
        case '7':
            return Input::Key::Alpha7;
        case '8':
            return Input::Key::Alpha8;
        case '9':
            return Input::Key::Alpha9;
        case VK_F1:
            return Input::Key::F1;
        case VK_F2:
            return Input::Key::F2;
        case VK_F3:
            return Input::Key::F3;
        case VK_F4:
            return Input::Key::F4;
        case VK_F5:
            return Input::Key::F5;
        case VK_F6:
            return Input::Key::F6;
        case VK_F7:
            return Input::Key::F7;
        case VK_F8:
            return Input::Key::F8;
        case VK_F9:
            return Input::Key::F9;
        case VK_F10:
            return Input::Key::F10;
        case VK_F11:
            return Input::Key::F11;
        case VK_F12:
            return Input::Key::F12;
        case VK_LEFT:
            return Input::Key::Left;
        case VK_RIGHT:
            return Input::Key::Right;
        case VK_UP:
            return Input::Key::Up;
        case VK_DOWN:
            return Input::Key::Down;
        case VK_HOME:
            return Input::Key::Home;
        case VK_END:
            return Input::Key::End;
        case VK_PRIOR:
            return Input::Key::PageUp;
        case VK_NEXT:
            return Input::Key::PageDown;
        case VK_INSERT:
            return Input::Key::Insert;
        case VK_DELETE:
            return Input::Key::Delete;
        case VK_SPACE:
            return Input::Key::Space;
        case VK_ESCAPE:
            return Input::Key::Escape;
        case VK_RETURN:
            return Input::Key::Enter;
        case VK_TAB:
            return Input::Key::Tab;
        case VK_BACK:
            return Input::Key::Backspace;
        case VK_CAPITAL:
            return Input::Key::Capital;
        case VK_SCROLL:
            return Input::Key::Scroll;
        case VK_NUMLOCK:
            return Input::Key::NumLock;
        case VK_SNAPSHOT:
            return Input::Key::Print;
        case VK_PAUSE:
            return Input::Key::Pause;
        case VK_SHIFT:
            if (GetKeyState(VK_LSHIFT) & 0x8000)
            {
                return Input::Key::LeftShift;
            }
            return Input::Key::RightShift;
        case VK_CONTROL:
            if (GetKeyState(VK_LCONTROL) & 0x8000)
            {
                return Input::Key::LeftCtrl;
            }
            return Input::Key::RightCtrl;
        case VK_MENU:
            if (GetKeyState(VK_LMENU) & 0x8000)
            {
                return Input::Key::LeftAlt;
            }
            return Input::Key::RightAlt;
        case VK_LWIN:
            return Input::Key::LeftSuper;
        case VK_RWIN:
            return Input::Key::RightSuper;
        case VK_APPS:
            return Input::Key::Menu;
        case VK_OEM_1:
            return Input::Key::Semicolon;
        case VK_OEM_2:
            return Input::Key::Slash;
        case VK_OEM_3:
            return Input::Key::Grave;
        case VK_OEM_4:
            return Input::Key::LeftBracket;
        case VK_OEM_5:
            return Input::Key::Backslash;
        case VK_OEM_6:
            return Input::Key::RightBracket;
        case VK_OEM_7:
            return Input::Key::Apostrophe;
        case VK_OEM_COMMA:
            return Input::Key::Comma;
        case VK_OEM_MINUS:
            return Input::Key::Minus;
        case VK_OEM_PERIOD:
            return Input::Key::Period;
        case VK_OEM_PLUS:
            return Input::Key::Equal;
        case VK_NUMPAD0:
            return Input::Key::Keypad0;
        case VK_NUMPAD1:
            return Input::Key::Keypad1;
        case VK_NUMPAD2:
            return Input::Key::Keypad2;
        case VK_NUMPAD3:
            return Input::Key::Keypad3;
        case VK_NUMPAD4:
            return Input::Key::Keypad4;
        case VK_NUMPAD5:
            return Input::Key::Keypad5;
        case VK_NUMPAD6:
            return Input::Key::Keypad6;
        case VK_NUMPAD7:
            return Input::Key::Keypad7;
        case VK_NUMPAD8:
            return Input::Key::Keypad8;
        case VK_NUMPAD9:
            return Input::Key::Keypad9;
        case VK_DECIMAL:
            return Input::Key::Decimal;
        case VK_DIVIDE:
            return Input::Key::Divide;
        case VK_MULTIPLY:
            return Input::Key::Multiply;
        case VK_SUBTRACT:
            return Input::Key::Subtract;
        case VK_ADD:
            return Input::Key::Add;
        default:
            return Input::Key::Unknown;
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
        struct States
        {
            /// Cursor position before lock.
            POINT Cursor;

            /// Window area (position + size) before fullscreen.
            RECT  Region;
        };

        /// The native Win32 window handle (HWND) associated with the window.
        HWND   Handle = nullptr;

        /// The snapshot of the window state for later restoration.
        States Snapshot;

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
                POINT Cursor = Snapshot.Cursor;
                ::ClientToScreen(Handle, AddressOf(Cursor));
                ::SetCursorPos(Cursor.x, Cursor.y);

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
                Dispatcher.Notify(Platform::Dispatcher::Notification::Monitor);
                break;
            }
            case WM_DEVICECHANGE:
            {
                switch (Arg0)
                {
                case 0x8000: // DBT_DEVICEARRIVAL
                case 0x8004: // DBT_DEVICEREMOVECOMPLETE
                    Dispatcher.Notify(Platform::Dispatcher::Notification::Monitor);
                    break;
                default:
                    break;
                }
                break;
            }
            case WM_WINDOWPOSCHANGED:
            {
                const ConstPtr<WINDOWPOS> Parameter = reinterpret_cast<ConstPtr<WINDOWPOS>>(Arg1);

                Window->mX      = Parameter->x;
                Window->mY      = Parameter->y;

                if (Window->mWidth != Parameter->cx || Window->mHeight != Parameter->cy)
                {
                    Window->mWidth  = Parameter->cx;
                    Window->mHeight = Parameter->cy;

                    if (!HasBit(Window->mStates, State::Held))
                    {
                        Dispatcher.QueueWindowResize(Window->mWidth, Window->mHeight);
                    }
                }
                break;
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
            case WM_ENTERSIZEMOVE:
            {
                Window->mStates = SetBit(Window->mStates, State::Held);
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
                Window->mStates = ClearBit(Window->mStates, State::Held);

                Dispatcher.QueueWindowResize(Window->mWidth, Window->mHeight);
                break;
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
                    const Input::Key Key = ConvertVirtualKey(static_cast<UInt32>(Arg0));

                    if (Key != Input::Key::Unknown)
                    {
                        Dispatcher.QueueKeyDown(Key);
                    }
                }
                break;
            }
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                const Input::Key Key = ConvertVirtualKey(static_cast<UInt32>(Arg0));

                if (Key != Input::Key::Unknown)
                {
                    Dispatcher.QueueKeyUp(Key);
                }
                break;
            }
            case WM_LBUTTONDOWN:
            {
                Dispatcher.QueueMouseButtonDown(Input::Button::Left);
                break;
            }
            case WM_LBUTTONUP:
            {
                Dispatcher.QueueMouseButtonUp(Input::Button::Left);
                break;
            }
            case WM_MBUTTONDOWN:
            {
                Dispatcher.QueueMouseButtonDown(Input::Button::Middle);
                break;
            }
            case WM_MBUTTONUP:
            {
                Dispatcher.QueueMouseButtonUp(Input::Button::Middle);
                break;
            }
            case WM_RBUTTONDOWN:
            {
                Dispatcher.QueueMouseButtonDown(Input::Button::Right);
                break;
            }
            case WM_RBUTTONUP:
            {
                Dispatcher.QueueMouseButtonUp(Input::Button::Right);
                break;
            }
            case WM_XBUTTONDOWN:
            {
                const Input::Button Button = (GET_XBUTTON_WPARAM(Arg0) == XBUTTON1)
                    ? Input::Button::Back
                    : Input::Button::Forward;
                Dispatcher.QueueMouseButtonDown(Button);
                break;
            }
            case WM_XBUTTONUP:
            {
                const Input::Button Button = (GET_XBUTTON_WPARAM(Arg0) == XBUTTON1)
                    ? Input::Button::Back
                    : Input::Button::Forward;
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
                Dispatcher.QueueMouseScroll(-Delta, 0.0f);
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
                    const SInt32 DeltaX = X - Snapshot.Cursor.x;
                    const SInt32 DeltaY = Y - Snapshot.Cursor.y;

                    Snapshot.Cursor.x = X;
                    Snapshot.Cursor.y = Y;
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
            default:
            {
                return ::DefWindowProcW(Handle, Message, Arg0, Arg1);
            }
            }
            return FALSE;
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

    void Window::SetTitle(Text Title)
    {
        mTitle = Title;

        Sequence<Wide, MAX_PATH> InTitle = StrConvertUTF16<MAX_PATH>(mTitle);
        StrEnsureEndsWith(InTitle, L'\0');
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
            ::GetWindowRect(mBackend->Handle, AddressOf(mBackend->Snapshot.Region));

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
                mBackend->Snapshot.Region.left,
                mBackend->Snapshot.Region.top,
                mBackend->Snapshot.Region.right - mBackend->Snapshot.Region.left,
                mBackend->Snapshot.Region.bottom - mBackend->Snapshot.Region.top,
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

    void Window::SetCursorLock(Bool State)
    {
        mBackend->ApplyCursorLock(State);

        if (State)
        {
            RAWINPUTDEVICE Device { };
            Device.usUsagePage = 0x01;
            Device.usUsage     = 0x02;
            Device.dwFlags     = 0x00;
            Device.hwndTarget  = mBackend->Handle;
            ::RegisterRawInputDevices(AddressOf(Device), 1, sizeof(RAWINPUTDEVICE));
        }
        else
        {
            RAWINPUTDEVICE Device { };
            Device.usUsagePage = 0x01;
            Device.usUsage     = 0x02;
            Device.dwFlags     = RIDEV_REMOVE;
            Device.hwndTarget  = nullptr;
            ::RegisterRawInputDevices(AddressOf(Device), 1, sizeof(RAWINPUTDEVICE));
        }
        mStates = State ? SetBit(mStates, State::Locked) : ClearBit(mStates, State::Locked);
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
        StrEnsureEndsWith(InTitle, L'\0');

        // Calculate required window size for desired client area.
        const DWORD Style0 = ConvertStyle(Fullscreen, Borderless);
        const DWORD Style1 = WS_EX_APPWINDOW | (Fullscreen ? WS_EX_TOPMOST : 0);

        RECT Rect = {
            .left = 0,
            .top = 0,
            .right = static_cast<LONG>(Width),
            .bottom = static_cast<LONG>(Height)
        };
        ::AdjustWindowRectEx(AddressOf(Rect), Style0, FALSE, Style1);

        mStates  = SetOrClearBit(mStates, State::Borderless, Borderless);
        mTitle   = Title;
        mWidth  = Rect.right - Rect.left;
        mHeight = Rect.bottom - Rect.top;

        if (Fullscreen)
        {
            const HMONITOR Monitor     = ::MonitorFromPoint(POINT{ .x = X, .y = Y }, MONITOR_DEFAULTTONEAREST);
            MONITORINFO    MonitorInfo = { .cbSize = sizeof(MONITORINFO) };
            ::GetMonitorInfoW(Monitor, AddressOf(MonitorInfo));

            X       = 0;
            Y       = 0;
            mWidth  = MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left;
            mHeight = MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top;
        }

        mBackend = Unique<Backend>::Create();
        mBackend->Handle = ::CreateWindowExW(
            Style1,
            L"ZyWindowClass",
            InTitle.GetData(),
            Style0,
            X,
            Y,
            mWidth,
            mHeight,
            nullptr,
            nullptr,
            Win32Class.hInstance,
            this);

        return (mBackend->Handle != nullptr);
    }
}