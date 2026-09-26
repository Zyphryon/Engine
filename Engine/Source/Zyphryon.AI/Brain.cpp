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

#include "Brain.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAI
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Brain::Brain()
        : mClock { 0.0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Status Brain::Tick(ConstRef<Behaviour> Tree, Real32 Delta, Ref<Host> Host)
    {
        const ConstSpan<Behaviour::Node> Nodes = Tree.GetNodes();

        if (Nodes.IsEmpty())
        {
            return Status::Failure;
        }

        if (mMemory.GetSize() != Nodes.GetSize())
        {
            mMemory.Clear();
            mMemory.Resize(Nodes.GetSize());
        }

        mClock += Delta;

        return Visit(Nodes, 0, Host);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Brain::Halt(ConstRef<Behaviour> Tree, Ref<Host> Host)
    {
        const ConstSpan<Behaviour::Node> Nodes = Tree.GetNodes();

        if (!Nodes.IsEmpty() && mMemory.GetSize() == Nodes.GetSize())
        {
            Stop(Nodes, 0, Host);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Brain::Reset()
    {
        mMemory.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Status Brain::Visit(ConstSpan<Behaviour::Node> Nodes, UInt32 Index, Ref<Host> Host)
    {
        ConstRef<Behaviour::Node> Current = Nodes[Index];

        const Bool   Fresh = (mMemory[Index].Last != Status::Running);
        const UInt32 First = Index + 1;
        const UInt32 Last  = Index + Current.Span;

        Status Result = Status::Failure;

        switch (Current.Type)
        {
        case Behaviour::Kind::Leaf:
        {
            if (Fresh)
            {
                mMemory[Index].Mark = mClock;
            }
            Result = Host.Tick(Index, Fresh, mClock - mMemory[Index].Mark, mBoard);
            break;
        }
        case Behaviour::Kind::Compare:
        {
            Result = Compare(Current) ? Status::Success : Status::Failure;
            break;
        }
        case Behaviour::Kind::Sequence:
        case Behaviour::Kind::Selector:
        {
            const Bool   Ordered = (Current.Type == Behaviour::Kind::Sequence);
            const Status Ending  = Ordered ? Status::Failure : Status::Success;

            if (Fresh)
            {
                mMemory[Index].Cursor = static_cast<UInt16>(First);
            }

            Result = Ordered ? Status::Success : Status::Failure;

            for (UInt32 Child = mMemory[Index].Cursor; Child < Last; Child += Nodes[Child].Span)
            {
                const Status Got = Visit(Nodes, Child, Host);

                if (Got == Status::Running)
                {
                    mMemory[Index].Cursor = static_cast<UInt16>(Child);
                    Result = Status::Running;
                    break;
                }

                if (Got == Ending)
                {
                    Result = Ending;
                    break;
                }
            }
            break;
        }
        case Behaviour::Kind::Parallel:
        {
            const UInt32 Needed = (Current.Count > 0 && Current.Count < Current.Children)
                ? Current.Count
                : Current.Children;

            UInt32 Successes = 0;
            UInt32 Failures  = 0;

            for (UInt32 Child = First; Child < Last; Child += Nodes[Child].Span)
            {
                const Status Kept = mMemory[Child].Last;
                const Status Got  = (!Fresh && (Kept == Status::Success || Kept == Status::Failure))
                    ? Kept
                    : Visit(Nodes, Child, Host);

                Successes += (Got == Status::Success);
                Failures  += (Got == Status::Failure);
            }

            if (Successes >= Needed)
            {
                Result = Status::Success;
            }
            else if (Failures > Current.Children - Needed)
            {
                Result = Status::Failure;
            }
            else
            {
                Result = Status::Running;
            }

            // Stopping the whole subtree also idles this node, which is overwritten with the result below.
            if (Result != Status::Running)
            {
                Stop(Nodes, Index, Host);
            }
            break;
        }
        case Behaviour::Kind::Invert:
        {
            Result = Visit(Nodes, First, Host);

            if (Result == Status::Success)
            {
                Result = Status::Failure;
            }
            else if (Result == Status::Failure)
            {
                Result = Status::Success;
            }
            break;
        }
        case Behaviour::Kind::Repeat:
        {
            if (Fresh)
            {
                mMemory[Index].Count = 0;
            }

            Result = Visit(Nodes, First, Host);

            if (Result == Status::Success)
            {
                ++mMemory[Index].Count;

                if (Current.Count == 0 || mMemory[Index].Count < Current.Count)
                {
                    Result = Status::Running;
                }
            }
            break;
        }
        case Behaviour::Kind::Cooldown:
        {
            if (Fresh && mClock < mMemory[Index].Mark)
            {
                Result = Status::Failure;
                break;
            }

            Result = Visit(Nodes, First, Host);

            if (Result != Status::Running)
            {
                mMemory[Index].Mark = mClock + Current.Value;
            }
            break;
        }
        case Behaviour::Kind::Timeout:
        {
            if (Fresh)
            {
                mMemory[Index].Mark = mClock;
            }

            if (mClock - mMemory[Index].Mark >= Current.Value)
            {
                Stop(Nodes, First, Host);
                Result = Status::Failure;
                break;
            }

            Result = Visit(Nodes, First, Host);
            break;
        }
        case Behaviour::Kind::Chance:
        {
            if (Fresh && !Host.Roll(static_cast<Real32>(Current.Value)))
            {
                Result = Status::Failure;
                break;
            }

            Result = Visit(Nodes, First, Host);
            break;
        }
        }

        mMemory[Index].Last = Result;
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Brain::Stop(ConstSpan<Behaviour::Node> Nodes, UInt32 Index, Ref<Host> Host)
    {
        const UInt32 Last = Index + Nodes[Index].Span;

        for (UInt32 Each = Index; Each < Last; ++Each)
        {
            if (mMemory[Each].Last != Status::Running)
            {
                continue;
            }

            if (Nodes[Each].Type == Behaviour::Kind::Leaf)
            {
                Host.Halt(Each);
            }

            mMemory[Each].Last = Status::Idle;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Brain::Compare(ConstRef<Behaviour::Node> Node) const
    {
        const ConstPtr<Blackboard::Value> Entry = mBoard.Lookup(Node.Name);

        if (Entry == nullptr)
        {
            return false;
        }

        if (Node.Check == Behaviour::Test::Exists)
        {
            return true;
        }

        // Only a flag or a number can be compared, and a flag reads as `0` or `1`.
        Real64 Number;

        if (const ConstPtr<Real64> Found = Entry->TryGet<Real64>())
        {
            Number = (* Found);
        }
        else if (const ConstPtr<Bool> Flag = Entry->TryGet<Bool>())
        {
            Number = (* Flag) ? 1.0 : 0.0;
        }
        else
        {
            return false;
        }

        switch (Node.Check)
        {
        case Behaviour::Test::Equal:
            return Number == Node.Value;
        case Behaviour::Test::NotEqual:
            return Number != Node.Value;
        case Behaviour::Test::Less:
            return Number < Node.Value;
        case Behaviour::Test::LessEqual:
            return Number <= Node.Value;
        case Behaviour::Test::Greater:
            return Number > Node.Value;
        case Behaviour::Test::GreaterEqual:
            return Number >= Node.Value;
        default:
            return false;
        }
    }
}