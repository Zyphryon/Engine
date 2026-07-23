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

namespace Platform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Display::Poll()
    {
        mMonitors.Clear();

        // Enumerate all monitors and populate basic information.

        static constexpr auto OnEnumerate = [](HMONITOR Handle, HDC, LPRECT, LPARAM Context) -> BOOL
        {
            const Ptr<Sequence<Monitor>> Output = reinterpret_cast<Ptr<Sequence<Monitor>>>(Context);

            MONITORINFOEXW Info { };
            Info.cbSize = sizeof(Info);

            if (GetMonitorInfoW(Handle, AddressOf(Info)))
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
        };
        EnumDisplayMonitors(nullptr, nullptr, OnEnumerate, reinterpret_cast<LPARAM>(AddressOf(mMonitors)));

        // Query advanced display configuration to retrieve detailed properties using the DisplayConfig API.

        UINT32 PathCount = 0;
        UINT32 ModeCount = 0;

        if (FAILED(::GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, AddressOf(PathCount), AddressOf(ModeCount))))
        {
            return;
        }

        Sequence<DISPLAYCONFIG_PATH_INFO> Paths(PathCount);
        Sequence<DISPLAYCONFIG_MODE_INFO> Modes(ModeCount);

        Paths.Advance(PathCount);
        Modes.Advance(ModeCount);

        if (FAILED(::QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, AddressOf(PathCount), Paths.GetData(), AddressOf(ModeCount), Modes.GetData(), nullptr)))
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

            if (FAILED(::DisplayConfigGetDeviceInfo(AddressOf(SourceName.header))))
            {
                continue;
            }

            const Str  Name = Str::ConvertFromUTF16(StrConvert(SourceName.viewGdiDeviceName));
            const SInt Slot = mMonitors.Find([Identity = Hash(Name)](ConstRef<Monitor> Monitor) -> Bool
            {
                return Hash(Monitor.GetName()) == Identity;
            });

            if (Slot >= 0)
            {
                Ref<Monitor> Entry = mMonitors[Slot];

                DISPLAYCONFIG_TARGET_DEVICE_NAME TargetName { };
                TargetName.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
                TargetName.header.size      = sizeof(TargetName);
                TargetName.header.adapterId = Path.targetInfo.adapterId;
                TargetName.header.id        = Path.targetInfo.id;

                if (SUCCEEDED(::DisplayConfigGetDeviceInfo(AddressOf(TargetName.header))))
                {
                    Entry.SetName(Str::ConvertFromUTF16(StrConvert(TargetName.monitorFriendlyDeviceName)));
                }

                DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO ColorInfo { };
                ColorInfo.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
                ColorInfo.header.size      = sizeof(ColorInfo);
                ColorInfo.header.adapterId = Path.targetInfo.adapterId;
                ColorInfo.header.id        = Path.targetInfo.id;

                if (SUCCEEDED(::DisplayConfigGetDeviceInfo(AddressOf(ColorInfo.header))))
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
}