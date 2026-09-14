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
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasKeyDown(SInt32, ConstPtr<EmscriptenKeyboardEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                if (!Event->repeat)
                {
                    if (const ZyInput::Key Key = ConvertWebKey(StrConvert(Event->code)); Key != ZyInput::Key::Unknown)
                    {
                        Canvas->mDispatcher.QueueKeyDown(Key);
                    }
                }

                if (const Text Data = StrConvert(Event->key); Data.GetSize() == 1)
                {
                    Canvas->mDispatcher.QueueKeyType(Data);
                }
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
                const Real32 X         = static_cast<Real32>(Event->targetX);
                const Real32 Y         = static_cast<Real32>(Event->targetY);
                const Real32 RelativeX = static_cast<Real32>(Event->movementX);
                const Real32 RelativeY = static_cast<Real32>(Event->movementY);
                Canvas->mDispatcher.QueueMouseMove(X, Y, RelativeX, RelativeY);
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasMouseDown(SInt32, ConstPtr<EmscriptenMouseEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Canvas->mDispatcher.QueueMouseButtonDown(ConvertWebButton(Event->button));
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasMouseUp(SInt32, ConstPtr<EmscriptenMouseEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Canvas->mDispatcher.QueueMouseButtonUp(ConvertWebButton(Event->button));
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

        static EM_BOOL OnCanvasTouchStart(SInt32, ConstPtr<EmscriptenTouchEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                for (SInt32 Index = 0; Index < Event->numTouches; ++Index)
                {
                    if (ConstRef<EmscriptenTouchPoint> Touch = Event->touches[Index]; Touch.isChanged)
                    {
                        Canvas->mDispatcher.QueueTouchDown(
                            static_cast<UInt32>(Touch.identifier),
                            static_cast<Real32>(Touch.targetX),
                            static_cast<Real32>(Touch.targetY));
                    }
                }
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasTouchMove(SInt32, ConstPtr<EmscriptenTouchEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                for (SInt32 Index = 0; Index < Event->numTouches; ++Index)
                {
                    if (ConstRef<EmscriptenTouchPoint> Touch = Event->touches[Index]; Touch.isChanged)
                    {
                        Canvas->mDispatcher.QueueTouchMove(
                            static_cast<UInt32>(Touch.identifier),
                            static_cast<Real32>(Touch.targetX),
                            static_cast<Real32>(Touch.targetY));
                    }
                }
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasTouchEnd(SInt32, ConstPtr<EmscriptenTouchEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                for (SInt32 Index = 0; Index < Event->numTouches; ++Index)
                {
                    if (ConstRef<EmscriptenTouchPoint> Touch = Event->touches[Index]; Touch.isChanged)
                    {
                        Canvas->mDispatcher.QueueTouchUp(
                            static_cast<UInt32>(Touch.identifier),
                            static_cast<Real32>(Touch.targetX),
                            static_cast<Real32>(Touch.targetY));
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

        static EM_BOOL OnCanvasFullscreen(SInt32, ConstPtr<void> Reserved, Ptr<void> Context)
        {
            OnTabResize(0, nullptr, Context);
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnTabResize(SInt32, ConstPtr<EmscriptenUiEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Real64 Width, Height;
                emscripten_get_element_css_size("!ZyWindowHTML5", AddressOf(Width), AddressOf(Height));

                if (Canvas->IsBorderless())
                {
                    const Real64 DPI = EM_ASM_DOUBLE({ return window.devicePixelRatio || 1.0; });
                    emscripten_set_canvas_element_size("!ZyWindowHTML5", Width * DPI, Height * DPI);
                }

                Canvas->mWidth  = static_cast<UInt32>(Width);
                Canvas->mHeight = static_cast<UInt32>(Height);
                Canvas->mDispatcher.QueueWindowResize(Canvas->mWidth, Canvas->mHeight);
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
        emscripten_set_keydown_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
        emscripten_set_keyup_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
        emscripten_set_mousemove_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
        emscripten_set_mousedown_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
        emscripten_set_mouseup_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
        emscripten_set_wheel_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
        emscripten_set_touchstart_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
        emscripten_set_touchend_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
        emscripten_set_touchmove_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
        emscripten_set_touchcancel_callback("!ZyWindowHTML5", nullptr, EM_TRUE, nullptr);
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
        // NOTE: Not supported by the browser.
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetVisible(Bool Visible)
    {
        mStates = Visible ? SetBit(mStates, State::Visible) : ClearBit(mStates, State::Visible);

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
        if (Fullscreen)
        {
            if (!IsFullscreen())
            {
                EmscriptenFullscreenStrategy Strategy { };
                Strategy.scaleMode                     = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
                Strategy.canvasResolutionScaleMode     = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
                Strategy.filteringMode                 = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
                Strategy.canvasResizedCallback         = Window::Backend::OnCanvasFullscreen;
                Strategy.canvasResizedCallbackUserData = this;

                emscripten_request_fullscreen_strategy("!ZyWindowHTML5", EM_TRUE, AddressOf(Strategy));
            }
        }
        else
        {
            if (IsFullscreen())
            {
                emscripten_exit_fullscreen();
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetSize(UInt32 Width, UInt32 Height)
    {
        mWidth  = Width;
        mHeight = Height;
        emscripten_set_element_css_size("!ZyWindowHTML5", mWidth, mHeight);

        const Real32 DPI = EM_ASM_DOUBLE({ return window.devicePixelRatio || 1.0; });
        emscripten_set_canvas_element_size("!ZyWindowHTML5", mWidth * DPI, mHeight  * DPI);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetCursorPosition(UInt32 X, UInt32 Y)
    {
        // NOTE: Not supported by the browser.
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Window::SetCursorLock(Bool State)
    {
        if (State)
        {
            emscripten_request_pointerlock("!ZyWindowHTML5", EM_TRUE);
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
        // Register the canvas element as a special target for emscripten's HTML5 event system.
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
            const Real32 DPI = EM_ASM_DOUBLE({ return window.devicePixelRatio || 1.0; });
            SetSize(Width / DPI, Height / DPI);
        }

        SetTitle(Title);
        SetFullscreen(Fullscreen);
        SetFocus();

        // Register HTML5 event callbacks for the canvas window.
        emscripten_set_keydown_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasKeyDown);
        emscripten_set_keyup_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasKeyUp);
        emscripten_set_mousemove_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasMouseMove);
        emscripten_set_mousedown_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasMouseDown);
        emscripten_set_mouseup_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasMouseUp);
        emscripten_set_wheel_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasMouseWheel);
        emscripten_set_touchstart_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasTouchStart);
        emscripten_set_touchend_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasTouchEnd);
        emscripten_set_touchmove_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasTouchMove);
        emscripten_set_touchcancel_callback("!ZyWindowHTML5", this, EM_TRUE, Window::Backend::OnCanvasTouchCancel);
        emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, Window::Backend::OnTabFocus);
        emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, Window::Backend::OnTabFocus);
        emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, Window::Backend::OnTabResize);
        emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, Window::Backend::OnDocumentFullscreen);
        emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, Window::Backend::OnDocumentLock);
        emscripten_set_visibilitychange_callback(this, EM_TRUE, Window::Backend::OnTabVisibility);
        return true;
    }
}