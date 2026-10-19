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

#include "Zyphryon.Base/System/Library.hpp"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/Xrandr.h>

#ifdef Bool
#undef Bool
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define ZY_X11(Source, Name) inline decltype(& ::Name) zy##Name = nullptr;
#include "X11.inl"
#undef ZY_X11

// Every call keeps its X11 name, which is a macro over the loader's own pointer, so nothing links against X11.
#define XBlackPixel                  zyXBlackPixel
#define XChangeProperty              zyXChangeProperty
#define XCheckTypedWindowEvent       zyXCheckTypedWindowEvent
#define XCloseDisplay                zyXCloseDisplay
#define XCloseIM                     zyXCloseIM
#define XConnectionNumber            zyXConnectionNumber
#define XConvertSelection            zyXConvertSelection
#define XCreateIC                    zyXCreateIC
#define XCreatePixmap                zyXCreatePixmap
#define XCreatePixmapCursor          zyXCreatePixmapCursor
#define XCreateWindow                zyXCreateWindow
#define XDefaultRootWindow           zyXDefaultRootWindow
#define XDefaultScreen               zyXDefaultScreen
#define XDefineCursor                zyXDefineCursor
#define XDestroyIC                   zyXDestroyIC
#define XDestroyWindow               zyXDestroyWindow
#define XFilterEvent                 zyXFilterEvent
#define XFlush                       zyXFlush
#define XFree                        zyXFree
#define XFreeCursor                  zyXFreeCursor
#define XFreePixmap                  zyXFreePixmap
#define XGetSelectionOwner           zyXGetSelectionOwner
#define XGetWindowAttributes         zyXGetWindowAttributes
#define XGetWindowProperty           zyXGetWindowProperty
#define XGrabPointer                 zyXGrabPointer
#define XInternAtom                  zyXInternAtom
#define XLookupKeysym                zyXLookupKeysym
#define XMapRaised                   zyXMapRaised
#define XMoveWindow                  zyXMoveWindow
#define XNextEvent                   zyXNextEvent
#define XOpenDisplay                 zyXOpenDisplay
#define XOpenIM                      zyXOpenIM
#define XPending                     zyXPending
#define XRaiseWindow                 zyXRaiseWindow
#define XResizeWindow                zyXResizeWindow
#define XRootWindow                  zyXRootWindow
#define XSendEvent                   zyXSendEvent
#define XSetErrorHandler             zyXSetErrorHandler
#define XSetICFocus                  zyXSetICFocus
#define XSetInputFocus               zyXSetInputFocus
#define XSetSelectionOwner           zyXSetSelectionOwner
#define XSetWMProtocols              zyXSetWMProtocols
#define XStoreName                   zyXStoreName
#define XSync                        zyXSync
#define XTranslateCoordinates        zyXTranslateCoordinates
#define XUndefineCursor              zyXUndefineCursor
#define XUngrabPointer               zyXUngrabPointer
#define XUnmapWindow                 zyXUnmapWindow
#define XUnsetICFocus                zyXUnsetICFocus
#define XVisualIDFromVisual          zyXVisualIDFromVisual
#define XWarpPointer                 zyXWarpPointer
#define XkbSetDetectableAutoRepeat   zyXkbSetDetectableAutoRepeat
#define Xutf8LookupString            zyXutf8LookupString
#define XRRFreeCrtcInfo              zyXRRFreeCrtcInfo
#define XRRFreeOutputInfo            zyXRRFreeOutputInfo
#define XRRFreeScreenResources       zyXRRFreeScreenResources
#define XRRGetCrtcInfo               zyXRRGetCrtcInfo
#define XRRGetOutputInfo             zyXRRGetOutputInfo
#define XRRGetOutputPrimary          zyXRRGetOutputPrimary
#define XRRGetScreenResourcesCurrent zyXRRGetScreenResourcesCurrent

namespace ZyPlatform
{
    /// \brief Opens the system's X11 libraries, which the window, the monitors and the OpenGL driver call into.
    class X11 final
    {
    public:

        /// \brief Opens the libraries on the first call, and answers every later call with how that one went.
        ///
        /// \return `true` when every library opened and every entry point was found, `false` otherwise.
        ZY_INLINE static Bool Open()
        {
            static const Bool Opened = Load();
            return Opened;
        }

    private:

        /// \brief Opens the libraries, which stay open until the process ends, and takes every entry point out of them.
        ///
        /// \return `true` when every library opened and every entry point was found, `false` otherwise.
        ZY_INLINE static Bool Load()
        {
            static Library Xlib("libX11.so.6");
            static Library Xrandr("libXrandr.so.2");

            Bool Complete = Xlib.IsOpen() && Xrandr.IsOpen();

#define ZY_X11(Source, Name)                                          \
            zy##Name = Source.GetFunction<decltype(zy##Name)>(#Name); \
            Complete = Complete && (zy##Name != nullptr);
#include "X11.inl"
#undef ZY_X11

            if (!Complete)
            {
                LOG_E("Platform: Failed to load X11 (libX11.so.6 and libXrandr.so.2)");
            }
            return Complete;
        }
    };
}