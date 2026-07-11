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

#include "Zyphryon.Graphic/Backend/Common.hpp"
#include <d3dcompiler.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief TODO_DOC
    class D3D11Compiler final
    {
    public:

        /// \brief Constructs a new compiler instance.
        D3D11Compiler();

        /// \brief Destructs the compiler instance.
        ~D3D11Compiler();

        /// \brief Compiles a HLSL shader into DXBC bytecode from the given source code and macros.
        ///
        /// \param Code   The shader source code to compile.
        /// \param Tier   The shader compilation tier.
        /// \param Stage  The shader stage (vertex, pixel, etc.) for which to compile the code.
        /// \param Macros Optional macros to define during compilation.
        /// \return A pointer to the compiled shader bytecode blob.
        Ptr<ID3DBlob> Compile(ConstRef<Blob> Code, Tier Tier, ShaderStage Stage, ConstPtr<D3D_SHADER_MACRO> Macros);

    private:

        /// \brief List of available D3DCompiler libraries.
        static constexpr LPCWSTR kLibraries[] =
        {
            L"D3DCompiler_47",
            L"D3DCompiler_46",
            L"D3DCompiler_43",
            L"D3DCompiler_42",
            L"D3DCompiler_41",
            L"D3DCompiler_40"
        };

        /// \brief List of available D3DCompiler profiles.
        static constexpr LPCSTR  kProfiles[][2] =
        {
            { "vs_4_0_level_9_3", "ps_4_0_level_9_3" },
            { "vs_4_0",           "ps_4_0"           },
            { "vs_5_0",           "ps_5_0"           },
            { "vs_6_0",           "ps_6_0"           }
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        HMODULE     mModule;
        pD3DCompile mFunction;
    };
}