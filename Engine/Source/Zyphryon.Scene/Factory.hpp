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

namespace Scene
{
    /// \brief Type-erased serialization dispatcher for a single component type.
    class Factory final
    {
    public:

        /// \brief Signature for a component read or write handler.
        ///
        /// \tparam Type Either `Reader` or `Writer`, selecting the I/O direction.
        template<typename Type>
        using Handler = void (*)(Archive<Type>, Ptr<void>);

    public:

        /// \brief Constructs an invalid factory with no handlers attached.
        ZY_INLINE Factory() = default;

        /// \brief Constructs a factory with explicit read and write handlers.
        ///
        /// \param Reader The handler invoked when deserializing a component from a binary stream.
        /// \param Writer The handler invoked when serializing a component into a binary stream.
        ZY_INLINE Factory(Handler<Reader> Reader, Handler<Writer> Writer)
            : mReader { Reader },
              mWriter { Writer }
        {
        }

        /// \brief Deserializes a component from a binary stream.
        ///
        /// \param Reader    The binary stream to read component data from.
        /// \param Component The raw pointer to the component instance.
        ZY_INLINE void Read(Ref<Reader> Reader, Ptr<void> Component) const
        {
            mReader(Archive(Reader), Component);
        }

        /// \brief Serializes a component into a binary stream.
        ///
        /// \param Writer    The binary stream to write component data into.
        /// \param Component The raw pointer to the component instance.
        ZY_INLINE void Write(Ref<Writer> Writer, Ptr<void> Component) const
        {
            mWriter(Archive(Writer), Component);
        }

    public:

        /// \brief Creates a `Factory` bound to a specific component type.
        ///
        /// \tparam Component The component type to bind. Must be a complete type.
        /// \return A fully initialized `Factory` ready to read and write instances of `Component`.
        template<typename Component>
        ZY_INLINE static Factory Create()
        {
            return Factory(OnRead<Component>, OnWrite<Component>);
        }

    private:

        /// \brief Read handler generated for `Type` by `Create<Type>()`.
        ///
        /// \param Archive   The binary stream to read component data from.
        /// \param Component The raw pointer to the component instance.
        template<typename Type>
        ZY_INLINE static void OnRead(Archive<Reader> Archive, Ptr<void> Component)
        {
            Archive.Serialize(Text::Empty(), * static_cast<Ptr<Type>>(Component));
        }

        /// \brief Write handler generated for `Type` by `Create<Type>()`.
        ///
        /// \param Archive   The binary stream to write component data into.
        /// \param Component The raw pointer to the component instance.
        template<typename Type>
        ZY_INLINE static void OnWrite(Archive<Writer> Archive, Ptr<void> Component)
        {
            Archive.Serialize(Text::Empty(), * static_cast<Ptr<Type>>(Component));
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handler<Reader> mReader;
        Handler<Writer> mWriter;
    };
}