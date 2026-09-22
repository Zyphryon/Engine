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

#include "Zyphryon.Base/Container/Span.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Holds a library the system loaded, and answers for the symbols it was built with.
    class ZY_API Library final
    {
    public:

        /// \brief The handle the system knows a loaded library by.
        using Handle = Ptr<void>;

#if   defined(ZY_PLATFORM_WINDOWS)
        static constexpr Text kExtension = "dll";
#elif defined(ZY_PLATFORM_MACOS)
        static constexpr Text kExtension = "dylib";
#elif defined(ZY_PLATFORM_WEB)
        static constexpr Text kExtension = "wasm";
#else
        static constexpr Text kExtension = "so";
#endif

    public:

        /// \brief Constructs a library holding nothing.
        Library();

        /// \brief Constructs a library by loading the file at a path, holding nothing where it will not load.
        ///
        /// \param Path The path of the library file, as the system names one.
        explicit Library(Text Path);

        /// \brief Takes ownership of whatever another library held, leaving it holding nothing.
        ///
        /// \param Other The library to take from.
        Library(AnyRef<Library> Other);

        /// \brief Lets go of the library, and with it every symbol taken out of it.
        ~Library();

        /// \brief Libraries are not copied, since each one is let go when its holder is.
        Library(ConstRef<Library> Other) = delete;

        /// \brief Loads the library at a path, letting go of whatever was held before.
        ///
        /// \param Path The path of the library file, as the system names one.
        /// \return `true` when the library loaded, `false` otherwise.
        Bool Open(Text Path);

        /// \brief Lets go of the library, and with it every symbol taken out of it.
        void Close();

        /// \brief Checks whether a library is held.
        ///
        /// \return `true` when one is held, `false` otherwise.
        ZY_INLINE Bool IsOpen() const
        {
            return mHandle != nullptr;
        }

        /// \brief Gets the handle the system knows the library by.
        ///
        /// \return The handle, or `nullptr` where none is held.
        ZY_INLINE Handle GetHandle() const
        {
            return mHandle;
        }

        /// \brief Finds a symbol the library was built with.
        ///
        /// \param Name The name the symbol was exported under, which a C++ one is decorated beyond recognition by.
        /// \return The address of the symbol, or `nullptr` where the library holds none of that name.
        Ptr<void> GetSymbol(Text Name) const;

        /// \brief Finds a function the library was built with, read as the signature it is called through.
        ///
        /// \tparam Type The signature of the function, which nothing checks against what was exported.
        /// \param  Name The name the function was exported under.
        /// \return The function, or `nullptr` where the library holds none of that name.
        template<typename Type>
        ZY_INLINE Type GetFunction(Text Name) const
        {
            return reinterpret_cast<Type>(GetSymbol(Name));
        }

        /// \brief Libraries are not copied, since each one is let go when its holder is.
        Ref<Library> operator=(ConstRef<Library> Other) = delete;

        /// \brief Takes ownership of whatever another library held, letting go of whatever this one held.
        ///
        /// \param Other The library to take from.
        /// \return This library, now holding what the other did.
        Ref<Library> operator=(AnyRef<Library> Other);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle mHandle;
    };
}