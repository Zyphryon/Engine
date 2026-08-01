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

namespace Job
{
    /// \brief Identifies the pool of threads a job executes on.
    enum class Lane : UInt8
    {
        Main,       ///< Runs on the main thread, drained once per frame during \ref Service::OnTick.
        Compute,    ///< Runs on a background worker sized to the machine's core count.
        IO,         ///< Runs on a small pool reserved for work that blocks in the kernel, such as filesystem access.
    };

    /// \brief Number of distinct lanes, used to size the per-lane pool storage.
    inline constexpr UInt32 kMaxLanes = Enum::Count<Lane>();

    /// \brief Maximum number of jobs that may be in flight at once.
    inline constexpr UInt32 kMaxJobs  = 1024;

    /// \brief Opaque reference to a submitted job, where `0` denotes an invalid job.
    using Handle = UInt32;

    /// \brief A job's place in the registry, where `0` can stand for no job.
    using Slot   = UInt32;

    /// \brief Signature for a unit of work submitted to the scheduler.
    using Task   = Delegate<void()>;
}