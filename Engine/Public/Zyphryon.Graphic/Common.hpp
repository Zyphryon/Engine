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

namespace Graphic
{
    /// \brief Constants used across the graphic module.
    enum : UInt
    {
        /// \brief Alignment requirement for GPU uniform buffer offsets.
        kUniformBlockAlignment = 0x0100,

        /// \brief Maximum block size (64 KB) used for uniform buffer pages.
        kUniformBlockCapacity  = 0x10000,

        /// \brief Default swapchain buffer for display presentation.
        kDisplay               = 0x0000,

        /// \brief Maximum number of color attachments in a render pass.
        kMaxAttachments        = 0x0004,

        /// \brief Maximum number of vertex attributes in a vertex buffer.
        kMaxAttributes         = 0x0010,

        /// \brief Maximum number of buffer resources.
        kMaxBuffers            = 0x1000,

        /// \brief Maximum number of frames in flight.
        kMaxFrames             = 0x0002,

        /// \brief Maximum number of material resources.
        kMaxMaterials          = 0x1000,

        /// \brief Maximum number of mipmap levels per texture.
        kMaxMipmaps            = 0x0010,

        /// \brief Maximum number of multisampling levels supported.
        kMaxMultisamples       = 0x0008,

        /// \brief Maximum number of render pass resources.
        kMaxPasses             = 0x0040,

        /// \brief Maximum number of pipeline resources.
        kMaxPipelines          = 0x00C0,

        /// \brief Maximum number of shader resources.
        kMaxResources          = 0x0008,

        /// \brief Maximum number of texture resources.
        kMaxTextures           = 0x1000,
    };

    /// \brief Describes the available graphics backend.
    enum class Backend : UInt8
    {
        None,         ///< No backend selected.
        D3D11,        ///< Direct3D 11 backend.
        GLES3,        ///< OpenGL ES 3.x backend.
    };

    /// \brief Specifies the memory access of a resource.
    enum class Access : UInt8
    {
        Immutable, ///< Initialized once at creation; read-only by the GPU.
        Dynamic,   ///< Written frequently by the CPU; read by the GPU (mapped access).
        Stream,    ///< Written occasionally by the CPU via explicit uploads; read by the GPU.
    };

    /// \brief Specifies how source and destination colors are factored in blending.
    enum class BlendFactor : UInt8
    {
        Zero,              ///< Use 0.0 as blend factor.
        One,               ///< Use 1.0 as blend factor.
        SrcColor,          ///< Use the source color.
        OneMinusSrcColor,  ///< Use 1.0 minus the source color.
        SrcAlpha,          ///< Use the source alpha.
        OneMinusSrcAlpha,  ///< Use 1.0 minus the source alpha.
        DstColor,          ///< Use the destination color.
        OneMinusDstColor,  ///< Use 1.0 minus the destination color.
        DstAlpha,          ///< Use the destination alpha.
        OneMinusDstAlpha,  ///< Use 1.0 minus the destination alpha.
        Src1Color,         ///< Use the second source color.
        OneMinusSrc1Color, ///< Use 1.0 minus the second source color.
        Src1Alpha,         ///< Use the second source alpha.
        OneMinusSrc1Alpha, ///< Use 1.0 minus the second source alpha.
    };

    /// \brief Specifies the blending function used to combine source and destination.
    enum class BlendFunction : UInt8
    {
        Add,             ///< Add source and destination.
        Subtract,        ///< Subtract destination from source.
        ReverseSubtract, ///< Subtract source from destination.
        Min,             ///< Use the minimum of source and destination.
        Max,             ///< Use the maximum of source and destination.
    };

    /// \brief Specifies which color channels are affected by certain operations.
    enum class Channel : UInt8
    {
        RGBA = 0b00001111, ///< Red, Green, Blue, Alpha.
        RGB  = 0b00000111, ///< Red, Green, Blue.
        RGA  = 0b00001011, ///< Red, Green, Alpha.
        RBA  = 0b00001101, ///< Red, Blue, Alpha.
        RG   = 0b00000011, ///< Red, Green.
        RB   = 0b00000101, ///< Red, Blue.
        RA   = 0b00001001, ///< Red, Alpha.
        R    = 0b00000001, ///< Red.
        GBA  = 0b00001110, ///< Green, Blue, Alpha.
        GB   = 0b00000110, ///< Green, Blue.
        GA   = 0b00001010, ///< Green, Alpha.
        G    = 0b00000010, ///< Green.
        BA   = 0b00001100, ///< Blue, Alpha.
        B    = 0b00000100, ///< Blue.
        A    = 0b00001000, ///< Alpha.
        None = 0b00000000, ///< No channels.
    };
    ZYPHRYON_DEFINE_BITWISE_ENUM(Channel)

