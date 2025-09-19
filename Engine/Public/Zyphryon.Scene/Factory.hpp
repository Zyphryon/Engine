// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Base/IO/Archive.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Handles serialization and deserialization for a specific component type.
    class Factory final
    {
    public:

        /// \brief Type alias for serializer functions.
        template<typename Type>
        using Serializer = void (*)(Archive<Type>, Ptr<void>);

    public:

        /// \brief Constructs an empty factory with no serializers.
        ZYPHRYON_INLINE Factory() = default;

        /// \brief Constructs a factory with explicit serializer functions.
        ///
        /// \param Reader Function pointer used for deserialization.
        /// \param Writer Function pointer used for serialization.
        ZYPHRYON_INLINE Factory(Serializer<Reader> Reader, Serializer<Writer> Writer)
            : mReader { Reader },
              mWriter { Writer }
        {
        }

        /// \brief Deserializes a component from an archive.
        ///
        /// \param Reader    The archive instance performing the deserialization.
        /// \param Component Pointer to the raw memory of the component instance.
        ZYPHRYON_INLINE void Read(Ref<Reader> Reader, Ptr<void> Component) const
        {
            mReader(Archive(Reader), Component);
        }

        /// \brief Serializes a component into an archive.
        ///
        /// \param Writer    The archive instance performing the serialization.
        /// \param Component Pointer to the raw memory of the component instance.
        ZYPHRYON_INLINE void Write(Ref<Writer> Writer, Ptr<void> Component) const
        {
            mWriter(Archive(Writer), Component);
        }

    public:

        /// \brief Creates a Factory instance for a specific component type.
        ///
        /// \tparam Component The component type to generate serialization support for.
        ///
        /// \return Factory instance with type-specific serialization handlers.
        template<typename Component>
        ZYPHRYON_INLINE static Factory Create()
        {
            return Factory(&OnComponentRead<Component>, &OnComponentWrite<Component>);
        }

    private:

        /// \brief Static deserialization callback for component data.
        ///
        /// \param Archive   The archive instance performing the deserialization.
        /// \param Component Pointer to the raw memory of the component instance.
        template<typename Type>
        ZYPHRYON_INLINE static void OnComponentRead(Archive<Reader> Archive, Ptr<void> Component)
        {
            if constexpr(CanSerialize<Type, Base::Archive<Reader>>)
            {
                static_cast<Ptr<Type>>(Component)->OnSerialize(Archive);
            }
        }

        /// \brief Static serialization callback for component data.
        ///
        /// \param Archive   The archive instance performing the serialization.
        /// \param Component Pointer to the raw memory of the component instance.
        template<typename Type>
        ZYPHRYON_INLINE static void OnComponentWrite(Archive<Writer> Archive, Ptr<void> Component)
        {
            if constexpr(CanSerialize<Type, Base::Archive<Writer>>)
            {
                static_cast<Ptr<Type>>(Component)->OnSerialize(Archive);
            }
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Serializer<Reader> mReader;
        Serializer<Writer> mWriter;
    };
}