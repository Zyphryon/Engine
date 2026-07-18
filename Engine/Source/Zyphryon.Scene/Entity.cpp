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

#include "Entity.hpp"
#include "Codec.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Entity::Load(Ref<Reader> Archive) const
    {
        // Read the entity's name.
        if (const Str Name = Archive.ReadText(); !Name.IsEmpty())
        {
            SetName(Name);
        }

        // Read the entity's alias.
        if (const Str Alias = Archive.ReadText(); !Alias.IsEmpty())
        {
            SetAlias(Alias);
        }

        // Read the entity's archetype.
        if (const UInt64 Archetype = Archive.Read<UInt64>(); Archetype)
        {
            // The link is stored as the full flecs id (generation included).
            // A slot that was freed and reacquired carries a bumped generation, so a stale link fails `IsAlive`
            // and is dropped here rather than silently resolving to whatever now occupies the same id.
            if (const Entity Prefab(mHandle.world().entity(Archetype)); Prefab.IsAlive() && Prefab.IsArchetype())
            {
                SetArchetype(Prefab);
            }
            else
            {
                LOG_W("Entity references a missing or stale archetype '{0}', dropping the link", Archetype);
            }
        }

        // Read the entity's components.
        Codec::ReadComponentsOf(mHandle.world(), Archive, * this);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Entity::Save(Ref<Writer> Archive) const
    {
        // Write the entity's name.
        Archive.WriteText(GetName());

        // Write the entity's alias.
        Archive.WriteText(GetAlias());

        // Write the entity's archetype (or '0' if not valid).
        const Entity Archetype = GetArchetype();
        Archive.Write<UInt64>(Archetype.IsValid() ? Archetype.GetID() : 0);

        // Write the entity's components.
        Codec::WriteComponentsOf(Archive, * this);
    }
}