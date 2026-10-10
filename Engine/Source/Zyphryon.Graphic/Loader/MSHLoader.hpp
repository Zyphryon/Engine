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

#include "Zyphryon.Content/Loader.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    /// \brief Represents the content loader for the engine's native, chunked binary mesh format.
    class MSHLoader final : public ZyContent::Loader
    {
    public:

        /// \brief The extensions this content loader handles.
        static constexpr Text kTypes[] = { "msh" };

    public:

        /// \see ZyContent::Loader::Load(Ref<ZyContent::Service>, Ref<ZyContent::Scope>, AnyRef<Blob>)
        Bool Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data) override;

    private:

        /// \brief The tag of the chunk describing the mesh.
        static constexpr UInt32 kMesh     = 'M' | ('E' << 8) | ('S' << 16) | ('H' << 24);

        /// \brief The tag of the chunk holding the vertex buffer.
        static constexpr UInt32 kVertices = 'V' | ('B' << 8) | ('U' << 16) | ('F' << 24);

        /// \brief The tag of the chunk holding the index buffer.
        static constexpr UInt32 kIndices  = 'I' | ('B' << 8) | ('U' << 16) | ('F' << 24);
    };
}