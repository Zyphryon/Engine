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

#include <emscripten/html5.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyPlatform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static ZyInput::Key ConvertWebKey(Text Code)
    {
        switch (Hash(Code))
        {
        case "KeyA"_Hash:
            return ZyInput::Key::A;
        case "KeyB"_Hash:
            return ZyInput::Key::B;
        case "KeyC"_Hash:
            return ZyInput::Key::C;
        case "KeyD"_Hash:
            return ZyInput::Key::D;
        case "KeyE"_Hash:
            return ZyInput::Key::E;
        case "KeyF"_Hash:
            return ZyInput::Key::F;
        case "KeyG"_Hash:
            return ZyInput::Key::G;
        case "KeyH"_Hash:
            return ZyInput::Key::H;
        case "KeyI"_Hash:
            return ZyInput::Key::I;
        case "KeyJ"_Hash:
            return ZyInput::Key::J;
        case "KeyK"_Hash:
            return ZyInput::Key::K;
        case "KeyL"_Hash:
            return ZyInput::Key::L;
        case "KeyM"_Hash:
            return ZyInput::Key::M;
        case "KeyN"_Hash:
            return ZyInput::Key::N;
        case "KeyO"_Hash:
            return ZyInput::Key::O;
        case "KeyP"_Hash:
            return ZyInput::Key::P;
        case "KeyQ"_Hash:
            return ZyInput::Key::Q;
        case "KeyR"_Hash:
            return ZyInput::Key::R;
        case "KeyS"_Hash:
            return ZyInput::Key::S;
        case "KeyT"_Hash:
            return ZyInput::Key::T;
        case "KeyU"_Hash:
            return ZyInput::Key::U;
        case "KeyV"_Hash:
            return ZyInput::Key::V;
        case "KeyW"_Hash:
            return ZyInput::Key::W;
        case "KeyX"_Hash:
            return ZyInput::Key::X;
        case "KeyY"_Hash:
            return ZyInput::Key::Y;
        case "KeyZ"_Hash:
            return ZyInput::Key::Z;
        case "Digit0"_Hash:
            return ZyInput::Key::Alpha0;
        case "Digit1"_Hash:
            return ZyInput::Key::Alpha1;
        case "Digit2"_Hash:
            return ZyInput::Key::Alpha2;
        case "Digit3"_Hash:
            return ZyInput::Key::Alpha3;
        case "Digit4"_Hash:
            return ZyInput::Key::Alpha4;
        case "Digit5"_Hash:
            return ZyInput::Key::Alpha5;
        case "Digit6"_Hash:
            return ZyInput::Key::Alpha6;
        case "Digit7"_Hash:
            return ZyInput::Key::Alpha7;
        case "Digit8"_Hash:
            return ZyInput::Key::Alpha8;
        case "Digit9"_Hash:
            return ZyInput::Key::Alpha9;
        case "F1"_Hash:
            return ZyInput::Key::F1;
        case "F2"_Hash:
            return ZyInput::Key::F2;
        case "F3"_Hash:
            return ZyInput::Key::F3;
        case "F4"_Hash:
            return ZyInput::Key::F4;
        case "F5"_Hash:
            return ZyInput::Key::F5;
        case "F6"_Hash:
            return ZyInput::Key::F6;
        case "F7"_Hash:
            return ZyInput::Key::F7;
        case "F8"_Hash:
            return ZyInput::Key::F8;
        case "F9"_Hash:
            return ZyInput::Key::F9;
        case "F10"_Hash:
            return ZyInput::Key::F10;
        case "F11"_Hash:
            return ZyInput::Key::F11;
        case "F12"_Hash:
            return ZyInput::Key::F12;
        case "NumLock"_Hash:
            return ZyInput::Key::NumLock;
        case "ShiftLeft"_Hash:
            return ZyInput::Key::LeftShift;
        case "ShiftRight"_Hash:
            return ZyInput::Key::RightShift;
        case "ControlLeft"_Hash:
            return ZyInput::Key::LeftCtrl;
        case "ControlRight"_Hash:
            return ZyInput::Key::RightCtrl;
        case "AltLeft"_Hash:
            return ZyInput::Key::LeftAlt;
        case "AltRight"_Hash:
            return ZyInput::Key::RightAlt;
        case "MetaLeft"_Hash:
            return ZyInput::Key::LeftSuper;
        case "MetaRight"_Hash:
            return ZyInput::Key::RightSuper;
        case "Space"_Hash:
            return ZyInput::Key::Space;
        case "Enter"_Hash:
            return ZyInput::Key::Enter;
        case "Escape"_Hash:
            return ZyInput::Key::Escape;
        case "Backspace"_Hash:
            return ZyInput::Key::Backspace;
        case "Tab"_Hash:
            return ZyInput::Key::Tab;
        case "CapsLock"_Hash:
            return ZyInput::Key::Capital;
        case "ArrowUp"_Hash:
            return ZyInput::Key::Up;
        case "ArrowDown"_Hash:
            return ZyInput::Key::Down;
        case "ArrowLeft"_Hash:
            return ZyInput::Key::Left;
        case "ArrowRight"_Hash:
            return ZyInput::Key::Right;
        case "Home"_Hash:
            return ZyInput::Key::Home;
        case "End"_Hash:
            return ZyInput::Key::End;
        case "PageUp"_Hash:
            return ZyInput::Key::PageUp;
        case "PageDown"_Hash:
            return ZyInput::Key::PageDown;
        case "Insert"_Hash:
            return ZyInput::Key::Insert;
        case "Delete"_Hash:
            return ZyInput::Key::Delete;
        case "Minus"_Hash:
            return ZyInput::Key::Minus;
        case "Equal"_Hash:
            return ZyInput::Key::Equal;
        case "BracketLeft"_Hash:
            return ZyInput::Key::LeftBracket;
        case "BracketRight"_Hash:
            return ZyInput::Key::RightBracket;
        case "Backslash"_Hash:
            return ZyInput::Key::Backslash;
        case "Semicolon"_Hash:
            return ZyInput::Key::Semicolon;
        case "Quote"_Hash:
            return ZyInput::Key::Apostrophe;
        case "Comma"_Hash:
            return ZyInput::Key::Comma;
        case "Period"_Hash:
            return ZyInput::Key::Period;
        case "Slash"_Hash:
            return ZyInput::Key::Slash;
        case "Backquote"_Hash:
            return ZyInput::Key::Grave;
        case "Numpad0"_Hash:
            return ZyInput::Key::Keypad0;
        case "Numpad1"_Hash:
            return ZyInput::Key::Keypad1;
        case "Numpad2"_Hash:
            return ZyInput::Key::Keypad2;
        case "Numpad3"_Hash:
            return ZyInput::Key::Keypad3;
        case "Numpad4"_Hash:
            return ZyInput::Key::Keypad4;
        case "Numpad5"_Hash:
            return ZyInput::Key::Keypad5;
        case "Numpad6"_Hash:
            return ZyInput::Key::Keypad6;
        case "Numpad7"_Hash:
            return ZyInput::Key::Keypad7;
        case "Numpad8"_Hash:
            return ZyInput::Key::Keypad8;
        case "Numpad9"_Hash:
            return ZyInput::Key::Keypad9;
        case "NumpadAdd"_Hash:
            return ZyInput::Key::Add;
        case "NumpadSubtract"_Hash:
            return ZyInput::Key::Subtract;
        case "NumpadMultiply"_Hash:
            return ZyInput::Key::Multiply;
        case "NumpadDivide"_Hash:
            return ZyInput::Key::Divide;
        case "NumpadDecimal"_Hash:
            return ZyInput::Key::Decimal;
        case "NumpadEnter"_Hash:
            return ZyInput::Key::Enter;
        default:
            return ZyInput::Key::Unknown;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static ZyInput::Button ConvertWebButton(SInt32 Button)
    {
        switch (Button)
        {
        case 0:
            return ZyInput::Button::Left;
        case 1:
            return ZyInput::Button::Middle;
        case 2:
            return ZyInput::Button::Right;
        case 3:
            return ZyInput::Button::Back;
        case 4:
            return ZyInput::Button::Forward;
        default:
            return ZyInput::Button::Unknown;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Window::Backend
    {
        /// The target the html5 calls find the canvas by, registered in specialHTMLTargets by Initialize.
        static constexpr ConstPtr<Char> kCanvas = "!ZyWindowHTML5";

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasKeyDown(SInt32, ConstPtr<EmscriptenKeyboardEvent> Event, Ptr<void> Context)
        {
            const Bool Shortcut = Event->metaKey || (Event->ctrlKey && !Event->altKey);

            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                if (const ZyInput::Key Key = ConvertWebKey(StrConvert(Event->code)); Key != ZyInput::Key::Unknown)
                {
                    if (Event->repeat)
                    {
                        Canvas->mDispatcher.QueueKeyRepeat(Key);
                    }
                    else
                    {
                        Canvas->mDispatcher.QueueKeyDown(Key);
                    }
                }

                if (const Text Data = StrConvert(Event->key); !Shortcut && !Data.IsEmpty())
                {
                    // Named keys such as Shift spell more than one character and are not text.
                    UInt Cursor = 0;
                    Unicode::Decode(Data, Cursor);

                    if (Cursor == Data.GetSize())
                    {
                        Canvas->mDispatcher.QueueKeyType(Data);
                    }
                }
            }

            // The browser copies, cuts and pastes only when it runs its own action, and it picks those keys by keyCode.
            if (Shortcut && (Event->keyCode == 'C' || Event->keyCode == 'V' || Event->keyCode == 'X'))
            {
                return EM_FALSE;
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasKeyUp(SInt32, ConstPtr<EmscriptenKeyboardEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                if (const ZyInput::Key Key = ConvertWebKey(StrConvert(Event->code)); Key != ZyInput::Key::Unknown)
                {
                    Canvas->mDispatcher.QueueKeyUp(Key);
                }
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasMouseMove(SInt32, ConstPtr<EmscriptenMouseEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                // The page reports positions in css pixels and the window is measured in its drawing buffer's pixels.
                Real64 Width;
                Real64 Height;
                emscripten_get_element_css_size(kCanvas, AddressOf(Width), AddressOf(Height));

                const Real64 ScaleX    = Canvas->mWidth / Max(Width, 1.0);
                const Real64 ScaleY    = Canvas->mHeight / Max(Height, 1.0);
                const Real32 X         = static_cast<Real32>(Event->targetX * ScaleX);
                const Real32 Y         = static_cast<Real32>(Event->targetY * ScaleY);
                const Real32 RelativeX = static_cast<Real32>(Event->movementX);
                const Real32 RelativeY = static_cast<Real32>(Event->movementY);
                Canvas->mDispatcher.QueueMouseMove(X, Y, RelativeX, RelativeY);
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        template<void (Dispatcher::* Queue)(ZyInput::Button)>
        static EM_BOOL OnCanvasMouseButton(SInt32, ConstPtr<EmscriptenMouseEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                (Canvas->mDispatcher.*Queue)(ConvertWebButton(Event->button));
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasMouseWheel(SInt32, ConstPtr<EmscriptenWheelEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Real32 DeltaY = Event->deltaY;
                Real32 DeltaX = Event->deltaX;

                // Scales the browser's pixels, lines or pages to the notches the desktop drivers report.
                switch (Event->deltaMode)
                {
                case DOM_DELTA_PIXEL:
                    DeltaX *= 0.01f;
                    DeltaY *= 0.01f;
                    break;
                case DOM_DELTA_LINE:
                    DeltaX *= (1.0f / 3.0f);
                    DeltaY *= (1.0f / 3.0f);
                    break;
                case DOM_DELTA_PAGE:
                    DeltaX *= 80.0f;
                    DeltaY *= 80.0f;
                    break;
                default:
                    break;
                }
                Canvas->mDispatcher.QueueMouseScroll(DeltaX, -DeltaY);
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        template<void (Dispatcher::* Queue)(UInt32, Real32, Real32)>
        static EM_BOOL OnCanvasTouch(SInt32, ConstPtr<EmscriptenTouchEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                // The page reports positions in css pixels and the window is measured in its drawing buffer's pixels.
                Real64 Width;
                Real64 Height;
                emscripten_get_element_css_size(kCanvas, AddressOf(Width), AddressOf(Height));

                const Real64 ScaleX = Canvas->mWidth / Max(Width, 1.0);
                const Real64 ScaleY = Canvas->mHeight / Max(Height, 1.0);

                for (SInt32 Index = 0; Index < Event->numTouches; ++Index)
                {
                    if (ConstRef<EmscriptenTouchPoint> Touch = Event->touches[Index]; Touch.isChanged)
                    {
                        (Canvas->mDispatcher.*Queue)(
                            static_cast<UInt32>(Touch.identifier),
                            static_cast<Real32>(Touch.targetX * ScaleX),
                            static_cast<Real32>(Touch.targetY * ScaleY));
                    }
                }
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasTouchCancel(SInt32, ConstPtr<EmscriptenTouchEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                for (SInt32 Index = 0; Index < Event->numTouches; ++Index)
                {
                    if (ConstRef<EmscriptenTouchPoint> Touch = Event->touches[Index]; Touch.isChanged)
                    {
                        Canvas->mDispatcher.QueueTouchCancel(static_cast<UInt32>(Touch.identifier));
                    }
                }
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnTabFocus(SInt32 Type, ConstPtr<EmscriptenFocusEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Canvas->mStates = SetOrClearBit(Canvas->mStates, State::Focused, (Type == EMSCRIPTEN_EVENT_FOCUS));

                Canvas->mDispatcher.QueueWindowFocus(Canvas->IsFocused());
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasFullscreen(SInt32, ConstPtr<void>, Ptr<void> Context)
        {
            OnTabResize(0, nullptr, Context);
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnTabResize(SInt32, ConstPtr<EmscriptenUiEvent>, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                const Real64 DPI = emscripten_get_device_pixel_ratio();

                if (Canvas->IsBorderless())
                {
                    Real64 Width;
                    Real64 Height;
                    emscripten_get_element_css_size(kCanvas, AddressOf(Width), AddressOf(Height));
                    emscripten_set_canvas_element_size(kCanvas, Width * DPI, Height * DPI);
                }

                // The size told is the drawing buffer's, in device pixels.
                SInt32 Width;
                SInt32 Height;
                emscripten_get_canvas_element_size(kCanvas, AddressOf(Width), AddressOf(Height));

                Canvas->mWidth  = static_cast<UInt32>(Width);
                Canvas->mHeight = static_cast<UInt32>(Height);
                Canvas->mDispatcher.QueueWindowResize(Canvas->mWidth, Canvas->mHeight);

                // Zooming the page resizes it and changes its pixel ratio, so the ratio is told with every resize.
                Canvas->mDispatcher.QueueWindowScale(static_cast<Real32>(DPI));
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnDocumentFullscreen(SInt32, ConstPtr<EmscriptenFullscreenChangeEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Canvas->mStates = SetOrClearBit(Canvas->mStates, State::Fullscreen, Event->isFullscreen);
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnDocumentLock(SInt32, ConstPtr<EmscriptenPointerlockChangeEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Canvas->mStates = SetOrClearBit(Canvas->mStates, State::Locked, Event->isActive);
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnTabVisibility(SInt32, ConstPtr<EmscriptenVisibilityChangeEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Canvas->mStates = SetOrClearBit(Canvas->mStates, State::Visible, !Event->hidden);

                if (Canvas->IsVisible())
                {
                    Canvas->SetFocus();
                }
            }
            return EM_TRUE;
        }
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Window::~Window()
    {
        emscripten_set_keydown_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_keyup_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_mousemove_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_mousedown_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_mouseup_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_wheel_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_touchstart_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_touchend_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_touchmove_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_touchcancel_callback(Backend::kCanvas, nullptr, EM_TRUE, nullptr);
        emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
        emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
        emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
        emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, nullptr);
        emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, nullptr);
        emscripten_set_visibilitychange_callback(nullptr, EM_TRUE, nullptr);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Window::Handle Window::GetHandle() const
    {
        return nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::Poll()
    {
        // The browser delivers every event through the callbacks as it happens.
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetTitle(Text Title)
    {
        mTitle = Title;

        EM_ASM(
        {
            document.title = UTF8ToString($0, $1);
        }, Title.GetData(), Title.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetPosition(SInt32 X, SInt32 Y)
    {
        // The browser does not let a page move its window.
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetClipboard(Text Content)
    {
        EM_ASM(
        {
            Module.zyClipboard = UTF8ToString($0, $1);

            if (navigator.clipboard && navigator.clipboard.writeText)
            {
                navigator.clipboard.writeText(Module.zyClipboard).catch(function() { });
            }
        }, Content.GetData(), Content.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Str Window::GetClipboard()
    {
        const UInt Length = static_cast<UInt>(EM_ASM_INT({ return lengthBytesUTF8(Module.zyClipboard || ""); }));

        Str Result;

        if (Length != 0)
        {
            Result.Append('\0', Length);

            EM_ASM(
            {
                stringToUTF8(Module.zyClipboard || "", $0, $1);
            }, Result.GetData(), Length + 1);
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetVisible(Bool Visible)
    {
        mStates = SetOrClearBit(mStates, State::Visible, Visible);

        EM_ASM(
        {
            var jsElement = specialHTMLTargets['!ZyWindowHTML5'];

            if (jsElement)
            {
                jsElement.style.display = $0 ? 'block' : 'none';
            }
        }, Visible);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetFocus()
    {
        EM_ASM(
        {
            var jsElement = specialHTMLTargets['!ZyWindowHTML5'];

            if (jsElement)
            {
                jsElement.focus();
            }
        });
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
            EmscriptenFullscreenStrategy Strategy { };
            Strategy.scaleMode                     = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
            Strategy.canvasResolutionScaleMode     = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
            Strategy.filteringMode                 = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
            Strategy.canvasResizedCallback         = Backend::OnCanvasFullscreen;
            Strategy.canvasResizedCallbackUserData = this;

            emscripten_request_fullscreen_strategy(Backend::kCanvas, EM_TRUE, AddressOf(Strategy));
        }
        else
        {
            emscripten_exit_fullscreen();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetSize(UInt32 Width, UInt32 Height)
    {
        mWidth  = Width;
        mHeight = Height;

        // The size is in device pixels, and the page lays the element out in css pixels.
        const Real64 DPI = emscripten_get_device_pixel_ratio();
        emscripten_set_element_css_size(Backend::kCanvas, mWidth / DPI, mHeight / DPI);
        emscripten_set_canvas_element_size(Backend::kCanvas, mWidth, mHeight);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetCursorPosition(UInt32 X, UInt32 Y)
    {
        // The browser does not let a page move the cursor.
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetCursorLock(Bool Lock)
    {
        if (Lock)
        {
            emscripten_request_pointerlock(Backend::kCanvas, EM_TRUE);
        }
        else
        {
            emscripten_exit_pointerlock();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Window::Initialize(Text Title, SInt32 X, SInt32 Y, UInt32 Width, UInt32 Height, Bool Borderless, Bool Fullscreen)
    {
        // The html5 calls below find the canvas by this name.
        EM_ASM(
        {
            specialHTMLTargets['!ZyWindowHTML5'] = Module.canvas;
        });

        if (Borderless)
        {
            mStates = SetBit(mStates, State::Borderless);

            Backend::OnTabResize(0, nullptr, this);
        }
        else
        {
            SetSize(Width, Height);
        }

        SetTitle(Title);
        SetFullscreen(Fullscreen);
        SetFocus();

        // Text pasted into the page is what the clipboard holds, as far as the page may know.
        EM_ASM(
        {
            document.addEventListener('paste', function(Event)
            {
                const Content = Event.clipboardData ? Event.clipboardData.getData('text/plain') : "";

                if (Content)
                {
                    Module.zyClipboard = Content;
                }
            });
        });

        emscripten_set_keydown_callback(Backend::kCanvas, this, EM_TRUE, Backend::OnCanvasKeyDown);
        emscripten_set_keyup_callback(Backend::kCanvas, this, EM_TRUE, Backend::OnCanvasKeyUp);
        emscripten_set_mousemove_callback(Backend::kCanvas, this, EM_TRUE, Backend::OnCanvasMouseMove);
        emscripten_set_mousedown_callback(
            Backend::kCanvas,
            this,
            EM_TRUE,
            Backend::OnCanvasMouseButton<& Dispatcher::QueueMouseButtonDown>);
        emscripten_set_mouseup_callback(
            Backend::kCanvas,
            this,
            EM_TRUE,
            Backend::OnCanvasMouseButton<& Dispatcher::QueueMouseButtonUp>);
        emscripten_set_wheel_callback(Backend::kCanvas, this, EM_TRUE, Backend::OnCanvasMouseWheel);
        emscripten_set_touchstart_callback(
            Backend::kCanvas,
            this,
            EM_TRUE,
            Backend::OnCanvasTouch<& Dispatcher::QueueTouchDown>);
        emscripten_set_touchend_callback(
            Backend::kCanvas,
            this,
            EM_TRUE,
            Backend::OnCanvasTouch<& Dispatcher::QueueTouchUp>);
        emscripten_set_touchmove_callback(
            Backend::kCanvas,
            this,
            EM_TRUE,
            Backend::OnCanvasTouch<& Dispatcher::QueueTouchMove>);
        emscripten_set_touchcancel_callback(Backend::kCanvas, this, EM_TRUE, Backend::OnCanvasTouchCancel);
        emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, Backend::OnTabFocus);
        emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, Backend::OnTabFocus);
        emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, Backend::OnTabResize);
        emscripten_set_fullscreenchange_callback(
            EMSCRIPTEN_EVENT_TARGET_DOCUMENT,
            this,
            EM_TRUE,
            Backend::OnDocumentFullscreen);
        emscripten_set_pointerlockchange_callback(
            EMSCRIPTEN_EVENT_TARGET_DOCUMENT,
            this,
            EM_TRUE,
            Backend::OnDocumentLock);
        emscripten_set_visibilitychange_callback(this, EM_TRUE, Backend::OnTabVisibility);
        return true;
    }
}