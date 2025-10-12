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

#include "Collection.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A type-safe, high-performance delegate.
    template<typename Signature>
    class ZYPHRYON_ALIGN_CPU Delegate;

    /// \brief A type-safe, high-performance delegate.
    template<typename Return, typename... Arguments>
    class ZYPHRYON_ALIGN_CPU Delegate<Return(Arguments...)> final
    {
    public:

        /// \brief Capacity of the internal storage buffer, in bytes.
        static constexpr UInt kCapacity  = 4 * sizeof(UInt);

        /// \brief Default constructor, creates an empty delegate.
        ZYPHRYON_INLINE constexpr Delegate()
            : mStorage { },
              mExecute { &InvokeEmpty },
              mRelease { &ReleaseEmpty }
        {
        }

        /// \brief Construct a delegate from a free function or static member function.
        ///
        /// \tparam Function The function pointer to bind.
        template<auto Function>
        ZYPHRYON_INLINE constexpr Delegate(std::integral_constant<decltype(Function), Function>)
            : mStorage { },
              mExecute { &InvokeDirect<Function> },
              mRelease { &ReleaseEmpty }
        {
        }

        /// \brief Construct a delegate from a function pointer.
        ///
        /// \param Function The function pointer to bind.
        ZYPHRYON_INLINE constexpr Delegate(Return (* Function)(Arguments...))
            : mStorage { },
              mExecute { &InvokeFunction },
              mRelease { &ReleaseEmpty }
        {
            * reinterpret_cast<Return(**)(Arguments...)>(mStorage) = Function;
        }

        /// \brief Construct a delegate from a member function and object instance.
        ///
        /// \tparam Method The member function pointer to bind.
        /// \param  Object The object instance to bind the method to.
        template<auto Method, typename Type>
        ZYPHRYON_INLINE constexpr Delegate(std::integral_constant<decltype(Method), Method>, Ptr<Type> Object)
            : mStorage { },
              mExecute { &InvokeMethod<Method, Type> },
              mRelease { &ReleaseEmpty }
        {
            * reinterpret_cast<Ptr<Ptr<Type>>>(mStorage) = Object;
        }

        /// \brief Construct a delegate from a lambda or callable object.
        ///
        /// \param Object The lambda or callable object to bind.
        template<typename Callable>
        ZYPHRYON_INLINE constexpr Delegate(AnyRef<Callable> Object)
            requires (!std::is_same_v<std::decay_t<Callable>, Delegate>)
        {
            using Type = std::decay_t<Callable>;

            if constexpr (sizeof(Type) <= kCapacity && std::is_trivially_copyable_v<Type>)
            {
                InPlaceCreate<Callable>(mStorage, Forward<Callable>(Object));

                mExecute = &InvokeLambda<Type>;
                mRelease = &ReleaseStack<Type>;
            }
            else
            {
                * reinterpret_cast<Ptr<Type> *>(mStorage) = new Type(Forward<Callable>(Object));

                mExecute = &InvokeDynamicLambda<Type>;
                mRelease = &ReleaseHeap<Type>;
            }
        }

        /// \brief Copy constructor, duplicates another delegate.
        ///
        /// \param Other The other delegate to copy from.
        ZYPHRYON_INLINE constexpr Delegate(ConstRef<Delegate> Other)
            : mStorage { },
              mExecute { Other.mExecute },
              mRelease { Other.mRelease }
        {
            if (mExecute != &InvokeEmpty)
            {
                std::memcpy(mStorage, Other.mStorage, sizeof(mStorage));
            }
        }

        /// \brief Move constructor, transfers ownership from another delegate.
        ///
        /// \param Other The other delegate to move from.
        ZYPHRYON_INLINE constexpr Delegate(AnyRef<Delegate> Other)
            : mStorage { },
              mExecute { Exchange(Other.mExecute, &InvokeEmpty)  },
              mRelease { Exchange(Other.mRelease, &ReleaseEmpty) }
        {
            std::memcpy(mStorage, Other.mStorage, kCapacity);
        }

        /// \brief Destructor, releases any resources held by the delegate.
        ZYPHRYON_INLINE ~Delegate()
        {
            Reset();
        }

        /// \brief Binds a free function or static member function to the delegate.
        ///
        /// \tparam Function The function pointer to bind.
        template<auto Function>
        ZYPHRYON_INLINE void Bind()
        {
            Reset();
            mExecute = &InvokeDirect<Function>;
            mRelease = &ReleaseEmpty;
        }

        /// \brief Binds a member function and object instance to the delegate.
        ///
        /// \tparam Method The member function pointer to bind.
        /// \param  Object The object instance to bind the method to.
        template<auto Method, typename Type>
        ZYPHRYON_INLINE void Bind(Ptr<Type> Object)
        {
            Reset();
            * reinterpret_cast<Ptr<Ptr<Type>>>(mStorage) = Object;
            mExecute = &InvokeMethod<Method, Type>;
            mRelease = &ReleaseEmpty;
        }

        /// \brief Binds a function pointer to the delegate.
        ///
        /// \param Function The function pointer to bind.
        ZYPHRYON_INLINE void Bind(Return (* Function)(Arguments...))
        {
            Reset();
            * reinterpret_cast<Return(**)(Arguments...)>(mStorage) = Function;
            mExecute = &InvokeFunction;
            mRelease = &ReleaseEmpty;
        }

        /// \brief Binds a lambda or callable object to the delegate.
        ///
        /// \param Object The lambda or callable object to bind.
        template<typename Callable>
        ZYPHRYON_INLINE void Bind(AnyRef<Callable> Object)
        {
            Reset();

            using Type = std::decay_t<Callable>;

            if constexpr (sizeof(Type) <= kCapacity && std::is_trivially_copyable_v<Type>)
            {
                InPlaceCreate<Callable>(mStorage, Forward<Callable>(Object));
                mExecute = &InvokeLambda<Type>;
                mRelease = &ReleaseStack<Type>;
            }
            else
            {
                * reinterpret_cast<Ptr<Type> *>(mStorage) = new Type(Forward<Callable>(Object));
                mExecute = &InvokeDynamicLambda<Type>;
                mRelease = &ReleaseHeap<Type>;
            }
        }

        /// \brief Returns whether the delegate is empty.
        ///
        /// \return `true` if the delegate is empty, otherwise `false`.
        ZYPHRYON_INLINE Bool IsEmpty() const
        {
            return (mExecute == &InvokeEmpty);
        }

        /// \brief Returns whether the delegate is bound to the specified free function or static member function.
        ///
        /// \tparam Function The function pointer to check.
        /// \return `true` if the delegate is bound to the specified function, otherwise `false`.
        template<auto Function>
        ZYPHRYON_INLINE Bool IsBoundTo() const
        {
            return (mExecute == &InvokeDirect<Function>);
        }

        /// \brief Returns whether the delegate is bound to the specified function pointer.
        ///
        /// \param Function The function pointer to check.
        /// \return `true` if the delegate is bound to the specified function, otherwise `false`.
        ZYPHRYON_INLINE Bool IsBoundTo(Return (* Function)(Arguments...)) const
        {
            return (mExecute == &InvokeFunction) && (* reinterpret_cast<Return(* const *)(Arguments...)>(mStorage) == Function);
        }

        /// \brief Returns whether the delegate is bound to the specified member function and object instance.
        ///
        /// \tparam Method The member function pointer to check.
        /// \param  Object The object instance to check.
        /// \return `true` if the delegate is bound to the specified method and object, otherwise `false`.
        template<auto Method, typename T>
        ZYPHRYON_INLINE Bool IsBoundTo(ConstPtr<T> Object) const
        {
            return (mExecute == &InvokeMethod<Method, T>) && (* reinterpret_cast<Ptr<T> const *>(mStorage) == Object);
        }

        /// \brief Copy assignment operator.
        ///
        /// \param Other The other delegate to copy from.
        /// \return Reference to this delegate.
        ZYPHRYON_INLINE Ref<Delegate> operator=(ConstRef<Delegate> Other)
        {
            if (this != &Other)
            {
                Reset();

                mExecute = Other.mExecute;
                mRelease = Other.mRelease;
                std::memcpy(mStorage, Other.mStorage, kCapacity);
            }
            return (* this);
        }

        /// \brief Move assignment operator, transfers ownership from another delegate.
        ///
        /// \param Other The other delegate to move from.
        /// \return Reference to this delegate.
        ZYPHRYON_INLINE Ref<Delegate> operator=(AnyRef<Delegate> Other)
        {
            if (this != &Other)
            {
                Reset();

                mExecute = Exchange(Other.mExecute, &InvokeEmpty);
                mRelease = Exchange(Other.mRelease, &ReleaseEmpty);

                std::memcpy(mStorage, Other.mStorage, kCapacity);
            }
            return (* this);
        }

        /// \brief Compares this delegate to another for equality.
        ///
        /// \param Other The other delegate to compare against.
        /// \return `true` if both delegates are bound to the same function and object,
        ZYPHRYON_INLINE Bool operator==(ConstRef<Delegate> Other) const
        {
            if (mExecute != Other.mExecute || mRelease != Other.mRelease)
            {
                return false;
            }
            return std::memcmp(mStorage, Other.mStorage, kCapacity) == 0;
        }

        /// \brief Compares this delegate to another for inequality.
        ///
        /// \param Other The other delegate to compare against.
        /// \return `true` if the delegates are not equal, otherwise `false`.
        ZYPHRYON_INLINE Bool operator!=(ConstRef<Delegate> Other) const
        {
            return !(*this == Other);
        }

        /// \brief Calls the bound function with the provided arguments.
        ///
        /// \param Parameters The arguments to forward to the bound function.
        ZYPHRYON_INLINE Return operator()(Arguments... Parameters) const
        {
            return mExecute(mStorage, Forward<Arguments>(Parameters)...);
        }

        /// \brief Returns if the delegate is bound to anything.
        ///
        /// \return `true` if the delegate is bound, otherwise `false`.
        ZYPHRYON_INLINE operator Bool() const
        {
            return (mExecute != & InvokeEmpty);
        }

    public:

        /// \brief Creates a delegate bound to a free function or static member function.
        ///
        /// \tparam Function The function pointer to bind.
        /// \return A delegate bound to the specified function.
        template<auto Function>
        ZYPHRYON_INLINE static constexpr Delegate Create()
        {
            return Delegate(std::integral_constant<decltype(Function), Function>{});
        }

        /// \brief Creates a delegate bound to a function pointer.
        ///
        /// \param Function The function pointer to bind.
        /// \return A delegate bound to the specified function.
        ZYPHRYON_INLINE static constexpr Delegate Create(Return (*Function)(Arguments...))
        {
            return Delegate(Function);
        }

        /// \brief Creates a delegate bound to a member function and object instance.
        ///
        /// \tparam Method The member function pointer to bind.
        /// \param Object  The object instance to bind the method to.
        /// \return A delegate bound to the specified method and object.
        template<auto Method, typename Type>
        ZYPHRYON_INLINE static constexpr Delegate Create(Ptr<Type> Object)
        {
            return Delegate(std::integral_constant<decltype(Method), Method>{}, Object);
        }

        /// \brief Creates a delegate bound to a lambda or callable object.
        ///
        /// \param Object The lambda or callable object to bind.
        /// \return A delegate bound to the specified callable.
        template<typename Callable>
        ZYPHRYON_INLINE static constexpr Delegate Create(AnyRef<Callable> Object)
            requires (!std::is_same_v<std::decay_t<Callable>, Delegate>)
        {
            return Delegate(Forward<Callable>(Object));
        }

    private:

        /// \brief Resets the delegate to an empty state, releasing any resources.
        ZYPHRYON_INLINE void Reset()
        {
            if (mRelease != &ReleaseEmpty)
            {
                mRelease(mStorage);
            }

            mExecute = &InvokeEmpty;
            mRelease = &ReleaseEmpty;
        }

        /// \brief Type alias for the internal trampoline function signature.
        using Execute = Return(*)(ConstPtr<void>, Arguments...);

        /// \brief Type alias for the internal release function signature.
        using Release = void(*)(Ptr<void>);

        /// \brief Empty trampoline function for uninitialized delegates.
        ZYPHRYON_INLINE static auto InvokeEmpty(ConstPtr<void>, Arguments...)
        {
            if constexpr (!std::is_void_v<Return>)
            {
                return Return { };
            }
        }

        /// \brief Invokes a free function or static member function.
        template<auto Function>
        ZYPHRYON_INLINE static auto InvokeDirect(ConstPtr<void>, Arguments... Parameters)
        {
            return std::invoke(Function, Forward<Arguments>(Parameters)...);
        }

        /// \brief Invokes a function pointer stored in the internal buffer.
        ZYPHRYON_INLINE static auto InvokeFunction(ConstPtr<void> Buffer, Arguments... Parameters)
        {
            return (* static_cast<ConstPtr<Return(*)(Arguments...)>>(Buffer))(Forward<Arguments>(Parameters)...);
        }

        /// \brief Invokes a member function on the stored object instance.
        template<auto Function, typename T>
        ZYPHRYON_INLINE static auto InvokeMethod(ConstPtr<void> Buffer, Arguments... Parameters)
        {
            return std::invoke(Function, ** static_cast<Ptr<T> const *>(Buffer), Forward<Arguments>(Parameters)...);
        }

        /// \brief Invokes a lambda or callable object stored in the internal buffer.
        template<typename Callable>
        ZYPHRYON_INLINE static auto InvokeLambda(ConstPtr<void> Buffer, Arguments... Parameters)
        {
            return std::invoke(* std::launder(static_cast<ConstPtr<Callable>>(Buffer)), Forward<Arguments>(Parameters)...);
        }

        /// \brief Invokes a heap-allocated lambda or callable object.
        template<typename Callable>
        ZYPHRYON_INLINE static auto InvokeDynamicLambda(ConstPtr<void> Buffer, Arguments... Parameters)
        {
            return std::invoke(** static_cast<Ptr<Callable> const *>(Buffer), Forward<Arguments>(Parameters)...);
        }

        /// \brief Empty release function for uninitialized delegates.
        ZYPHRYON_INLINE static void ReleaseEmpty(Ptr<void>)
        {
        }

        /// \brief Releases a heap-allocated callable object.
        template<typename Callable>
        ZYPHRYON_INLINE static void ReleaseHeap(Ptr<void> Buffer)
        {
            delete * static_cast<Ptr<Callable> *>(Buffer);
        }

        /// \brief Releases a stack-allocated callable object.
        template<typename Callable>
        ZYPHRYON_INLINE static void ReleaseStack(Ptr<void> Buffer)
        {
            InPlaceDelete(* static_cast<Ptr<Callable>>(Buffer));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Byte    mStorage[kCapacity];
        Execute mExecute;
        Release mRelease;
    };

    /// \brief A type-safe, high-performance multicast delegate.
    template<typename Signature>
    class ZYPHRYON_ALIGN_CPU MulticastDelegate;

    /// \brief A type-safe, high-performance multicast delegate.
    template<typename Return, typename... Arguments>
    class MulticastDelegate<Return(Arguments...)> final // TODO: Automatic cleanup
    {
    public:

        /// \brief Returns whether the multicast delegate has no bound delegates.
        ///
        /// \return `true` if there are no bound delegates, otherwise `false`.
        ZYPHRYON_INLINE Bool IsEmpty() const
        {
            return mDelegates.empty();
        }

        /// \brief Adds a delegate to the multicast list.
        ///
        /// \param Delegate The delegate to add.
        ZYPHRYON_INLINE void Add(ConstRef<Delegate<Return(Arguments...)>> Delegate)
        {
            mDelegates.push_back(Delegate);
        }

        /// \brief Adds a delegate to the multicast list.
        ///
        /// \param Delegate The delegate to add.
        ZYPHRYON_INLINE void Add(AnyRef<Delegate<Return(Arguments...)>> Delegate)
        {
            mDelegates.push_back(Move(Delegate));
        }

        /// \brief Adds a free function or static member function to the multicast list.
        ///
        /// \tparam Function The function pointer to add.
        template<auto Function>
        ZYPHRYON_INLINE void AddFunction()
        {
            mDelegates.emplace_back(std::integral_constant<decltype(Function), Function>{});
        }

        /// \brief Adds a function pointer to the multicast list.
        ///
        /// \param Function The function pointer to add.
        ZYPHRYON_INLINE void AddFunction(Return (* Function)(Arguments...))
        {
            mDelegates.emplace_back(Function);
        }

        /// \brief Adds a member function and object instance to the multicast list.
        ///
        /// \tparam Method The member function pointer to add.
        /// \param Object The object instance to bind the method to.
        template<auto Method, typename Type>
        ZYPHRYON_INLINE void AddMethod(Ptr<Type> Object)
        {
            mDelegates.emplace_back(std::integral_constant<decltype(Method), Method>{}, Object);
        }

        /// \brief Adds a lambda or callable object to the multicast list.
        ///
        /// \param Lambda The lambda or callable object to add.
        template<typename Callable>
        ZYPHRYON_INLINE void AddLambda(AnyRef<Callable> Lambda)
        {
            mDelegates.emplace_back(Forward<Callable>(Lambda));
        }

        /// \brief Removes a delegate from the multicast list.
        ///
        /// \param Delegate The delegate to remove.
        ZYPHRYON_INLINE void Remove(ConstRef<Delegate<Return(Arguments...)>> Delegate)
        {
            if (const auto Iterator = std::ranges::find(mDelegates, Delegate); Iterator != mDelegates.end())
            {
                mDelegates.erase(Iterator);
            }
        }

        /// \brief Removes a free function or static member function from the multicast list.
        ///
        /// \tparam Function The function pointer to remove.
        template<auto Function>
        ZYPHRYON_INLINE void RemoveFunction()
        {
            constexpr auto Find = [](ConstRef<Delegate<Return(Arguments...)>> Delegate)
            {
                return Delegate.template IsBoundTo<Function>();
            };

            if (const auto Iterator = std::ranges::find_if(mDelegates, Find); Iterator != mDelegates.end())
            {
                mDelegates.erase(Iterator);
            }
        }

        /// \brief Removes a function pointer from the multicast list.
        ///
        /// \param Function The function pointer to remove.
        ZYPHRYON_INLINE void RemoveFunction(Return (* Function)(Arguments...))
        {
            const auto Find = [Function](ConstRef<Delegate<Return(Arguments...)>> Delegate)
            {
                return Delegate.IsBoundTo(Function);
            };

            if (const auto Iterator = std::ranges::find_if(mDelegates, Find); Iterator != mDelegates.end())
            {
                mDelegates.erase(Iterator);
            }
        }

        /// \brief Removes a member function and object instance from the multicast list.
        ///
        /// \tparam Method The member function pointer to remove.
        /// \param Object  The object instance to remove.
        template<auto Method, typename Type>
        ZYPHRYON_INLINE void RemoveMethod(Ptr<Type> Object)
        {
            const auto Find = [Object](ConstRef<Delegate<Return(Arguments...)>> Delegate)
            {
                return Delegate.template IsBoundTo<Method>(Object);
            };

            if (const auto Iterator = std::ranges::find_if(mDelegates, Find); Iterator != mDelegates.end())
            {
                mDelegates.erase(Iterator);
            }
        }

        /// \brief Removes all delegates from the multicast list.
        ZYPHRYON_INLINE void Clear()
        {
            mDelegates.Clear();
        }

        /// \brief Invokes all bound delegates with the provided arguments.
        ///
        /// \param Parameters The arguments to forward to all bound delegates.
        ZYPHRYON_INLINE void Broadcast(Arguments... Parameters) const
        {
            for (ConstRef<Delegate<Return(Arguments...)>> Delegate : mDelegates)
            {
                Delegate(Forward<Arguments>(Parameters)...);
            }
        }

        /// \brief Invokes bound delegates in order until one returns a truthy value.
        ///
        /// \param Parameters The arguments to forward to the bound delegates.
        ZYPHRYON_INLINE auto Propagate(Arguments... Parameters) const
        {
            for (ConstRef<Delegate<Return(Arguments...)>> Delegate : mDelegates)
            {
                if (const Return Result = Delegate(Forward<Arguments>(Parameters)...); Result)
                {
                    return Result;
                }
            }
            return Return { };
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector<Delegate<Return(Arguments...)>> mDelegates;
    };
}
