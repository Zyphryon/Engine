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

#include "Zyphryon.Content/Resource.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a shader resource containing module source code for a pipeline stage.
    class Shader final : public Content::AbstractResource<Shader>
    {
    public:

        /// \brief Constructs a shader resource with the given content key.
        ///
        /// \param Key The unique content key identifying this shader.
        ZY_INLINE explicit Shader(AnyRef<Content::Uri> Key)
            : AbstractResource { Move(Key) }
        {
        }

        /// \brief Adds a fragment the source is stitched together from.
        ///
        /// \param Module The shader the directive names.
        /// \param Offset The offset of the directive in the source.
        /// \param Length The length of the directive in the source.
        ZY_INLINE void AddInclude(ConstRetainer<Shader> Module, UInt32 Offset, UInt32 Length)
        {
            mFragments.Append(Module, Offset, Length);
        }

        /// \brief Sets the shader source data.
        ///
        /// \param Source The shader data blob.
        ZY_INLINE void SetSource(AnyRef<Blob> Source)
        {
            SetFootprint(Source.GetSize());

            mSource = Move(Source);
        }

        /// \brief Gets the shader source data.
        ///
        /// \return A constant view over the shader data.
        ZY_INLINE ConstSpan<Byte> GetSource() const
        {
            return ConstSpan(mSource.GetData<Byte>(), mSource.GetSize());
        }

        /// \brief Gets the shader source data as text.
        ///
        /// \return A constant view over the shader data.
        ZY_INLINE Text GetText() const
        {
            return Text(mSource.GetData<Char>(), mSource.GetSize());
        }

        /// \see Content::Resource::OnCreate(Ref<Engine::Subsystem::Host>)
        Bool OnCreate(Ref<Engine::Subsystem::Host> Host) override
        {
            // Every fragment has finished by now, and carries its own already stitched, so one pass is enough.
            if (!mFragments.IsEmpty())
            {
                Stitch();
            }
            return true;
        }

        /// \see Content::Resource::OnDelete(Ref<Engine::Subsystem::Host>)
        void OnDelete(Ref<Engine::Subsystem::Host> Host) override
        {
            mFragments.Clear();
        }

    private:

        /// \brief Represents the span of the source one directive occupies.
        struct Fragment final
        {
            /// The shader the directive names.
            Retainer<Shader> Module;

            /// The offset of the directive in the source.
            UInt32           Offset;

            /// The length of the directive in the source.
            UInt32           Length;
        };

        /// \brief Replaces the span each directive occupies with the source of the shader it names.
        ZY_INLINE void Stitch()
        {
            const Text Source = GetText();

            Str Result;
            Result.Reserve(Source.GetSize());

            UInt Cursor = 0;

            for (ConstRef<Fragment> Fragment : mFragments)
            {
                // A zero count asks `Slice` for everything that remains, so an empty gap is skipped instead.
                if (Fragment.Offset > Cursor)
                {
                    Result.Append(Source.Slice(Cursor, Fragment.Offset - Cursor));
                }

                if (Fragment.Module->HasFailed())
                {
                    LOG_W("Shader: '{0}' includes '{1}', which never loaded",
                        GetKey().GetUrl(), Fragment.Module->GetKey().GetUrl());
                }
                else
                {
                    Result.Append(Fragment.Module->GetText());
                }

                Cursor = Fragment.Offset + Fragment.Length;
            }
            Result.Append(Source.Slice(Cursor));

            Blob Stitched = Blob::Allocate<Char>(Result.GetSize());
            Stitched.Copy(Result.GetData(), static_cast<UInt32>(Result.GetSize()));

            SetSource(Move(Stitched));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Blob               mSource;
        Sequence<Fragment> mFragments;
    };
}