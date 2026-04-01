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

#include "Zyphryon.Base/Utility.hpp"
#include <variant>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A type-safe union that can hold one of several specified types at a time.
    ///
    /// \tparam Types The list of types that the variant can hold.
    template<typename... Types>
    class Variant
    {
    public:

        /// \brief Constructs a variant with the first alternative type default-initialized.
        ZYPHRYON_INLINE Variant()
            : mStorage { std::in_place_index<0> }
        {
        }

        /// \brief Constructs a new variant with the specified type.
        ///
        /// \param Parameters Arguments to forward to the constructor of the type.
        template<typename Type, typename... Arguments>
        ZYPHRYON_INLINE Variant(AnyRef<Arguments>... Parameters)
            : mStorage { std::in_place_type<Type>, Forward<Arguments>(Parameters)... }
        {
        }

        /// \brief Constructs a new variant by copying or moving an existing value.
        ///
        /// \param Value The value to copy or move into the variant.
        template<typename Type>
        ZYPHRYON_INLINE explicit Variant(AnyRef<Type> Value)
            : mStorage{ Forward<Type>(Value) }
        {
        }

        /// \brief Constructs a new object of the specified type in-place within the variant.
        ///
        /// \param Parameters The arguments to forward to the constructor.
        /// \return A reference to the newly constructed object.
        template<typename Type, typename... Arguments>
        ZYPHRYON_INLINE Ref<Type> Emplace(AnyRef<Arguments>... Parameters)
        {
            return mStorage.template emplace<Type>(Forward<Arguments>(Parameters)...);
        }

        /// \brief Returns the index of the currently active alternative type.
        ///
        /// \return The zero-based index of the active type.
        ZYPHRYON_INLINE UInt32 GetID() const
        {
            return mStorage.index();
        }

        /// \brief Retrieves a reference to the currently stored value of the specified type.
        ///
        /// \return A reference to the stored value of the specified type.
        template<typename Type>
        ZYPHRYON_INLINE Ref<Type> GetData()
        {
            return std::get<Type>(mStorage);
        }

        /// \brief Retrieves a const reference to the currently stored value of the specified type.
        ///
        /// \return A const reference to the stored value of the specified type.
        template<typename Type>
        ZYPHRYON_INLINE ConstRef<Type> GetData() const
        {
            return std::get<Type>(mStorage);
        }

        /// \brief Checks whether the variant currently holds the specified type.
        ///
        /// \tparam Type The type to check for.
        /// \return `true` if the variant currently contains the specified type, `false` otherwise.
        template<typename Type>
        ZYPHRYON_INLINE Bool Contains() const
        {
            return std::holds_alternative<Type>(mStorage);
        }

        /// \brief Applies a visitor to the variant's stored value.
        ///
        /// \param Visitor A callable object that can handle all possible variant types.
        /// \return The result of invoking the visitor with the stored value.
        template<typename Function>
        ZYPHRYON_INLINE decltype(auto) Visit(AnyRef<Function> Visitor)
        {
            return std::visit(Forward<Function>(Visitor), mStorage);
        }

        /// \brief Applies a visitor to the variant's stored value.
        ///
        /// \param Visitor A callable object that can handle all possible variant types.
        /// \return The result of invoking the visitor with the stored value.
        template<typename Function>
        ZYPHRYON_INLINE decltype(auto) Visit(AnyRef<Function> Visitor) const
        {
            return std::visit(Forward<Function>(Visitor), mStorage);
        }

        /// \brief Applies a visitor to multiple variants simultaneously.
        ///
        /// \param Visitor A callable object that can handle all combinations of variant types.
        /// \param Others  The other variants to visit along with this one.
        /// \return The result of invoking the visitor with all variant values.
        template<typename Function, typename... Arguments>
        ZYPHRYON_INLINE decltype(auto) Visit(AnyRef<Function> Visitor, AnyRef<Arguments>... Others)
        {
            return std::visit(Forward<Function>(Visitor), mStorage, Others.mStorage...);
        }

        /// \brief Applies a visitor to multiple variants simultaneously.
        ///
        /// \param Visitor A callable object that can handle all combinations of variant types.
        /// \param Others  The other variants to visit along with this one.
        /// \return The result of invoking the visitor with all variant values.
        template<typename Function, typename... Arguments>
        ZYPHRYON_INLINE decltype(auto) Visit(AnyRef<Function> Visitor, AnyRef<Arguments>... Others) const
        {
            return std::visit(Forward<Function>(Visitor), mStorage, Others.mStorage...);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            // Serialize the current type index to identify which alternative is active.
            UInt32 ID = mStorage.index();
            Archive.SerializeUInt(ID);

            // Serialize the actual data based on the active type index.
            RunByIndex(ID, [&]<UInt32 Index>()
            {
                if constexpr (Serializer::IsReader)
                {
                    mStorage.template emplace<Index>();
                }
                Archive.SerializeObject(std::get<Index>(mStorage));
            });
        }

    private:

        /// \brief Alias for the underlying storage type of the variant, which is a `std::variant` of the specified types.
        using Storage = std::variant<Types...>;

        /// \brief Helper function to run a callback based on the active type index of the variant.
        ///
        /// \param Index  The index of the active type in the variant.
        /// \param Action The callback to execute, which should be a template that can be instantiated with the index.
        template <typename Callback>
        static constexpr void RunByIndex(UInt32 Index, AnyRef<Callback> Action)
        {
            RunByIndex(Index, Action, std::make_index_sequence<std::variant_size_v<Storage>> { });
        }

        /// \brief Helper function to run a callback based on the active type index of the variant, using an index sequence to handle multiple types.
        ///
        /// \tparam Values The index sequence representing the possible type indices in the variant.
        /// \param  Index  The index of the active type in the variant.
        /// \param  Action The callback to execute, which should be a template that can be instantiated with the index.
        template <typename Callback, UInt... Values>
        static constexpr void RunByIndex(UInt32 Index, AnyRef<Callback> Action, std::index_sequence<Values...>)
        {
            ((Values == Index ? (Action.template operator()<Values>(), true) : false) || ...);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Storage mStorage;
    };
}
