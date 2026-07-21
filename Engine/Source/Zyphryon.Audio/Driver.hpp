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

#include "Types.hpp"
#include "Zyphryon.Base/Memory/Unique.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief The audio output device backend backend per operating system.
    class Driver final
    {
    public:

        /// \brief Callback invoked on the audio thread to fill an interleaved 32-bit float output buffer.
        ///
        /// \param Output The destination buffer, to fill with interleaved samples.
        /// \param Frames The number of frames requested by the device for this invocation.
        using Callback = Delegate<void(Span<Real32> Output, UInt32 Frames)>;

    public:

        /// \brief Constructs the driver in a closed state.
        Driver();

        /// \brief Closes the device and destroys the driver.
        ~Driver();

        /// \brief Opens the output device and starts driving the render callback.
        ///
        /// \param Device The name of the endpoint to open, or empty for the system default.
        /// \param Render The callback the device invokes to obtain interleaved output frames.
        /// \return `true` if the device was opened successfully, otherwise `false`.
        Bool Open(Text Device, AnyRef<Callback> Render);

        /// \brief Closes the output device and stops driving the render callback.
        void Close();

        /// \brief Fills the provided structure with the backend name, device, and available endpoints.
        ///
        /// \param Output The structure to populate with the driver's current device information.
        void Probe(Ref<Description> Output) const;

        /// \brief Advances the driver on the main thread.
        ///
        /// \param Delta The time elapsed since the last tick, in seconds.
        void Advance(Real64 Delta);

        /// \brief Suspends audio output.
        void Suspend();

        /// \brief Restores audio output after a suspension.
        void Restore();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        struct Backend;
        Unique<Backend> mBackend;
    };
}
