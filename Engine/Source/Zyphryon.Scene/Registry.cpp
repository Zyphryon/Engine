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

namespace Scene::_
{
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

        const ConstPtr<Char> Label = ecs_get_name(World, Component);

        if (Label == nullptr)
        {
            return;
        }

        const ConstPtr<Factory> Serializer = Context::Get(World).GetFactory(Component);

        if (Serializer == nullptr)
        {
            return;
        }

        const Text Name = StrConvert(Label);

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

        for (ecs_entity_t Holder : Holders)
        {
            const Entity       Actor(World, Holder);
            const Ptr<Salvage> Kept = Actor.TryGet<Salvage>();

            if (Kept == nullptr)
            {
                continue;
            }

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

                const Entity Slot(World, Component);

                if (const Ptr<void> Memory = Relation.IsValid() ? Actor.Ensure(Relation, Slot) : Actor.Ensure(Slot))
                {
                    Reader Source(ConstSpan<Byte>(Record.Data.GetData(), Record.Data.GetSize()));
                    Serializer->Read(Source, Memory);
                }

                if (Relation.IsValid())
                {
                    Actor.Notify(Relation, Slot);
                }
                else
                {
                    Actor.Notify(Slot);
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