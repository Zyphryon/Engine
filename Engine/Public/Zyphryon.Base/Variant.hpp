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

#include "Utility.hpp"
#include <variant>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A type-safe union that holds one of several alternative types.
    ///
    /// \tparam Types The types that can be stored in the variant.
    template<typename... Types>
    class Variant final
    {
    public:

        /// \brief Constructs a variant with the first alternative type default-initialized.
        ZYPHRYON_INLINE Variant()
            : mHolder { std::in_place_index<0> }
        {
        }

        /// \brief Constructs a variant holding the specified value.
        ///
        /// \param Value The value to initialize the variant with.
        template<typename Type>
        ZYPHRYON_INLINE explicit Variant(AnyRef<Type> Value)
            : mHolder { Forward<Type>(Value) }
        {
        }

        /// \brief Constructs a new object of the specified type in-place within the variant.
        ///
        /// \param Parameters The arguments to forward to the constructor.
        /// \return A reference to the newly constructed object.
        template<typename Type, typename... Arguments>
        ZYPHRYON_INLINE Ref<Type> Emplace(AnyRef<Arguments>... Parameters)
        {
            return mHolder.template emplace<Type>(Forward<Arguments>(Parameters)...);
        }

        /// \brief Retrieves a reference to the stored value of the specified type.
        ///
        /// \tparam Type The type of value to retrieve.
        /// \return A reference to the stored value.
        template<typename Type>
        ZYPHRYON_INLINE Ref<Type> GetData()
        {
            return std::get<Type>(mHolder);
        }

        /// \brief Retrieves a const reference to the stored value of the specified type.
        ///
        /// \tparam Type The type of value to retrieve.
        /// \return A const reference to the stored value.
        template<typename Type>
        ZYPHRYON_INLINE ConstRef<Type> GetData() const
        {
            return std::get<Type>(mHolder);
        }

        /// \brief Returns the index of the currently active alternative type.
        ///
        /// \return The zero-based index of the active type.
        ZYPHRYON_INLINE UInt32 GetIndex() const
        {
            return mHolder.index();
        }

        /// \brief Checks whether the variant currently holds the specified type.
        ///
        /// \tparam Type The type to check for.
        /// \return `true` if the variant currently contains the specified type, `false` otherwise.
        template<typename Type>
        ZYPHRYON_INLINE Bool Contains() const
        {
            return std::holds_alternative<Type>(mHolder);
        }

        /// \brief Applies a visitor to the variant's stored value.
        ///
        /// \param Visitor A callable object that can handle all possible variant types.
        /// \return The result of invoking the visitor with the stored value.
        template<typename Function>
        ZYPHRYON_INLINE decltype(auto) Visit(AnyRef<Function> Visitor)
        {
            return std::visit(Forward<Function>(Visitor), mHolder);
        }

        /// \brief Applies a visitor to the variant's stored value.
        ///
        /// \param Visitor A callable object that can handle all possible variant types.
        /// \return The result of invoking the visitor with the stored value.
        template<typename Function>
        ZYPHRYON_INLINE decltype(auto) Visit(AnyRef<Function> Visitor) const
        {
            return std::visit(Forward<Function>(Visitor), mHolder);
        }

        /// \brief Applies a visitor to multiple variants simultaneously.
        ///
        /// \param Visitor A callable object that can handle all combinations of variant types.
        /// \param Others  The other variants to visit along with this one.
        /// \return The result of invoking the visitor with all variant values.
        template<typename Function, typename... Arguments>
        ZYPHRYON_INLINE decltype(auto) Visit(AnyRef<Function> Visitor, AnyRef<Arguments>... Others)
        {
            return std::visit(Forward<Function>(Visitor), mHolder, Others.mHolder...);
        }

        /// \brief Applies a visitor to multiple variants simultaneously.
        ///
        /// \param Visitor A callable object that can handle all combinations of variant types.
        /// \param Others  The other variants to visit along with this one.
        /// \return The result of invoking the visitor with all variant values.
        template<typename Function, typename... Arguments>
        ZYPHRYON_INLINE decltype(auto) Visit(AnyRef<Function> Visitor, AnyRef<Arguments>... Others) const
        {
            return std::visit(Forward<Function>(Visitor), mHolder, Others.mHolder...);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            // Serialize the current type index to identify which alternative is active.
            UInt32 Type = mHolder.index();
            Archive.SerializeUInt(Type);

            // Serialize the actual data based on the active type index.
            RunByIndex(Type, [&]<UInt32 Index>()
            {
                if constexpr (Serializer::IsReader)
                {
                    mHolder.template emplace<Index>();
                }
                Archive.SerializeObject(std::get<Index>(mHolder));
            });
        }

    private:

        /// \brief Helper to run a callback based on the current type index.
        ///
        /// \param Index    The current type index.
        /// \param Action   The callback to invoke.
        template <typename Callback>
        constexpr void RunByIndex(UInt32 Index, AnyRef<Callback> Action)
        {
            RunByIndex(Index, Action, std::make_index_sequence<std::variant_size_v<decltype(mHolder)>> { });
        }

        /// \brief Helper to run a callback based on the current type index.
        ///
        /// \param Index    The current type index.
        /// \param Action   The callback to invoke.
        template <typename Callback, UInt... Values>
        constexpr void RunByIndex(UInt32 Index, AnyRef<Callback> Action, std::index_sequence<Values...>)
        {
            const auto Filter = [Index]<UInt Type>()
            {
                return (Type == Index);
            };
            ((Filter.template operator()<Values>() ? (Action.template operator()<Values>(), true) : false) || ...);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        std::variant<Types...> mHolder;
    };
}