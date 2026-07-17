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

namespace Content
{
    /// \brief Represents a Uniform Resource Identifier for content addressing.
    class Uri final
    {
    public:

        /// \brief Constructs an empty URI.
        ZY_INLINE Uri() = default;

        /// \brief Constructs an URI from a text view.
        ///
        /// \param Url The URL string to parse.
        ZY_INLINE Uri(Text Url)
            : mUrl { Url }
        {
        }

        /// \brief Constructs an URI from a null-terminated string literal.
        ///
        /// \param Literal The string literal to copy into the URI.
        template<UInt Count>
        ZY_INLINE Uri(const Char (& Literal)[Count])
            : mUrl { Literal }
        {
        }

        /// \brief Constructs a URI from a string.
        ///
        /// \param Url The URL string to parse.
        ZY_INLINE Uri(AnyRef<Str> Url)
            : mUrl { Move(Url) }
        {
        }

        /// \brief Checks whether the URI is non-empty.
        ///
        /// \return `true` if the URI contains a URL, otherwise `false`.
        ZY_INLINE Bool IsValid() const
        {
            return !mUrl.IsEmpty();
        }

        /// \brief Checks whether the URI has a schema component.
        ///
        /// \return `true` if a schema is present, otherwise `false`.
        ZY_INLINE Bool HasSchema() const
        {
            return !GetSchema().IsEmpty();
        }

        /// \brief Checks whether the URI has a directory component.
        ///
        /// \return `true` if a directory is present, otherwise `false`.
        ZY_INLINE Bool HasDirectory() const
        {
            return !GetDirectory().IsEmpty();
        }

        /// \brief Checks whether the URI has an extension component.
        ///
        /// \return `true` if an extension is present, otherwise `false`.
        ZY_INLINE Bool HasExtension() const
        {
            return !GetExtension().IsEmpty();
        }

        /// \brief Gets the full URL string.
        ///
        /// \return The complete URL string.
        ZY_INLINE Text GetUrl() const
        {
            return mUrl;
        }

        /// \brief Gets the schema component of the URI.
        ///
        /// \return The schema portion before "://", or empty if none.
        ZY_INLINE Text GetSchema() const
        {
            return StrBefore(mUrl, "://");
        }

        /// \brief Gets the path component of the URI.
        ///
        /// \return The path portion after the schema, or the full URL if no schema.
        ZY_INLINE Text GetPath() const
        {
            return StrAfter(mUrl, "://");
        }

        /// \brief Gets the directory component of the URI.
        ///
        /// \return The directory portion before the last '/', or empty if none.
        ZY_INLINE Text GetDirectory() const
        {
            return StrBeforeLast(mUrl, '/');
        }

        /// \brief Gets the filename component of the URI.
        ///
        /// \return The filename portion after the last '/', or empty if none.
        ZY_INLINE Text GetFilename() const
        {
            return StrAfterLast(mUrl, '/');
        }

        /// \brief Gets the stem (filename without extension) of the URI.
        ///
        /// \return The filename without its extension, or empty if none.
        ZY_INLINE Text GetStem() const
        {
            return StrBeforeLast(GetFilename(), '.');
        }

        /// \brief Gets the extension component of the URI.
        ///
        /// \return The extension portion after the last '.', or empty if none.
        ZY_INLINE Text GetExtension() const
        {
            return StrAfterLast(GetFilename(), '.');
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mUrl);
        }

        /// \brief Computes a 64-bit hash of the object's state.
        ///
        /// \return A 64-bit hash of the object.
        ZY_INLINE UInt64 Hash() const
        {
            return Base::Hash(mUrl);
        }

    public:

        /// \brief Expands a relative URI against a parent URI.
        ///
        /// \param Relative The relative URI to expand.
        /// \param Parent   The parent URI to resolve against.
        /// \return A new URI with the relative path resolved against the parent.
        ZY_INLINE static Uri Expand(ConstRef<Uri> Relative, ConstRef<Uri> Parent)
        {
            if (!Relative.HasSchema())
            {
                return Str::Join(Parent.GetDirectory(), '/', Relative.GetUrl());
            }
            return Relative;
        }

    private:

        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-
        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-

        Str mUrl;
    };
}