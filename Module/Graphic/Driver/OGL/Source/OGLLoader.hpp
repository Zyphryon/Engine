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

#include "OGLDevice.hpp"
#include "Zyphryon.Base/System/Library.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Represents the system's OpenGL library and the entry points taken out of it.
    ///
    /// \note The entry points are process-wide, since the driver and Tracy call them by their global names.
    class Loader final
    {
    public:

        /// \brief Constructs a loader that holds no library until \ref Open opens one.
        Loader();

        /// \brief Forgets every entry point, then lets go of the library.
        ~Loader();

        /// \brief Opens the system's OpenGL library and the window system's own function finder.
        ///
        /// \note Nothing is opened on the web, where every entry point is linked.
        ///
        /// \return `true` when the library and its finder were found, `false` otherwise.
        Bool Open();

        /// \brief Finds a function by name, asking the window system first and the library's exports second.
        ///
        /// \note Window system functions need a current context to answer for anything but themselves.
        ///
        /// \param Name The function's name, as exported.
        /// \return The function's address, or `nullptr` when neither knows the name.
        Ptr<void> Find(Text Name) const;

        /// \brief Takes every entry point of the baseline out of the current context.
        ///
        /// \return `true` when every entry point was found, `false` when one is missing.
        Bool Load();

        /// \brief Takes the entry points of every feature the device grants, withdrawing a feature that lacks one.
        ///
        /// \param Device The device whose granted features are loaded.
        void Load(Ref<Device> Device);

    private:

        /// \brief Describes one entry point and the pointer it is loaded into.
        struct Entry final
        {
            /// `true` when the entry point belongs to the baseline every context has.
            Bool           Core;

            /// The feature that grants the entry point, ignored for the baseline.
            Feature        Feature;

            /// The entry point's name, without any vendor suffix.
            Text           Name;

            /// The global pointer the entry point is loaded into.
            Ptr<Ptr<void>> Pointer;
        };

        /// The entry points the loader knows, one per row of the API table.
        static const Entry kEntries[];

        /// \brief Takes every entry point a feature grants out of the current context.
        ///
        /// \param Value  The feature whose entry points are taken.
        /// \param Suffix The vendor suffix the entry points carry, empty where they keep their core name.
        /// \return `true` when every entry point was found, `false` when one is missing.
        Bool Fetch(Feature Value, Text Suffix);

        /// \brief Finds an entry point under its suffixed name, falling back to the bare one.
        ///
        /// \param Name   The entry point's name, without a suffix.
        /// \param Suffix The vendor suffix to try first, empty for none.
        /// \return The entry point's address, or `nullptr` when neither name is known.
        Ptr<void> Resolve(Text Name, Text Suffix) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Library   mLibrary;
        Ptr<void> mFinder;
    };
}