    /// \brief Specifies which parts of a render pass should be cleared.
    enum class Clear : UInt8
    {
        None      = 0b0000, ///< Do not clear any attachments.
        Color     = 0b0001, ///< Clear color attachments.
        Depth     = 0b0010, ///< Clear depth attachment.
        Stencil   = 0b0100, ///< Clear stencil attachment.
        All       = 0b0111, ///< Clear color, depth, and stencil attachments.
    };
    ZYPHRYON_DEFINE_BITWISE_ENUM(Clear)

    /// \brief Specifies the origin location for the coordinate systems.
    enum class Coordinates : UInt8
    {
        Northwest,  ///< Origin at the top-left corner (Y increases downward).
        Southwest   ///< Origin at the bottom-left corner (Y increases upward).
    };

    /// \brief Specifies face culling behavior in rasterization.
    enum class Cull : UInt8
    {
        None,  ///< Do not cull faces.
        Back,  ///< Cull back-facing triangles.
        Front, ///< Cull front-facing triangles.
    };

    /// \brief Specifies the polygon fill mode used in rasterization.
    enum class Fill : UInt8
    {
        Solid,     ///< Render filled triangles.
        Wireframe, ///< Render triangles as wireframes.
    };

    /// \brief Specifies the shader language version.
    enum class Language : UInt8
    {
        SM1, ///< Shader Model 1
        SM2, ///< Shader Model 2
        SM3, ///< Shader Model 3
        SM4, ///< Shader Model 4
        SM5, ///< Shader Model 5
        SM6, ///< Shader Model 6
    };

    /// \brief Specifies the number of samples for multisampling.
    enum class Multisample : UInt8
    {
        X1,  ///< Single sample (no multisampling).
        X2,  ///< 2x multisampling.
        X4,  ///< 4x multisampling.
        X8,  ///< 8x multisampling.
        X16, ///< 16x multisampling.
    };

    /// \brief Specifies the type of geometric primitive to render.
    enum class Primitive : UInt8
    {
        PointList,      ///< Independent points (1 vertex per point).
        LineList,       ///< Independent lines (2 vertices per line).
        LineStrip,      ///< Connected lines (1 vertex per segment after the first).
        TriangleList,   ///< Independent triangles (3 vertices per triangle).
        TriangleStrip,  ///< Connected triangles (1 triangle per vertex after the first two).
    };

    /// \brief Specifies the shader stage in the graphics pipeline.
    enum class Stage : UInt8
    {
        Vertex,   ///< Vertex shader stage.
        Fragment, ///< Fragment (pixel) shader stage.
    };

    /// \brief Defines the action to perform when a test (e.g., stencil) passes or fails.
    enum class TestAction : UInt8
    {
        Keep,              ///< Keep the current value.
        Replace,           ///< Replace with the reference value.
        Zero,              ///< Set to zero.
        Decrement,         ///< Decrement the value, wrapping if necessary.
        DecrementSaturate, ///< Decrement the value, but clamp to zero.
        Invert,            ///< Bitwise invert the current value.
        Increment,         ///< Increment the value, wrapping if necessary.
        IncrementSaturate, ///< Increment the value, but clamp to the maximum.
    };

    /// \brief Defines the comparison condition used in depth or stencil tests.
    enum class TestCondition : UInt8
    {
        Always,       ///< Always passes the test.
        Never,        ///< Never passes the test.
        Greater,      ///< Passes if source > destination.
        GreaterEqual, ///< Passes if source >= destination.
        Equal,        ///< Passes if source == destination.
        NotEqual,     ///< Passes if source != destination.
        Less,         ///< Passes if source < destination.
        LessEqual,    ///< Passes if source <= destination.
    };

    /// \brief Defines how texture coordinates outside the [0,1] range are handled.
    enum class TextureAddress : UInt8
    {
        Clamp,   ///< Clamp to edge; coordinates are clamped to the nearest valid texel.
        Border,  ///< Clamp to border; coordinates outside range use a border color.
        Repeat,  ///< Repeat texture; coordinates wrap around.
        Mirror,  ///< Mirror texture at every integer boundary.
    };

