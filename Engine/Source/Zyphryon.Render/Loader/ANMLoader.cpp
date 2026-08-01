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

#include "ANMLoader.hpp"
#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Render/Resource/Animation.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    static void ReadTrack(Ref<Reader> Input, Ref<Track<Type>> Track, Real32 Rate, UInt32 Count, Interpolation Mode, Bool Tangents)
    {
        Track.SetRate(Rate);
        Track.SetInterpolation(Mode);
        Track.SetValues(Input.Read<ConstPtr<Type>>(Count * sizeof(Type)), Count);

        if (Tangents)
        {
            const ConstPtr<Type> Incoming = Input.Read<ConstPtr<Type>>(Count * sizeof(Type));
            const ConstPtr<Type> Outgoing = Input.Read<ConstPtr<Type>>(Count * sizeof(Type));
            Track.SetTangents(Incoming, Outgoing, Count);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ANMLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        Reader Input(Data.GetData(), Data.GetSize());

        if (Input.Read<UInt32>() != ('Z' | ('A' << 8) | ('N' << 16) | ('M' << 24)))
        {
            LOG_W("'{0}' is not a ZANM file (bad magic)", Scope.GetResource()->GetKey());
            return false;
        }

        const UInt16 Version = Input.Read<UInt16>();

        if (Version != 1)
        {
            LOG_W("'{0}' has an unsupported ZANM version", Scope.GetResource()->GetKey());
            return false;
        }

        const Real64 Duration = Input.Read<Real64>();
        const Sphere Extent   = Input.Read<Sphere>();
        const UInt32 Total    = Input.Read<UInt32>();

        Sequence<Animation::Lane> Lanes;
        Lanes.Reserve(Total);

        for (UInt32 Index = 0; Index < Total; ++Index)
        {
            const UInt64        Name  = Input.Read<UInt64>();
            const Real32        Rate  = Input.Read<Real32>();
            const UInt32        Count = Input.Read<UInt32>();
            const Component     Mask  = Input.Read<Component>();
            const Component     Fixed = Input.Read<Component>();
            const Interpolation Mode  = Input.Read<Interpolation>();

            if (Rate <= 0.0f || Count == 0)
            {
                LOG_W("'{0}' has lane {1} with no cadence or no keys", Scope.GetResource()->GetKey(), Index);
                return false;
            }

            // One lane per bone is a must.
            const auto IsPresent = [Name](ConstRef<Animation::Lane> Entry)
            {
                return Entry.Name == Name;
            };

            if (Lanes.Contains(IsPresent))
            {
                LOG_W("'{0}' has lane {1} repeating a bone", Scope.GetResource()->GetKey(), Index);
                return false;
            }

            // A component the baker found unchanging is stored once instead of once per tick.
            const auto GetRate = [Count, Fixed](Component Which)
            {
                return HasBit(Fixed, Which) ? 1u : Count;
            };

            const auto HasTangents = [Mode, Fixed](Component Which)
            {
                return Mode == Interpolation::Hermite && !HasBit(Fixed, Which);
            };

            const auto GetStride = [&](Component Which, UInt Size)
            {
                return GetRate(Which) * Size * (HasTangents(Which) ? 3 : 1);
            };

            const UInt Length =
                  (HasBit(Mask,Component::Position) ? GetStride(Component::Position, sizeof(Vector3))    : 0)
                + (HasBit(Mask,Component::Scale)    ? GetStride(Component::Scale,    sizeof(Vector3))    : 0)
                + (HasBit(Mask,Component::Rotation) ? GetStride(Component::Rotation, sizeof(Quaternion)) : 0);

            if (Input.GetAvailable() < Length)
            {
                LOG_W("'{0}' has lane {1} truncated", Scope.GetResource()->GetKey(), Index);
                return false;
            }

            Ref<Animation::Lane> Entry = Lanes.Append();
            Entry.Name   = Name;
            Entry.Timing = Cadence(Rate, Count);

            if (HasBit(Mask,Component::Position))
            {
                ReadTrack(Input, Entry.Position, Rate, GetRate(Component::Position), Mode, HasTangents(Component::Position));
            }

            if (HasBit(Mask,Component::Scale))
            {
                ReadTrack(Input, Entry.Scale, Rate, GetRate(Component::Scale), Mode, HasTangents(Component::Scale));
            }

            if (HasBit(Mask,Component::Rotation))
            {
                ReadTrack(Input, Entry.Rotation, Rate, GetRate(Component::Rotation), Mode, HasTangents(Component::Rotation));
            }
        }

        const Retainer<Animation> Asset = Retainer<Animation>::Cast(Scope.GetResource());
        Asset->Arrange(Move(Lanes));
        Asset->SetExtent(Extent);

        // Set after the lanes, since adopting them derives a duration from the keys that this replaces.
        if (Duration > 0.0)
        {
            Asset->SetDuration(Duration);
        }
        return true;
    }
}