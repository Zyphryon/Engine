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

namespace Graphic
{
    /// \brief Content loader for technique assets using JSON.
    class VFXLoader final : public Content::Loader
    {
    public:

        /// \brief An array with the extension supported by this content loader.
        static constexpr Text kTypes[] = { "vfx" };

    public:

        /// \brief Constructs a VFX loader for the specified shader language.
        ///
        /// \param Language The shader language used when compiling technique assets.
        VFXLoader(ShaderLanguage Language);

        /// \see Loader::Load(Ref<Service>, Ref<Scope>, AnyRef<Blob>)
        Bool Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data) override;

    private:

        /// \brief Parses a properties section over the given states, reporting which blocks it declared.
        ///
        /// \param Section    The JSON object containing the properties section.
        /// \param States     The states each declared block is parsed over and written back into.
        /// \param Attributes Receives the vertex attributes, left as-is if the layout declares none.
        /// \return The bitmask of the state blocks the section declared, built from \ref Technique::Block.
        UInt8 LoadProperties(JsonObject Section, Ref<States> States, Ref<Attributes> Attributes);

        /// \brief Parses a program section, resolving the shader modules declared for the loader's language.
        ///
        /// \param Service The content service used to load the shader modules.
        /// \param Scope   The scope the loaded shader modules are tracked under.
        /// \param Section The JSON object containing the program section.
        /// \param Macros  Receives the preprocessor macros the section declares, appended to what it holds.
        /// \param Shaders Receives the shader module of every stage the section declares.
        void LoadProgram(
            Ref<Content::Service>   Service,
            Ref<Content::Scope>     Scope,
            JsonObject              Section,
            Ref<Sequence<Macro>>    Macros,
            Ref<Technique::Shaders> Shaders);

        /// \brief Parses a uniform parameter from JSON and constructs its default value.
        ///
        /// \param JsonParameter The JSON object containing the parameter type and default value.
        /// \return The parameter initialized with the parsed default value.
        Parameter LoadParameter(JsonObject JsonParameter);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Text mLanguage;
    };
}