    /// \brief Defines the border color used when border addressing mode is selected.
    enum class TextureBorder : UInt8
    {
        OpaqueBlack,        ///< Opaque black border color.
        OpaqueWhite,        ///< Opaque white border color.
        TransparentBlack,   ///< Transparent black border color.
        TransparentWhite,   ///< Transparent white border color.
    };

    /// \brief Specifies the sampling method used when accessing textures.
    enum class TextureFilter : UInt8
    {
        Point,           ///< Nearest-neighbor filtering.
        PointMipPoint,   ///< Nearest-neighbor filtering with point mipmaps.
        PointMipLinear,  ///< Nearest-neighbor filtering with linear mipmaps.
        Linear,          ///< Bilinear filtering.
        LinearMipPoint,  ///< Bilinear filtering with point mipmaps.
        LinearMipLinear, ///< Bilinear filtering with linear mipmaps.
        Anisotropic2x,   ///< Anisotropic filtering with max 2x samples.
        Anisotropic4x,   ///< Anisotropic filtering with max 4x samples.
        Anisotropic8x,   ///< Anisotropic filtering with max 8x samples.
        Anisotropic16x   ///< Anisotropic filtering with max 16x samples.
    };

    /// \brief Describes the format of texture data.
    enum class TextureFormat : UInt8
    {
        BC1UIntNorm,           ///< Block-compressed RGB (DXT1), unsigned normalized.
        BC1UIntNorm_sRGB,      ///< Block-compressed RGB (DXT1), sRGB gamma-corrected.
        BC2UIntNorm,           ///< Block-compressed RGBA (DXT3), unsigned normalized.
        BC2UIntNorm_sRGB,      ///< Block-compressed RGBA (DXT3), sRGB gamma-corrected.
        BC3UIntNorm,           ///< Block-compressed RGBA (DXT5), unsigned normalized.
        BC3UIntNorm_sRGB,      ///< Block-compressed RGBA (DXT5), sRGB gamma-corrected.
        BC4UIntNorm,           ///< Block-compressed single-channel (red), unsigned normalized.
        BC4SIntNorm,           ///< Block-compressed single-channel (red), signed normalized
        BC5UIntNorm,           ///< Block-compressed two-channel (red-green), unsigned normalized.
        BC5SIntNorm,           ///< Block-compressed two-channel (red-green), signed normalized
        BC6UFloat,             ///< Block-compressed HDR RGB, unsigned float
        BC6SFloat,             ///< Block-compressed HDR RGB, signed float
        BC7UIntNorm,           ///< High-quality block-compressed RGBA
        BC7UIntNorm_sRGB,      ///< High-quality block-compressed RGBA, sRGB
        R8SInt,                ///< 8-bit signed integer.
        R8SIntNorm,            ///< 8-bit signed normalized.
        R8UInt,                ///< 8-bit unsigned integer.
        R8UIntNorm,            ///< 8-bit unsigned normalized.
        R16SInt,               ///< 16-bit signed integer.
        R16SIntNorm,           ///< 16-bit signed normalized.
        R16UInt,               ///< 16-bit unsigned integer.
        R16UIntNorm,           ///< 16-bit unsigned normalized.
        R16Float,              ///< 16-bit floating-point.
        R32SInt,               ///< 32-bit signed integer.
        R32UInt,               ///< 32-bit unsigned integer.
        R32Float,              ///< 32-bit floating-point.
        RG8SInt,               ///< 2×8-bit signed integers.
        RG8SIntNorm,           ///< 2×8-bit signed normalized.
        RG8UInt,               ///< 2×8-bit unsigned integers.
        RG8UIntNorm,           ///< 2×8-bit unsigned normalized.
        RG16SInt,              ///< 2×16-bit signed integers.
        RG16SIntNorm,          ///< 2×16-bit signed normalized.
        RG16UInt,              ///< 2×16-bit unsigned integers.
        RG16UIntNorm,          ///< 2×16-bit unsigned normalized.
        RG16Float,             ///< 2×16-bit floating-point.
        RG32SInt,              ///< 2×32-bit signed integers.
        RG32UInt,              ///< 2×32-bit unsigned integers.
        RG32Float,             ///< 2×32-bit floating-point.
        RGB32SInt,             ///< 3×32-bit signed integers.
        RGB32UInt,             ///< 3×32-bit unsigned integers.
        RGB32Float,            ///< 3×32-bit floating-point.
        RGBA8SInt,             ///< 4×8-bit signed integers.
        RGBA8SIntNorm,         ///< 4×8-bit signed normalized.
        RGBA8UInt,             ///< 4×8-bit unsigned integers.
        RGBA8UIntNorm,         ///< 4×8-bit unsigned normalized.
        RGBA8UIntNorm_sRGB,    ///< 4×8-bit unsigned normalized with sRGB correction.
        RGBA16SInt,            ///< 4×16-bit signed integers.
        RGBA16SIntNorm,        ///< 4×16-bit signed normalized.
        RGBA16UInt,            ///< 4×16-bit unsigned integers.
        RGBA16UIntNorm,        ///< 4×16-bit unsigned normalized.
        RGBA16Float,           ///< 4×16-bit floating-point.
        RGBA32SInt,            ///< 4×32-bit signed integers.
        RGBA32UInt,            ///< 4×32-bit unsigned integers.
        RGBA32Float,           ///< 4×32-bit floating-point.
        RGB10A2UInt,           ///< 10-bit RGB + 2-bit alpha, unsigned integer
        RGB10A2UIntNorm,       ///< 10-bit RGB + 2-bit alpha, unsigned normalized
        D16Float,              ///< 16-bit floating-point depth.
        D16UIntNorm,           ///< 16-bit unsigned normalized depth.
        D32Float,              ///< 32-bit floating-point depth.
        D24S8UIntNorm,         ///< 24-bit floating-point depth + 8-bit stencil.
        D32S8UIntNorm,         ///< 32-bit floating-point depth + 8-bit stencil.
    };

