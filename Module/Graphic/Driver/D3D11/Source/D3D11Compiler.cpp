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

namespace ZyGraphic::D3D11
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Compiler::Compiler()
        : mModule  { nullptr },
          mCompile { nullptr },
          mReflect { nullptr }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Compiler::~Compiler()
    {
        if (mModule)
        {
            ::FreeLibrary(mModule);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Compiler::Initialize()
    {
        if (mModule = LoadSystemLibrary(L"D3DCompiler_47.dll"); mModule)
        {
            mCompile = reinterpret_cast<pD3DCompile>(::GetProcAddress(mModule, "D3DCompile"));
            mReflect = reinterpret_cast<decltype(& D3DReflect)>(::GetProcAddress(mModule, "D3DReflect"));
        }

        if (mCompile == nullptr || mReflect == nullptr)
        {
            LOG_E("Graphics: Failed to load D3DCompiler_47.dll");
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ComPtr<ID3DBlob> Compiler::Compile(ConstRef<Blob> Code, Tier Tier, ShaderStage Stage, ConstPtr<D3D_SHADER_MACRO> Macros) const
    {
        ZY_ASSERT(mCompile, "The compiler was used before it was initialized");

        const LPCSTR Profile = kProfiles[ZyEnum::Cast(Tier)][ZyEnum::Cast(Stage)];

        if (Profile == nullptr)
        {
            LOG_E("Graphics: Failed to compile shader: '{0}' carries no profile", ZyEnum::GetName(Tier));
            return nullptr;
        }

        ComPtr<ID3DBlob> Error;
        ComPtr<ID3DBlob> Bytecode;

        const HRESULT Result = mCompile(
            Code.GetData(),
            Code.GetSize(),
            nullptr,
            Macros,
            nullptr,
            "main",
            Profile,
            D3DCOMPILE_OPTIMIZATION_LEVEL3,
            0,
            Bytecode.GetAddressOf(),
            Error.GetAddressOf());

        if (FAILED(Result))
        {
            const ConstPtr<Char> Data   = (Error ? static_cast<ConstPtr<Char>>(Error->GetBufferPointer()) : nullptr);
            UInt                 Length = (Error ? Error->GetBufferSize() : 0);

            while (Length > 0 && (Data[Length - 1] == '\0' || Data[Length - 1] == '\n' || Data[Length - 1] == '\r'))
            {
                --Length;
            }

            LOG_E("Graphics: Failed to compile {0} shader: {1}", ZyEnum::GetName(Stage), Text(Data, Length));
            return nullptr;
        }
        return Bytecode;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ComPtr<ID3D11ShaderReflection> Compiler::Reflect(Ptr<ID3DBlob> Bytecode) const
    {
        ZY_ASSERT(mReflect, "The compiler was used before it was initialized");

        ComPtr<ID3D11ShaderReflection> Reflection;

        if (!Check(mReflect(Bytecode->GetBufferPointer(), Bytecode->GetBufferSize(), IID_PPV_ARGS(Reflection.GetAddressOf()))))
        {
            return nullptr;
        }
        return Reflection;
    }
}