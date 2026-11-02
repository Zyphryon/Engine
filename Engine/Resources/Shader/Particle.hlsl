// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_PARTICLE_INCLUDED
#define ZY_PARTICLE_INCLUDED

#include "Embedded://Shader/Noise.hlsl"

/// The emitter as ZyRender::Emitter::Pack writes it.
struct ZyEmitter
{
    float4 Origin;
    float4 AxisX;
    float4 AxisY;
    float4 AxisZ;
    float4 Cone;
    float4 Launch;
    float4 Pull;
    float4 Carry;
    float4 Sway;
    float4 Size;
    float4 Crop;
    float4 Card;
    float4 Colors[4];
    float4 Times;
    uint4  Count;
    uint4  Grid;
};

/// One particle as it stands at the emitter's moment.
struct ZyParticle
{
    float3 Position;
    float3 Velocity;
    float  Age;
    float  Elapsed;
    float  Size;
    float  Turn;
    float  Jitter;
    float4 Crop;
    float4 Color;
    bool   Alive;
};

/// Draws the next roll of a stream, from 0 to 1.
float ZyParticleRoll(inout uint Stream)
{
    Stream = ZyScatter(Stream + 0x9E3779B9u);

    return float(Stream >> 8u) * (1.0 / 16777216.0);
}

/// Turns a direction from the emitter's own frame by the way of its axes alone.
float3 ZyParticleTurn(ZyEmitter Emitter, float3 Direction)
{
    return ZySafeNormalize(Emitter.AxisX.xyz) * Direction.x
         + ZySafeNormalize(Emitter.AxisY.xyz) * Direction.y
         + ZySafeNormalize(Emitter.AxisZ.xyz) * Direction.z;
}

/// The colour a particle wears at a share of its life, held flat past the last stop.
float4 ZyParticleColor(ZyEmitter Emitter, float Elapsed)
{
    float4 Result = Emitter.Colors[0];

    [unroll]
    for (uint Next = 1u; Next < 4u; ++Next)
    {
        const float Span   = Emitter.Times[Next] - Emitter.Times[Next - 1u];
        const float Amount = (Span > 0.0)
            ? saturate((Elapsed - Emitter.Times[Next - 1u]) / Span)
            : step(Emitter.Times[Next], Elapsed);

        Result = lerp(Result, Emitter.Colors[Next], Amount);
    }
    return Result;
}