    /// \brief Describes how a texture is used within a rendering or compute pass.
    enum class TextureLayout : UInt8
    {
        Source,   ///< Used as input (e.g., sampled in a shader).
        Target,   ///< Used as output (e.g., render or compute target).
        Dual,     ///< Used as both input and output in the same pass.
    };

    /// \brief Describes logical texture semantic used in materials and pipelines.
    enum class TextureSemantic : UInt8
    {
        Albedo,       ///< Base color or albedo map.
        Normal,       ///< Tangent-space normal map.
        Roughness,    ///< Surface roughness map.
        Metallic,     ///< Metalness map.
        Specular,     ///< Specular intensity map.
        Emissive,     ///< Self-illumination map.
        Displacement, ///< Displacement map.
        Occlusion,    ///< Ambient occlusion map.
        Height,       ///< Height map (alternative to displacement).
        Ambient,      ///< Indirect lighting texture (rare).
        Detail,       ///< Secondary detail texture (e.g., overlays, wear).
        Translucency, ///< Translucency or subsurface scattering mask.
        None,         ///< No semantic assigned.
    };

    /// \brief Specifies the dimensionality of a texture.
    enum class TextureType : UInt8
    {
        Texture2D,      ///< 2D texture.
    };

    /// \brief Specifies the intended usage of a resource.
    enum class Usage : UInt8
    {
        Vertex,  ///< Used as a vertex buffer.
        Index,   ///< Used as an index buffer.
        Uniform, ///< Used as an uniform buffer.
    };

    /// \brief Specifies the format of a vertex attribute.
    enum class VertexFormat : UInt8
    {
        Float16x2,          ///< Two 16-bit floating-point.
        Float16x4,          ///< Four 16-bit floating-point.
        Float32x1,          ///< One 32-bit floating-point.
        Float32x2,          ///< Two 32-bit floating-point.
        Float32x3,          ///< Three 32-bit floating-point.
        Float32x4,          ///< Four 32-bit floating-point.
        SInt8x4,            ///< Four 8-bit signed integers.
        SIntNorm8x4,        ///< Four normalized 8-bit signed integers.
        UInt8x4,            ///< Four 8-bit unsigned integers.
        UIntNorm8x4,        ///< Four normalized 8-bit unsigned integers.
        SInt16x2,           ///< Two 16-bit signed integers.
        SIntNorm16x2,       ///< Two normalized 16-bit signed integers.
        UInt16x2,           ///< Two 16-bit unsigned integers.
        UIntNorm16x2,       ///< Two normalized 16-bit unsigned integers.
        SInt16x4,           ///< Four 16-bit signed integers.
        SIntNorm16x4,       ///< Four normalized 16-bit signed integers.
        UInt16x4,           ///< Four 16-bit unsigned integers.
        UIntNorm16x4,       ///< Four normalized 16-bit unsigned integers.
        SInt32x1,           ///< One 32-bit signed integer.
        UInt32x1,           ///< One 32-bit unsigned integer.
        SInt32x2,           ///< Two 32-bi  t signed integers.
        UInt32x2,           ///< Two 32-bit unsigned integers.
        SInt32x3,           ///< Three 32-bit signed integers.
        UInt32x3,           ///< Three 32-bit unsigned integers.
        SInt32x4,           ///< Four 32-bit signed integers.
        UInt32x4,           ///< Four 32-bit unsigned integers.
        UInt10_10_10_2,     ///< Three 10-bit unsigned integers + 2-bit unsigned integer.
        UIntNorm10_10_10_2, ///< Three 10-bit unsigned normalized integers + 2-bit unsigned integer.
    };

