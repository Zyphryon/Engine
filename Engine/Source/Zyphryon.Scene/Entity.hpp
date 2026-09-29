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

    /// \brief Represents an entity of a world, named there by its handle.
    class ZY_API Entity final
    {
    public:

        /// \brief Constructs an entity that names nothing.
        ZY_INLINE constexpr Entity()
            : mStorage { nullptr },
              mID      { 0 }
        {
        }

        /// \brief Constructs an entity of a world.
        ///
        /// \param Owner  The world the entity lives in.
        /// \param Handle The handle naming it there.
        ZY_INLINE constexpr Entity(Ptr<Storage> Owner, Handle Handle)
            : mStorage { Owner },
              mID      { Handle.GetValue() }
        {
        }

        /// \brief Gets the number naming the entity, slot in the low half and generation in the high one.
        ///
        /// \return The identifier, the form an entity is saved and sent in.
        ZY_INLINE constexpr UInt64 GetID() const
        {
            return mID;
        }

        /// \brief Gets the handle naming the entity in its world.
        ///
        /// \return The handle.
        ZY_INLINE constexpr Handle GetHandle() const
        {
            return Handle(mID);
        }

        /// \brief Gets the world the entity lives in.
        ///
        /// \return The world, or `nullptr` for an entity that names nothing.
        Ptr<World> GetWorld() const;

        /// \brief Checks whether the entity names a living entity of its world.
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

        /// \brief Checks whether the entity is a part, an archetype built into another or a copy of one.
        ///
        /// \return `true` if it is, `false` otherwise or when it is not alive.
        ZY_INLINE Bool IsPart() const
        {
            return Has<Part>();
        }

        /// \brief Destroys the entity, every component it carries and everything that stands beneath it.
        ZY_INLINE void Destruct() const
        {
            mStorage->Destroy(GetHandle());
        }

        /// \brief Wakes the entity or puts it to sleep, where no query that does not ask for it sees it.
        ///
        /// \param Awake     `true` to wake it, `false` to put it to sleep.
        /// \param Recursive `true` to do the same to everything beneath it, all at once when the call ends.
        /// \return This entity.
        ZY_INLINE Entity SetAwake(Bool Awake, Bool Recursive = false) const
        {
            mStorage->SetAwake(GetHandle(), Awake, Recursive);
            return (* this);
        }

        /// \brief Checks whether the entity is awake.
        ///
        /// \return `true` if it is, `false` while it sleeps or when it is not alive.
        ZY_INLINE Bool IsAwake() const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();
            return Entry && !Entry->Holder->Has(mStorage->mAsleep);
        }

        /// \brief Gives the entity a component of its own, or overwrites the one it holds.
        ///
        /// \param Data The component, moved into place.
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
                const SInt16              Column     = Entry.Holder->Find(Identifier);

                // In place, even mid-walk, unless a reader watches changes or a change of this entity waits already.
                if (Column >= 0
                    && !mStorage->IsWatched(Identifier, Pull::Changed)
                    && !mStorage->mDeferral.IsMarked(GetIndex()))
                {
                    * reinterpret_cast<Ptr<Type>>(Entry.Holder->At(Column, Entry.Row)) = Forward<Value>(Data);
                }
                else
                {
                    Store<Type>(Identifier, Column, Forward<Value>(Data));
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
            const UInt32 Identifier = IdentifierOf<Type>();

            if (mStorage->mDeferral.IsWalking() || mStorage->IsWatched(Identifier, Pull::Added))
            {
                mStorage->Add(GetHandle(), Identifier);
            }
            else
            {
                ConstRef<Chunk> Holder = GetChunk();

                if (!Holder.Has(Identifier) && !mStorage->mDirectory.FindLent(Holder.GetBase(), Identifier))
                {
                    mStorage->Insert<StripAll<Type>>(GetIndex(), Identifier, StripAll<Type>());
                }
            }
            return (* this);
        }

        /// \brief Gives the entity and everything beneath it a default component, like \ref Add.
        ///
        /// \return This entity.
        template<typename Type>
        ZY_INLINE Entity AddRecursively() const
        {
            Add<Type>();
            Descendants([](Entity Child)
            {
                Child.Add<Type>();
            });
            return (* this);
        }

        /// \brief Gives the entity and every ancestor above it a default component, like \ref Add.
        ///
        /// \return This entity.
        template<typename Type>
        ZY_INLINE Entity AddAncestrally() const
        {
            for (Entity Cursor = (* this); Cursor.IsAlive(); Cursor = Cursor.GetParent())
            {
                Cursor.Add<Type>();
            }
            return (* this);
        }

        /// \brief Gives the entity a default component named at runtime, unless it holds or inherits one already.
        ///
        /// \param Type The component.
        /// \return This entity.
        ZY_INLINE Entity Add(Component Type) const
        {
            mStorage->Add(GetHandle(), Type.GetID());
            return (* this);
        }

        /// \brief Takes a component the entity holds off it, which then reads its archetype's again if it has one.
        ///
        /// \return This entity.
        template<typename Type>
        ZY_INLINE Entity Remove() const
        {
            const UInt32 Identifier = IdentifierOf<Type>();

            if (mStorage->mDeferral.IsWalking() || mStorage->IsWatched(Identifier, Pull::Changed | Pull::Removed))
            {
                mStorage->Remove(GetHandle(), Identifier);
            }
            else
            {
                Ref<Chunk> Holder = GetChunk();

                if (Holder.Has(Identifier))
                {
                    mStorage->Transfer(GetIndex(), * mStorage->FindOrCreateRemoval(Holder, Identifier));
                }
            }
            return (* this);
        }

        /// \brief Takes a component off the entity and everything beneath it, like \ref Remove.
        ///
        /// \return This entity.
        template<typename Type>
        ZY_INLINE Entity RemoveRecursively() const
        {
            Remove<Type>();
            Descendants([](Entity Child)
            {
                Child.Remove<Type>();
            });
            return (* this);
        }

        /// \brief Takes a component named at runtime off the entity.
        ///
        /// \param Type The component.
        /// \return This entity.
        ZY_INLINE Entity Remove(Component Type) const
        {
            mStorage->Remove(GetHandle(), Type.GetID());
            return (* this);
        }

        /// \brief Checks whether the entity carries a component, held or inherited.
        ///
        /// \return `true` if it does, `false` otherwise.
        template<typename Type>
        ZY_INLINE Bool Has() const
        {
            return Has(Component(IdentifierOf<Type>()));
        }

        /// \brief Checks whether the entity carries a component named at runtime, held or inherited.
        ///
        /// \param Type The component.
        /// \return `true` if it does, `false` otherwise or when it is not alive.
        ZY_INLINE Bool Has(Component Type) const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();

            return Entry && (Entry->Holder->Has(Type.GetID())
                || mStorage->mDirectory.FindLent(Entry->Holder->GetBase(), Type.GetID()));
        }

        /// \brief Checks whether the entity holds a component itself rather than inheriting it.
        ///
        /// \return `true` if it holds it, `false` if it inherits it or lacks it.
        template<typename Type>
        ZY_INLINE Bool Owns() const
        {
            return Owns(Component(IdentifierOf<Type>()));
        }

        /// \brief Checks whether the entity holds a component named at runtime itself rather than inheriting it.
        ///
        /// \param Type The component.
        /// \return `true` if it holds it, `false` if it inherits it, lacks it or is not alive.
        ZY_INLINE Bool Owns(Component Type) const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();
            return Entry && Entry->Holder->Has(Type.GetID());
        }

        /// \brief Hands every component the entity holds itself to a callback, leaving out those it inherits.
        ///
        /// \param Callback The callable, taking each one, tags and markers like `Prefab`, `Asleep` or `Named` too.
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

        /// \brief Gives the entity its own copy of a component it only inherits, recorded as a change of it.
        ///
        /// \param Type The component, left alone when the entity holds it already or its archetype lends none.
        /// \return This entity.
        ZY_INLINE Entity Override(Component Type) const
        {
            mStorage->Override(GetHandle(), Type.GetID());
            return (* this);
        }

        /// \brief Gets a component the entity carries, the one it inherits included only when asked as `const`.
        ///
        /// \return The component, or `nullptr` when the entity does not carry it or is not alive, and for a tag.
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

                if (const SInt16 Column = Entry->Holder->Find(Identifier); Column >= 0)
                {
                    return reinterpret_cast<Ptr<Type>>(Entry->Holder->At(Column, Entry->Row));
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

        /// \brief Gets a copy of a component the entity carries, held or inherited, or a default one when it carries none.
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
        /// \param Type The component.
        /// \return The first byte of the component, or `nullptr` when it is missing, a tag, or the entity is not alive.
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

        /// \brief Gets the entity's own copy of a component, adding a default one or overriding the inherited one first.
        ///
        /// \return The component the entity holds.
        template<typename Type>
        ZY_INLINE Ref<Type> Ensure() const
        {
            static_assert(!IsImmutable<Type> && !IsEmpty<StripAll<Type>>, "A component is ensured as a writable value");
            ZY_ASSERT(!mStorage->mDeferral.IsWalking(), "A component cannot be ensured while a query walks");

            Override(Component(IdentifierOf<Type>()));
            Add<Type>();
            return * Get<Type>();
        }

        /// \brief Writes a component the entity holds, or the one it would hold, then records the change.
        ///
        /// \param Callback The callable, taking the component to write, never called when the entity is not alive.
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

            if (const Ptr<Type> Held = Get<Type>())
            {
                Callback(* Held);
                return Notify<Type>();
            }

            const ConstPtr<Type> Lent  = Get<const Type>();
            Type                 Value = Lent ? * Lent : Type();

            Callback(Value);
            return Set(Move(Value));
        }

        /// \brief Records that a component written in place changed, for readers of its changes.
        ///
        /// \return This entity.
        template<typename Type>
        ZY_INLINE Entity Notify() const
        {
            return Notify(Component(IdentifierOf<Type>()));
        }

        /// \brief Records that a component named at runtime and written in place changed, for readers of its changes.
        ///
        /// \param Type The component.
        /// \return This entity, which records nothing once it is gone, like a load finishing after it.
        ZY_INLINE Entity Notify(Component Type) const
        {
            if (IsAlive() && mStorage->IsWatched(Type.GetID(), Pull::Changed))
            {
                mStorage->MarkChanged(GetHandle(), Type.GetID());
            }
            return (* this);
        }

        /// \brief Writes the bytes of a component the entity holds itself, as saves carry it, behind their length.
        ///
        /// \param Output The writer.
        /// \return `true` when written, `false` when the entity does not hold it or saves never carry it.
        template<typename Type>
        ZY_INLINE Bool Write(Ref<Writer> Output) const
        {
            return Write(Component(IdentifierOf<Type>()), Output);
        }

        /// \brief Writes the bytes of a component named at runtime the entity holds itself, behind their length.
        ///
        /// \param Type   The component.
        /// \param Output The writer.
        /// \return `true` when written, `false` when the entity does not hold it, saves never carry it or it is gone.
        ZY_INLINE Bool Write(Component Type, Ref<Writer> Output) const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();

            if (!Entry)
            {
                return false;
            }

            ConstRef<Chunk> Holder = * Entry->Holder;
            const UInt32    Row    = Entry->Row;
            const SInt16    Column = Holder.Find(Type.GetID());

            if (Column == Chunk::kAbsent || !(Holder.GetFlags(Type.GetID()) & Chunk::kSaves))
            {
                return false;
            }

            Output.WriteBlock<UInt32>([&Holder, Row, Column](Ref<Writer> Block)
            {
                if (Column >= 0)
                {
                    Holder.GetColumns()[Column].Info->Save(Block, Holder.At(Column, Row));
                }
            });
            return true;
        }

        /// \brief Reads what \ref Write wrote over the component, adding it when the entity lacks it.
        ///
        /// \param Input The reader, at the length.
        /// \return This entity.
        template<typename Type>
        ZY_INLINE Entity Read(Ref<Reader> Input) const
        {
            return Read(Component(IdentifierOf<Type>()), Input);
        }

        /// \brief Reads what \ref Write wrote over a component named at runtime, adding it when the entity lacks it.
        ///
        /// \param Type  The component.
        /// \param Input The reader, at the length.
        /// \return This entity.
        ZY_INLINE Entity Read(Component Type, Ref<Reader> Input) const
        {
            Reader Block(Input.ReadBlock<UInt32, Byte>());
            mStorage->Read(GetHandle(), Type.GetID(), Block);
            return (* this);
        }

        /// \brief Makes the entity the last child of another, taking it from any parent it had.
        ///
        /// \param Parent The entity to attach it to, or one naming nothing to detach.
        /// \return `true` if it stands there now, `false` otherwise.
        ZY_INLINE Bool Attach(Entity Parent) const
        {
            if (!IsFree(Parent.IsAlive() ? Parent.GetIndex() : 0, GetName()) || Parent.IsDescendantOf(* this, true))
            {
                return false;
            }

            mStorage->Attach(GetHandle(), Parent.GetHandle());
            return true;
        }

        /// \brief Gets the entity a part was built into, which it stands, goes and comes with.
        ///
        /// \return The whole, or one that names nothing for an entity that is not a part.
        ZY_INLINE Entity GetWhole() const
        {
            return IsPart() ? GetParent() : Entity();
        }

        /// \brief Gets the parent of the entity.
        ///
        /// \return The parent, or one that names nothing for a root or an entity that is not alive.
        ZY_INLINE Entity GetParent() const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();
            return Entry && Entry->Parent ? Entity(mStorage, mStorage->mDirectory.GetHandle(Entry->Parent)) : Entity();
        }

        /// \brief Hands every child of the entity to a callback, in the order they were attached.
        ///
        /// \param Callback The callable, taking each child.
        template<typename Callable>
        ZY_INLINE void Children(AnyRef<Callable> Callback) const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();

            for (UInt32 Cursor = Entry ? Entry->First : 0; Cursor;)
            {
                // Read ahead, so the callback can detach or destroy the child.
                const UInt32 Next = mStorage->mDirectory[Cursor].Next;
                Callback(Entity(mStorage, mStorage->mDirectory.GetHandle(Cursor)));
                Cursor = Next;
            }
        }

        /// \brief Hands everything beneath the entity to a callback, each entity before what stands beneath it.
        ///
        /// \param Callback The callable, taking each descendant, which may destroy it along with what stands beneath.
        template<typename Callable>
        ZY_INLINE void Descendants(AnyRef<Callable> Callback) const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();

            for (UInt32 Cursor = Entry ? Entry->First : 0; Cursor;)
            {
                // Read ahead, so the callback can detach or destroy the child.
                const UInt32 Next  = mStorage->mDirectory[Cursor].Next;
                const Entity Child = Entity(mStorage, mStorage->mDirectory.GetHandle(Cursor));

                Callback(Child);
                Child.Descendants(Callback);
                Cursor = Next;
            }
        }

        /// \brief Finds a child of the entity by the name it was given.
        ///
        /// \param Name The name of the child.
        /// \return The child, or one that names nothing when none answers to the name or the entity is not alive.
        ZY_INLINE Entity Lookup(Text Name) const
        {
            const UInt32 Found = IsAlive() ? mStorage->FindChild(GetIndex(), Name) : 0;
            return Found ? Entity(mStorage, mStorage->mDirectory.GetHandle(Found)) : Entity();
        }

        /// \brief Finds the nearest ancestor that carries a component, held or lent.
        ///
        /// \param Inclusive `true` to look at the entity itself first, `false` to start at its parent.
        /// \return The ancestor, or one that names nothing when no ancestor carries it.
        template<typename Type>
        ZY_INLINE Entity Find(Bool Inclusive = false) const
        {
            UInt32 Found = IsAlive() ? (Inclusive ? GetIndex() : mStorage->mDirectory[GetIndex()].Parent) : 0;

            if (Found && mStorage->mDirectory.FindAncestor(Found, IdentifierOf<Type>()))
            {
                return Entity(mStorage, mStorage->mDirectory.GetHandle(Found));
            }
            return Entity();
        }

        /// \brief Checks whether the entity stands anywhere beneath another.
        ///
        /// \param Ancestor  The entity to look for up the parent chain.
        /// \param Inclusive `true` to count the entity itself as beneath itself, `false` to start at its parent.
        /// \return `true` if the ancestor is found up the chain, `false` otherwise or when either is not alive.
        ZY_INLINE Bool IsDescendantOf(Entity Ancestor, Bool Inclusive = false) const
        {
            if (!IsAlive() || !Ancestor.IsAlive())
            {
                return false;
            }

            ConstRef<Directory> Slots = mStorage->mDirectory;

            for (UInt32 Cursor = Inclusive ? GetIndex() : Slots[GetIndex()].Parent; Cursor; Cursor = Slots[Cursor].Parent)
            {
                if (Cursor == Ancestor.GetIndex())
                {
                    return true;
                }
            }
            return false;
        }

        /// \brief Makes the entity read from another archetype, or from none, keeping what it holds itself.
        ///
        /// \param Archetype The archetype, or one that names nothing to read from none.
        /// \return This entity.
        ZY_INLINE Entity SetArchetype(Entity Archetype) const
        {
            mStorage->SetArchetype(GetHandle(), Archetype.GetHandle());
            return (* this);
        }

        /// \brief Gets the archetype the entity reads from.
        ///
        /// \return The archetype, or one that names nothing for an entity made from none or not alive.
        ZY_INLINE Entity GetArchetype() const
        {
            const ConstPtr<Directory::Slot> Entry = GetSlot();
            const UInt32                    Base  = Entry ? Entry->Holder->GetBase() : 0;
            return Base ? Entity(mStorage, mStorage->mDirectory.GetHandle(Base)) : Entity();
        }

        /// \brief Hands every entity made straight from this archetype to a callback, archetypes made from it included.
        ///
        /// \param Callback The callable, taking each heir, which never runs for an entity that is not an archetype.
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

        /// \brief Checks whether the entity reads from an archetype.
        ///
        /// \param Base The archetype to look for.
        /// \return `true` if the entity reads from it at any remove, `false` otherwise.
        ZY_INLINE Bool IsInstanceOf(Entity Base) const
        {
            for (Entity Cursor = GetArchetype(); Cursor.IsAlive(); Cursor = Cursor.GetArchetype())
            {
                if (Cursor == Base)
                {
                    return true;
                }
            }
            return false;
        }

        /// \brief Gives the entity the name it is looked up by among its siblings, and that tools show for it.
        ///
        /// \param Name The name, or empty to take it away.
        /// \return `true` if the entity answers to the name now, `false` when a sibling already does and nothing changed.
        ZY_INLINE Bool SetName(Text Name) const
        {
            if (!IsFree(mStorage->mDirectory[GetIndex()].Parent, Name))
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

        /// \brief Gets the name the entity is looked up by.
        ///
        /// \return The name, or empty when it has none.
        ZY_INLINE Text GetName() const
        {
            const ConstPtr<Named> Found = Get<const Named>();
            return Found ? Text(Found->Value) : Text();
        }

        /// \brief Gets a hash value for the entity based on its identifier.
        ///
        /// \return The identifier, used as its hash.
        ZY_INLINE constexpr UInt64 Hash(UInt64) const
        {
            return mID;
        }

        /// \brief Checks whether two entities name the same generation of the same slot.
        ///
        /// \param Other The entity to compare against.
        /// \return `true` if both name the same entity, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(ConstRef<Entity> Other) const
        {
            return mID == Other.mID;
        }

    public:

        /// \brief Provides the name this type is registered under in the reflection system.
        ///
        /// \return The fully qualified reflection name of the type, and how it is shown.
        ZY_INLINE static constexpr auto OnClassify()
        {
            return ZyReflection::Presentation { .Name = "Scene.Entity", .Flat = true };
        }

        /// \brief Provides the reflected members of this type.
        ///
        /// \return The set of reflected fields.
        ZY_INLINE static constexpr auto OnDescribe()
        {
            return Array(ZyReflection::Field::Property<&Entity::mID>("Id"));
        }

    private:

        /// \brief Writes a component that cannot be written in place.
        ///
        /// \param Identifier The component.
        /// \param Column     The column the entity holds it in, or negative when it holds none.
        /// \param Data       The value.
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

        /// \brief Gets the slot number of the entity.
        ///
        /// \return The slot number.
        ZY_INLINE constexpr UInt32 GetIndex() const
        {
            return static_cast<UInt32>(mID);
        }

        /// \brief Gets the chunk the entity sits in.
        ///
        /// \return The chunk.
        ZY_INLINE Ref<Chunk> GetChunk() const
        {
            ZY_ASSERT(IsAlive(), "The entity is not alive");
            return * mStorage->mDirectory[GetIndex()].Holder;
        }

        /// \brief Gets the directory slot of the entity, which reads answer nothing without.
        ///
        /// \return The slot, or `nullptr` for an entity destroyed or never made.
        ZY_INLINE ConstPtr<Directory::Slot> GetSlot() const
        {
            return mStorage ? mStorage->mDirectory.FindSlot(GetHandle()) : nullptr;
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