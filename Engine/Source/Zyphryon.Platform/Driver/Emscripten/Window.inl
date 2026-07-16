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

namespace Platform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Input::Key ConvertWebKey(Text Code)
    {
        switch (Hash(Code))
        {
        case "KeyA"_Hash:
            return Input::Key::A;
        case "KeyB"_Hash:
            return Input::Key::B;
        case "KeyC"_Hash:
            return Input::Key::C;
        case "KeyD"_Hash:
            return Input::Key::D;
        case "KeyE"_Hash:
            return Input::Key::E;
        case "KeyF"_Hash:
            return Input::Key::F;
        case "KeyG"_Hash:
            return Input::Key::G;
        case "KeyH"_Hash:
            return Input::Key::H;
        case "KeyI"_Hash:
            return Input::Key::I;
        case "KeyJ"_Hash:
            return Input::Key::J;
        case "KeyK"_Hash:
            return Input::Key::K;
        case "KeyL"_Hash:
            return Input::Key::L;
        case "KeyM"_Hash:
            return Input::Key::M;
        case "KeyN"_Hash:
            return Input::Key::N;
        case "KeyO"_Hash:
            return Input::Key::O;
        case "KeyP"_Hash:
            return Input::Key::P;
        case "KeyQ"_Hash:
            return Input::Key::Q;
        case "KeyR"_Hash:
            return Input::Key::R;
        case "KeyS"_Hash:
            return Input::Key::S;
        case "KeyT"_Hash:
            return Input::Key::T;
        case "KeyU"_Hash:
            return Input::Key::U;
        case "KeyV"_Hash:
            return Input::Key::V;
        case "KeyW"_Hash:
            return Input::Key::W;
        case "KeyX"_Hash:
            return Input::Key::X;
        case "KeyY"_Hash:
            return Input::Key::Y;
        case "KeyZ"_Hash:
            return Input::Key::Z;
        case "Digit0"_Hash:
            return Input::Key::Alpha0;
        case "Digit1"_Hash:
            return Input::Key::Alpha1;
        case "Digit2"_Hash:
            return Input::Key::Alpha2;
        case "Digit3"_Hash:
            return Input::Key::Alpha3;
        case "Digit4"_Hash:
            return Input::Key::Alpha4;
        case "Digit5"_Hash:
            return Input::Key::Alpha5;
        case "Digit6"_Hash:
            return Input::Key::Alpha6;
        case "Digit7"_Hash:
            return Input::Key::Alpha7;
        case "Digit8"_Hash:
            return Input::Key::Alpha8;
        case "Digit9"_Hash:
            return Input::Key::Alpha9;
        case "F1"_Hash:
            return Input::Key::F1;
        case "F2"_Hash:
            return Input::Key::F2;
        case "F3"_Hash:
            return Input::Key::F3;
        case "F4"_Hash:
            return Input::Key::F4;
        case "F5"_Hash:
            return Input::Key::F5;
        case "F6"_Hash:
            return Input::Key::F6;
        case "F7"_Hash:
            return Input::Key::F7;
        case "F8"_Hash:
            return Input::Key::F8;
        case "F9"_Hash:
            return Input::Key::F9;
        case "F10"_Hash:
            return Input::Key::F10;
        case "F11"_Hash:
            return Input::Key::F11;
        case "F12"_Hash:
            return Input::Key::F12;
        case "NumLock"_Hash:
            return Input::Key::NumLock;
        case "ShiftLeft"_Hash:
            return Input::Key::LeftShift;
        case "ShiftRight"_Hash:
            return Input::Key::RightShift;
        case "ControlLeft"_Hash:
            return Input::Key::LeftCtrl;
        case "ControlRight"_Hash:
            return Input::Key::RightCtrl;
        case "AltLeft"_Hash:
            return Input::Key::LeftAlt;
        case "AltRight"_Hash:
            return Input::Key::RightAlt;
        case "MetaLeft"_Hash:
            return Input::Key::LeftSuper;
        case "MetaRight"_Hash:
            return Input::Key::RightSuper;
        case "Space"_Hash:
            return Input::Key::Space;
        case "Enter"_Hash:
            return Input::Key::Enter;
        case "Escape"_Hash:
            return Input::Key::Escape;
        case "Backspace"_Hash:
            return Input::Key::Backspace;
        case "Tab"_Hash:
            return Input::Key::Tab;
        case "CapsLock"_Hash:
            return Input::Key::Capital;
        case "ArrowUp"_Hash:
            return Input::Key::Up;
        case "ArrowDown"_Hash:
            return Input::Key::Down;
        case "ArrowLeft"_Hash:
            return Input::Key::Left;
        case "ArrowRight"_Hash:
            return Input::Key::Right;
        case "Home"_Hash:
            return Input::Key::Home;
        case "End"_Hash:
            return Input::Key::End;
        case "PageUp"_Hash:
            return Input::Key::PageUp;
        case "PageDown"_Hash:
            return Input::Key::PageDown;
        case "Insert"_Hash:
            return Input::Key::Insert;
        case "Delete"_Hash:
            return Input::Key::Delete;
        case "Minus"_Hash:
            return Input::Key::Minus;
        case "Equal"_Hash:
            return Input::Key::Equal;
        case "BracketLeft"_Hash:
            return Input::Key::LeftBracket;
        case "BracketRight"_Hash:
            return Input::Key::RightBracket;
        case "Backslash"_Hash:
            return Input::Key::Backslash;
        case "Semicolon"_Hash:
            return Input::Key::Semicolon;
        case "Quote"_Hash:
            return Input::Key::Apostrophe;
        case "Comma"_Hash:
            return Input::Key::Comma;
        case "Period"_Hash:
            return Input::Key::Period;
        case "Slash"_Hash:
            return Input::Key::Slash;
        case "Backquote"_Hash:
            return Input::Key::Grave;
        case "Numpad0"_Hash:
            return Input::Key::Keypad0;
        case "Numpad1"_Hash:
            return Input::Key::Keypad1;
        case "Numpad2"_Hash:
            return Input::Key::Keypad2;
        case "Numpad3"_Hash:
            return Input::Key::Keypad3;
        case "Numpad4"_Hash:
            return Input::Key::Keypad4;
        case "Numpad5"_Hash:
            return Input::Key::Keypad5;
        case "Numpad6"_Hash:
            return Input::Key::Keypad6;
        case "Numpad7"_Hash:
            return Input::Key::Keypad7;
        case "Numpad8"_Hash:
            return Input::Key::Keypad8;
        case "Numpad9"_Hash:
            return Input::Key::Keypad9;
        case "NumpadAdd"_Hash:
            return Input::Key::Add;
        case "NumpadSubtract"_Hash:
            return Input::Key::Subtract;
        case "NumpadMultiply"_Hash:
            return Input::Key::Multiply;
        case "NumpadDivide"_Hash:
            return Input::Key::Divide;
        case "NumpadDecimal"_Hash:
            return Input::Key::Decimal;
        case "NumpadEnter"_Hash:
            return Input::Key::Enter;
        default:
            return Input::Key::Unknown;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Input::Button ConvertWebButton(SInt32 Button)
    {
        switch (Button)
        {
        case 0:
            return Input::Button::Left;
        case 1:
            return Input::Button::Middle;
        case 2:
            return Input::Button::Right;
        case 3:
            return Input::Button::Back;
        case 4:
            return Input::Button::Forward;
        default:
            return Input::Button::Unknown;
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
                    if (const Input::Key Key = ConvertWebKey(StrConvert(Event->code)); Key != Input::Key::Unknown)
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
                if (const Input::Key Key = ConvertWebKey(StrConvert(Event->code)); Key != Input::Key::Unknown)
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

        static EM_BOOL OnCanvasTouchStart(SInt32 eventType, ConstPtr<EmscriptenTouchEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                if (Event->numTouches > 0)
                {
                    ConstRef<EmscriptenTouchPoint> Touch = Event->touches[0];

                    Canvas->mDispatcher.QueueMouseMove(
                        static_cast<Real32>(Touch.targetX),
                        static_cast<Real32>(Touch.targetY),
                        0.0f,
                        0.0f);
                    Canvas->mDispatcher.QueueMouseButtonDown(Input::Button::Left);
                }
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasTouchEnd(SInt32 eventType, ConstPtr<EmscriptenTouchEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Canvas->mDispatcher.QueueMouseButtonUp(Input::Button::Left);
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasTouchMove(SInt32 eventType, ConstPtr<EmscriptenTouchEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                if (Event->numTouches > 0)
                {
                    ConstRef<EmscriptenTouchPoint> Touch = Event->touches[0];

                    Canvas->mDispatcher.QueueMouseMove(
                        static_cast<Real32>(Touch.targetX),
                        static_cast<Real32>(Touch.targetY),
                        0.0f,
                        0.0f);
                }
            }
            return EM_TRUE;
        }

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        static EM_BOOL OnCanvasTouchCancel(SInt32 eventType, ConstPtr<EmscriptenTouchEvent> Event, Ptr<void> Context)
        {
            if (const Ptr<Window> Canvas = static_cast<Ptr<Window>>(Context))
            {
                Canvas->mDispatcher.QueueMouseButtonUp(Input::Button::Left);
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