    /// \brief Identifies the semantic meaning of a vertex attribute.
    enum class VertexSemantic : UInt8
    {
        Position,      ///< Defines the vertex position.
        Normal,        ///< Defines the vertex normal vector.
        Tangent,       ///< Defines the vertex tangent vector.
        Bitangent,     ///< Defines the vertex bitangent vector.
        Color0,        ///< Defines the primary vertex color.
        Color1,        ///< Defines the secondary vertex color.
        Color2,        ///< Defines the tertiary vertex color.
        Color3,        ///< Defines the quaternary vertex color.
        BlendIndices,  ///< Defines the bone indices for skinning.
        BlendWeights,  ///< Defines the bone weights for skinning.
        TexCoord0,     ///< Defines the first set of texture coordinates.
        TexCoord1,     ///< Defines the second set of texture coordinates.
        TexCoord2,     ///< Defines the third set of texture coordinates.
        TexCoord3,     ///< Defines the fourth set of texture coordinates.
        TexCoord4,     ///< Defines the fifth set of texture coordinates.
        TexCoord5,     ///< Defines the sixth set of texture coordinates.
        TexCoord6,     ///< Defines the seventh set of texture coordinates.
        TexCoord7,     ///< Defines the eighth set of texture coordinates.
        Custom0,       ///< Defines a custom vertex attribute.
        Custom1,       ///< Defines a custom vertex attribute.
        Custom2,       ///< Defines a custom vertex attribute.
        Custom3,       ///< Defines a custom vertex attribute.
        None,          ///< No semantic assigned.
    };

    /// \brief Defines a handle to a graphics object.
    using Object = UInt16;

    /// \brief Describes a display resolution supported by a graphics adapter.
    struct Resolution final
    {
        /// The width of the display mode in pixels.
        UInt16 Width;

        /// The height of the display mode in pixels.
        UInt16 Height;

        /// The refresh rate of the display mode in hertz (Hz).
        UInt16 Frequency;
    };

    /// \brief Describes a graphics adapter (GPU) available on the system.
    struct Adapter final
    {
        /// The human-readable name of the graphics adapter.
        Str8   Description;

        /// The dedicated video memory in megabytes (MB).
        UInt32 Memory;

        /// The list of supported display resolutions.
        Vector<Resolution> Resolutions;
    };

    /// \brief Describes the capabilities of a graphics device.
    struct Capabilities final
    {
        /// The maximum number of texture dimension (width or height).
        UInt32 MaxTextureDimension;

        /// The maximum number of texture array layers.
        UInt32 MaxTextureLayers;
    };

    /// \brief Describes the current graphics device and its capabilities.
    struct Device final
    {
        /// The current graphics backend in use.
        Backend         Backend  = Backend::None;

        /// The current multisampling level.
        Multisample     Samples  = Multisample::X1;

        /// The current graphics API version.
        Language        Language = Language::SM1;

        /// The list of available graphics adapters.
        Vector<Adapter> Adapters;

        /// The graphics capabilities of the system.
        Capabilities    Capabilities;
    };

    /// \brief Defines a rectangular scissor region for pixel clipping during rendering.
    struct Scissor final
    {
        /// The X screen coordinate of the region’s origin, in pixels.
        UInt16 X      = 0;

        /// The Y screen coordinate of the region’s origin, in pixels.
        UInt16 Y      = 0;

