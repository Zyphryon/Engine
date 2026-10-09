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

#include "Zyphryon.Base/Container/Sequence.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Specifies the predefined inline storage sizes for delegates.
    enum class DelegateInlineSize
    {
        None      = 0,    ///< No inline storage
        Smallest  = 1,    ///< Enough for function pointers and stateless lambdas.
        Small     = 2,    ///< Enough for function pointers and small lambdas.
        Standard  = 4,    ///< Reasonable default size for most use cases.
        Large     = 8,    ///< Enough for larger lambdas and small functor objects.
        Largest   = 16,   ///< Maximum size for inline storage, enough for most functor objects.
    };

    /// \brief Represents a type-safe delegate that can bind to free functions, member functions, and lambdas.
    template<typename Signature, DelegateInlineSize InlineSize = DelegateInlineSize::Standard>
    class ZY_ALIGN_CPU Delegate;

    /// \brief Represents a type-safe delegate that can bind to free functions, member functions, and lambdas.
    template<typename Return, typename... Arguments, DelegateInlineSize InlineSize>
    class ZY_ALIGN_CPU Delegate<Return(Arguments...), InlineSize> final
    {
    public:

        /// \brief Capacity of the internal storage buffer, in bytes.
        static constexpr UInt32 kCapacity  = static_cast<UInt32>(InlineSize) * sizeof(void *);

    public:

        /// \brief Default constructor, creates an empty delegate.
        ZY_INLINE constexpr Delegate()
            : mStorage { },
              mExecute { AddressOf(InvokeEmpty) },
              mManage  { AddressOf(ManageEmpty) }
        {
        }

        /// \brief Construct a delegate from a free function or static member function.
        ///
        /// \tparam Function The function pointer to bind.
        template<auto Function>
        ZY_INLINE constexpr Delegate(Constant<Function>)
            : mStorage { },
              mExecute { AddressOf(InvokeDirect<Function>) },
              mManage  { AddressOf(ManageEmpty) }
        {
        }

        /// \brief Construct a delegate from a function pointer.
        ///
        /// \param Function The function pointer to bind.
        ZY_INLINE constexpr Delegate(Return (* Function)(Arguments...))
            : mStorage { },
              mExecute { AddressOf(InvokeFunction) },
              mManage  { AddressOf(ManagePlain) }
        {
            * reinterpret_cast<Return(**)(Arguments...)>(mStorage) = Function;
        }

        /// \brief Construct a delegate from a member function and object instance.
        ///
        /// \tparam Method The member function pointer to bind.
        /// \param  Object The object instance to bind the method to.
        template<auto Method, typename Type>
        ZY_INLINE constexpr Delegate(Constant<Method>, Ptr<Type> Object)
            : mStorage { },
              mExecute { AddressOf(InvokeMethod<Method, Type>) },
              mManage  { AddressOf(ManagePlain) }
        {
            * reinterpret_cast<Ptr<Ptr<Type>>>(mStorage) = Object;
        }

        /// \brief Construct a delegate from a lambda or callable object.
        ///
        /// \param Object The lambda or callable object to bind.
        template<typename Callable>
        ZY_INLINE constexpr Delegate(AnyRef<Callable> Object)
            requires (!IsAnyOf<StripAll<Callable>, Delegate>)
            : mStorage { }
        {
            using Type = StripAll<Callable>;

            if constexpr (sizeof(Type) <= kCapacity)
            {
                Construct<Type>(reinterpret_cast<Ptr<Type>>(mStorage), Forward<Callable>(Object));

                mExecute = AddressOf(InvokeMoveableLambda<Type>);
                mManage  = IsTriviallyCopyable<Type> ? AddressOf(ManagePlain) : AddressOf(ManageStack<Type>);
            }
            else
            {
                * reinterpret_cast<Ptr<Type> *>(mStorage) = new Type(Forward<Callable>(Object));

                mExecute = AddressOf(InvokeDynamicLambda<Type>);
                mManage  = AddressOf(ManageHeap<Type>);
            }
        }

        /// \brief Copy constructor, duplicates another delegate.
        ///
        /// \param Other The other delegate to copy from.
        ZY_INLINE constexpr Delegate(ConstRef<Delegate> Other)
            : mStorage { },
              mExecute { Other.mExecute },
              mManage  { Other.mManage }
        {
            Duplicate(Other);
        }

        /// \brief Move constructor, transfers ownership from another delegate.
        ///
        /// \param Other The other delegate to move from.
        ZY_INLINE constexpr Delegate(AnyRef<Delegate> Other)
            : mStorage { },
              mExecute { Exchange(Other.mExecute, AddressOf(InvokeEmpty)) },
              mManage  { Exchange(Other.mManage, AddressOf(ManageEmpty)) }
        {
            mManage(Operation::Move, mStorage, Other.mStorage);
        }

        /// \brief Destructor, releases any resources held by the delegate.
        ZY_INLINE ~Delegate()
        {
            Reset();
        }

        /// \brief Binds a free function or static member function to the delegate.
        ///
        /// \tparam Function The function pointer to bind.
        template<auto Function>
        ZY_INLINE void Bind()
        {
            Reset();
            mExecute = AddressOf(InvokeDirect<Function>);
            mManage  = AddressOf(ManageEmpty);
        }

        /// \brief Binds a member function and object instance to the delegate.
        ///
        /// \tparam Method The member function pointer to bind.
        /// \param  Object The object instance to bind the method to.
        template<auto Method, typename Type>
        ZY_INLINE void Bind(Ptr<Type> Object)
        {
            Reset();
            * reinterpret_cast<Ptr<Ptr<Type>>>(mStorage) = Object;
            mExecute = AddressOf(InvokeMethod<Method, Type>);
            mManage  = AddressOf(ManagePlain);
        }

        /// \brief Binds a function pointer to the delegate.
        ///
        /// \param Function The function pointer to bind.
        ZY_INLINE void Bind(Return (* Function)(Arguments...))
        {
            Reset();
            * reinterpret_cast<Return(**)(Arguments...)>(mStorage) = Function;
            mExecute = AddressOf(InvokeFunction);
            mManage  = AddressOf(ManagePlain);
        }

        /// \brief Binds a lambda or callable object to the delegate.
        ///
        /// \param Object The lambda or callable object to bind.
        template<typename Callable>
        ZY_INLINE void Bind(AnyRef<Callable> Object)
        {
            Reset();

            using Type = StripAll<Callable>;

            if constexpr (sizeof(Type) <= kCapacity)
            {
                Construct<Type>(reinterpret_cast<Ptr<Type>>(mStorage), Forward<Callable>(Object));

                mExecute = AddressOf(InvokeMoveableLambda<Type>);
                mManage  = IsTriviallyCopyable<Type> ? AddressOf(ManagePlain) : AddressOf(ManageStack<Type>);
            }
            else
            {
                * reinterpret_cast<Ptr<Type> *>(mStorage) = new Type(Forward<Callable>(Object));

                mExecute = AddressOf(InvokeDynamicLambda<Type>);
                mManage  = AddressOf(ManageHeap<Type>);
            }
        }

        /// \brief Returns whether the delegate is empty.
        ///
        /// \return `true` if the delegate is empty, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return (mExecute == AddressOf(InvokeEmpty));
        }

        /// \brief Returns whether the delegate is bound to the specified free function or static member function.
        ///
        /// \tparam Function The function pointer to check.
        /// \return `true` if the delegate is bound to the specified function, otherwise `false`.
        template<auto Function>
        ZY_INLINE Bool IsBoundTo() const
        {
            return (mExecute == AddressOf(InvokeDirect<Function>));
        }

        /// \brief Returns whether the delegate is bound to the specified function pointer.
        ///
        /// \param Function The function pointer to check.
        /// \return `true` if the delegate is bound to the specified function, otherwise `false`.
        ZY_INLINE Bool IsBoundTo(Return (* Function)(Arguments...)) const
        {
            return (mExecute == AddressOf(InvokeFunction)) && (* reinterpret_cast<Return(* const *)(Arguments...)>(mStorage) == Function);
        }

        /// \brief Returns whether the delegate is bound to the specified member function and object instance.
        ///
        /// \tparam Method The member function pointer to check.
        /// \param  Object The object instance to check.
        /// \return `true` if the delegate is bound to the specified method and object, otherwise `false`.
        template<auto Method, typename T>
        ZY_INLINE Bool IsBoundTo(ConstPtr<T> Object) const
        {
            return (mExecute == AddressOf(InvokeMethod<Method, T>)) && (* reinterpret_cast<Ptr<T> const *>(mStorage) == Object);
        }

        /// \brief Invokes the bound function once and then resets the delegate to an empty state.
        ///
        /// \param Parameters The arguments to forward to the bound function.
        ZY_INLINE void Consume(Arguments... Parameters)
        {
            if (!IsEmpty())
            {
                Delegate Temp = Move(* this);

                Temp(Forward<Arguments>(Parameters)...);
            }
        }

        /// \brief Copy assignment operator.
        ///
        /// \param Other The other delegate to copy from.
        /// \return Reference to this delegate.
        ZY_INLINE Ref<Delegate> operator=(ConstRef<Delegate> Other)
        {
            if (this != AddressOf(Other))
            {
                Reset();

                mExecute = Other.mExecute;
                mManage  = Other.mManage;
                Duplicate(Other);
            }
            return (* this);
        }

        /// \brief Move assignment operator, transfers ownership from another delegate.
        ///
        /// \param Other The other delegate to move from.
        /// \return Reference to this delegate.
        ZY_INLINE Ref<Delegate> operator=(AnyRef<Delegate> Other)
        {
            if (this != AddressOf(Other))
            {
                Reset();

                mExecute = Exchange(Other.mExecute, AddressOf(InvokeEmpty));
                mManage  = Exchange(Other.mManage, AddressOf(ManageEmpty));

                mManage(Operation::Move, mStorage, Other.mStorage);
            }
            return (* this);
        }

        /// \brief Compares this delegate to another for equality.
        ///
        /// \param Other The other delegate to compare against.
        /// \return `true` if both delegates are bound to the same function and object, otherwise `false`.
        ZY_INLINE Bool operator==(ConstRef<Delegate> Other) const
        {
            if (mExecute != Other.mExecute || mManage != Other.mManage)
            {
                return false;
            }
            return Compare(mStorage, Other.mStorage, kCapacity);
        }


        /// \brief Calls the bound function with the provided arguments.
        ///
        /// \param Parameters The arguments to forward to the bound function.
        ZY_INLINE Return operator()(Arguments... Parameters) const
        {
            return mExecute(mStorage, Forward<Arguments>(Parameters)...);
        }

    public:

        /// \brief Creates a delegate bound to a free function or static member function.
        ///
        /// \tparam Function The function pointer to bind.
        /// \return A delegate bound to the specified function.
        template<auto Function>
        ZY_INLINE static constexpr Delegate Create()
        {
            return Delegate(Constant<Function>{ });
        }

        /// \brief Creates a delegate bound to a function pointer.
        ///
        /// \param Function The function pointer to bind.
        /// \return A delegate bound to the specified function.
        ZY_INLINE static constexpr Delegate Create(Return (*Function)(Arguments...))
        {
            return Delegate(Function);
        }

        /// \brief Creates a delegate bound to a member function and object instance.
        ///
        /// \tparam Method The member function pointer to bind.
        /// \param Object  The object instance to bind the method to.
        /// \return A delegate bound to the specified method and object.
        template<auto Method, typename Type>
        ZY_INLINE static constexpr Delegate Create(Ptr<Type> Object)
        {
            return Delegate(Constant<Method>{ }, Object);
        }

        /// \brief Creates a delegate bound to a lambda or callable object.
        ///
        /// \param Object The lambda or callable object to bind.
        /// \return A delegate bound to the specified callable.
        template<typename Callable>
        ZY_INLINE static constexpr Delegate Create(AnyRef<Callable> Object)
            requires (!IsAnyOf<StripAll<Callable>, Delegate>)
        {
            return Delegate(Forward<Callable>(Object));
        }

    private:

        /// \brief Resets the delegate to an empty state, releasing any resources.
        ZY_INLINE void Reset()
        {
            mManage(Operation::Release, mStorage, nullptr);

            mExecute = AddressOf(InvokeEmpty);
            mManage  = AddressOf(ManageEmpty);
        }

        /// \brief Copies the callable of another delegate, leaving this one empty when it cannot be copied.
        ///
        /// \param Other The delegate to copy the callable of.
        ZY_INLINE void Duplicate(ConstRef<Delegate> Other)
        {
            if (!mManage(Operation::Copy, mStorage, Other.mStorage))
            {
                mExecute = AddressOf(InvokeEmpty);
                mManage  = AddressOf(ManageEmpty);
            }
        }

        /// \brief Specifies what a manage function does to the callable a buffer holds.
        enum class Operation : UInt8
        {
            Copy,       ///< Builds a copy of the source's callable in the target.
            Move,       ///< Hands the source's callable over to the target, leaving the source to be forgotten.
            Release,    ///< Destroys the target's callable.
        };

        /// \brief Type alias for the internal trampoline function signature.
        using Execute = Return(*)(ConstPtr<void>, Arguments...);

        /// \brief Type alias for the internal function that copies, moves and releases the callable.
        using Manage = Bool(*)(Operation, Ptr<void>, ConstPtr<void>);

        /// \brief Empty trampoline function for uninitialized delegates.
        ZY_INLINE static auto InvokeEmpty(ConstPtr<void>, Arguments...)
        {
            if constexpr (!IsVoid<Return>)
            {
                return Return { };
            }
        }

        /// \brief Invokes a free function or static member function.
        template<auto Function>
        ZY_INLINE static auto InvokeDirect(ConstPtr<void>, Arguments... Parameters)
        {
            return Function(Forward<Arguments>(Parameters)...);
        }

        /// \brief Invokes a function pointer stored in the internal buffer.
        ZY_INLINE static auto InvokeFunction(ConstPtr<void> Buffer, Arguments... Parameters)
        {
            return (* static_cast<ConstPtr<Return(*)(Arguments...)>>(Buffer))(Forward<Arguments>(Parameters)...);
        }

        /// \brief Invokes a member function on the stored object instance.
        template<auto Function, typename T>
        ZY_INLINE static auto InvokeMethod(ConstPtr<void> Buffer, Arguments... Parameters)
        {
            return ((** static_cast<Ptr<T> const *>(Buffer)).* Function)(Forward<Arguments>(Parameters)...);
        }

        /// \brief Invokes a trivially copyable lambda stored in the internal buffer via a const reference.
        template<typename Callable>
        ZY_INLINE static auto InvokeLambda(ConstPtr<void> Buffer, Arguments... Parameters)
        {
            return (* Launder(static_cast<ConstPtr<Callable>>(Buffer)))(Forward<Arguments>(Parameters)...);
        }

        /// \brief Invokes a move-only lambda stored in the internal buffer via a non-const reference.
        ///
        /// Used for lambdas that are not trivially copyable but fit within the inline storage.
        template<typename Callable>
        ZY_INLINE static auto InvokeMoveableLambda(ConstPtr<void> Buffer, Arguments... Parameters)
        {
            return (* Launder(static_cast<Ptr<Callable>>(const_cast<Ptr<void>>(Buffer))))(Forward<Arguments>(Parameters)...);
        }

        /// \brief Invokes a heap-allocated lambda or callable object.
        template<typename Callable>
        ZY_INLINE static auto InvokeDynamicLambda(ConstPtr<void> Buffer, Arguments... Parameters)
        {
            return (** static_cast<Ptr<Callable> const *>(Buffer))(Forward<Arguments>(Parameters)...);
        }

        /// \brief Manages a binding that holds nothing.
        ///
        /// \return `true` always.
        ZY_INLINE static Bool ManageEmpty(Operation, Ptr<void>, ConstPtr<void>)
        {
            return true;
        }

        /// \brief Manages a function pointer, object pointer or trivially copyable callable as plain bytes.
        ///
        /// \param Action The operation to perform.
        /// \param Target The buffer to copy or move into.
        /// \param Source The buffer to copy or move from.
        /// \return `true` always.
        ZY_INLINE static Bool ManagePlain(Operation Action, Ptr<void> Target, ConstPtr<void> Source)
        {
            if (Action != Operation::Release)
            {
                Copy(static_cast<Ptr<Byte>>(Target), kCapacity, static_cast<ConstPtr<Byte>>(Source));
            }
            return true;
        }

        /// \brief Manages a callable held in the buffer itself, through its own constructors and destructor.
        ///
        /// \param Action The operation to perform.
        /// \param Target The buffer to copy or move into, or whose callable is released.
        /// \param Source The buffer to copy or move from.
        /// \return `false` when asked to copy a callable that cannot be copied, `true` otherwise.
        template<typename Callable>
        ZY_INLINE static Bool ManageStack(Operation Action, Ptr<void> Target, ConstPtr<void> Source)
        {
            switch (Action)
            {
            case Operation::Copy:
                if constexpr (IsCopyConstructible<Callable>)
                {
                    Construct<Callable>(static_cast<Ptr<Callable>>(Target), * Launder(static_cast<ConstPtr<Callable>>(Source)));
                    return true;
                }
                else
                {
                    ZY_ASSERT(false, "A delegate holding a callable that cannot be copied was copied");
                    return false;
                }
            case Operation::Move:
            {
                // The source is left empty by its owner, so its callable is destroyed once moved.
                const Ptr<Callable> Origin = Launder(static_cast<Ptr<Callable>>(const_cast<Ptr<void>>(Source)));
                Construct<Callable>(static_cast<Ptr<Callable>>(Target), Move(* Origin));
                Destruct(* Origin);
                break;
            }
            case Operation::Release:
                Destruct(* Launder(static_cast<Ptr<Callable>>(Target)));
                break;
            }
            return true;
        }

        /// \brief Manages a callable too large for the buffer, held on the heap by the address in the buffer.
        ///
        /// \param Action The operation to perform.
        /// \param Target The buffer to copy or move into, or whose callable is released.
        /// \param Source The buffer to copy or move from.
        /// \return `false` when asked to copy a callable that cannot be copied, `true` otherwise.
        template<typename Callable>
        ZY_INLINE static Bool ManageHeap(Operation Action, Ptr<void> Target, ConstPtr<void> Source)
        {
            switch (Action)
            {
            case Operation::Copy:
                if constexpr (IsCopyConstructible<Callable>)
                {
                    * static_cast<Ptr<Ptr<Callable>>>(Target) = new Callable(** static_cast<ConstPtr<Ptr<Callable>>>(Source));
                    return true;
                }
                else
                {
                    ZY_ASSERT(false, "A delegate holding a callable that cannot be copied was copied");
                    return false;
                }
            case Operation::Move:
                * static_cast<Ptr<Ptr<Callable>>>(Target) = * static_cast<ConstPtr<Ptr<Callable>>>(Source);
                break;
            case Operation::Release:
                delete * static_cast<Ptr<Ptr<Callable>>>(Target);
                break;
            }
            return true;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Byte    mStorage[kCapacity];
        Execute mExecute;
        Manage  mManage;
    };

    /// \brief Represents a type-safe delegate that can bind to free functions and static member functions only.
    template<typename Return, typename... Arguments>
    class ZY_ALIGN_CPU Delegate<Return(Arguments...), DelegateInlineSize::None> final
    {
    public:

        /// \brief Default constructor, creates an empty delegate.
        ZY_INLINE constexpr Delegate()
            : mExecute { AddressOf(InvokeEmpty) }
        {
        }

        /// \brief Construct a delegate from a free function or static member function.
        ///
        /// \tparam Function The function pointer to bind.
        template<auto Function>
        ZY_INLINE constexpr Delegate(Constant<Function>)
            : mExecute { AddressOf(InvokeDirect<Function>) }
        {
        }

        /// \brief Construct a delegate from a function pointer.
        ///
        /// \param Function The function pointer to bind.
        ZY_INLINE constexpr Delegate(Return (* Function)(Arguments...))
            : mExecute { Function }
        {
        }

        /// \brief Copy constructor, duplicates another delegate.
        ///
        /// \param Other The other delegate to copy from.
        ZY_INLINE constexpr Delegate(ConstRef<Delegate> Other)
            : mExecute { Other.mExecute }
        {
        }

        /// \brief Move constructor, transfers ownership from another delegate.
        ///
        /// \param Other The other delegate to move from.
        ZY_INLINE constexpr Delegate(AnyRef<Delegate> Other)
            : mExecute { Exchange(Other.mExecute, AddressOf(InvokeEmpty))  }
        {
        }

        /// \brief Destructor, releases any resources held by the delegate.
        ZY_INLINE ~Delegate()
        {
            Reset();
        }

        /// \brief Binds a free function or static member function to the delegate.
        ///
        /// \tparam Function The function pointer to bind.
        template<auto Function>
        ZY_INLINE void Bind()
        {
            Reset();
            mExecute = AddressOf(InvokeDirect<Function>);
        }

        /// \brief Returns whether the delegate is empty.
        ///
        /// \return `true` if the delegate is empty, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return (mExecute == AddressOf(InvokeEmpty));
        }

        /// \brief Returns whether the delegate is bound to the specified free function or static member function.
        ///
        /// \tparam Function The function pointer to check.
        /// \return `true` if the delegate is bound to the specified function, otherwise `false`.
        template<auto Function>
        ZY_INLINE Bool IsBoundTo() const
        {
            return (mExecute == AddressOf(InvokeDirect<Function>));
        }

        /// \brief Copy assignment operator.
        ///
        /// \param Other The other delegate to copy from.
        /// \return Reference to this delegate.
        ZY_INLINE Ref<Delegate> operator=(ConstRef<Delegate> Other)
        {
            if (this != AddressOf(Other))
            {
                Reset();

                mExecute = Other.mExecute;
            }
            return (* this);
        }

        /// \brief Move assignment operator, transfers ownership from another delegate.
        ///
        /// \param Other The other delegate to move from.
        /// \return Reference to this delegate.
        ZY_INLINE Ref<Delegate> operator=(AnyRef<Delegate> Other)
        {
            if (this != AddressOf(Other))
            {
                Reset();

                mExecute = Exchange(Other.mExecute, AddressOf(InvokeEmpty));
            }
            return (* this);
        }

        /// \brief Compares this delegate to another for equality.
        ///
        /// \param Other The other delegate to compare against.
        /// \return `true` if both delegates are bound to the same function and object, otherwise `false`.
        ZY_INLINE Bool operator==(ConstRef<Delegate> Other) const
        {
            return (mExecute == Other.mExecute);
        }


        /// \brief Calls the bound function with the provided arguments.
        ///
        /// \param Parameters The arguments to forward to the bound function.
        ZY_INLINE Return operator()(Arguments... Parameters) const
        {
            return mExecute(Forward<Arguments>(Parameters)...);
        }

    public:

        /// \brief Creates a delegate bound to a free function or static member function.
        ///
        /// \tparam Function The function pointer to bind.
        /// \return A delegate bound to the specified function.
        template<auto Function>
        ZY_INLINE static constexpr Delegate Create()
        {
            return Delegate(Constant<Function>{});
        }

    private:

        /// \brief Resets the delegate to an empty state, releasing any resources.
        ZY_INLINE void Reset()
        {
            mExecute = AddressOf(InvokeEmpty);
        }

        /// \brief Type alias for the internal trampoline function signature.
        using Execute = Return(*)(Arguments...);

        /// \brief Empty trampoline function for uninitialized delegates.
        ZY_INLINE static auto InvokeEmpty(Arguments...)
        {
            if constexpr (!IsVoid<Return>)
            {
                return Return { };
            }
        }

        /// \brief Invokes a free function or static member function.
        template<auto Function>
        ZY_INLINE static auto InvokeDirect(Arguments... Parameters)
        {
            return Function(Forward<Arguments>(Parameters)...);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Execute mExecute;
    };

    /// \brief Represents a multicast delegate that can hold and invoke multiple delegates.
    template<typename Signature, DelegateInlineSize InlineSize = DelegateInlineSize::Standard>
    class ZY_ALIGN_CPU MulticastDelegate;

    /// \brief Represents a multicast delegate that can hold and invoke multiple delegates.
    template<typename Return, typename... Arguments, DelegateInlineSize InlineSize>
    class MulticastDelegate<Return(Arguments...), InlineSize> final
    {
    public:

        /// \brief Delegate type used by the multicast delegate.
        using Type = Delegate<Return(Arguments...), InlineSize>;

    public:

        /// \brief Returns whether the multicast delegate has no bound delegates.
        ///
        /// \return `true` if there are no bound delegates, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mDelegates.IsEmpty();
        }

        /// \brief Adds a delegate to the multicast list.
        ///
        /// \param Entry The delegate to add.
        ZY_INLINE void Add(ConstRef<Type> Entry)
        {
            mDelegates.Append(Entry);
        }

        /// \brief Adds a delegate to the multicast list.
        ///
        /// \param Entry The delegate to add.
        ZY_INLINE void Add(AnyRef<Type> Entry)
        {
            mDelegates.Append(Move(Entry));
        }

        /// \brief Adds a free function or static member function to the multicast list.
        ///
        /// \tparam Function The function pointer to add.
        template<auto Function>
        ZY_INLINE void AddFunction()
        {
            mDelegates.Append(Type::template Create<Function>());
        }

        /// \brief Adds a function pointer to the multicast list.
        ///
        /// \param Function The function pointer to add.
        ZY_INLINE void AddFunction(Return (* Function)(Arguments...))
        {
            mDelegates.Append(Type(Function));
        }

        /// \brief Adds a member function and object instance to the multicast list.
        ///
        /// \tparam Method The member function pointer to add.
        /// \param  Object The object instance to bind the method to.
        template<auto Method, typename ObjectType>
        ZY_INLINE void AddMethod(Ptr<ObjectType> Object)
        {
            mDelegates.Append(Type::template Create<Method>(Object));
        }

        /// \brief Adds a lambda or callable object to the multicast list.
        ///
        /// \param Lambda The lambda or callable object to add.
        template<typename Callable>
        ZY_INLINE void AddLambda(AnyRef<Callable> Lambda)
        {
            mDelegates.Append(Type(Forward<Callable>(Lambda)));
        }

        /// \brief Removes the first delegate equal to \p Entry from the multicast list.
        ///
        /// \param Entry The delegate to remove.
        ZY_INLINE void Remove(ConstRef<Type> Entry)
        {
            mDelegates.Remove(Entry);
        }

        /// \brief Removes the first delegate bound to \p Function from the multicast list.
        ///
        /// \tparam Function The function pointer to remove.
        template<auto Function>
        ZY_INLINE void RemoveFunction()
        {
            mDelegates.RemoveIf([](ConstRef<Type> Delegate)
            {
                return Delegate.template IsBoundTo<Function>();
            });
        }

        /// \brief Removes the first delegate bound to \p Function from the multicast list.
        ///
        /// \param Function The function pointer to remove.
        ZY_INLINE void RemoveFunction(Return (* Function)(Arguments...))
        {
            mDelegates.RemoveIf([Function](ConstRef<Type> Delegate)
            {
                return Delegate.IsBoundTo(Function);
            });
        }

        /// \brief Removes the first delegate bound to \p Method on \p Object from the multicast list.
        ///
        /// \tparam Method The member function pointer to remove.
        /// \param  Object The object instance to remove.
        template<auto Method, typename ObjectType>
        ZY_INLINE void RemoveMethod(Ptr<ObjectType> Object)
        {
            mDelegates.RemoveIf([Object](ConstRef<Type> Delegate)
            {
                return Delegate.template IsBoundTo<Method>(Object);
            });
        }

        /// \brief Removes all delegates from the multicast list.
        ZY_INLINE void Clear()
        {
            mDelegates.Clear();
        }

        /// \brief Invokes all bound delegates with the provided arguments.
        ///
        /// \param Parameters The arguments to forward to all bound delegates.
        ZY_INLINE void Broadcast(Arguments... Parameters) const
        {
            for (ConstRef<Type> Entry : mDelegates)
            {
                Entry(Forward<Arguments>(Parameters)...);
            }
        }

        /// \brief Invokes bound delegates in order until one returns a truthy value.
        ///
        /// \param Parameters The arguments to forward to the bound delegates.
        /// \return The first truthy result, or a default-constructed value if no delegate returned a truthy result.
        ZY_INLINE Return Propagate(Arguments... Parameters) const
        {
            for (ConstRef<Type> Entry : mDelegates)
            {
                if (const Return Result = Entry(Forward<Arguments>(Parameters)...); Result)
                {
                    return Result;
                }
            }
            return Return { };
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Type> mDelegates;
    };
}