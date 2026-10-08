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

#include "Zyphryon.Content/Loader.hpp"
#include "Zyphryon.Graphic/Technique.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic
{
    /// \brief Represents the content loader for technique assets using JSON.
    class VFXLoader final : public ZyContent::Loader
    {
    public:

        /// \brief An array with the extension supported by this content loader.
        static constexpr Text kTypes[] = { "vfx" };

    public:

        /// \brief Constructs a VFX loader for the specified shader language and tier.
        ///
        /// \param Language The shader language used when compiling technique assets.
        /// \param Tier     The tier the device reaches, which every technique is compiled for as `ZY_TIER`.
        VFXLoader(ShaderLanguage Language, Tier Tier);

        /// \see Loader::Load(Ref<Service>, Ref<Scope>, AnyRef<Blob>)
        Bool Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data) override;

    private:

        /// \brief Parses a technique's properties section, every block of it, over the given states.
        ///
        /// \param Section    The JSON object containing the properties section.
        /// \param States     The states each declared block is parsed over and written back into.
        /// \param Attributes Receives the vertex attributes, left as-is if the layout declares none.
        void LoadProperties(JsonObject Section, Ref<States> States, Ref<Attributes> Attributes);

        /// \brief Parses the blend, depth and rasterizer blocks of a properties section, the ones a feature may replace.
        ///
        /// \param Section The JSON object containing the properties section.
        /// \param States  The states each declared block is parsed over and written back into.
        /// \return The bitmask of the blocks the section declared, built from \ref Technique::Block.
        UInt8 LoadPatch(JsonObject Section, Ref<States> States);

        /// \brief Parses a program section, resolving the shader modules declared for the loader's language.
        ///
        /// \param Service The content service used to load the shader modules.
        /// \param Scope   The scope the loaded shader modules are tracked under.
        /// \param Section The JSON object containing the program section.
        /// \param Macros  Receives the preprocessor macros the section declares, appended to what it holds.
        /// \param Shaders Receives the shader module of every stage the section declares.
        void LoadProgram(
            Ref<ZyContent::Service> Service,
            Ref<ZyContent::Scope>   Scope,
            JsonObject              Section,
            Ref<Sequence<Macro>>    Macros,
            Ref<Technique::Shaders> Shaders);

        /// \brief Parses the defines of a program section.
        ///
        /// \param Section The JSON object containing the program section.
        /// \param Macros  Receives the preprocessor macros the section declares, appended to what it holds.
        void LoadDefines(JsonObject Section, Ref<Sequence<Macro>> Macros);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Text mLanguage;
        Tier mTier;
    };
}