        /// The width of the scissor region, in pixels.
        UInt16 Width  = UINT16_MAX;

        /// The height of the scissor region, in pixels.
        UInt16 Height = UINT16_MAX;
    };

    /// \brief Defines a rectangular viewport for transforming normalized device coordinates.
    struct Viewport final
    {
        /// The X coordinate of the top-left corner in screen space.
        Real32 X        = 0.0f;

        /// The Y coordinate of the top-left corner in screen space.
        Real32 Y        = 0.0f;

        /// The width of the viewport in pixels.
        Real32 Width    = 0.0f;

        /// The height of the viewport in pixels.
        Real32 Height   = 0.0f;

        /// The minimum depth value for depth range mapping.
        Real32 MinDepth = 0.0f;

        /// The maximum depth value for depth range mapping.
        Real32 MaxDepth = 1.0f;
    };

    /// \brief Defines a resource binding entry mapping a register to a resource.
    template<typename Value>
    struct Entry final
    {
        /// The binding register index (slot).
        UInt8 Register;

        /// The bound resource element (e.g., buffer, texture, sampler).
        Value Resource;
    };

    /// \brief Defines a list of resource binding entries.
    template<typename Value, UInt Capacity>
    using EntryList = Vector<Entry<Value>, Capacity>;

    /// \brief Defines a texture attachment in a render pass.
    struct Attachment final
    {
        /// The target texture that receives the rendered output.
        Object TargetTexture = 0;

        /// The mipmap level of the target texture to render to.
        UInt8  TargetLevel   = 0;

        /// The multisample source texture that provides input data for rendering.
        Object SourceTexture = 0;

        /// The mipmap level of the source texture to sample from.
        UInt8  SourceLevel   = 0;
    };

    /// \brief Defines a vertex attribute within a vertex layout.
    struct Attribute final
    {
        /// The semantic meaning of the vertex attribute.
        VertexSemantic Semantic = VertexSemantic::None;

        /// The data format of the vertex attribute.
        VertexFormat   Format   = VertexFormat::Float32x4;

        /// The byte offset of the attribute within the vertex structure.
        UInt16         Offset   = 0;

        /// The input stream index for the attribute.
        UInt16         Stream   = 0;

        /// The divisor for instanced rendering (0 = per-vertex, 1 = per-instance, n = per-step).
        UInt16         Divisor  = 0;
    };

    /// \brief Defines a buffer stream used for vertex, index, or uniform input.
    struct Stream final
    {
        /// The buffer object providing the data for this stream.
        Object Buffer = 0;

        /// The byte stride between consecutive elements in the buffer.
        UInt16 Stride = 0;

        /// The byte offset within the buffer where the stream data begins.
        UInt32 Offset = 0;
    };

    /// \brief Defines the shader program bytecode for a rendering pipeline.
    using  Shaders = Array<Blob, Enum::Count<Stage>()>;

    /// \brief Defines the fixed-function GPU state used by a rendering pipeline.
    struct States final
    {
        /// Face culling mode (e.g., back-face or front-face culling).
        Cull           Cull                  = Cull::Back;

        /// Polygon fill mode (solid or wireframe).
        Fill           Fill                  = Fill::Solid;

        /// Alpha-to-coverage enable flag for multisampling.
        Bool           AlphaToCoverage       = false;

        /// Source blend factor for color channels.
        BlendFactor    BlendSrcColor         = BlendFactor::One;

        /// Destination blend factor for the color channels.
        BlendFactor    BlendDstColor         = BlendFactor::Zero;

        /// Blend operation for color channels.
        BlendFunction  BlendEquationColor    = BlendFunction::Add;

        /// Source blend factor for the alpha channel.
        BlendFactor    BlendSrcAlpha         = BlendFactor::One;

        /// Destination blend factor for the alpha channel.
        BlendFactor    BlendDstAlpha         = BlendFactor::Zero;

        /// Blend function for the alpha channel.
        BlendFunction  BlendEquationAlpha    = BlendFunction::Add;

        /// Write mask controlling which color channels are affected.
        Channel        Channel               = Channel::RGBA;

        /// Enables or disables depth clipping.
        Bool           DepthClip             = true;

        /// Enables or disables writing to the depth buffer.
        Bool           DepthMask             = true;

        /// Depth comparison function used for depth testing.
        TestCondition  DepthTest             = TestCondition::LessEqual;

        /// Constant offset added to fragment depth.
        Real32         DepthBias             = 0.0f;

