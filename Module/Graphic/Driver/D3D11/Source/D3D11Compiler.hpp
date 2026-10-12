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

#include "D3D11Common.hpp"
#include <d3dcompiler.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    /// \brief Represents the HLSL shader compiler.
    class Compiler final
    {
    public:

        /// \brief Constructs a compiler that holds no library until \ref Initialize.
        Compiler();

        /// \brief Releases the compiler library.
        ~Compiler();

        /// \brief Loads the compiler library, which every supported Windows ships in its system folder.
        ///
        /// \return `true` if the library and its entry point were found, `false` otherwise.
        Bool Initialize();

        /// \brief Compiles an HLSL shader into DXBC bytecode from the given source code and macros.
        ///
        /// \param Code   The shader source code to compile.
        /// \param Tier   The feature tier, which picks the shader model.
        /// \param Stage  The shader stage (vertex, pixel, etc.) for which to compile the code.
        /// \param Macros The macros to define during compilation, ended by a null entry.
        /// \return The compiled bytecode, or empty when compilation fails.
        ComPtr<ID3DBlob> Compile(ConstRef<Blob> Code, Tier Tier, ShaderStage Stage, ConstPtr<D3D_SHADER_MACRO> Macros) const;

        /// \brief Reflects compiled bytecode, so the resources its stage reads can be listed.
        ///
        /// \param Bytecode The bytecode returned by \ref Compile.
        /// \return The reflection of the bytecode, or empty when it cannot be reflected.
        ComPtr<ID3D11ShaderReflection> Reflect(Ptr<ID3DBlob> Bytecode) const;

    private:

        /// The vertex and pixel profiles of each tier, `nullptr` where none applies.
        static constexpr LPCSTR kProfiles[][2] =
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

        HMODULE                mModule;
        pD3DCompile            mCompile;
        decltype(& D3DReflect) mReflect;
    };
}