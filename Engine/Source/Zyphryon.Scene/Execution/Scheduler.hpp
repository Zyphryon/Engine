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
    /// \brief Represents the phases a world runs each progress, in the order they were made, and the systems in each.
    class ZY_API Scheduler final
    {
    public:

        /// \brief Represents what a phase runs for one system, a walk over its query, a task or a reader of changes.
        using Runner = Delegate<void()>;

    public:

        /// \brief Constructs a scheduler with no phase and no system.
        Scheduler();

        /// \brief Schedulers are not copied, since a running system points into its own.
        Scheduler(ConstRef<Scheduler> Other) = delete;

        /// \brief Schedulers are not assigned, since a running system points into its own.
        Ref<Scheduler> operator=(ConstRef<Scheduler> Other) = delete;

        /// \brief Creates a phase, which runs after every phase made before it.
        ///
        /// \param Name    The name profiles show.
        /// \param Package The module that makes it, or zero.
        /// \return The phase, never zero.
        UInt32 CreatePhase(Text Name, UInt32 Package);

        /// \brief Adds a system to the end of a phase.
        ///
        /// \param Name    The name profiles show.
        /// \param Phase   The phase to run it in.
        /// \param Rate    The progresses from one run to the next, one to run on every one.
        /// \param Run     The callable the phase runs.
        /// \param Package The module that makes it, or zero.
        void CreateSystem(Text Name, UInt32 Phase, UInt16 Rate, AnyRef<Runner> Run, UInt32 Package);

        /// \brief Sets whether a phase runs, which every phase does from the moment it is made.
        ///
        /// \param Phase  The phase, as \ref CreatePhase returned it.
        /// \param Active `true` to run its systems each progress, `false` to skip every one of them.
        void SetActive(UInt32 Phase, Bool Active);

        /// \brief Takes out every phase and system a module made.
        ///
        /// \param Package The module, as the world imported it.
        void Discard(UInt32 Package);

        /// \brief Runs every phase once, in order, and every system in each, in the order they were made.
        void Progress();

        /// \brief Destroys the systems taken out, once no phase runs, letting go of what each walked.
        void Purge();

        /// \brief Takes out every system, so none runs code that points back into the world while it is torn down.
        void Clear();

    private:

        /// \brief Represents one system.
        struct Routine final
        {
            /// The name profiles show.
            Str    Name;

            /// The callable the phase runs.
            Runner Run;

            /// The progresses from one run to the next.
            UInt16 Rate;

            /// The progresses since the last run.
            UInt16 Count;

            /// The module that made it, or zero.
            UInt32 Package;

            /// `true` until its module is discarded.
            Bool   Alive = true;
        };

        /// \brief Represents one phase.
        struct Stage final
        {
            /// The name profiles show.
            Str                       Name;

            /// The systems it runs, in the order they were made.
            Sequence<Unique<Routine>> Systems;

            /// The module that made it, or zero.
            UInt32                    Package;

            /// `true` until its module is discarded.
            Bool                      Alive  = true;

            /// `true` while its systems run each progress, `false` while they are skipped.
            Bool                      Active = true;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Unique<Stage>> mPhases;
        Bool                    mRunning;
        Bool                    mDismissed;
    };
}