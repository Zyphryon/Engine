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

#include "Storage/Storage.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    class World;

    /// \brief Represents an entity of a world, a number naming it there that stays valid only while it lives.
    class ZY_API Entity final
    {
        template<typename> friend class Cursor;
        friend class Storage;
        friend class Walk;
        friend class World;

    public:

        /// \brief Constructs an entity that names nothing.
        ZY_INLINE Entity()
            : mStorage { nullptr },
              mID      { 0 }
        {
        }

        /// \brief Gets the number naming the entity, the slot in the low half and its generation in the high one.
        ///
        /// \return The identifier, the form an entity is saved and sent in, which \ref Storage::GetEntity takes back.
        ZY_INLINE UInt64 GetID() const
        {
            return mID;
        }

        /// \brief Gets the world the entity lives in.
        ///
        /// \return The world, or `nullptr` for an entity that names nothing.
        Ptr<World> GetWorld() const;

        /// \brief Checks whether the entity names one living in its world.
        ///
        /// \return `true` if it does, `false` for one destroyed or never made.
        ZY_INLINE Bool IsAlive() const
        {
            return mStorage && mStorage->mDirectory.IsAlive(GetHandle());
        }

        /// \brief Checks whether the entity is an archetype.
        ///
        /// \return `true` if it is, `false` otherwise or when it is not alive.
        ZY_INLINE Bool IsArchetype() const
        {
            return IsAlive() && Directory::IsArchetype(GetIndex());
        }

        /// \brief Checks whether the entity is a part, an archetype built into another or the copy of one.
        ///
        /// \return `true` if it is, `false` otherwise or when it is not alive.
        ZY_INLINE Bool IsPart() const
        {
            return Has<Part>();
        }

        /// \brief Destroys the entity and everything beneath it, once the running walk ends when there is one.
        ZY_INLINE void Destruct() const
        {
            mStorage->Destroy(GetHandle());
        }

        /// \brief Gives the entity a value of its own, adding the component or writing over the one it holds.
        ///
        /// \param Data The value, moved into place.
        /// \return This entity.
        template<typename Value>
        ZY_INLINE Entity Set(AnyRef<Value> Data) const
        {
            using Type = StripAll<Value>;

            if constexpr (IsEmpty<Type>)
            {
                return Add<Type>();
            }
            else
            {
                const UInt32              Identifier = IdentifierOf<Type>();
                ConstRef<Directory::Slot> Entry      = mStorage->mDirectory[GetIndex()];
                const Ptr<Type>           Held       = Entry.Holder->Fetch<Type>(Identifier, Entry.Row);

                // In place, even mid-walk, unless a change of this entity waits already; a reader is told after.
                if (Held && !mStorage->mDeferral.IsMarked(GetIndex()))
                {
                    * Held = Forward<Value>(Data);
                    mStorage->Record(Identifier, Pull::Changed, GetHandle(), nullptr);
                }
                else
                {
                    Store<Type>(Identifier, Entry.Holder->Find(Identifier), Forward<Value>(Data));
                }
                return (* this);
            }
        }

        /// \brief Gives the entity a default component, unless it holds or inherits one already.
        ///
        /// \return This entity.
        template<typename Type>
        ZY_INLINE Entity Add() const
        {
            return Add(Component(IdentifierOf<Type>()));
        }

        /// \brief Gives the entity a default component named at runtime, unless it holds or inherits one already.
        ///
        /// \param Type The component to add.
        /// \return This entity.
        ZY_INLINE Entity Add(Component Type) const
        {
            mStorage->Add(GetHandle(), Type.GetID());
            return (* this);
        }

        /// \brief Takes a component the entity holds off it, which then reads its archetype's if there is one.
        ///
        /// \return This entity.
        template<typename Type>
        ZY_INLINE Entity Remove() const
        {
            return Remove(Component(IdentifierOf<Type>()));
        }

        /// \brief Takes a component named at runtime the entity holds off it.
        ///
        /// \param Type The component to take off.
        /// \return This entity.
        ZY_INLINE Entity Remove(Component Type) const
        {
            mStorage->Remove(GetHandle(), Type.GetID());
            return (* this);
        }

        /// \brief Checks whether the entity carries a component, held or inherited.
        ///
        /// \return `true` if it holds the component or its archetype lends it, `false` otherwise.
        template<typename Type>
        ZY_INLINE Bool Has() const
        {
            return Has(Component(IdentifierOf<Type>()));
        }

        /// \brief Checks whether the entity carries a component named at runtime, held or inherited.
        ///
        /// \param Type The component to look for.
        /// \return `true` if it holds it or its archetype lends it, `false` otherwise or when it is not alive.
        ZY_INLINE Bool Has(Component Type) const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();
            return Entry && mStorage->mDirectory.Sees(* Entry->Holder, Type.GetID());
        }

        /// \brief Checks whether the entity holds a component itself rather than inheriting it.
        ///
        /// \return `true` if it holds it, `false` if it inherits it, lacks it or is not alive.
        template<typename Type>
        ZY_INLINE Bool Owns() const
        {
            return Owns(Component(IdentifierOf<Type>()));
        }

        /// \brief Checks whether the entity holds a component named at runtime itself rather than inheriting it.
        ///
        /// \param Type The component to look for.
        /// \return `true` if it holds it, `false` if it inherits it, lacks it or is not alive.
        ZY_INLINE Bool Owns(Component Type) const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();
            return Entry && Entry->Holder->Has(Type.GetID());
        }

        /// \brief Gets a component the entity carries, the one it inherits only when asked for as `const`.
        ///
        /// \return The component, or `nullptr` when the entity lacks it, is not alive, or for a tag.
        template<typename Type>
        ZY_INLINE Ptr<Type> Get() const
        {
            if constexpr (IsEmpty<StripAll<Type>>)
            {
                return nullptr;
            }
            else
            {
                const UInt32                    Identifier = IdentifierOf<Type>();
                const ConstPtr<Directory::Slot> Entry      = GetSlot();

                if (!Entry)
                {
                    return nullptr;
                }

                if (const Ptr<Type> Held = Entry->Holder->Fetch<Type>(Identifier, Entry->Row))
                {
                    return Held;
                }

                if constexpr (IsImmutable<Type>)
                {
                    const ConstPtr<Byte> Lent = mStorage->mDirectory.FindLent(Entry->Holder->GetBase(), Identifier);
                    return reinterpret_cast<Ptr<Type>>(const_cast<Ptr<Byte>>(Lent));
                }
                else
                {
                    return nullptr;
                }
            }
        }

        /// \brief Gets a copy of a component the entity carries, held or inherited, or a default one.
        ///
        /// \return The component, or a default one when the entity does not carry it or is not alive.
        template<typename Type>
        ZY_INLINE StripAll<Type> GetOrDefault() const
        {
            const ConstPtr<StripAll<Type>> Found = Get<const StripAll<Type>>();
            return Found ? * Found : StripAll<Type>();
        }

        /// \brief Gets a component named at runtime the entity carries, held or inherited.
        ///
        /// \param Type The component to look up.
        /// \return The first byte of the value, or `nullptr` when it is missing, a tag, or the entity is not alive.
        ZY_INLINE Ptr<void> Get(Component Type) const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();

            if (!Entry)
            {
                return nullptr;
            }

            const ConstPtr<Byte> Found = mStorage->mDirectory.FindValue(* Entry->Holder, Entry->Row, Type.GetID());
            return Found == AddressOf(Directory::kPresent) ? nullptr : const_cast<Ptr<Byte>>(Found);
        }

        /// \brief Gets the entity's own value of a component, adding a default or copying the inherited one first.
        ///
        /// \return The value the entity holds, which no walk may be running to ask for.
        template<typename Type>
        ZY_INLINE Ref<Type> Ensure() const
        {
            static_assert(!IsImmutable<Type> && !IsEmpty<StripAll<Type>>, "A component is ensured as a writable value");

            return * static_cast<Ptr<Type>>(Ensure(Component(IdentifierOf<Type>())));
        }

        /// \brief Gets the entity's own value of a component named at runtime, like \ref Ensure.
        ///
        /// \param Type The component, never a tag.
        /// \return The first byte of the value the entity holds.
        ZY_INLINE Ptr<void> Ensure(Component Type) const
        {
            return mStorage->Ensure(GetHandle(), Type.GetID());
        }

        /// \brief Writes part of a component in place, or of the one it would hold, and tells readers it changed.
        ///
        /// \param Callback The callable, taking the value to write, never called when the entity is not alive.
        /// \return This entity.
        template<typename Type, typename Callable>
        ZY_INLINE Entity Modify(AnyRef<Callable> Callback) const
        {
            static_assert(!IsImmutable<Type>, "A component is modified through a writable type");

            // A stale handle may name a reused slot, so nothing is written through it.
            if (!IsAlive())
            {
                return (* this);
            }

            // Behind a change of this entity that waits, the one held is stale and the waiting one would overwrite it.
            const Ptr<Type> Held = Get<Type>();

            if (Held && !(mStorage->mDeferral.IsWalking() && mStorage->mDeferral.IsMarked(GetIndex())))
            {
                Callback(* Held);
                return Notify<Type>();
            }

            const ConstPtr<Type> Seen  = Held ? Held : Get<const Type>();
            Type                 Value = Seen ? * Seen : Type();

            Callback(Value);
            return Set(Move(Value));
        }

        /// \brief Tells readers a component changed, after something wrote it without the entity knowing.
        ///
        /// \return This entity.
        template<typename Type>
        ZY_INLINE Entity Notify() const
        {
            return Notify(Component(IdentifierOf<Type>()));
        }

        /// \brief Tells readers a component named at runtime changed, after something wrote it in place.
        ///
        /// \param Type The component that changed.
        /// \return This entity, which tells nobody once it is gone.
        ZY_INLINE Entity Notify(Component Type) const
        {
            if (IsAlive() && mStorage->IsWatched(Type.GetID(), Pull::Changed))
            {
                mStorage->Notify(GetHandle(), Type.GetID());
            }
            return (* this);
        }

        /// \brief Checks whether a resource has loaded, and otherwise tells readers a component changed once it has.
        ///
        /// \param Content The content service the resource loads through.
        /// \param Asset   The resource to wait on, or empty for none.
        /// \return `true` when it has loaded already, `false` when it is on its way or absent.
        template<typename Type, typename Service, typename Handle>
        ZY_INLINE Bool Await(Ref<Service> Content, ConstRef<Handle> Asset) const
        {
            const Entity Actor = (* this);

            return Content.Await(Asset, [Actor]<typename Resource>(Ref<Resource>)
            {
                Actor.Notify<Type>();
            });
        }

        /// \brief Hands every component the entity holds itself to a callback, tags and markers included.
        ///
        /// \param Callback The callable, taking each component.
        template<typename Callable>
        ZY_INLINE void Each(AnyRef<Callable> Callback) const
        {
            if (const ConstPtr<Directory::Slot> Entry = GetSlot())
            {
                for (const UInt32 Identifier : Entry->Holder->GetSignature())
                {
                    Callback(Component(Identifier));
                }
            }
        }

        /// \brief Writes the bytes of a component named at runtime the entity holds itself.
        ///
        /// \param Type   The component to write.
        /// \param Output The writer to append the bytes to, which carry no length.
        /// \return `true` when written, `false` when the entity does not hold it, saves never carry it, or it is gone.
        ZY_INLINE Bool Write(Component Type, Ref<Writer> Output) const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();

            if (!Entry)
            {
                return false;
            }

            ConstRef<Chunk> Holder = * Entry->Holder;
            const SInt16    Column = Holder.Find(Type.GetID());

            if (Column == Chunk::kAbsent || !(Holder.GetFlags(Type.GetID()) & Chunk::kSaves))
            {
                return false;
            }

            // A tag is written as no bytes at all.
            if (Column >= 0)
            {
                Holder.GetColumns()[Column].Info->Save(Output, Holder.At(Column, Entry->Row));
            }
            return true;
        }

        /// \brief Reads the bytes \ref Write wrote over a component named at runtime, adding it when it is lacking.
        ///
        /// \param Type  The component the bytes belong to.
        /// \param Input The reader, at the bytes, which it is left past.
        /// \return This entity.
        ZY_INLINE Entity Read(Component Type, Ref<Reader> Input) const
        {
            mStorage->Read(GetHandle(), Type.GetID(), Input);
            return (* this);
        }

        /// \brief Makes the entity the last child of another, taking it from any parent it had.
        ///
        /// \param Parent The entity to attach it to, or one naming nothing to make it a root.
        /// \return `true` if it moved, `false` when a sibling there has its name or the parent stands beneath it.
        ZY_INLINE Bool Attach(Entity Parent) const
        {
            if (!IsFree(Parent.IsAlive() ? Parent.GetIndex() : 0, GetName()) || Parent.IsDescendantOf(* this, true))
            {
                return false;
            }

            mStorage->Attach(GetHandle(), Parent.GetHandle());
            return true;
        }

        /// \brief Gets the parent of the entity.
        ///
        /// \return The parent, or one that names nothing for a root or an entity that is not alive.
        ZY_INLINE Entity GetParent() const
        {
            return Wrap(GetSlot() ? mStorage->mDirectory.GetParent(GetIndex()) : 0);
        }

        /// \brief Checks whether anything is attached beneath the entity, without visiting it.
        ///
        /// \return `true` if the entity has at least one child, `false` otherwise.
        ZY_INLINE Bool IsParent() const
        {
            return GetSlot() && mStorage->mDirectory.GetFirst(GetIndex()) != 0;
        }

        /// \brief Hands every child of the entity to a callback, in the order they were attached.
        ///
        /// \param Callback The callable, taking each child, which it may detach or destroy.
        template<typename Callable>
        ZY_INLINE void Children(AnyRef<Callable> Callback) const
        {
            for (UInt32 Cursor = GetSlot() ? mStorage->mDirectory.GetFirst(GetIndex()) : 0; Cursor;)
            {
                // Read ahead, so the callback can detach or destroy the child.
                const UInt32 Next = mStorage->mDirectory.GetNext(Cursor);
                Callback(Wrap(Cursor));
                Cursor = Next;
            }
        }

        /// \brief Hands everything beneath the entity to a callback, each entity before what stands beneath it.
        ///
        /// \param Callback The callable, taking each descendant, which it may destroy with what stands beneath it.
        template<typename Callable>
        ZY_INLINE void Descendants(AnyRef<Callable> Callback) const
        {
            Children([&Callback](Entity Child)
            {
                Callback(Child);
                Child.Descendants(Callback);
            });
        }

        /// \brief Finds the nearest ancestor carrying a component, held or inherited.
        ///
        /// \param Inclusive `true` to look at the entity itself first, `false` to start at its parent.
        /// \return The ancestor, or one that names nothing when none carries it.
        template<typename Type>
        ZY_INLINE Entity Find(Bool Inclusive = false) const
        {
            UInt32 Found = IsAlive() ? (Inclusive ? GetIndex() : mStorage->mDirectory.GetParent(GetIndex())) : 0;

            if (Found && mStorage->mDirectory.FindAncestor(Found, IdentifierOf<Type>()))
            {
                return Wrap(Found);
            }
            return Entity();
        }

        /// \brief Checks whether the entity stands anywhere beneath another.
        ///
        /// \param Ancestor  The entity to look for up the parent chain.
        /// \param Inclusive `true` to count the entity as beneath itself, `false` to start at its parent.
        /// \return `true` if the ancestor is up the chain, `false` otherwise or when either is not alive.
        ZY_INLINE Bool IsDescendantOf(Entity Ancestor, Bool Inclusive = false) const
        {
            if (!IsAlive() || !Ancestor.IsAlive())
            {
                return false;
            }

            ConstRef<Directory> Slots = mStorage->mDirectory;

            const UInt32        First = Inclusive ? GetIndex() : Slots.GetParent(GetIndex());

            for (UInt32 Cursor = First; Cursor; Cursor = Slots.GetParent(Cursor))
            {
                if (Cursor == Ancestor.GetIndex())
                {
                    return true;
                }
            }
            return false;
        }

        /// \brief Gets the archetype the entity was made from, which stays the same for its whole life.
        ///
        /// \return The archetype, or one that names nothing for a plain entity or one not alive.
        ZY_INLINE Entity GetArchetype() const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();
            return Wrap(Entry ? Entry->Holder->GetBase() : 0);
        }

        /// \brief Hands every entity made straight from this archetype to a callback, archetypes among them.
        ///
        /// \param Callback The callable, taking each heir, which it may destroy.
        template<typename Callable>
        ZY_INLINE void Heirs(AnyRef<Callable> Callback) const
        {
            if (!IsArchetype())
            {
                return;
            }

            // Gathered first, so the callback can change or destroy the heirs.
            for (const Handle Heir : mStorage->GetHeirs(GetIndex()))
            {
                Callback(Entity(mStorage, Heir));
            }
        }

        /// \brief Gives the entity the name it is found by among its siblings, and that tools show.
        ///
        /// \param Name The name, or empty to take it away.
        /// \return `true` if the entity answers to the name now, `false` when a sibling already does.
        ZY_INLINE Bool SetName(Text Name) const
        {
            if (!IsFree(mStorage->mDirectory.GetParent(GetIndex()), Name))
            {
                return false;
            }

            if (Name.IsEmpty())
            {
                Remove<Named>();
            }
            else
            {
                Set(Named(Name));
            }
            return true;
        }

        /// \brief Gets the name the entity answers to.
        ///
        /// \return The name, or empty when it has none.
        ZY_INLINE Text GetName() const
        {
            const ConstPtr<Named> Found = Get<const Named>();
            return Found ? Text(Found->Value) : Text();
        }

        /// \brief Gets a hash of the entity.
        ///
        /// \return The identifier, which is its hash.
        ZY_INLINE UInt64 Hash(UInt64) const
        {
            return mID;
        }

        /// \brief Checks whether two entities name the same generation of the same slot.
        ///
        /// \param Other The entity to compare against.
        /// \return `true` if both name the same entity, `false` otherwise.
        ZY_INLINE Bool operator==(ConstRef<Entity> Other) const
        {
            return mID == Other.mID;
        }

    public:

        /// \brief Provides the name the type is registered under in the reflection system.
        ///
        /// \return The reflection name of the type, and how it is shown.
        ZY_INLINE static constexpr auto OnClassify()
        {
            return ZyReflection::Presentation { .Name = "Scene.Entity", .Flat = true };
        }

        /// \brief Provides the reflected members of the type.
        ///
        /// \return The reflected fields.
        ZY_INLINE static constexpr auto OnDescribe()
        {
            return Array(ZyReflection::Field::Property<&Entity::mID>("Id"));
        }

    private:

        /// \brief Constructs an entity of a world.
        ///
        /// \param Owner The world the entity lives in.
        /// \param Actor The handle naming it there.
        ZY_INLINE Entity(Ptr<Storage> Owner, Handle Actor)
            : mStorage { Owner },
              mID      { Actor.GetValue() }
        {
        }

        /// \brief Writes a component that cannot be written in place.
        ///
        /// \param Identifier The identifier of the component to write.
        /// \param Column     The column the entity holds it in, or negative when it holds none.
        /// \param Data       The value to write.
        template<typename Type, typename Value>
        void Store(UInt32 Identifier, SInt16 Column, AnyRef<Value> Data) const
        {
            if (Column < 0
                && !mStorage->mDeferral.IsWalking()
                && !mStorage->IsWatched(Identifier, Pull::Added | Pull::Changed))
            {
                mStorage->Insert<Type>(GetIndex(), Identifier, Forward<Value>(Data));
            }
            else
            {
                alignas(Type) Byte Fresh[sizeof(Type)];
                ::Construct(reinterpret_cast<Ptr<Type>>(Fresh), Forward<Value>(Data));

                mStorage->Commit(GetHandle(), Identifier, Fresh);
            }
        }

        /// \brief Gets the handle naming the entity in its world.
        ///
        /// \return The handle, invalid for an entity that names nothing.
        ZY_INLINE Handle GetHandle() const
        {
            return Handle(mID);
        }

        /// \brief Gets the slot number of the entity.
        ///
        /// \return The slot number, the low half of the identifier.
        ZY_INLINE UInt32 GetIndex() const
        {
            return static_cast<UInt32>(mID);
        }

        /// \brief Gets the directory slot of the entity, without which every read answers nothing.
        ///
        /// \return The slot, or `nullptr` for an entity destroyed or never made.
        ZY_INLINE ConstPtr<Directory::Slot> GetSlot() const
        {
            return mStorage ? mStorage->mDirectory.FindSlot(GetHandle()) : nullptr;
        }

        /// \brief Gets the entity in a slot of the same world.
        ///
        /// \param Index The slot number, or zero for none.
        /// \return The entity, or one that names nothing for slot zero.
        ZY_INLINE Entity Wrap(UInt32 Index) const
        {
            return Index ? Entity(mStorage, mStorage->mDirectory.GetHandle(Index)) : Entity();
        }

        /// \brief Checks whether a name is free among the children of a parent.
        ///
        /// \param Parent The slot number of the parent, or zero for a root, whose name nothing looks up.
        /// \param Name   The name, or empty for none.
        /// \return `true` if no other child answers to it, `false` otherwise.
        ZY_INLINE Bool IsFree(UInt32 Parent, Text Name) const
        {
            if (Parent != 0 && !Name.IsEmpty())
            {
                const UInt32 Holder = mStorage->FindChild(Parent, Name);
                return Holder == 0 || Holder == GetIndex();
            }
            return true;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Storage> mStorage;
        UInt64       mID;
    };
}