        /// Multiplier applied to fragment slope for depth biasing.
        Real32         DepthBiasSlope        = 0.0f;

        /// Maximum value applied to depth bias (clamping).
        Real32         DepthBiasClamp        = 0.0f;

        /// Bitmask selecting readable bits of the stencil buffer.
        UInt8          StencilReadMask       = 0xFF;

        /// Bitmask selecting writable bits of the stencil buffer.
        UInt8          StencilWriteMask      = 0xFF;

        /// Stencil comparison function for front-facing geometry.
        TestCondition  StencilFrontTest      = TestCondition::Always;

        /// Action when stencil test fails (front-facing).
        TestAction     StencilFrontFail      = TestAction::Keep;

        /// Action when stencil passes but depth test fails (front-facing).
        TestAction     StencilFrontDepthFail = TestAction::Keep;

        /// Action when both stencil and depth tests pass (front-facing).
        TestAction     StencilFrontDepthPass = TestAction::Keep;

        /// Stencil comparison function for back-facing geometry.
        TestCondition  StencilBackTest       = TestCondition::Always;

        /// Action when stencil test fails (back-facing).
        TestAction     StencilBackFail       = TestAction::Keep;

        /// Action when stencil passes but depth test fails (back-facing).
        TestAction     StencilBackDepthFail  = TestAction::Keep;

        /// Action when both stencil and depth tests pass (back-facing).
        TestAction     StencilBackDepthPass  = TestAction::Keep;

        /// Attributes describing the input layout for vertex data.
        Vector<Attribute, kMaxAttributes> InputAttributes;

        /// Primitive topology used for vertex interpretation.
        Primitive      InputPrimitive        = Primitive::TriangleList;
    };

    /// \brief Defines the sampling parameters for a texture sampler.
    struct Sampler final
    {
        /// The wrapping mode for the U (horizontal) texture coordinate.
        TextureAddress AddressModeU = TextureAddress::Clamp;

        /// The wrapping mode for the V (vertical) texture coordinate.
        TextureAddress AddressModeV = TextureAddress::Clamp;

        /// The wrapping mode for the W (depth) texture coordinate.
        TextureAddress AddressModeW = TextureAddress::Clamp;

        /// The filtering method used when sampling the texture.
        TextureFilter  Filter       = TextureFilter::LinearMipLinear;

        /// The comparison function used for comparison sampling (shadow/depth maps).
        TestCondition  Comparison   = TestCondition::Always;

        //// The border color used when wrap mode border is selected.
        TextureBorder  Border       = TextureBorder::OpaqueBlack;
    };

    /// \brief Defines a draw command for rendering operations, supporting both indexed and non-indexed rendering.
    struct DrawCommand final
    {
        /// The number of vertices or indices to draw.
        UInt32 Count     = 0;

        /// The starting index or vertex location.
        SInt32 Base      = 0;

        /// The byte offset within the vertex or index buffer.
        UInt32 Offset    = 0;

        /// The number of instances to render (for instanced draws).
        UInt32 Instances = 1;
    };

    /// \brief Defines a complete draw packet for rendering operations.
    struct DrawPacket final
    {
        /// \brief Maximum number of vertex buffer streams.
        static constexpr UInt8 kMaxVertexStreams  = 0x0008;

        /// \brief Maximum number of uniform buffer streams.
        static constexpr UInt8 kMaxUniformStreams = 0x0004;

        /// The scissor recTtangle used for pixel clipping.
        Scissor                           Scissor;

        /// The stencil reference value for stencil testing.
        UInt8                             Stencil  = 0;

        /// The pipeline object to bind for rendering.
        Object                            Pipeline = 0;

        /// Collection of vertex buffer bindings for geometry input.
        Array<Stream,  kMaxVertexStreams> Vertices;     // TODO: EntryList?

        /// Index buffer binding used for indexed rendering.
        Stream                            Indices;

        /// Collection of uniform buffer bindings for the pipeline.
        Array<Stream, kMaxUniformStreams> Uniforms;     // TODO: EntryList?

        /// Collection of sampler bindings for texture sampling.
        EntryList<Sampler, kMaxResources> Samplers;

        /// Collection of texture bindings for the pipeline.
        EntryList<Object,  kMaxResources> Textures;

        /// The draw parameters for the rendering command.
        DrawCommand                       Command;
    };
}