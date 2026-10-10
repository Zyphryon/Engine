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
#include <shellscalingapi.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyPlatform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static BOOL CALLBACK OnEnumerateMonitor(HMONITOR Handle, HDC, LPRECT, LPARAM Context)
    {
        const Ptr<Sequence<Monitor>> Output = reinterpret_cast<Ptr<Sequence<Monitor>>>(Context);

        MONITORINFOEXW Info { };
        Info.cbSize = sizeof(Info);

        if (::GetMonitorInfoW(Handle, AddressOf(Info)))
        {
            Ref<Monitor> Monitor = Output->Append(Handle);
            Monitor.SetName(Str::ConvertFromUTF16(StrConvert(Info.szDevice)));
            Monitor.SetX(Info.rcMonitor.left);
            Monitor.SetY(Info.rcMonitor.top);
            Monitor.SetWidth(Info.rcMonitor.right - Info.rcMonitor.left);
            Monitor.SetHeight(Info.rcMonitor.bottom - Info.rcMonitor.top);

            if (HasBit(Info.dwFlags, MONITORINFOF_PRIMARY))
            {
                Monitor.SetAttribute(Monitor::Attribute::Primary);
            }

            DEVICE_SCALE_FACTOR Factor;
            if (FAILED(::GetScaleFactorForMonitor(Handle, AddressOf(Factor))))
            {
                Factor = DEVICE_SCALE_FACTOR::SCALE_100_PERCENT;
            }
            Monitor.SetScale(static_cast<Real32>(Factor) * 0.01f);
        }
        return TRUE;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Display::Poll()
    {
        mMonitors.Clear();

        ::EnumDisplayMonitors(nullptr, nullptr, OnEnumerateMonitor, reinterpret_cast<LPARAM>(AddressOf(mMonitors)));

        // The friendly names, HDR state and refresh rates come from the DisplayConfig paths, matched by GDI name.
        UINT32 PathCount = 0;
        UINT32 ModeCount = 0;

        if (::GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, AddressOf(PathCount), AddressOf(ModeCount)) != ERROR_SUCCESS)
        {
            return;
        }

        Sequence<DISPLAYCONFIG_PATH_INFO> Paths(PathCount);
        Sequence<DISPLAYCONFIG_MODE_INFO> Modes(ModeCount);

        Paths.Advance(PathCount);
        Modes.Advance(ModeCount);

        if (::QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, AddressOf(PathCount), Paths.GetData(), AddressOf(ModeCount), Modes.GetData(), nullptr) != ERROR_SUCCESS)
        {
            return;
        }

        for (ConstRef<DISPLAYCONFIG_PATH_INFO> Path : Paths)
        {
            DISPLAYCONFIG_SOURCE_DEVICE_NAME SourceName { };
            SourceName.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            SourceName.header.size      = sizeof(SourceName);
            SourceName.header.adapterId = Path.sourceInfo.adapterId;
            SourceName.header.id        = Path.sourceInfo.id;

            if (::DisplayConfigGetDeviceInfo(AddressOf(SourceName.header)) != ERROR_SUCCESS)
            {
                continue;
            }

            const Str  Name = Str::ConvertFromUTF16(StrConvert(SourceName.viewGdiDeviceName));
            const SInt Slot = mMonitors.Find([&](ConstRef<Monitor> Monitor)
            {
                return Monitor.GetName() == Name;
            });

            if (Slot < 0)
            {
                continue;
            }

            Ref<Monitor> Entry = mMonitors[Slot];

            DISPLAYCONFIG_TARGET_DEVICE_NAME TargetName { };
            TargetName.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
            TargetName.header.size      = sizeof(TargetName);
            TargetName.header.adapterId = Path.targetInfo.adapterId;
            TargetName.header.id        = Path.targetInfo.id;

            // Built-in panels often report no friendly name, and those keep their GDI one.
            if (::DisplayConfigGetDeviceInfo(AddressOf(TargetName.header)) == ERROR_SUCCESS && TargetName.monitorFriendlyDeviceName[0] != L'\0')
            {
                Entry.SetName(Str::ConvertFromUTF16(StrConvert(TargetName.monitorFriendlyDeviceName)));
            }

            DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO ColorInfo { };
            ColorInfo.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
            ColorInfo.header.size      = sizeof(ColorInfo);
            ColorInfo.header.adapterId = Path.targetInfo.adapterId;
            ColorInfo.header.id        = Path.targetInfo.id;

            if (::DisplayConfigGetDeviceInfo(AddressOf(ColorInfo.header)) == ERROR_SUCCESS)
            {
                if (ColorInfo.advancedColorEnabled != 0)
                {
                    Entry.SetAttribute(Monitor::Attribute::HDR);
                }
            }

            if (Path.targetInfo.refreshRate.Denominator != 0)
            {
                Entry.SetFrequency(Path.targetInfo.refreshRate.Numerator / Path.targetInfo.refreshRate.Denominator);
            }
        }
    }
}