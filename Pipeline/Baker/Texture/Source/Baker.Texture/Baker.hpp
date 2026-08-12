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

#include "Exporter.hpp"
#include "Importer.hpp"
#include "Manifest.hpp"
#include <Zyphryon.Job/Service.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Texture
{
    /// \brief Bakes source images into the engine's native texture format.
    class Baker final
    {
    public:

        /// \brief The key an importer is registered under: a source extension, lowercased and without its dot.
        using Extension = Str16;

        /// \brief Maps every accepted extension to the importer that claims it.
        using Registry  = Table<Extension, Retainer<Importer>>;

    public:

        /// \brief Constructs a baker with every importer the build enables already registered.
        ///
        /// \param Scheduler The pool the slices are baked on, which must outlive the baker.
        explicit Baker(Ref<Job::Service> Scheduler);

        /// \brief Registers an importer under each extension it accepts.
        ///
        /// An extension another importer already claims is taken over by \p Codec, which is how a host
        /// overrides a built-in decoder with its own.
        ///
        /// \param Codec The importer to register.
        void Register(ConstRetainer<Importer> Codec);

        /// \brief Unregisters whichever importer claims an extension.
        ///
        /// \param Type The source extension, with or without its leading dot.
        /// \return `true` when an importer was unregistered, otherwise `false`.
        Bool Unregister(Text Type);

        /// \brief Finds the importer that claims a source extension.
        ///
        /// \param Type The source extension, with or without its leading dot.
        /// \return The matching importer, or `nullptr` when no importer claims the extension.
        ConstPtr<Importer> Find(Text Type) const;

        /// \brief Gets every extension this baker accepts, keyed to the importer that claims it.
        ///
        /// \return The importer registry.
        ZY_INLINE ConstRef<Registry> GetRegistry() const
        {
            return mRegistry;
        }

        /// \brief Bakes an encoded source image held in memory.
        ///
        /// \param Source  The encoded source bytes.
        /// \param Type    The source extension, which selects the importer.
        /// \param Profile The settings controlling the bake.
        /// \return A blob holding the native texture bytes, or an empty blob on failure.
        Blob Bake(ConstSpan<Byte> Source, Text Type, ConstRef<Profile> Profile) const;

        /// \brief Bakes the frames a manifest names into one array texture, one slice per frame.
        ///
        /// \param Entries The frames to gather, in the order they become slices.
        /// \param Profile The settings controlling the bake.
        /// \return A blob holding the native texture bytes, or an empty blob on failure.
        Blob Assemble(ConstSpan<Manifest::Entry> Entries, ConstRef<Profile> Profile) const;

        /// \brief Bakes a source image on disk and writes the native texture to another path.
        ///
        /// \param Source      The source image path, whose extension selects the importer.
        /// \param Destination The native output path; any folder in it that does not exist yet is created.
        /// \param Profile     The settings controlling the bake.
        /// \return `true` when the texture was written, otherwise `false`.
        Bool Bake(Text Source, Text Destination, ConstRef<Profile> Profile) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Job::Service> mScheduler;
        Registry          mRegistry;
    };
}