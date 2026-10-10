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

#include "Zyphryon.Graphic/Types.hpp"
#include <d3dcompiler.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    /// \brief Represents the HLSL shader compiler.
    class D3D11Compiler final
    {
    public:

        /// \brief Constructs a new compiler instance.
        D3D11Compiler();

        /// \brief Destructs the compiler instance.
        ~D3D11Compiler();

        /// \brief Compiles an HLSL shader into DXBC bytecode from the given source code and macros.
        ///
        /// \param Code   The shader source code to compile.
        /// \param Tier   The feature tier, which picks the shader model.
        /// \param Stage  The shader stage (vertex, pixel, etc.) for which to compile the code.
        /// \param Macros The macros to define during compilation, ended by a null entry.
        /// \return The compiled bytecode, which the caller releases, or `nullptr` when compilation fails.
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

        /// \brief List of available D3DCompiler profiles, indexed by \ref Tier.
        static constexpr LPCSTR  kProfiles[][2] =
        {
            { nullptr,  nullptr  },  // Tier::Level1
            { "vs_4_0", "ps_4_0" },  // Tier::Level2
            { "vs_5_0", "ps_5_0" },  // Tier::Level3
            { "vs_5_0", "ps_5_0" },  // Tier::Level4
            { "vs_5_0", "ps_5_0" },  // Tier::Level5
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        HMODULE     mModule;
        pD3DCompile mFunction;
    };
}