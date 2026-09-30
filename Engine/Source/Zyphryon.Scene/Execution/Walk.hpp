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

#include "Plan.hpp"
#include "Zyphryon.Job/Service.hpp"
#include "Zyphryon.Scene/Entity.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the ways a callback visits a query's matches, on one thread, spread, or parents first.
    class ZY_API Walk final
    {
        template<typename> friend class Cursor;

    public:

        /// \brief Hands every match to a callback on this thread, holding every structural change until it ends.
        ///
        /// \param Owner    The storage the query belongs to.
        /// \param State    The query, whose fields are declared for the callback.
        /// \param Callback The callable.
        template<typename Callable>
        static void Run(Ref<Storage> Owner, Ref<Selection> State, Ref<Callable> Callback)
        {
            // A walk with no row to visit holds nothing, which is what most idle systems cost.
            UInt32 First = 0;

            while (First < State.Chunks.GetSize() && State.Chunks[First]->GetSize() == 0)
            {
                ++First;
            }

            if (First == State.Chunks.GetSize() || !HasGlobals(Owner, State))
            {
                return;
            }

            Owner.Enter();

            if (State.Ordered)
            {
                RunOrdered(Owner, State, Callback);
            }
            else
            {
                for (UInt32 Match = First; Match < State.Chunks.GetSize(); ++Match)
                {
                    if (const UInt32 Rows = State.Chunks[Match]->GetSize())
                    {
                        RunChunk(Owner, State, Match, 0, Rows, Callback);
                    }
                }
            }

            Owner.Leave();
        }

        /// \brief Hands every match to a callback, spread over the workers when the last walk says it pays.
        ///
        /// \param Owner    The storage the query belongs to.
        /// \param State    The query, whose fields are declared for the callback and which is not ordered.
        /// \param Callback The callable, safe to call from many threads at once and writing only in place.
        template<typename Callable>
        static void Spread(Ref<Storage> Owner, Ref<Selection> State, Ref<Callable> Callback)
        {
            ZY_ASSERT(!State.Ordered, "A cascade visits parents first, which a spread walk cannot promise");
            ZY_ASSERT(!Owner.mDeferral.IsSpreading(), "A spread walk cannot start inside another");

            State.Spread = false;

            if (!HasGlobals(Owner, State))
            {
                return;
            }

            // Every match's rows laid end to end, so shares cut across chunks.
            UInt32 Total = 0;
            State.Offsets.Clear();

            for (const Ptr<Chunk> Target : State.Chunks)
            {
                State.Offsets.Append(Total);
                Total += Target->GetSize();
            }
            State.Offsets.Append(Total);

            if (Total == 0)
            {
                return;
            }

            Owner.Enter();

            // The first walk runs here to time a row, which later walks decide by and spread walks only lower.
            if (State.Cost * Total > kSpreadSeconds)
            {
                State.Pace   = Distribute(Owner, State, 0, Total, Callback);
                State.Cost   = Min(State.Cost, State.Pace);
                State.Spread = true;
            }
            else
            {
                const Real64 Row = Measure(Owner, State, 0, Total, Callback);
                State.Cost = State.Cost > 0 ? Min(Row, State.Cost * 1.25) : Row;
            }

            Owner.Leave();
        }

    private:

        /// The seconds of work left above which spreading pays for waking the workers.
        static constexpr Real64 kSpreadSeconds   = 20e-6;

        /// The seconds of work each share aims at, short enough to even out and long enough to be worth taking.
        static constexpr Real64 kShareSeconds    = 4e-6;

        /// The shares a spread walk cuts per thread at most, enough to even out what one thread was slow at.
        static constexpr UInt32 kSharesPerThread = 8;

        /// The workers a spread walk calls on at most.
        static constexpr UInt32 kMaxHelpers      = 63;

        /// The place one field is read from, spelled per field so a row loop can take one apart for each.
        template<typename Field>
        using Address = Ptr<Byte>;

        /// \brief Specifies how the fields of a page step from one row to the next.
        enum class Stride : UInt8
        {
            Dense,   ///< Every field steps.
            Written, ///< The written fields step and the read ones stay in one place.
            Chunk,   ///< Each field steps as the chunk says, and the ancestor ones climb.
        };

        /// \brief Represents the loops of a walk for one list of fields, spelled out only with their positions.
        template<typename Positions, typename... Parts>
        struct Kernel;

        /// \brief Represents the loops of a walk for one list of fields, each known by its position in the callback.
        ///
        /// \tparam Index  The position of each field.
        /// \tparam Parts The fields, in the order the callback takes them.
        template<UInt... Index, typename... Parts>
        struct Kernel<IntegerSequence<UInt, Index...>, Parts...> final
        {
            /// The number of fields.
            static constexpr UInt kCount = sizeof...(Parts);

            /// \brief Represents where each field of one matched chunk is read from, one extra slot so none is empty.
            struct Places final
            {
                /// The first byte of each field's run inside a page.
                UInt32    Offsets[kCount + 1];

                /// The one place every row reads each field from, when it does not step.
                Ptr<Byte> Fixed[kCount + 1];

                /// The instances each field steps per row, zero for one read from one place.
                UInt      Steps[kCount + 1];

                /// `true` for each field read from the nearest ancestor.
                Bool      Climbs[kCount + 1];

                /// `true` when every field steps.
                Bool      Dense;

                /// `true` when the written fields step and the read ones stay in one place.
                Bool      Invariant;

                /// \brief Gets where a field starts in a page, or the one place every row reads it from.
                ///
                /// \param Page  The first byte of the page.
                /// \param Field The position of the field.
                /// \return The first byte.
                ZY_INLINE Ptr<Byte> Start(Ptr<Byte> Page, UInt Field) const
                {
                    return Steps[Field] ? Page + Offsets[Field] : Fixed[Field];
                }
            };

            /// \brief Gets where a field reads for an entity below a parent, climbing when it reads an ancestor's.
            ///
            /// \param Owner   The storage.
            /// \param State   The query.
            /// \param Where   Where each field of the chunk is read from.
            /// \param Parent  The slot of the parent.
            /// \param Field   The position of the field.
            /// \param Current Where the field reads from now, kept when it does not climb.
            /// \return The first byte of the value, or `nullptr` when the field climbs and no ancestor carries it.
            ZY_INLINE static Ptr<Byte> Climb(
                Ref<Storage>        Owner,
                ConstRef<Selection> State,
                ConstRef<Places>    Where,
                UInt32              Parent,
                UInt                Field,
                Ptr<Byte>           Current)
            {
                return Where.Climbs[Field] ? FindAncestor(Owner, Parent, State.Fields[Field]) : Current;
            }

            /// \brief Gets how many instances a field steps per row, as the way a page is walked says.
            ///
            /// \param Where    Where each field of the chunk is read from.
            /// \param Position The position of the field.
            /// \return One for a field that steps, zero for one read from one place.
            template<Stride Mode, typename Field>
            ZY_INLINE static UInt StepOf(ConstRef<Places> Where, UInt Position)
            {
                if constexpr (Mode == Stride::Chunk)
                {
                    return Where.Steps[Position];
                }
                else
                {
                    return Mode == Stride::Dense || Field::kWritten;
                }
            }

            /// \brief Gets a field's rows of a page as a span, or its one instance when every row reads the same.
            ///
            /// \param Base  The first instance in the page, or the one place every row reads it from.
            /// \param Step  The instances it steps per row, one or zero.
            /// \param First The first row of the page.
            /// \param Last  The row past the last one.
            /// \return The span.
            template<typename Field>
            ZY_INLINE static Span<typename Field::Value> SpanOf(Ptr<Byte> Base, UInt Step, UInt First, UInt Last)
            {
                using Value = typename Field::Value;

                return Span<Value>(reinterpret_cast<Ptr<Value>>(Base) + Step * First, Step ? Last - First : 1);
            }

            /// \brief Works out where each field of one matched chunk is read from.
            ///
            /// \param Owner The storage.
            /// \param State The query.
            /// \param Match The position of the chunk among the matches.
            /// \param Into  Receives where each field is read from.
            static void Prepare(Ref<Storage> Owner, ConstRef<Selection> State, UInt32 Match, Ref<Places> Into)
            {
                constexpr Bool kTags[kCount + 1]   = { Parts::kTag..., false };
                constexpr Bool kWrites[kCount + 1] = { Parts::kWritten..., false };

                ConstRef<Chunk> Target = * State.Chunks[Match];

                Zero(AddressOf(Into), 1);
                Into.Dense     = true;
                Into.Invariant = true;

                for (UInt32 Field = 0; Field < kCount; ++Field)
                {
                    const UInt16 Source = State.GetSource(Match, Field);

                    if (kTags[Field])
                    {
                        continue;
                    }

                    if (Source < Selection::kAncestor)
                    {
                        Into.Offsets[Field] = Target.GetColumns()[Source].Offset;
                        Into.Steps[Field]   = 1;
                        Into.Invariant      = Into.Invariant && kWrites[Field];
                        continue;
                    }

                    Into.Dense     = false;
                    Into.Invariant = Into.Invariant && !kWrites[Field] && Source != Selection::kAncestor;

                    switch (Source)
                    {
                    case Selection::kLent:
                        Into.Fixed[Field] = FindLent(Owner, Target.GetBase(), State.Fields[Field]);
                        break;
                    case Selection::kGlobal:
                        Into.Fixed[Field] = GetSingleton(Owner, State.Fields[Field]);
                        break;
                    case Selection::kAncestor:
                        Into.Climbs[Field] = true;
                        break;
                    default:
                        break;
                    }
                }
            }

            /// \brief Hands the rows of a stretch of one matched chunk to a callback, page by page.
            ///
            /// \param Owner    The storage.
            /// \param State    The query.
            /// \param Match    The position of the chunk among the matches.
            /// \param Begin    The first row.
            /// \param End      The row past the last one.
            /// \param Callback The callable.
            template<typename Callable>
            static void RunPages(
                Ref<Storage>        Owner,
                ConstRef<Selection> State,
                UInt32              Match,
                UInt32              Begin,
                UInt32              End,
                Ref<Callable>       Callback)
            {
                ConstRef<Chunk> Target = * State.Chunks[Match];

                Places Where;
                Prepare(Owner, State, Match, Where);

                // Page by page, each page's rows from where the stretch enters it to where it leaves.
                for (UInt32 Row = Begin; Row < End;)
                {
                    const UInt32      First   = Target.GetPlace(Row);
                    const UInt32      Last    = Min(First + (End - Row), Target.GetRows());
                    const Ptr<Byte>   Base    = Target.GetPage(Row);
                    const Ptr<Handle> Handles = Target.GetHandles(Row);

                    Ptr<Byte> Bases[kCount + 1];

                    for (UInt32 Field = 0; Field < kCount; ++Field)
                    {
                        Bases[Field] = Where.Start(Base, Field);
                    }

                    if constexpr (Plan<Callable>::kBatched)
                    {
                        static_assert(!(Parts::kTag || ...), "A tag has no span to hand over");

                        Callback(SpanOf<Parts>(Bases[Index], Where.Steps[Index], First, Last)...);
                    }
                    else if (Where.Dense)
                    {
                        RunRows<Stride::Dense>(Owner, Callback, State, Handles, Where, First, Last, Bases[Index]...);
                    }
                    else if (Where.Invariant)
                    {
                        RunRows<Stride::Written>(Owner, Callback, State, Handles, Where, First, Last, Bases[Index]...);
                    }
                    else
                    {
                        RunRows<Stride::Chunk>(Owner, Callback, State, Handles, Where, First, Last, Bases[Index]...);
                    }
                    Row += Last - First;
                }
            }

            /// \brief Calls back for rows in the order given, jumping between chunks and preparing each on entry.
            ///
            /// \param Owner    The storage.
            /// \param State    The query.
            /// \param Rows     The rows, each the match in the high half and the row in the low one.
            /// \param Callback The callable, which is handed one row at a time.
            template<typename Callable>
            static void RunSorted(
                Ref<Storage>        Owner,
                ConstRef<Selection> State,
                ConstSpan<UInt64>   Rows,
                Ref<Callable>       Callback)
            {
                Places     Where;
                Ptr<Chunk> Target = nullptr;

                // The parent the ancestor fields last climbed from, which siblings sorted side by side share.
                UInt32 Above = ~0u;

                for (const UInt64 Key : Rows)
                {
                    const UInt32 Match = static_cast<UInt32>(Key >> 32);
                    const UInt32 Row   = static_cast<UInt32>(Key);

                    if (State.Chunks[Match] != Target)
                    {
                        Target = State.Chunks[Match];
                        Prepare(Owner, State, Match, Where);
                        Above = ~0u;
                    }

                    const Handle Actor = Target->GetHandle(Row);

                    if constexpr ((Parts::kPointer || ...))
                    {
                        const UInt32 Parent = Owner.mDirectory[Actor.GetIndex()].Parent;

                        if (Parent != Above)
                        {
                            Above = Parent;

                            ((Where.Fixed[Index] = Climb(Owner, State, Where, Parent, Index, Where.Fixed[Index])), ...);
                        }
                    }

                    const Ptr<Byte> Base  = Target->GetPage(Row);
                    const UInt      Place = Target->GetPlace(Row);

                    Call(Owner, Callback, Actor, Fetch<Parts>(Where.Start(Base, Index), Where.Steps[Index], Place)...);
                }
            }

            /// \brief Calls back once per row, each field's start held apart so the loop keeps it in a register.
            ///
            /// \param Owner    The storage.
            /// \param Callback The callable.
            /// \param State    The query.
            /// \param Handles  The entities of the page.
            /// \param Where    Where each field of the chunk is read from.
            /// \param First    The first row of the page.
            /// \param Last     The row past the last one.
            /// \param Bases    The start of each field in the page, or the one place every row reads it from.
            template<Stride Mode, typename Callable>
            ZY_INLINE static void RunRows(
                Ref<Storage>        Owner,
                Ref<Callable>       Callback,
                ConstRef<Selection> State,
                ConstPtr<Handle>    Handles,
                ConstRef<Places>    Where,
                UInt                First,
                UInt                Last,
                Address<Parts>...   Bases)
            {
                // The parent the ancestor fields last climbed from, which siblings share.
                UInt32 Above = ~0u;

                for (UInt Row = First; Row < Last; ++Row)
                {
                    if constexpr (Mode == Stride::Chunk && (Parts::kPointer || ...))
                    {
                        const UInt32 Parent = Owner.mDirectory[Handles[Row].GetIndex()].Parent;

                        if (Parent != Above)
                        {
                            Above = Parent;
                            ((Bases = Climb(Owner, State, Where, Parent, Index, Bases)), ...);
                        }
                    }

                    ZY_INLINE_CALL Call(
                        Owner,
                        Callback,
                        Handles[Row],
                        Fetch<Parts>(Bases, StepOf<Mode, Parts>(Where, Index), Row)...);
                }
            }

            /// \brief Calls back for one entity when it carries every field the callback needs, as a reader does.
            ///
            /// \param Owner    The storage.
            /// \param State    The selection the reader filters by, whose fields are declared.
            /// \param Actor    The entity, alive and matching.
            /// \param Callback The callable.
            template<typename Callable>
            ZY_INLINE static void RunEntry(
                Ref<Storage>        Owner,
                ConstRef<Selection> State,
                Handle              Actor,
                Ref<Callable>       Callback)
            {
                // One extra slot, so a callback taking no field still makes a valid array.
                const Ptr<Byte> Found[kCount + 1] = { Locate<Parts>(Owner, State, Actor, Index)... };

                if (((!Parts::kPointer && !Parts::kTag && !Found[Index]) || ...))
                {
                    return;
                }
                Call(Owner, Callback, Actor, Fetch<Parts>(Found[Index], 0, 0)...);
            }

            /// \brief Finds where one field of an entity is, held, lent, an ancestor's or the world's.
            ///
            /// \param Owner    The storage.
            /// \param State    The selection, whose fields are declared.
            /// \param Actor    The entity, alive.
            /// \param Position The place of the field in the callback.
            /// \return The first byte of the value, or `nullptr` when there is none or the field is a tag.
            template<typename Field>
            ZY_INLINE static Ptr<Byte> Locate(
                Ref<Storage>        Owner,
                ConstRef<Selection> State,
                Handle              Actor,
                UInt                Position)
            {
                if constexpr (Field::kTag)
                {
                    return nullptr;
                }
                else
                {
                    const UInt32              Identifier = State.Fields[Position];
                    ConstRef<Directory::Slot> Entry      = Owner.mDirectory[Actor.GetIndex()];

                    if (State.Modes[Position] == Selection::Mode::Ancestor)
                    {
                        return FindAncestor(Owner, Entry.Parent, Identifier);
                    }

                    const ConstPtr<Byte> Found = Owner.mDirectory.FindValue(* Entry.Holder, Entry.Row, Identifier);
                    return Found ? const_cast<Ptr<Byte>>(Found) : GetSingleton(Owner, Identifier);
                }
            }
        };

        /// \brief Represents the rows of a spread walk still to hand out, drained by the caller and the workers.
        template<typename Callable>
        struct SpreadWork final
        {
            /// The storage the query belongs to.
            Ptr<Storage>   Owner;

            /// The query.
            Ptr<Selection> State;

            /// The callable.
            Ptr<Callable>  Callback;

            /// The next share to hand out.
            Atomic<UInt32> Next;

            /// The rows of each share.
            UInt32         Grain;

            /// The number of shares.
            UInt32         Shares;

            /// The first row the shares cover.
            UInt32         First;

            /// The row past the last one they cover.
            UInt32         End;

            /// \brief Constructs the work of a spread walk, cut into shares of as many rows each.
            ///
            /// \param Owner    The storage the query belongs to.
            /// \param State    The query.
            /// \param Callback The callable.
            /// \param Begin    The first row.
            /// \param End      The row past the last one.
            /// \param Shares   The number of shares to aim at, never zero.
            ZY_INLINE SpreadWork(
                Ref<Storage>   Owner,
                Ref<Selection> State,
                Ref<Callable>  Callback,
                UInt32         Begin,
                UInt32         End,
                UInt32         Shares)
                : Owner    { AddressOf(Owner) },
                  State    { AddressOf(State) },
                  Callback { AddressOf(Callback) },
                  Next     { 0 },
                  Grain    { (End - Begin + Shares - 1) / Shares },
                  Shares   { (End - Begin + Grain - 1) / Grain },
                  First    { Begin },
                  End      { End }
            {
            }

            /// \brief Takes the next share, when one is left, and holds what the calling thread changes in it apart.
            ///
            /// \param Begin Receives the first row of the share.
            /// \param Until Receives the row past the last one.
            /// \return `true` if a share was taken, `false` when none is left.
            ZY_INLINE Bool Take(Ref<UInt32> Begin, Ref<UInt32> Until)
            {
                const UInt32 Share = Next.fetch_add(1, std::memory_order_relaxed);

                if (Share >= Shares)
                {
                    return false;
                }
                Owner->mDeferral.Bind(Share);

                Begin = First + Share * Grain;
                Until = Min(Begin + Grain, End);
                return true;
            }

            /// \brief Walks shares until none is left.
            ZY_INLINE void Drain()
            {
                UInt32 Begin;
                UInt32 Until;

                while (Take(Begin, Until))
                {
                    RunRange(* Owner, * State, Begin, Until, * Callback);
                }
            }
        };

        /// \brief Checks whether the world holds every singleton the query reads, without which it walks nothing.
        ///
        /// \param Owner The storage.
        /// \param State The query.
        /// \return `true` if every one is there, `false` otherwise.
        ZY_INLINE static Bool HasGlobals(Ref<Storage> Owner, ConstRef<Selection> State)
        {
            for (const UInt32 Identifier : State.Globals)
            {
                if (!GetSingleton(Owner, Identifier))
                {
                    return false;
                }
            }
            return true;
        }

        /// \brief Gets where the world entity keeps a component.
        ///
        /// \param Owner      The storage.
        /// \param Identifier The component.
        /// \return The first byte of the value, or `nullptr` when the world holds none.
        ZY_INLINE static Ptr<Byte> GetSingleton(Ref<Storage> Owner, UInt32 Identifier)
        {
            ConstRef<Directory::Slot> World = Owner.mDirectory[Storage::kWorld.GetIndex()];
            return const_cast<Ptr<Byte>>(Owner.mDirectory.FindValue(* World.Holder, World.Row, Identifier));
        }

        /// \brief Finds where the archetype an entity is made from, or one it is made from in turn, lends a component.
        ///
        /// \param Owner      The storage.
        /// \param Base       The slot of the archetype, or zero for none.
        /// \param Identifier The component.
        /// \return The first byte of the value, or `nullptr` when no archetype up the chain lends it.
        ZY_INLINE static Ptr<Byte> FindLent(Ref<Storage> Owner, UInt32 Base, UInt32 Identifier)
        {
            return const_cast<Ptr<Byte>>(Owner.mDirectory.FindLent(Base, Identifier));
        }

        /// \brief Finds where the nearest entity up a chain keeps a component, held or lent.
        ///
        /// \param Owner      The storage.
        /// \param From       The slot to look at first, or zero for none.
        /// \param Identifier The component.
        /// \return The first byte of the value, or `nullptr` when nothing up the chain carries it.
        ZY_INLINE static Ptr<Byte> FindAncestor(Ref<Storage> Owner, UInt32 From, UInt32 Identifier)
        {
            return const_cast<Ptr<Byte>>(Owner.mDirectory.FindAncestor(From, Identifier));
        }

        /// \brief Gets the time on a steady clock.
        ///
        /// \return The seconds since an arbitrary start.
        static Real64 GetTime();

        /// \brief Hands the rows of a stretch of the matches laid end to end to a callback.
        ///
        /// \param Owner    The storage.
        /// \param State    The query, whose offsets are laid out.
        /// \param Begin    The first row.
        /// \param End      The row past the last one.
        /// \param Callback The callable.
        template<typename Callable>
        static void RunRange(
            Ref<Storage>        Owner,
            ConstRef<Selection> State,
            UInt32              Begin,
            UInt32              End,
            Ref<Callable>       Callback)
        {
            for (UInt32 Match = FindMatch(State, Begin); Begin < End; ++Match)
            {
                const UInt32 Start = State.Offsets[Match];
                const UInt32 Stop  = Min(End, State.Offsets[Match + 1]);

                if (Stop > Begin)
                {
                    RunChunk(Owner, State, Match, Begin - Start, Stop - Start, Callback);
                }
                Begin = Stop;
            }
        }

        /// \brief Finds the match a row of the matches laid end to end falls in.
        ///
        /// \param State The query, whose offsets are laid out.
        /// \param Row   The row.
        /// \return The position of the last match starting at or before the row.
        static UInt32 FindMatch(ConstRef<Selection> State, UInt32 Row);

        /// \brief Hands a stretch of rows to a callback on this thread, timing it.
        ///
        /// \param Owner    The storage.
        /// \param State    The query, whose offsets are laid out.
        /// \param Begin    The first row.
        /// \param End      The row past the last one, which is past the first.
        /// \param Callback The callable.
        /// \return The seconds one row took.
        template<typename Callable>
        static Real64 Measure(
            Ref<Storage>        Owner,
            ConstRef<Selection> State,
            UInt32              Begin,
            UInt32              End,
            Ref<Callable>       Callback)
        {
            const Real64 Start = GetTime();
            RunRange(Owner, State, Begin, End, Callback);
            return Max(GetTime() - Start, 1e-9) / (End - Begin);
        }

        /// \brief Spreads a stretch of rows over the caller and the compute workers, in shares of a few microseconds.
        ///
        /// \param Owner    The storage.
        /// \param State    The query, whose offsets are laid out and whose cost per row is known.
        /// \param Begin    The first row.
        /// \param End      The row past the last one.
        /// \param Callback The callable, thread-safe and changing only its own entity.
        /// \return The seconds one row took on the share the caller timed.
        template<typename Callable>
        static Real64 Distribute(
            Ref<Storage>   Owner,
            Ref<Selection> State,
            UInt32         Begin,
            UInt32         End,
            Ref<Callable>  Callback)
        {
            Ref<ZyJob::Service> Jobs    = Owner.GetService<ZyJob::Service>();
            const UInt32        Threads = Jobs.GetConcurrency(ZyJob::Lane::Compute) + 1;
            const UInt32        Shares  = GetShares(State, End - Begin, Threads);

            SpreadWork<Callable> Work(Owner, State, Callback, Begin, End, Shares);

            const UInt32 Helpers = Min(Min(Threads - 2, Work.Shares - 1), kMaxHelpers);

            Array<ZyJob::Handle, kMaxHelpers> Tasks;

            // Each share holds its changes apart, and they are appended in share order once every worker is done.
            Owner.mDeferral.Spread(Work.Shares, Owner.mDirectory.GetSize());

            for (UInt32 Helper = 0; Helper < Helpers; ++Helper)
            {
                Tasks[Helper] = Jobs.Submit(ZyJob::Lane::Compute, [Where = AddressOf(Work)]
                {
                    Where->Drain();
                });
            }

            // The caller times one share, then drains like any worker, so one slow to wake costs only its share.
            Real64 Cost = State.Cost;
            UInt32 First;
            UInt32 Until;

            if (Work.Take(First, Until))
            {
                Cost = Measure(Owner, State, First, Until, Callback);
            }
            Work.Drain();

            for (UInt32 Helper = 0; Helper < Helpers; ++Helper)
            {
                Jobs.Wait(Tasks[Helper]);
            }

            Owner.mDeferral.Gather();
            return Cost;
        }

        /// \brief Gets how many shares a spread walk cuts its rows into.
        ///
        /// \param State   The query, whose cost per row is known.
        /// \param Rows    The number of rows, never zero.
        /// \param Threads The threads that drain the shares, the caller included.
        /// \return The number of shares, never zero.
        static UInt32 GetShares(ConstRef<Selection> State, UInt32 Rows, UInt32 Threads);

        /// \brief Hands every match to a callback parents first, sorting the rows by how deep they stand.
        ///
        /// \param Owner    The storage.
        /// \param State    The query.
        /// \param Callback The callable.
        template<typename Callable>
        static void RunOrdered(Ref<Storage> Owner, Ref<Selection> State, Ref<Callable> Callback)
        {
            ZY_ASSERT(!Plan<Callable>::kBatched, "A cascade hands one row at a time, parents before their children");

            if constexpr (!Plan<Callable>::kBatched)
            {
                const UInt32            First = Sort(Owner, State);
                const ConstSpan<UInt64> Rows(State.Gathered.GetData() + First, State.Gathered.GetSize() - First);

                Plan<Callable>::template Apply<Kernel>::RunSorted(Owner, State, Rows, Callback);
            }
        }

        /// \brief Gathers the rows of every match and sorts them parents first, behind the rows as gathered.
        ///
        /// \param Owner The storage.
        /// \param State The query.
        /// \return The position of the first sorted row among the gathered ones.
        static UInt32 Sort(Ref<Storage> Owner, Ref<Selection> State);

        /// \brief Checks whether the rows an ordered walk sorted last are still where they were and as deep.
        ///
        /// \param Owner The storage.
        /// \param State The query.
        /// \return `true` if every row holds the entity it held and no link changed since, `false` otherwise.
        static Bool IsSorted(Ref<Storage> Owner, ConstRef<Selection> State);

        /// \brief Gets the key an ordered walk gathers a row under.
        ///
        /// \param Match The position of the chunk among the matches.
        /// \param Row   The row.
        /// \return The key, the match in the high half and the row in the low one.
        ZY_INLINE static constexpr UInt64 Pack(UInt32 Match, UInt32 Row)
        {
            return static_cast<UInt64>(Match) << 32 | Row;
        }

        /// \brief Hands the rows of a stretch of one matched chunk to a callback, through the loops for its fields.
        ///
        /// \param Owner    The storage.
        /// \param State    The query.
        /// \param Match    The position of the chunk among the matches.
        /// \param Begin    The first row.
        /// \param End      The row past the last one.
        /// \param Callback The callable.
        template<typename Callable>
        ZY_INLINE static void RunChunk(
            Ref<Storage>        Owner,
            ConstRef<Selection> State,
            UInt32              Match,
            UInt32              Begin,
            UInt32              End,
            Ref<Callable>       Callback)
        {
            Plan<Callable>::template Apply<Kernel>::RunPages(Owner, State, Match, Begin, End, Callback);
        }

        /// \brief Gets what a field hands the callback for one row.
        ///
        /// \param Base The first instance in the page, or the one place every row reads it from.
        /// \param Step The instances it steps per row, one or zero.
        /// \param Row  The row.
        /// \return The value, a pointer to it, or a fresh tag.
        template<typename Field>
        ZY_INLINE static decltype(auto) Fetch(Ptr<Byte> Base, UInt Step, UInt Row)
        {
            if constexpr (Field::kTag)
            {
                return StripAll<typename Field::Value>();
            }
            else
            {
                using Value = typename Field::Value;

                const Ptr<Value> Where = reinterpret_cast<Ptr<Value>>(Base) + Row * Step;

                if constexpr (Field::kPointer)
                {
                    return Where;
                }
                else
                {
                    return static_cast<Ref<Value>>(* Where);
                }
            }
        }

        /// \brief Calls back for one entity, handing it first when the callback asks for it.
        ///
        /// \param Owner     The storage.
        /// \param Callback  The callable.
        /// \param Actor     The entity.
        /// \param Arguments The fields.
        template<typename Callable, typename... Values>
        ZY_INLINE static void Call(
            Ref<Storage>      Owner,
            Ref<Callable>     Callback,
            Handle            Actor,
            AnyRef<Values>... Arguments)
        {
            if constexpr (Plan<Callable>::kEntity)
            {
                Callback(Entity(AddressOf(Owner), Actor), Forward<Values>(Arguments)...);
            }
            else
            {
                Callback(Forward<Values>(Arguments)...);
            }
        }
    };
}