// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Font.hpp"
#include "Zyphryon.Content/Proxy.hpp"
#include "Zyphryon.Math/Color.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Represents a text with rendering properties.
    class Text final
    {
    public:

        /// \brief Constructor initializing default text properties.
        ZYPHRYON_INLINE Text()
            : mSize { 12.0f }
        {
        }

        /// \brief Sets the font used for rendering the text.
        ///
        /// \param Font The font to set.
        ZYPHRYON_INLINE void SetFont(ConstTracker<Font> Font)
        {
            mFont = Content::Proxy(Font);
        }

        /// \brief Gets the font used for rendering the text.
        ///
        /// \return The current font.
        ZYPHRYON_INLINE ConstTracker<Font> GetFont() const
        {
            return mFont.GetResource();
        }

        /// \brief Sets the size of the text.
        ///
        /// \param Size The size to set.
        ZYPHRYON_INLINE void SetSize(Real32 Size)
        {
            mSize = Size;
        }

        /// \brief Gets the size of the text.
        ///
        /// \return The current text size.
        ZYPHRYON_INLINE Real32 GetSize() const
        {
            return mSize;
        }

        /// \brief Sets the tint color applied to the text.
        ///
        /// \param Tint The tint color  to set.
        ZYPHRYON_INLINE void SetTint(IntColor8 Tint)
        {
            mTint = Tint;
        }

        /// \brief Gets the tint color applied to the text.
        ///
        /// \return The current tint color .
        ZYPHRYON_INLINE IntColor8 GetTint() const
        {
            return mTint;
        }

        /// \brief Sets the content of the text.
        ///
        /// \param Content The text content to set.
        ZYPHRYON_INLINE void SetContent(ConstStr8 Content)
        {
            mContent = Content;
        }

        /// \brief Gets the content of the text.
        ///
        /// \return The current text content.
        ZYPHRYON_INLINE ConstStr8 GetContent() const
        {
            return mContent;
        }

        /// \brief Sets the padding between characters in the text.
        ///
        /// \param Padding The padding to set.
        ZYPHRYON_INLINE void SetPadding(Vector2 Padding)
        {
            mPadding = Padding;
        }

        /// \brief Gets the padding between characters in the text.
        ///
        /// \return The current character padding.
        ZYPHRYON_INLINE Vector2 GetPadding() const
        {
            return mPadding;
        }

        /// \brief Sets the origin point for text alignment.
        ///
        /// \param Origin The origin to set.
        ZYPHRYON_INLINE void SetOrigin(Pivot Origin)
        {
            mOrigin = Origin;
        }

        /// \brief Gets the origin point for text alignment.
        ///
        /// \return The current origin.
        ZYPHRYON_INLINE Pivot GetOrigin() const
        {
            return mOrigin;
        }

        /// \brief Resolves the deferred component using the provided service.
        ///
        /// \param Service The service used to load deferred resources.
        ZYPHRYON_INLINE void OnResolve(Ref<Content::Service> Service)
        {
            mFont.Resolve(Service);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mFont);
            Archive.SerializeObject(mSize);
            Archive.SerializeObject(mTint);
            Archive.SerializeObject(mContent);
            Archive.SerializeObject(mPadding);
            Archive.SerializeObject(mOrigin);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Content::Proxy<Font> mFont;
        Real32               mSize;
        IntColor8            mTint;
        Str8                 mContent;
        Vector2              mPadding;
        Pivot                mOrigin;
    };
}