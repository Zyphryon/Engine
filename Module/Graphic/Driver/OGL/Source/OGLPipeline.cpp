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

#include "OGLPipeline.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Pipeline::Pipeline()
        : mHandle   { 0 },
          mKey      { 0 },
          mTopology { GL_TRIANGLES }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Pipeline::Create(ConstRef<Device> Device, Ref<Compiler> Compiler, ConstRef<Program> Program, ConstRef<Signature> Signature, ConstRef<States> States)
    {
        // Clamped depth and wireframe only change what crosses the near or far plane and how edges look, so both degrade.
        if (!States.DepthClip && !Device.Has(Feature::DepthClamp))
        {
            LOG_W("Graphics: The context lacks DepthClamp, so the pipeline's depth is clipped rather than clamped");
        }

        if (States.Fill == Fill::Wireframe && !Device.Has(Feature::PolygonMode))
        {
            LOG_W("Graphics: The context lacks PolygonMode, so the pipeline's wireframe is drawn filled");
        }

        mKey = Compiler.Acquire(Program);

        if (mKey == 0)
        {
            return false;
        }

        mBlend.Enable        = States.UsesBlending();
        mBlend.SrcColor      = Convert(States.BlendSrcColor);
        mBlend.DstColor      = Convert(States.BlendDstColor);
        mBlend.EquationColor = Convert(States.BlendEquationColor);
        mBlend.SrcAlpha      = Convert(States.BlendSrcAlpha);
        mBlend.DstAlpha      = Convert(States.BlendDstAlpha);
        mBlend.EquationAlpha = Convert(States.BlendEquationAlpha);
        mBlend.Mask          = ZyEnum::Cast(States.Channel);

        mDepth.Enable = States.UsesDepth();
        mDepth.Write  = States.DepthMask;
        mDepth.Test   = Convert(States.DepthTest);
        mDepth.Clip   = States.DepthClip;
        mDepth.Bias   = States.DepthBias;
        mDepth.Slope  = States.DepthBiasSlope;

        mStencil.Enable          = States.UsesStencil();
        mStencil.Read            = States.StencilReadMask;
        mStencil.Write           = States.StencilWriteMask;
        mStencil.Front.Test      = Convert(States.StencilFrontTest);
        mStencil.Front.Fail      = Convert(States.StencilFrontFail);
        mStencil.Front.DepthFail = Convert(States.StencilFrontDepthFail);
        mStencil.Front.DepthPass = Convert(States.StencilFrontDepthPass);
        mStencil.Back.Test       = Convert(States.StencilBackTest);
        mStencil.Back.Fail       = Convert(States.StencilBackFail);
        mStencil.Back.DepthFail  = Convert(States.StencilBackDepthFail);
        mStencil.Back.DepthPass  = Convert(States.StencilBackDepthPass);

        mRaster.Cull    = States.Cull != Cull::None;
        mRaster.Face    = Convert(States.Cull);
        mRaster.Scissor = States.Scissor;
        mRaster.Fill    = (States.Fill == Fill::Wireframe && Device.Has(Feature::PolygonMode)) ? GL_LINE : GL_FILL;

        mTopology = Convert(States.Topology);
        mLayout   = Signature.Attributes;
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Pipeline::Settle(Ref<Compiler> Compiler)
    {
        if (mKey == 0)
        {
            return false;
        }

        mHandle = Compiler.Settle(mKey);
        return mHandle != 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Pipeline::Delete(Ref<Compiler> Compiler)
    {
        if (mKey)
        {
            Compiler.Release(mKey);
        }
        (* this) = Pipeline();
    }
}