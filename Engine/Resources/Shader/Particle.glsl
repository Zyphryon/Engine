// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_PARTICLE_INCLUDED
#define ZY_PARTICLE_INCLUDED

#include "Embedded://Shader/Noise.glsl"

/// The emitter as ZyRender::Emitter::Pack writes it.
struct ZyEmitter
{
    vec4  Origin;
    vec4  AxisX;
    vec4  AxisY;
    vec4  AxisZ;
    vec4  Cone;
    vec4  Launch;
    vec4  Pull;
    vec4  Carry;
    vec4  Sway;
    vec4  Size;
    vec4  Crop;
    vec4  Card;
    vec4  Stops[4];
    vec4  Times;
    uvec4 Count;
    uvec4 Grid;
};

/// One particle as it stands at the emitter's moment.
struct ZyParticle
{
    vec3  Position;
    vec3  Velocity;
    float Age;
    float Elapsed;
    float Size;
    float Turn;
    float Jitter;
    vec4  Crop;
    vec4  Color;
    bool  Alive;
};

/// Draws the next roll of a stream, from 0 to 1.
float ZyParticleRoll(inout uint Stream)
{
    Stream = ZyScatter(Stream + 0x9E3779B9u);

    return float(Stream >> 8u) * (1.0 / 16777216.0);
}

/// Turns a direction from the emitter's own frame by the way of its axes alone.
vec3 ZyParticleTurn(ZyEmitter Emitter, vec3 Direction)
{
    return ZySafeNormalize(Emitter.AxisX.xyz) * Direction.x
         + ZySafeNormalize(Emitter.AxisY.xyz) * Direction.y
         + ZySafeNormalize(Emitter.AxisZ.xyz) * Direction.z;
}

/// The colour a particle wears at a share of its life, held flat past the last stop.
vec4 ZyParticleColor(ZyEmitter Emitter, float Elapsed)
{
    vec4 Result = Emitter.Stops[0];

    for (int Next = 1; Next < 4; ++Next)
    {
        float Span   = Emitter.Times[Next] - Emitter.Times[Next - 1];
        float Amount = (Span > 0.0)
            ? clamp((Elapsed - Emitter.Times[Next - 1]) / Span, 0.0, 1.0)
            : step(Emitter.Times[Next], Elapsed);

        Result = mix(Result, Emitter.Stops[Next], Amount);
    }
    return Result;
}

