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
#include <X11/extensions/Xrandr.h>

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

    static UInt16 ConvertRefreshRate(ConstPtr<XRRScreenResources> Resources, RRMode Identity)
    {
        for (SInt32 Index = 0; Index < Resources->nmode; ++Index)
        {
            ConstRef<XRRModeInfo> Mode = Resources->modes[Index];

            if (Mode.id == Identity)
            {
                const UInt64 Total = static_cast<UInt64>(Mode.hTotal) * Mode.vTotal;

                return Total ? static_cast<UInt16>((Mode.dotClock + Total / 2) / Total) : 0;
            }
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Real32 ConvertScale(UInt32 Extent, UInt64 Length)
    {
        static constexpr Real32 kReferenceDPI = 96.0f;

        if (Length == 0)
        {
            return 1.0f;
        }

        const Real32 Scale = (static_cast<Real32>(Extent) * 25.4f) / (static_cast<Real32>(Length) * kReferenceDPI);
        return Scale > 0.0f ? Scale : 1.0f;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Display::Poll()
    {
        mMonitors.Clear();

        const Ptr<::Display> Connection = ::XOpenDisplay(nullptr);

        if (!Connection)
        {
            LOG_W("Platform: Failed to open a connection to the X server while enumerating monitors");
            return;
        }

        const ::Window Root = ::XDefaultRootWindow(Connection);

        const Ptr<XRRScreenResources> Resources = ::XRRGetScreenResourcesCurrent(Connection, Root);

        if (!Resources)
        {
            LOG_W("Platform: The X server reported no RandR screen resources");

            ::XCloseDisplay(Connection);
            return;
        }

        const RROutput Primary = ::XRRGetOutputPrimary(Connection, Root);

        for (SInt32 Index = 0; Index < Resources->noutput; ++Index)
        {
            const RROutput           Identity = Resources->outputs[Index];
            const Ptr<XRROutputInfo> Output   = ::XRRGetOutputInfo(Connection, Resources, Identity);

            if (!Output)
            {
                continue;
            }

            if (Output->connection == RR_Connected && Output->crtc != 0)
            {
                if (const Ptr<XRRCrtcInfo> Region = ::XRRGetCrtcInfo(Connection, Resources, Output->crtc))
                {
                    Ref<Monitor> Monitor = mMonitors.Append(
                        reinterpret_cast<Monitor::Handle>(static_cast<UInt>(Identity)));

                    Monitor.SetName(Text(Output->name, static_cast<UInt>(Output->nameLen)));
                    Monitor.SetX(Region->x);
                    Monitor.SetY(Region->y);
                    Monitor.SetWidth(Region->width);
                    Monitor.SetHeight(Region->height);
                    Monitor.SetFrequency(ConvertRefreshRate(Resources, Region->mode));

                    const Bool Rotated = (Region->rotation & (RR_Rotate_90 | RR_Rotate_270)) != 0;
                    Monitor.SetScale(ConvertScale(Region->width, Rotated ? Output->mm_height : Output->mm_width));

                    if (Identity == Primary)
                    {
                        Monitor.SetAttribute(Monitor::Attribute::Primary);
                    }

                    ::XRRFreeCrtcInfo(Region);
                }
            }

            ::XRRFreeOutputInfo(Output);
        }

        if (Primary == None && !mMonitors.IsEmpty())
        {
            mMonitors.GetFront().SetAttribute(Monitor::Attribute::Primary);
        }

        ::XRRFreeScreenResources(Resources);
        ::XCloseDisplay(Connection);
    }
}
