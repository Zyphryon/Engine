// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the phases a world runs each progress and the systems in each, by handles never reused.
    class ZY_API Scheduler final
    {
    public:

        /// \brief Represents what a phase runs for one system, a walk over its query, a task or a pull list reader.
        using Runner = Delegate<void()>;

    public:

        /// \brief Constructs a scheduler with no phase and no system.
        Scheduler();

        /// \brief Prevents copying a scheduler, since a running system points into its own.
        Scheduler(ConstRef<Scheduler> Other) = delete;

        /// \brief Prevents assigning a scheduler, since a running system points into its own.
        Ref<Scheduler> operator=(ConstRef<Scheduler> Other) = delete;

        /// \brief Creates a phase, which runs its systems in the order they were created when its turn comes.
        ///
        /// \param Name    The name of the phase, which profiles and logs show.
        /// \param After   The phase it runs right after, or zero to run after every phase made so far.
        /// \param Package The module that declares it, or zero.
        /// \return The handle of the phase, never zero.
        UInt32 CreatePhase(Text Name, UInt32 After, UInt32 Package);

        /// \brief Adds a system or task to the end of a phase.
        ///
        /// \param Name    The name of the system.
        /// \param Phase   The phase.
        /// \param Rate    The progresses between two runs, one to run on every one.
        /// \param Runner  The callable the phase runs.
        /// \param Package The module that declares it, or zero.
        /// \return The handle of the system, never zero.
        UInt32 CreateSystem(Text Name, UInt32 Phase, UInt16 Rate, AnyRef<Runner> Runner, UInt32 Package);

        /// \brief Takes a system out of its phase, so it runs no more.
        ///
        /// \param Handle The handle of the system, or one already taken out.
        void DestroySystem(UInt32 Handle);

        /// \brief Holds a system off or lets it run again, keeping its place in its phase.
        ///
        /// \param Handle  The handle of the system, or one already taken out.
        /// \param Enabled `true` to run it again, `false` to hold it off.
        void SetEnabled(UInt32 Handle, Bool Enabled);

        /// \brief Checks whether a system is run when its phase comes.
        ///
        /// \param Handle The handle of the system.
        /// \return `true` while it is run, `false` while it is held off or taken out.
        Bool IsEnabled(UInt32 Handle) const;

        /// \brief Takes out every system and phase a module declared.
        ///
        /// \param Package The module.
        void Discard(UInt32 Package);

        /// \brief Runs every phase once, in order, and every system in each, in the order they were created.
        void Progress();

        /// \brief Destroys the systems taken out, once no phase runs, letting go of what each walked.
        void Purge();

        /// \brief Takes out every system, so none runs code that points back into the world while it is torn down.
        void Clear();

    private:

        /// \brief Represents one system, with what it runs, its phase and the module that declared it.
        struct Routine final
        {
            /// The name profiles and logs show.
            Str    Name;

            /// The callable the phase runs.
            Runner Run;

            /// The progresses between two runs.
            UInt16 Rate;

            /// The progresses since the last run.
            UInt16 Count;

            /// The phase it runs in.
            UInt32 Phase;

            /// The module that declared it, or zero.
            UInt32 Package;

            /// `true` until it is taken out.
            Bool   Alive;

            /// `true` while it is run, `false` while it is held off.
            Bool   Enabled;

            /// \brief Constructs a system.
            ///
            /// \param Name    The name profiles and logs show.
            /// \param Run     The callable the phase runs.
            /// \param Rate    The progresses between two runs.
            /// \param Phase   The phase it runs in.
            /// \param Package The module that declared it, or zero.
            ZY_INLINE Routine(Text Name, AnyRef<Runner> Run, UInt16 Rate, UInt32 Phase, UInt32 Package)
                : Name    { Name },
                  Run     { Move(Run) },
                  Rate    { Rate },
                  Count   { 0 },
                  Phase   { Phase },
                  Package { Package },
                  Alive   { true },
                  Enabled { true }
            {
            }
        };

        /// \brief Represents one phase, with its name and the systems it runs.
        struct Stage final
        {
            /// The name profiles and logs show.
            Str              Name;

            /// The systems it runs, by handle, in the order they were created.
            Sequence<UInt32> Systems;

            /// The module that declared it, or zero.
            UInt32           Package;

            /// \brief Constructs a phase that runs nothing yet.
            ///
            /// \param Name    The name profiles and logs show.
            /// \param Package The module that declared it, or zero.
            ZY_INLINE Stage(Text Name, UInt32 Package)
                : Name    { Name },
                  Package { Package }
            {
            }
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Unique<Stage>>   mPhases;
        Sequence<UInt32>          mOrder;
        Sequence<Unique<Routine>> mSystems;
        Sequence<UInt32>          mDismissed;
        Bool                      mRunning;
    };
}