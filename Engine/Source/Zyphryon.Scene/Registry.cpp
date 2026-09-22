// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Registry.hpp"
#include "Context.hpp"
#include "Entity.hpp"
#include "Salvage.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene::_
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Ref<Table<Digest, Unique<ecs_entity_t>>> GetSlots()
    {
        static Table<Digest, Unique<ecs_entity_t>> Registry;
        return Registry;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<ecs_entity_t> Slot(Text Name)
    {
        Ref<Unique<ecs_entity_t>> Held = GetSlots().FindOrInsert(Digest(Hash(Name)));

        if (!Held)
        {
            Held = Unique<ecs_entity_t>::Create(0);
        }
        return (* Held);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Forget(ecs_entity_t Component)
    {
        GetSlots().ForEach([Component](Digest, Ref<Unique<ecs_entity_t>> Held)
        {
            if (Held && (* Held) == Component)
            {
                (* Held) = 0;
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Preserve(Ptr<ecs_world_t> World, ecs_entity_t Component)
    {
        const ecs_entity_t      Keeper     = Identity<Salvage>::Value;
        const ConstPtr<Factory> Serializer = Context::Get(World).GetFactory(Component);

        if (!Keeper || Component == Keeper || Serializer == nullptr)
        {
            return;
        }

        const Str128 Name = Entity(World, Component).GetPath();
        const Bool   Tag  = (ecs_get_type_info(World, Component) == nullptr);

        Sequence<ecs_entity_t> Holders;

        for (ecs_iter_t Iterator = ecs_each_id(World, Component); ecs_each_next(AddressOf(Iterator)); )
        {
            for (SInt32 Index = 0; Index < Iterator.count; ++Index)
            {
                Holders.Append(Iterator.entities[Index]);
            }
        }

        for (ecs_entity_t Holder : Holders)
        {
            Writer Output;

            if (!Tag)
            {
                Serializer->Write(Output, ecs_get_mut_id(World, Holder, Component));
            }

            const Entity Actor(World, Holder == Component ? Keeper : Holder);

            if (const Ptr<Salvage> Kept = static_cast<Ptr<Salvage>>(Actor.Ensure(Entity(World, Keeper))))
            {
                Kept->Keep(Text(), Name, ConstSpan<Byte>(Output.GetData(), Output.GetSize()));
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Reconcile(Ptr<ecs_world_t> World, ecs_entity_t Component)
    {
        const ecs_entity_t Keeper = Identity<Salvage>::Value;

        // Nothing has been kept while the keeper has no name of its own, and the keeper keeps nothing of itself.
        if (!Keeper || Component == Keeper || Component == 0)
        {
            return;
        }

        const Str128 Name = Entity(World, Component).GetPath();

        if (Name.IsEmpty())
        {
            return;
        }

        const ConstPtr<Factory> Serializer = Context::Get(World).GetFactory(Component);

        // Letting go of the last record takes an entity out of what is being walked, so the holders are
        // gathered before any of them is touched.
        Sequence<ecs_entity_t> Holders;

        for (ecs_iter_t Iterator = ecs_each_id(World, Keeper); ecs_each_next(AddressOf(Iterator)); )
        {
            for (SInt32 Index = 0; Index < Iterator.count; ++Index)
            {
                Holders.Append(Iterator.entities[Index]);
            }
        }

        const Entity Slot(World, Component);

        for (ecs_entity_t Holder : Holders)
        {
            const Entity       Actor(World, Holder);
            const Ptr<Salvage> Kept = Actor.TryGet<Salvage>();

            if (Kept == nullptr)
            {
                continue;
            }

            const Entity Target = (Holder == Keeper ? Slot : Actor);

            for (ConstRef<Salvage::Record> Record : Kept->GetRecords())
            {
                if (Record.Name != Name)
                {
                    continue;
                }

                // A pair whose relation is still nameless waits, since the half that names it may yet arrive.
                const Entity Relation = Record.Relation.IsEmpty()
                    ? Entity()
                    : Entity(World, ecs_lookup_path_w_sep(World, 0, Record.Relation.GetData(), "::", "::", true));

                if (!Record.Relation.IsEmpty() && !Relation.IsValid())
                {
                    continue;
                }

                // A tag stands for itself and holds nothing, so it is put on rather than written into.
                if (Record.Data.IsEmpty())
                {
                    if (Relation.IsValid())
                    {
                        Target.Add(Relation, Slot);
                    }
                    else
                    {
                        Target.Add(Slot);
                    }
                    continue;
                }

                if (Serializer != nullptr)
                {
                    if (const Ptr<void> Memory = Relation.IsValid()
                        ? Target.Ensure(Relation, Slot)
                        : Target.Ensure(Slot))
                    {
                        Reader Source(ConstSpan<Byte>(Record.Data.GetData(), Record.Data.GetSize()));
                        Serializer->Read(Source, Memory);
                    }

                    if (Relation.IsValid())
                    {
                        Target.Notify(Relation, Slot);
                    }
                    else
                    {
                        Target.Notify(Slot);
                    }
                }
            }

            // Whatever still has no name stays kept, so a later arrival can put that one back too.
            Kept->Forget(Name);

            if (Kept->IsEmpty())
            {
                Actor.Remove(Entity(World, Keeper));
            }
        }
    }
}