/// The particle a slot holds at the emitter's moment, reborn with rolls of its own every time it dies.
ZyParticle ZyParticleAt(ZyEmitter Emitter, uint Slot)
{
    ZyParticle Result;

    uint  Lasting = ZyScatter(Slot ^ ZyScatter(Emitter.Count.y));
    float Lived   = mix(Emitter.Launch.z, Emitter.Launch.w, ZyParticleRoll(Lasting));
    float Life    = max(Lived, ZY_EPSILON);
    float Clock = Emitter.Carry.w + ZyParticleRoll(Lasting) * Life;
    float Cycle = floor(Clock / Life);
    float Age   = Clock - Cycle * Life;

    uint Stream = ZyScatter(Lasting ^ ZyScatter(uint(Cycle)));

    vec3 Local = vec3(0.0);

    vec2  Spot   = vec2(ZyParticleRoll(Stream), ZyParticleRoll(Stream));
    float Height = ZyParticleRoll(Stream);

    if (Emitter.Count.z == 1u)
    {
        Local = vec3(Spot.x - 0.5, Height, Spot.y - 0.5);
    }
    else if (Emitter.Count.z == 2u)
    {
        float Radius = sqrt(Spot.x) * 0.5;
        float Angle  = Spot.y * ZY_TWO_PI;

        Local = vec3(cos(Angle) * Radius, Height, sin(Angle) * Radius);
    }

    vec3  Forward = Emitter.Cone.xyz;
    vec3  Aside   = normalize(abs(Forward.y) > 0.99
        ? cross(Forward, vec3(1.0, 0.0, 0.0))
        : cross(Forward, vec3(0.0, 1.0, 0.0)));
    vec3  Above   = cross(Forward, Aside);
    float Bearing = ZyParticleRoll(Stream) * ZY_TWO_PI;
    float Lean    = mix(Emitter.Cone.w, 1.0, ZyParticleRoll(Stream));
    float Open    = sqrt(max(0.0, 1.0 - Lean * Lean));
    vec3  Heading = Forward * Lean + (Aside * cos(Bearing) + Above * sin(Bearing)) * Open;
    float Speed   = mix(Emitter.Launch.x, Emitter.Launch.y, ZyParticleRoll(Stream));
    float Jitter  = ZyParticleRoll(Stream);
    uint  Variant = Stream;

    vec3  Thrown  = ZyParticleTurn(Emitter, Heading) * Speed;
    vec3  Pull    = ZyParticleTurn(Emitter, Emitter.Pull.xyz);
    float Drag    = Emitter.Pull.w;

    vec3 Velocity;
    vec3 Travel;

    if (Drag > ZY_EPSILON)
    {
        float Fade     = exp(-Drag * Age);
        vec3  Terminal = Pull / Drag;

        Velocity = Terminal + (Thrown - Terminal) * Fade;
        Travel   = Terminal * Age + (Thrown - Terminal) * ((1.0 - Fade) / Drag);
    }
    else
    {
        Velocity = Thrown + Pull * Age;
        Travel   = Thrown * Age + Pull * (0.5 * Age * Age);
    }

    float Elapsed = Age / Life;

    vec3 Wander = vec3(0.0);

    if (Emitter.Sway.x != 0.0)
    {
        float Phase = Age * Emitter.Sway.y + Jitter * ZY_TWO_PI;

        Wander = vec3(sin(Phase), 0.0, sin(Phase * 0.73 + Jitter * 5.0)) * (Emitter.Sway.x * Elapsed);
    }

    vec3 Born = Emitter.Origin.xyz
              + Emitter.AxisX.xyz * Local.x + Emitter.AxisY.xyz * Local.y + Emitter.AxisZ.xyz * Local.z;

    vec3 Position = Born + Travel + Emitter.Carry.xyz * Age + ZyParticleTurn(Emitter, Wander);

    bool Landed = Emitter.Count.w != 0u && Position.y < Emitter.Origin.y;

    if (Landed)
    {
        Position.y = Emitter.Origin.y;
        Velocity   = vec3(0.0);
    }

    float Growth = Emitter.Size.z;
    float Grown  = (Elapsed < Growth)
        ? Emitter.Size.x * (Elapsed / Growth)
        : mix(Emitter.Size.x, Emitter.Size.y, (Elapsed - Growth) / (1.0 - Growth));
    float Pick   = Jitter * 61.7;
    float Start  = Emitter.Sway.z * (Jitter * 2.0 - 1.0);
    float Rate   = Emitter.Sway.w * (fract(Pick) * 2.0 - 1.0);

    uint Columns = max(Emitter.Grid.x, 1u);
    uint Cells   = Columns * max(Emitter.Grid.y, 1u);
    uint Index   = (Emitter.Grid.z != 0u) ? min(uint(Elapsed * float(Cells)), Cells - 1u) : Variant % Cells;
    vec2 Cell    = Emitter.Crop.zw / vec2(float(Columns), float(Cells / Columns));

    Result.Position = Position;
    Result.Velocity = Velocity + Emitter.Carry.xyz;
    Result.Age      = Age;
    Result.Elapsed  = Elapsed;
    Result.Size     = Grown * (1.0 + Emitter.Size.w * (Jitter * 2.0 - 1.0));
    Result.Turn     = radians(Start + Rate * Age);
    Result.Jitter   = Jitter;
    Result.Crop     = vec4(Emitter.Crop.xy + vec2(float(Index % Columns), float(Index / Columns)) * Cell, Cell);
    Result.Color    = ZyParticleColor(Emitter, Elapsed);
    Result.Alive    = Lived > 0.0 && !(Landed && Emitter.Count.w == 1u);
    return Result;
}

#endif // ZY_PARTICLE_INCLUDED
