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

#include "BHVLoader.hpp"
#include "Zyphryon.AI/Behaviour.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAI
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ReadNode(ConstRef<JsonObject> Definition, UInt32 Depth, Ref<Sequence<Behaviour::Node>> Nodes,
        Ref<Sequence<JsonValue>> Arguments)
    {
        if (!Definition.IsValid() || Depth > 64 || Nodes.GetSize() >= Behaviour::kMaxNodes)
        {
            return false;
        }

        const UInt32 Index = static_cast<UInt32>(Nodes.GetSize());

        Ref<Behaviour::Node> Node = Nodes.Append();
        Node.Type  = Definition.GetEnum("Type", Behaviour::Kind::Leaf);
        Node.Count = Definition.GetNumber<UInt16>("Count");
        Node.Value = Definition.GetNumber<Real64>("Value");

        if (Node.Type == Behaviour::Kind::Leaf)
        {
            Node.Name = Definition.GetString("Name");

            // Arguments are kept as parsed JSON, since only the host knows which fields to read.
            if (const ConstPtr<JsonValue> Declared = Definition.GetValue("Arguments"))
            {
                if (!Declared->IsObject())
                {
                    return false;
                }
                Arguments.Append(* Declared);
                Node.Arguments = static_cast<UInt16>(Arguments.GetSize());
            }
        }
        else if (Node.Type == Behaviour::Kind::Compare)
        {
            Node.Name  = Definition.GetString("Key");
            Node.Check = Definition.GetEnum("Test", Behaviour::Test::Exists);
        }
        else if (Node.Type == Behaviour::Kind::Set || Node.Type == Behaviour::Kind::Clear)
        {
            Node.Name = Definition.GetString("Key");
        }

        const JsonArray List     = Definition.GetArray("Children");
        const UInt32    Children = List.IsNullOrEmpty() ? 0 : static_cast<UInt32>(List.GetSize());

        // A leaf or a blackboard node has no children, a decorator has exactly one, and a composite has at least one.
        switch (Node.Type)
        {
        case Behaviour::Kind::Leaf:
        case Behaviour::Kind::Compare:
        case Behaviour::Kind::Set:
        case Behaviour::Kind::Clear:
            if (Children != 0 || Node.Name.IsEmpty())
            {
                return false;
            }
            break;
        case Behaviour::Kind::Sequence:
        case Behaviour::Kind::Selector:
        case Behaviour::Kind::Parallel:
            if (Children == 0)
            {
                return false;
            }
            break;
        default:
            if (Children != 1)
            {
                return false;
            }
            break;
        }

        for (UInt32 Slot = 0; Slot < Children; ++Slot)
        {
            if (!ReadNode(List.GetObject(Slot), Depth + 1, Nodes, Arguments))
            {
                return false;
            }
        }

        // Indexed again, since appending the children may have reallocated the list.
        Nodes[Index].Children = static_cast<UInt16>(Children);
        Nodes[Index].Span     = static_cast<UInt16>(Nodes.GetSize() - Index);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BHVLoader::Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data)
    {
        const Retainer<Behaviour> Asset = Retainer<Behaviour>::Cast(Scope.GetResource());

        JsonValue Document;
        JsonError Error;

        if (!JsonDocument::Parse(Text(Data.GetData<Char>(), Data.GetSize()), Document, Error))
        {
            LOG_W("'{0}' is not valid JSON: {1}", Scope.GetResource()->GetKey(), Error);
            return false;
        }

        if (!Document.IsObject())
        {
            LOG_W("'{0}' is not a valid behaviour", Scope.GetResource()->GetKey());
            return false;
        }

        const JsonObject Root(Document);

        Sequence<Behaviour::Node> Nodes;
        Sequence<JsonValue>       Arguments;

        if (!ReadNode(Root.GetObject("Root"), 0, Nodes, Arguments))
        {
            LOG_W("'{0}' has an invalid node", Scope.GetResource()->GetKey());
            return false;
        }

        Asset->SetNodes(Move(Nodes), Move(Arguments));
        return true;
    }
}