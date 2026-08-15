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

#include "Technique.hpp"
#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Technique::Technique(AnyRef<Content::Uri> Key)
        : AbstractResource { Move(Key) },
          mHandle          { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Technique::Setup(AnyRef<Description> Description, AnyRef<Schema> Schema)
    {
        mDescription = Move(Description);
        mSchema      = Move(Schema);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Technique::Obtain(Ref<Service> Service, Key Key)
    {
        if (Key == 0)
        {
            return mHandle;
        }

        if (const ConstPtr<Object> Handle = mVariants.Find(Key))
        {
            return (* Handle ? * Handle : mHandle);
        }

        const Object Handle = Compile(Service, Key);
        mVariants.Assign(Key, Handle);

        return (Handle ? Handle : mHandle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Technique::Key Technique::Resolve(ConstRef<Material> Material) const
    {
        Key Result = 0;

        for (UInt Index = 0, Limit = mDescription.Features.GetSize(); Index < Limit; ++Index)
        {
            ConstRef<Feature> Feature = mDescription.Features[Index];

            const Bool Enabled = (Feature.Texture   && Material.GetImage(Feature.Texture))
                              || (Feature.Parameter && Material.GetParameter(Feature.Parameter));

            if (Enabled)
            {
                Result = SetBit(Result, 1u << Index);
            }
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Technique::Upload(Ref<Service> Service)
    {
        ZY_ASSERT(mHandle == 0, "Technique has already been created");

        mSchema.Resolve(Service);

        mHandle = Compile(Service, 0);

        // Building the preloaded variants here keeps their shader compile off the frame that first draws them.
        for (const Key Key : mDescription.Preload)
        {
            if (Key != 0 && !mVariants.Contains(Key))
            {
                mVariants.Assign(Key, Compile(Service, Key));
            }
        }
        return (mHandle > 0);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Technique::Unload(Ref<Service> Service)
    {
        const ConstPtr<Table<Key, Object>::Pair> Variants = mVariants.GetData();

        for (UInt Index = 0, Limit = mVariants.GetSize(); Index < Limit; ++Index)
        {
            if (const Object Handle = Variants[Index].Second)
            {
                Service.DeletePipeline(Handle);
            }
        }
        mVariants.Clear();

        if (mHandle != 0)
        {
            Service.DeletePipeline(mHandle);

            mHandle = 0;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Technique::OnReload(Ref<Engine::Subsystem::Host> Host)
    {
        Ref<Content::Service> Service = * Host.GetService<Content::Service>();

        for (ConstRetainer<Shader> Shader : mDescription.Base.Shaders)
        {
            if (Shader)
            {
                Service.Reload(Shader);
            }
        }

        for (ConstRef<Feature> Feature : mDescription.Features)
        {
            for (ConstRetainer<Shader> Shader : Feature.Patch.Shaders)
            {
                if (Shader)
                {
                    Service.Reload(Shader);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Technique::Assemble(Key Key, Ref<Program> Program, Ref<Signature> Signature, Ref<States> States) const
    {
        ConstRef<Layer> Base = mDescription.Base;

        Shaders Modules = Base.Shaders;

        // Only the attributes are taken from the layer, since one schema describes every variant's bindings.
        Program.Macros       = Base.Macros;
        Signature            = mDescription.Signature;
        Signature.Attributes = Base.Attributes;
        States               = Base.States;

        // A feature replaces whole state blocks, so the last enabled one declaring a block wins it outright.
        UInt8 Claimed = 0;

        for (UInt Index = 0, Limit = mDescription.Features.GetSize(); Index < Limit; ++Index)
        {
            if (!HasBit(Key, 1u << Index))
            {
                continue;
            }

            ConstRef<Feature> Feature = mDescription.Features[Index];
            ConstRef<Layer>   Patch   = Feature.Patch;

            Program.Macros.Append(Patch.Macros);

            for (UInt Stage = 0, Stages = Enum::Count<ShaderStage>(); Stage < Stages; ++Stage)
            {
                if (Patch.Shaders[Stage])
                {
                    Modules[Stage] = Patch.Shaders[Stage];
                }
            }

            if (!Patch.Attributes.IsEmpty())
            {
                Signature.Attributes = Patch.Attributes;
            }

            // Two features writing one block is an authoring mistake, since only the later one survives.
            if (Feature.Blocks & Claimed)
            {
                LOG_W("'{0}' variant {1} lets '{2}' overwrite a state block", GetKey(), Key, Feature.Name);
            }

            Converge(States, Patch.States, Feature.Blocks);
            Claimed = SetBit(Claimed, Feature.Blocks);
        }

        for (UInt Index = 0, Limit = Enum::Count<ShaderStage>(); Index < Limit; ++Index)
        {
            if (ConstRetainer<Shader> Shader = Modules[Index])
            {
                Program.Modules[Index] = Blob::Borrow(Shader->GetSource());
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Technique::Converge(Ref<States> Destination, ConstRef<States> Source, UInt8 Blocks) const
    {
        if (HasBit(Blocks, GetBlockMask(Block::Blend)))
        {
            Destination.AlphaToCoverage    = Source.AlphaToCoverage;
            Destination.Channel            = Source.Channel;
            Destination.BlendSrcColor      = Source.BlendSrcColor;
            Destination.BlendDstColor      = Source.BlendDstColor;
            Destination.BlendEquationColor = Source.BlendEquationColor;
            Destination.BlendSrcAlpha      = Source.BlendSrcAlpha;
            Destination.BlendDstAlpha      = Source.BlendDstAlpha;
            Destination.BlendEquationAlpha = Source.BlendEquationAlpha;
        }

        if (HasBit(Blocks, GetBlockMask(Block::Depth)))
        {
            Destination.DepthClip      = Source.DepthClip;
            Destination.DepthMask      = Source.DepthMask;
            Destination.DepthTest      = Source.DepthTest;
            Destination.DepthBias      = Source.DepthBias;
            Destination.DepthBiasClamp = Source.DepthBiasClamp;
            Destination.DepthBiasSlope = Source.DepthBiasSlope;
        }

        if (HasBit(Blocks, GetBlockMask(Block::Stencil)))
        {
            Destination.StencilReadMask       = Source.StencilReadMask;
            Destination.StencilWriteMask      = Source.StencilWriteMask;
            Destination.StencilBackTest       = Source.StencilBackTest;
            Destination.StencilBackFail       = Source.StencilBackFail;
            Destination.StencilBackDepthFail  = Source.StencilBackDepthFail;
            Destination.StencilBackDepthPass  = Source.StencilBackDepthPass;
            Destination.StencilFrontTest      = Source.StencilFrontTest;
            Destination.StencilFrontFail      = Source.StencilFrontFail;
            Destination.StencilFrontDepthFail = Source.StencilFrontDepthFail;
            Destination.StencilFrontDepthPass = Source.StencilFrontDepthPass;
        }

        if (HasBit(Blocks, GetBlockMask(Block::Rasterizer)))
        {
            Destination.Fill    = Source.Fill;
            Destination.Cull    = Source.Cull;
            Destination.Scissor = Source.Scissor;
        }

        if (HasBit(Blocks, GetBlockMask(Block::Layout)))
        {
            Destination.Topology = Source.Topology;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Technique::Compile(Ref<Service> Service, Key Key)
    {
        Program   Program;
        Signature Signature;
        States    States;

        Assemble(Key, Program, Signature, States);

        return Service.CreatePipeline(Move(Program), Signature, States);
    }
}