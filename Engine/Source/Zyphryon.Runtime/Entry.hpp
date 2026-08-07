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

#include "Kernel.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Runtime
{
    /// \brief Creates the set of modules the application was linked against.
    ///
    /// \note Defined by the translation unit `ZyApplyModules` generates for every application target.
    ///
    /// \return The modules the kernel attaches on startup.
    Engine::Modules ZyRegisterModules();

    /// \brief Boots a kernel of the given type and runs it until the application exits.
    ///
    /// \note Called by \ref ZY_APPLICATION, which supplies the arguments the platform handed the process.
    ///
    /// \param Count     The number of entries in \p Arguments.
    /// \param Arguments The argument vector, as handed to the platform entry point.
    /// \return The exit code the process reports.
    template<typename Type>
    SInt32 Launch(UInt Count, ConstPtr<ConstPtr<Char>> Arguments)
    {
        static_assert(IsDerived<Kernel, Type>, "ZY_APPLICATION expects a type that derives from Runtime::Kernel");

        Type Application;
        Application.Run(Count, Arguments, ZyRegisterModules());

        return 0;
    }
}

/// \def ZY_APPLICATION
/// \brief Defines the platform's entry point, booting \p Type as the application.
///
#if    defined(ZY_PLATFORM_ANDROID)

#error "Zyphryon: the Android entry point is not implemented yet"

#else

#define ZY_APPLICATION(Type)                                                \
        int main(int Count, char * Arguments[])                                \
        {                                                                      \
            return Runtime::Launch<Type>(static_cast<UInt>(Count), Arguments); \
        }

#endif