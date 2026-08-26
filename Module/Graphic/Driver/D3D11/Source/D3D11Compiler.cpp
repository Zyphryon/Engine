// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "D3D11Compiler.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    D3D11Compiler::D3D11Compiler()
        : mModule   { nullptr },
          mFunction { nullptr }
    {
        for (const LPCWSTR Library : kLibraries)
        {
            if (mModule = ::LoadLibraryW(Library); mModule)
            {
                mFunction = reinterpret_cast<pD3DCompile>(::GetProcAddress(mModule, "D3DCompile"));
                break;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    D3D11Compiler::~D3D11Compiler()
    {
        if (mModule)
        {
            ::FreeLibrary(mModule);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<ID3DBlob> D3D11Compiler::Compile(ConstRef<Blob> Code, Tier Tier, ShaderStage Stage, ConstPtr<D3D_SHADER_MACRO> Macros)
    {
        if (mFunction == nullptr)
        {
            LOG_E("Failed to compile shader: no D3DCompiler library is available");
            return nullptr;
        }

        const LPCSTR Profile = kProfiles[Enum::Cast(Tier)][Enum::Cast(Stage)];

        if (Profile == nullptr)
        {
            LOG_E("Failed to compile shader: '{0}' carries no profile", Enum::GetName(Tier));
            return nullptr;
        }

        Ptr<ID3DBlob> Error    = nullptr;
        Ptr<ID3DBlob> Bytecode = nullptr;

        const HRESULT Result   = mFunction(
            Code.GetData(),
            Code.GetSize(),
            nullptr,
            Macros,
            nullptr,
            "main",
            Profile,
            D3DCOMPILE_OPTIMIZATION_LEVEL3,
            0,
            AddressOf(Bytecode),
            AddressOf(Error));

        if (FAILED(Result))
        {
            const ConstPtr<Char> Data   = (Error ? static_cast<ConstPtr<Char>>(Error->GetBufferPointer()) : nullptr);
            UInt                 Length = (Error ? Error->GetBufferSize() : 0);

            while (Length > 0 && (Data[Length - 1] == '\0' || Data[Length - 1] == '\n' || Data[Length - 1] == '\r'))
            {
                --Length;
            }

            LOG_E("Failed to compile {0} shader: {1}", Enum::GetName(Stage), Text(Data, Length));
        }
        return Bytecode;
    }
}