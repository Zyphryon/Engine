// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Base/Utility.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Represents a lightweight, immutable parser for Uniform Resource Identifiers (URIs).
    class Uri final
    {
    public:

        /// \brief Constructs an empty URI.
        ZYPHRYON_INLINE constexpr Uri() = default;

        /// \brief Constructs a URI from a text string.
        ///
        /// \param Url The full URI string to store.
        ZYPHRYON_INLINE constexpr Uri(AnyRef<Text> Url)
            : mUrl { Move(Url) }
        {
        }

        /// \brief Constructs a URI from a constant string view.
        ///
        /// \param Url The full URI string to store.
        ZYPHRYON_INLINE constexpr Uri(ConstText Url)
            : mUrl { Url }
        {
        }

        /// \brief Constructs a URI from a raw C-style string.
        ///
        /// \param Url The full URI string to store.
        ZYPHRYON_INLINE constexpr Uri(ConstPtr<Char> Url)
            : mUrl { Url }
        {
        }

        /// \brief Checks whether the URI includes a schema.
        ///
        /// \return `true` if a schema is present, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool HasSchema() const
        {
            return !GetSchema().empty();
        }

        /// \brief Checks whether the URI includes a folder path segment.
        ///
        /// \return `true` if a folder path is present, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool HasFolder() const
        {
            return !GetFolder().empty();
        }

        /// \brief Checks whether the URI includes a file extension.
        ///
        /// \return `true` if an extension is present, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool HasExtension() const
        {
            return !GetExtension().empty();
        }

        /// \brief Gets the full raw URI string.
        ///
        /// \return The full URI string.
        ZYPHRYON_INLINE constexpr ConstText GetUrl() const
        {
            return mUrl;
        }

        /// \brief Gets the URI without the filename.
        ///
        /// \return The URI string without its filename.
        ZYPHRYON_INLINE constexpr ConstText GetUrlWithoutFile() const
        {
            const auto Offset = mUrl.rfind('/');
            return (Offset != ConstText::npos ? GetUrl().substr(0, Offset) : GetUrl());
        }

        /// \brief Gets the URI without the file extension.
        ///
        /// \return The URI string without its file extension.
        ZYPHRYON_INLINE constexpr ConstText GetUrlWithoutExtension() const
        {
            const auto Offset = mUrl.rfind('.');
            return (Offset != ConstText::npos ? GetUrl().substr(0, Offset) : GetUrl());
        }

        /// \brief Gets the schema portion of the URI.
        ///
        /// \return The schema string (e.g., `http`, `file`), or an empty string if none exists.
        ZYPHRYON_INLINE constexpr ConstText GetSchema() const
        {
            const auto Offset = mUrl.find("://");
            return (Offset != ConstText::npos ? GetUrl().substr(0, Offset) : "");
        }

        /// \brief Gets the path portion of the URI, excluding the schema.
        ///
        /// \return The path string after the schema, or the full URI if no schema is present.
        ZYPHRYON_INLINE constexpr ConstText GetPath() const
        {
            const auto Offset = mUrl.find("://");
            return (Offset != ConstText::npos ? GetUrl().substr(Offset + 3) : mUrl);
        }

        /// \brief Gets the folder portion of the URI.
        ///
        /// \return The folder path before the first slash, or an empty string if none exists.
        ZYPHRYON_INLINE constexpr ConstText GetFolder() const
        {
            const auto Offset = mUrl.rfind('/');
            return (Offset != ConstText::npos ? GetPath().substr(0, Offset) : "");
        }

        /// \brief Gets the filename portion of the URI.
        ///
        /// \return The part after the last slash, or the full path if no slash exists.
        ZYPHRYON_INLINE constexpr ConstText GetFilename() const
        {
            const auto Offset = mUrl.rfind('/');
            return (Offset != ConstText::npos ? GetUrl().substr(Offset + 1) : GetPath());
        }

        /// \brief Gets the file extension portion of the URI.
        ///
        /// \return The part after the last dot, or an empty string if none exists.
        ZYPHRYON_INLINE constexpr ConstText GetExtension() const
        {
            const auto Offset = mUrl.rfind('.');
            return (Offset != ConstText::npos ? GetUrl().substr(Offset + 1, mUrl.length()) : "");
        }

    public:

        /// \brief Combines a base URI with a subresource name.
        ///
        /// \param Parent      The base URI.
        /// \param Subresource The subresource name to append.
        /// \return A new URI with the subresource appended.
        ZYPHRYON_INLINE constexpr static Uri Merge(ConstRef<Uri> Parent, ConstText Subresource)
        {
            return Uri(Format("{}#{}", Parent.GetUrlWithoutExtension(), Subresource));
        }

        /// \brief Expands a context-relative URI into an absolute URI..
        ///
        /// \param Relative The URI that may be context-relative.
        /// \param Parent   The base URI providing context.
        /// \return The expanded absolute URI.
        ZYPHRYON_INLINE constexpr static Uri Expand(ConstRef<Uri> Relative, ConstRef<Uri> Parent)
        {
            if (!Relative.HasSchema())
            {
                return Uri(Format("{}/{}", Parent.GetUrlWithoutFile(), Relative.GetPath()));
            }
            return Relative;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        const Text mUrl;
    };
}
