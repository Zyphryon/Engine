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

#include "Span.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Gets a pointer to the first element of any contiguous container for range-based iteration.
    ///
    /// \param Container The contiguous container to iterate over.
    /// \return A pointer to the first element.
    template<typename Type>
    constexpr auto begin(Ref<Type> Container)
        requires IsContiguous<Type>
    {
        return Container.GetData();
    }

    /// \brief Gets a pointer past the last element of any contiguous container for range-based iteration.
    ///
    /// \param Container The contiguous container to iterate over.
    /// \return A pointer one past the last element.
    template<typename Type>
    constexpr auto end(Ref<Type> Container)
        requires IsContiguous<Type>
    {
        return Container.GetData() + Container.GetSize();
    }

    /// \brief Gets a read-only pointer to the first element of any contiguous container for range-based iteration.
    ///
    /// \param Container The contiguous container to iterate over.
    /// \return A read-only pointer to the first element.
    template<typename Type>
    constexpr auto begin(ConstRef<Type> Container)
        requires IsContiguous<Type>
    {
        return Container.GetData();
    }

    /// \brief Gets a read-only pointer past the last element of any contiguous container for range-based iteration.
    ///
    /// \param Container The contiguous container to iterate over.
    /// \return A read-only pointer one past the last element.
    template<typename Type>
    constexpr auto end(ConstRef<Type> Container)
        requires IsContiguous<Type>
    {
        return Container.GetData() + Container.GetSize();
    }
}