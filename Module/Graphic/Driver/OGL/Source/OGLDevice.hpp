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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Specifies the flavour of OpenGL a context turns out to be.
    enum class Profile : UInt8
    {
        Desktop,    ///< A desktop core profile context.
        Embedded,   ///< An OpenGL ES context.
        Web,        ///< A WebGL context.
    };

    /// \brief Specifies what a context may offer beyond its baseline, each granted by a version or an extension.
    enum class Feature : UInt8
    {
        ClipControl,        ///< The clip origin and a [0, 1] clip depth range.
        TextureStorage,     ///< Immutable texture storage.
        BaseVertex,         ///< Indexed draws offset by a base vertex.
        VertexBinding,      ///< Vertex formats kept apart from the buffers they read.
        BufferStorage,      ///< Immutable buffer storage, mapped persistently.
        MultiBind,          ///< Binding runs of textures, samplers and buffers in one call.
        ShaderBinding,      ///< The `binding` layout qualifier in shader source.
        ProgramUniform,     ///< Uniforms set on a program that is not in use.
        DepthClamp,         ///< Clamping depth instead of clipping against the near and far planes.
        PolygonMode,        ///< Rasterizing edges alone.
        BorderClamp,        ///< Samplers that read a border color outside the texture.
        Anisotropy,         ///< Anisotropic texture filtering.
        Gather,             ///< Gathering four texels in one fetch.
        ParallelCompile,    ///< Compiling and linking on the driver's own threads.
        TimerQuery,         ///< Timestamps written by the GPU.
        Invalidate,         ///< Discarding framebuffer contents that will not be read again.
        PrimitiveRestart,   ///< Restarting strips at the highest index.
        S3TC,               ///< BC1 to BC3 compressed textures.
        S3TCsRGB,           ///< BC1 to BC3 compressed textures in sRGB.
        RGTC,               ///< BC4 and BC5 compressed textures.
        BPTC,               ///< BC6 and BC7 compressed textures.
        ETC2,               ///< ETC2 and EAC compressed textures.
        ASTC,               ///< ASTC compressed textures, low dynamic range.
        Norm16,             ///< 16-bit normalized textures.
    };

    /// \brief Represents what the current OpenGL context is and offers, read from it once and kept.
    class Device final
    {
    public:

        /// The number of features a context is checked for.
        static constexpr UInt kFeatures = ZyEnum::Count<Feature>();

    private:

        /// \brief Describes the versions that made a feature core, as major * 10 + minor, `0` where none did.
        struct Version final
        {
            /// The desktop version.
            UInt8 Desktop;

            /// The OpenGL ES version.
            UInt8 Embedded;

            /// `true` when WebGL 2 has the feature without an extension.
            Bool  Web;
        };

        /// \brief Describes an extension that grants a feature, and the suffix its entry points then carry.
        struct Alias final
        {
            /// The feature the extension grants.
            Feature Feature;

            /// The extension's name, without the `GL_` prefix.
            Text    Name;

            /// The suffix of the entry points the extension adds, empty where they keep the core name.
            Text    Suffix;
        };

        /// The versions of each feature, in the order \ref Feature declares them.
        static constexpr Version kVersions[] = {
            { 45,   0, false },     // Feature::ClipControl
            { 42,  30, true  },     // Feature::TextureStorage
            { 32,  32, false },     // Feature::BaseVertex
            { 43,  31, false },     // Feature::VertexBinding
            { 44,   0, false },     // Feature::BufferStorage
            { 44,   0, false },     // Feature::MultiBind
            { 42,  31, false },     // Feature::ShaderBinding
            { 41,  31, false },     // Feature::ProgramUniform
            { 32,   0, false },     // Feature::DepthClamp
            { 10,   0, false },     // Feature::PolygonMode
            { 10,  32, false },     // Feature::BorderClamp
            { 46,   0, false },     // Feature::Anisotropy
            { 40,  31, false },     // Feature::Gather
            {  0,   0, false },     // Feature::ParallelCompile
            { 33,   0, false },     // Feature::TimerQuery
            { 43,  30, true  },     // Feature::Invalidate
            { 43,  30, true  },     // Feature::PrimitiveRestart
            {  0,   0, false },     // Feature::S3TC
            {  0,   0, false },     // Feature::S3TCsRGB
            { 30,   0, false },     // Feature::RGTC
            { 42,   0, false },     // Feature::BPTC
            { 43,  30, false },     // Feature::ETC2
            {  0,  32, false },     // Feature::ASTC
            { 10,   0, false },     // Feature::Norm16
        };

        /// The extensions that grant a feature below its core version, tried in order.
        static constexpr Alias kAliases[] = {
            { Feature::ClipControl,      "ARB_clip_control",                               ""      },
            { Feature::ClipControl,      "EXT_clip_control",                               "EXT"   },
            { Feature::TextureStorage,   "ARB_texture_storage",                            ""      },
            { Feature::BaseVertex,       "OES_draw_elements_base_vertex",                  "OES"   },
            { Feature::BaseVertex,       "EXT_draw_elements_base_vertex",                  "EXT"   },
            { Feature::BaseVertex,       "WEBGL_draw_instanced_base_vertex_base_instance", ""      },
            { Feature::VertexBinding,    "ARB_vertex_attrib_binding",                      ""      },
            { Feature::BufferStorage,    "ARB_buffer_storage",                             ""      },
            { Feature::BufferStorage,    "EXT_buffer_storage",                             "EXT"   },
            { Feature::MultiBind,        "ARB_multi_bind",                                 ""      },
            { Feature::ShaderBinding,    "ARB_shading_language_420pack",                   ""      },
            { Feature::ProgramUniform,   "ARB_separate_shader_objects",                    ""      },
            { Feature::DepthClamp,       "ARB_depth_clamp",                                ""      },
            { Feature::DepthClamp,       "EXT_depth_clamp",                                ""      },
            { Feature::PolygonMode,      "NV_polygon_mode",                                "NV"    },
            { Feature::PolygonMode,      "WEBGL_polygon_mode",                             ""      },
            { Feature::BorderClamp,      "OES_texture_border_clamp",                       ""      },
            { Feature::BorderClamp,      "EXT_texture_border_clamp",                       ""      },
            { Feature::Anisotropy,       "ARB_texture_filter_anisotropic",                 ""      },
            { Feature::Anisotropy,       "EXT_texture_filter_anisotropic",                 ""      },
            { Feature::Gather,           "ARB_texture_gather",                             ""      },
            { Feature::ParallelCompile,  "KHR_parallel_shader_compile",                    "KHR"   },
            { Feature::ParallelCompile,  "ARB_parallel_shader_compile",                    "ARB"   },
            { Feature::TimerQuery,       "ARB_timer_query",                                ""      },
            { Feature::TimerQuery,       "EXT_disjoint_timer_query",                       "EXT"   },
            { Feature::TimerQuery,       "EXT_disjoint_timer_query_webgl2",                "EXT"   },
            { Feature::Invalidate,       "ARB_invalidate_subdata",                         ""      },
            { Feature::PrimitiveRestart, "ARB_ES3_compatibility",                          ""      },
            { Feature::S3TC,             "EXT_texture_compression_s3tc",                   ""      },
            { Feature::S3TC,             "WEBGL_compressed_texture_s3tc",                  ""      },
            { Feature::S3TCsRGB,         "EXT_texture_compression_s3tc_srgb",              ""      },
            { Feature::S3TCsRGB,         "EXT_texture_sRGB",                               ""      },
            { Feature::S3TCsRGB,         "WEBGL_compressed_texture_s3tc_srgb",             ""      },
            { Feature::RGTC,             "ARB_texture_compression_rgtc",                   ""      },
            { Feature::RGTC,             "EXT_texture_compression_rgtc",                   ""      },
            { Feature::BPTC,             "ARB_texture_compression_bptc",                   ""      },
            { Feature::BPTC,             "EXT_texture_compression_bptc",                   ""      },
            { Feature::ETC2,             "ARB_ES3_compatibility",                          ""      },
            { Feature::ETC2,             "WEBGL_compressed_texture_etc",                   ""      },
            { Feature::ASTC,             "KHR_texture_compression_astc_ldr",               ""      },
            { Feature::ASTC,             "WEBGL_compressed_texture_astc",                  ""      },
            { Feature::Norm16,           "EXT_texture_norm16",                             ""      },
        };

        /// The grant recorded for a feature its version made core, or that nothing granted.
        static constexpr UInt8 kNoAlias = 0xFF;

    public:

        /// \brief Constructs an empty device that offers no feature until \ref Initialize reads a context.
        Device();

        /// \brief Records an extension the platform enabled on its own, which the context does not list.
        ///
        /// \note Must be called before \ref Initialize, which resolves the features from every name it holds.
        ///
        /// \param Name The extension's name, without the `GL_` prefix.
        void Grant(Text Name);

        /// \brief Reads what the current context is and resolves what it offers.
        ///
        /// \note The context's baseline entry points must already be loaded, as the context is read through them.
        ///
        /// \return `true` when the context reaches the baseline, `false` otherwise.
        Bool Initialize();

        /// \brief Withdraws a feature, after one of its entry points turned out to be missing.
        ///
        /// \param Value The feature to withdraw.
        void Revoke(Feature Value);

        /// \brief Fills the description with the backend, tier, formats, limits and adapter of the context.
        ///
        /// \note Called once every feature whose entry points are missing has been withdrawn.
        void Describe();

        /// \brief Gets the suffix the entry points of a granted feature carry.
        ///
        /// \param Value The feature whose suffix is returned.
        /// \return The vendor suffix, empty where the entry points keep their core name.
        ZY_INLINE Text GetSuffix(Feature Value) const
        {
            const UInt8 Grant = mGrants[ZyEnum::Cast(Value)];
            return (Grant == kNoAlias ? Text() : kAliases[Grant].Suffix);
        }

        /// \brief Checks whether the context offers a feature.
        ///
        /// \param Value The feature to look for.
        /// \return `true` when the feature is granted, by its version or an extension, otherwise `false`.
        ZY_INLINE Bool Has(Feature Value) const
        {
            return mFeatures.Test(ZyEnum::Cast(Value));
        }

        /// \brief Gets the flavour of OpenGL the context turned out to be.
        ///
        /// \return The profile read from the context's version string.
        ZY_INLINE Profile GetProfile() const
        {
            return mProfile;
        }

        /// \brief Gets the version of the context, as major * 10 + minor.
        ///
        /// \return The version of the context, such as `46` for 4.6 or `30` for ES 3.0.
        ZY_INLINE UInt8 GetVersion() const
        {
            return mVersion;
        }

        /// \brief Gets the version of the shading language the context compiles, as major * 100 + minor.
        ///
        /// \return The shading language version, such as `460` for 4.60 or `300` for ES 3.00.
        ZY_INLINE UInt16 GetLanguage() const
        {
            return mLanguage;
        }

        /// \brief Gets the backend, tier, formats, limits and adapter of the context.
        ///
        /// \return The description filled once the context was read.
        ZY_INLINE ConstRef<Description> GetDescription() const
        {
            return mDescription;
        }

    private:

        /// \brief Checks whether the context lists an extension.
        ///
        /// \param Name The extension's name, without the `GL_` prefix.
        /// \return `true` when the context lists the extension or the platform granted it, otherwise `false`.
        Bool HasExtension(Text Name) const;

        /// \brief Checks whether the context's version reaches a feature's core version.
        ///
        /// \param Value The feature to check.
        /// \return `true` when the version alone grants the feature, otherwise `false`.
        Bool IsCore(Feature Value) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Profile                 mProfile;
        UInt8                   mVersion;
        UInt16                  mLanguage;
        Sequence<UInt64>        mExtensions;
        Bitset<kFeatures>       mFeatures;
        Array<UInt8, kFeatures> mGrants;
        Description             mDescription;
    };
}