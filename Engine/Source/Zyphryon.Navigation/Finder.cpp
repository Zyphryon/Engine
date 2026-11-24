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

#include "Finder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNavigation
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Finder::Outcome Finder::Find(
        ConstRef<Chart> Map,
        IntVector2      Anchor,
        Vector3         From,
        Vector3         Into,
        Real32          Reach,
        UInt32          Limit,
        Ref<Way>        Way)
    {
        Way.Clear();

        mAnchor = Anchor;

        const UInt32 Start = Map.Snap(mAnchor, From, Reach);
        const UInt32 Goal  = Map.Snap(mAnchor, Into, Reach);

        if (Start == Chart::kNowhere || Goal == Chart::kNowhere)
        {
            return Outcome::None;
        }

        if (Start == Goal)
        {
            Way.Append(From);
            Way.Append(Into);

            return Outcome::Reached;
        }

        Begin(Map);

        mFrontier.Clear();

        // The search runs on the ground, and the way gets its heights once it is pulled taut.
        const Vector2 Aim = Into.GetXZ();

        // The estimate assumes the cheapest ground, so it never tops the real cost.
        const Real32 Guess = Map.GetCheapest() * kGuess;
        const Real32 Final = Map.GetCost(Goal);

        Ref<Node> First = mNodes[Map.GetIndex(Start)];
        First.Cost   = 0.0f;
        First.Entry  = From.GetXZ();
        First.Closed = false;
        First.Epoch  = mEpoch;

        // The start is alone in the frontier, so its rank does not matter.
        mFrontier.Push(0.0f, Start);

        // Keep the expanded polygon nearest the goal, to walk toward a goal that is cut off or past the limit.
        UInt32 Nearest = Start;
        Real32 Closest = kMaximum<Real32>;
        Bool   Arrived = false;

        // Only expanded polygons count against the limit, not the stale copies skipped.
        for (UInt32 Stepped = 0; !mFrontier.IsEmpty() && (Limit == 0 || Stepped < Limit); )
        {
            const UInt32 At = mFrontier.Pop().Polygon;

            Ref<Node> Here = mNodes[Map.GetIndex(At)];

            // A polygon is pushed again for every cheaper way to it, so the costlier copies are skipped.
            if (Here.Closed)
            {
                continue;
            }
            Here.Closed = true;
            ++Stepped;

            if (At == Goal)
            {
                Arrived = true;
                break;
            }

            const Real32  Spent     = Here.Cost;
            const Vector2 Entry     = Here.Entry;
            const Vector2 Direction = Aim - Entry;
            const Real32  Toll      = Map.GetCost(At);

            if (const Real32 Apart = Aim.GetDistance(Entry); Apart < Closest)
            {
                Closest = Apart;
                Nearest = At;
            }

            Map.Visit(mAnchor, At, [&](UInt32 Next, Vector2 Left, Vector2 Right)
            {
                if (Next == Chart::kNowhere)
                {
                    return;
                }

                Ref<Node> There = mNodes[Map.GetIndex(Next)];

                if (There.Epoch == mEpoch && There.Closed)
                {
                    return;
                }

                // Cross the side where the straight line to the goal meets it, clamped to its ends.
                const Vector2 Along  = Right - Left;
                const Vector2 Offset = Entry - Left;
                const Real32  Spread = Vector2::Cross(Along, Direction);
                const Real32  Part   = Vector2::Cross(Offset, Direction);
                const Real32  Share  = Abs(Spread) > 1e-6f ? Clamp(Part / Spread, 0.0f, 1.0f) : 0.5f;

                const Vector2 Crossing = Left + Along * Share;
                const Real32  Ahead    = Aim.GetDistance(Crossing);

                // Each stretch is weighed by its polygon's cost, and the stretch into the goal is added in full.
                const Real32 Cost = Spent + Crossing.GetDistance(Entry) * Toll + (Next == Goal ? Ahead * Final : 0.0f);

                if (There.Epoch == mEpoch && There.Cost <= Cost)
                {
                    return;
                }

                There.Cost   = Cost;
                There.Entry  = Crossing;
                There.Left   = Left;
                There.Right  = Right;
                There.Parent = At;
                There.Closed = false;
                There.Epoch  = mEpoch;

                mFrontier.Push(Next == Goal ? Cost : Cost + Ahead * Guess, Next);
            });
        }

        // An unreached goal is replaced by the nearest point of the polygon that got closest to it.
        const UInt32 Last = Arrived ? Goal : Nearest;

        if (Last == Start)
        {
            return Outcome::None;
        }

        if (!Arrived)
        {
            Into = Map.GetNearest(mAnchor, Last, Into);
        }

        // The end goes in first as a side of no length, then the sides crossed back to the start.
        mSides.Clear();
        mSides.Append(Into.GetXZ(), Into.GetXZ());

        for (UInt32 At = Last; At != Start; At = mNodes[Map.GetIndex(At)].Parent)
        {
            ConstRef<Node> Each = mNodes[Map.GetIndex(At)];

            mSides.Append(Each.Left, Each.Right, At);
        }

        Pull(Map, From, Into, Way);
        return Arrived ? Outcome::Reached : Outcome::Partial;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Finder::Glide(
        ConstRef<Chart> Map,
        IntVector2      Anchor,
        UInt32          Polygon,
        Vector2         From,
        Vector2         Into,
        Ref<Vector3>    Stop)
    {
        mAnchor = Anchor;

        // Walk the straight line first, polygon by polygon, which keeps the point on the floor it started on.
        for (UInt32 At = Polygon, Before = Chart::kNowhere, Step = 0; At != Chart::kNowhere && Step < kStride; ++Step)
        {
            Bool   Inside = true;
            UInt32 Onto   = Chart::kNowhere;

            Map.Visit(mAnchor, At, [&](UInt32 Next, Vector2 Left, Vector2 Right)
            {
                if (Vector2::Cross(Left, Right, Into) > 0.0f)
                {
                    Inside = false;

                    // The line leaves through the side whose ends lie on either side of it.
                    if (Next != Before)
                    {
                        if (Vector2::Cross(From, Into, Left) >= 0.0f && Vector2::Cross(From, Into, Right) <= 0.0f)
                        {
                            Onto = Next;
                        }
                    }
                }
            });

            if (Inside)
            {
                Stop = Vector3::FromXZ(Into, Map.GetHeight(mAnchor, At, Into));
                return At;
            }

            Before = At;
            At     = Onto;
        }

        Begin(Map);

        // Only visit polygons with a side inside the circle around the move.
        const Vector2 Middle = (From + Into) * 0.5f;
        const Real32  Radius = Into.GetDistance(From) * 0.5f + 0.001f;

        UInt32  Nearest = Polygon;
        Vector2 Landed  = From;
        Real32  Closest = kMaximum<Real32>;

        mQueue.Clear();
        mQueue.Append(Polygon);

        mNodes[Map.GetIndex(Polygon)].Epoch = mEpoch;

        for (UInt32 Read = 0; Read < mQueue.GetSize(); ++Read)
        {
            const UInt32 At = mQueue[Read];

            // Polygons wind so a point inside lies right of every side.
            Bool Inside = true;

            Map.Visit(mAnchor, At, [&](UInt32 Next, Vector2 Left, Vector2 Right)
            {
                Inside = Inside && Vector2::Cross(Left, Right, Into) <= 0.0f;

                if (Next != Chart::kNowhere)
                {
                    Ref<Node> There = mNodes[Map.GetIndex(Next)];

                    if (There.Epoch != mEpoch && Line(Left, Right).GetDistance(Middle) <= Radius)
                    {
                        There.Epoch = mEpoch;
                        mQueue.Append(Next);
                    }
                    return;
                }

                // A wall stops the move at its point nearest the target.
                const Vector2 Point = Line(Left, Right).GetNearest(Into);
                const Real32  Apart = Into.GetDistance(Point);

                if (Apart < Closest)
                {
                    Closest = Apart;
                    Landed  = Point;
                    Nearest = At;
                }
            });

            if (Inside)
            {
                Stop = Vector3::FromXZ(Into, Map.GetHeight(mAnchor, At, Into));
                return At;
            }
        }

        Stop = Vector3(Landed.GetX(), Map.GetHeight(mAnchor, Nearest, Landed), Landed.GetY());
        return Nearest;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Finder::Begin(ConstRef<Chart> Map)
    {
        // A node is live only when it carries this search's stamp, so nothing is cleared between searches.
        mNodes.Expand(Map.GetCount());

        if (++mEpoch == 0)
        {
            for (UInt32 Index = 0; Index < mNodes.GetSize(); ++Index)
            {
                mNodes[Index].Epoch = 0;
            }
            mEpoch = 1;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Finder::Pull(ConstRef<Chart> Map, Vector3 From, Vector3 Into, Ref<Way> Way)
    {
        // The sides were gathered from the goal back, so they are read in reverse.
        const SInt32 Last = static_cast<SInt32>(mSides.GetSize()) - 1;

        Vector2 Apex    = From.GetXZ();
        Vector2 Left    = Apex;
        Vector2 Right   = Apex;
        SInt32  ApexAt  = -1;
        SInt32  LeftAt  = -1;
        SInt32  RightAt = -1;

        Way.Append(From);

        for (SInt32 Index = 0; Index <= Last; ++Index)
        {
            ConstRef<Side> Each   = mSides[Last - Index];
            Bool           Turned = false;

            // The right edge narrows while the new end stays inside the funnel, and turns a corner once past the left.
            if (Vector2::Cross(Apex, Right, Each.Right) >= 0.0f)
            {
                if (Apex == Right || Vector2::Cross(Apex, Left, Each.Right) < 0.0f)
                {
                    Right   = Each.Right;
                    RightAt = Index;
                }
                else
                {
                    Apex   = Left;
                    ApexAt = LeftAt;
                    Turned = true;
                }
            }

            if (!Turned && Vector2::Cross(Apex, Left, Each.Left) <= 0.0f)
            {
                if (Apex == Left || Vector2::Cross(Apex, Right, Each.Left) > 0.0f)
                {
                    Left   = Each.Left;
                    LeftAt = Index;
                }
                else
                {
                    Apex   = Right;
                    ApexAt = RightAt;
                    Turned = true;
                }
            }

            if (!Turned)
            {
                continue;
            }

            // A corner takes the height of the floor past its side, and the end is appended once after the loop.
            if (ApexAt < Last)
            {
                Way.Append(Vector3::FromXZ(Apex, Map.GetHeight(mAnchor, mSides[Last - ApexAt].Into, Apex)));
            }

            // Restart the funnel at the corner, from the side it lies on.
            Left    = Apex;
            Right   = Apex;
            LeftAt  = ApexAt;
            RightAt = ApexAt;
            Index   = ApexAt;
        }

        Way.Append(Into);
    }
}