/// The particle a slot holds at the emitter's moment, reborn with rolls of its own every time it dies.
ZyParticle ZyParticleAt(ZyEmitter Emitter, uint Slot)
{
    ZyParticle Result;

    uint        Lasting = ZyScatter(Slot ^ ZyScatter(Emitter.Count.y));
    const float Lived   = lerp(Emitter.Launch.z, Emitter.Launch.w, ZyParticleRoll(Lasting));
    const float Life    = max(Lived, ZY_EPSILON);

    const float Clock = Emitter.Carry.w + ZyParticleRoll(Lasting) * Life;
    const float Cycle = floor(Clock / Life);
    const float Age   = Clock - Cycle * Life;

    uint         Stream = ZyScatter(Lasting ^ ZyScatter(uint(Cycle)));
    const float2 Spot   = float2(ZyParticleRoll(Stream), ZyParticleRoll(Stream));
    const float  Height = ZyParticleRoll(Stream);

    float3 Local = 0.0;

    if (Emitter.Count.z == 1u)
    {
        Local = float3(Spot.x - 0.5, Height, Spot.y - 0.5);
    }
    else if (Emitter.Count.z == 2u)
    {
        const float Radius = sqrt(Spot.x) * 0.5;
        const float Angle  = Spot.y * ZY_TWO_PI;

        Local = float3(cos(Angle) * Radius, Height, sin(Angle) * Radius);
    }

    const float3 Forward = Emitter.Cone.xyz;
    const float3 Aside   = normalize(abs(Forward.y) > 0.99
        ? cross(Forward, float3(1.0, 0.0, 0.0))
        : cross(Forward, float3(0.0, 1.0, 0.0)));
    const float3 Above   = cross(Forward, Aside);
    const float  Bearing = ZyParticleRoll(Stream) * ZY_TWO_PI;
    const float  Lean    = lerp(Emitter.Cone.w, 1.0, ZyParticleRoll(Stream));
    const float  Open    = sqrt(max(0.0, 1.0 - Lean * Lean));
    const float3 Heading = Forward * Lean + (Aside * cos(Bearing) + Above * sin(Bearing)) * Open;
    const float  Speed   = lerp(Emitter.Launch.x, Emitter.Launch.y, ZyParticleRoll(Stream));
    const float  Jitter  = ZyParticleRoll(Stream);
    const uint   Variant = Stream;

    const float3 Thrown  = ZyParticleTurn(Emitter, Heading) * Speed;
    const float3 Pull    = ZyParticleTurn(Emitter, Emitter.Pull.xyz);
    const float  Drag    = Emitter.Pull.w;

    float3 Velocity;
    float3 Travel;

    if (Drag > ZY_EPSILON)
    {
        const float  Fade     = exp(-Drag * Age);
        const float3 Terminal = Pull / Drag;

        Velocity = Terminal + (Thrown - Terminal) * Fade;
        Travel   = Terminal * Age + (Thrown - Terminal) * ((1.0 - Fade) / Drag);
    }
    else
    {
        Velocity = Thrown + Pull * Age;
        Travel   = Thrown * Age + Pull * (0.5 * Age * Age);
    }

    const float Elapsed = Age / Life;

    float3 Wander = 0.0;

    if (Emitter.Sway.x != 0.0)
    {
        const float Phase = Age * Emitter.Sway.y + Jitter * ZY_TWO_PI;

        Wander = float3(sin(Phase), 0.0, sin(Phase * 0.73 + Jitter * 5.0)) * (Emitter.Sway.x * Elapsed);
    }

    const float3 Born = Emitter.Origin.xyz
                      + Emitter.AxisX.xyz * Local.x + Emitter.AxisY.xyz * Local.y + Emitter.AxisZ.xyz * Local.z;

    float3 Position = Born + Travel + Emitter.Carry.xyz * Age + ZyParticleTurn(Emitter, Wander);

    const bool Landed = Emitter.Count.w != 0u && Position.y < Emitter.Origin.y;

    if (Landed)
    {
        Position.y = Emitter.Origin.y;
        Velocity   = 0.0;
    }

    const float Growth = Emitter.Size.z;
    const float Grown  = (Elapsed < Growth)
        ? Emitter.Size.x * (Elapsed / Growth)
        : lerp(Emitter.Size.x, Emitter.Size.y, (Elapsed - Growth) / (1.0 - Growth));
    const float Pick   = Jitter * 61.7;
    const float Start  = Emitter.Sway.z * (Jitter * 2.0 - 1.0);
    const float Rate   = Emitter.Sway.w * (frac(Pick) * 2.0 - 1.0);

    const uint   Columns = max(Emitter.Grid.x, 1u);
    const uint   Cells   = Columns * max(Emitter.Grid.y, 1u);
    const uint   Index   = (Emitter.Grid.z != 0u) ? min(uint(Elapsed * float(Cells)), Cells - 1u) : Variant % Cells;
    const float2 Cell    = Emitter.Crop.zw / float2(Columns, Cells / Columns);

    Result.Position = Position;
    Result.Velocity = Velocity + Emitter.Carry.xyz;
    Result.Age      = Age;
    Result.Elapsed  = Elapsed;
    Result.Size     = Grown * (1.0 + Emitter.Size.w * (Jitter * 2.0 - 1.0));
    Result.Turn     = radians(Start + Rate * Age);
    Result.Jitter   = Jitter;
    Result.Crop     = float4(Emitter.Crop.xy + float2(Index % Columns, Index / Columns) * Cell, Cell);
    Result.Color    = ZyParticleColor(Emitter, Elapsed);
    Result.Alive    = Lived > 0.0 && !(Landed && Emitter.Count.w == 1u);
    return Result;
}

#endif // ZY_PARTICLE_INCLUDED
