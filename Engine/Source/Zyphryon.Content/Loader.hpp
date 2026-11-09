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

#include "Scope.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyContent
{
    class Service;

    /// \brief Defines the interface for decoding binary content into resource objects.
    class ZY_API Loader : public Retainable<Loader>
    {
    public:

        /// \brief Destructor for proper cleanup in derived classes.
        virtual ~Loader() = default;

        /// \brief Decodes binary data into a resource object within the given scope.
        ///
        /// \param Service The content service managing resource lifetimes.
        /// \param Scope   The scope that tracks dependencies for the loading operation.
        /// \param Data    The raw binary data to decode.
        /// \return `true` if the resource was loaded successfully, otherwise `false`.
        virtual Bool Load(Ref<Service> Service, Ref<Scope> Scope, AnyRef<Blob> Data) = 0;

    protected:

        /// \brief Reads the header a baked binary format opens with, refusing a file of another format or revision.
        ///
        /// \param Input   The reader standing on the first byte of the file, left past the header when it is taken.
        /// \param Key     The key of the resource being loaded, which a refusal is logged under.
        /// \param Magic   The four characters the format opens with.
        /// \param Version The layout revision the caller reads.
        /// \return `true` when the file opens with the magic and the revision, `false` when it was refused.
        static Bool Expect(Ref<Reader> Input, ConstRef<Uri> Key, Text Magic, UInt16 Version);

        /// \brief Parses the JSON object a textual format holds, refusing a file that holds anything else.
        ///
        /// \param Data     The raw text of the file.
        /// \param Key      The key of the resource being loaded, which a refusal is logged under.
        /// \param Kind     The name a refusal calls the format by.
        /// \param Document Receives the parsed document, whose root is an object.
        /// \return `true` when the file parsed into an object, `false` when it was refused.
        static Bool Interpret(ConstRef<Blob> Data, ConstRef<Uri> Key, Text Kind, Ref<JsonValue> Document);
    };
}