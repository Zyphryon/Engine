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

#include "Mapping.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyInput
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<IsEnum Type>
    static Bool Parse(JsonObject Item, Text Field, Ref<Type> Output)
    {
        const Text Name = Item.GetString(Field);

        // A misspelled name falls back to the default, so it only counts once it reads back the same.
        Output = ZyEnum::Cast<Type>(Name, Type());
        return !Name.IsEmpty() && StrEqualCaseInsensitive(ZyEnum::GetName(Output), Name);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool Decode(JsonObject Item, Ref<Binding> Output)
    {
        Output.Scale = Item.GetNumber<Real32>("Scale", 1.0f);

        if (Item.Contains("Key"))
        {
            Output.Kind = Source::Key;
            return Parse(Item, "Key", Output.Positive);
        }

        if (Item.Contains("Button"))
        {
            Output.Kind = Source::Button;
            return Parse(Item, "Button", Output.Button);
        }

        if (Item.Contains("Axis"))
        {
            Output.Kind = Source::Axis;
            return Parse(Item, "Axis", Output.Axis);
        }

        if (Item.Contains("Negative") && Item.Contains("Positive"))
        {
            Output.Kind = Source::Pair;
            return Parse(Item, "Negative", Output.Negative) && Parse(Item, "Positive", Output.Positive);
        }

        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Bind(Text Action, ConstRef<Binding> Binding)
    {
        Define(Action).Bindings.Append(Binding);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Unbind(Text Action)
    {
        if (const Ptr<Entry> Entry = mActions.Find(Identify(Action)))
        {
            Entry->Bindings.Clear();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Feed(Text Action, Real32 Value)
    {
        Define(Action).Fed += Value;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Clear()
    {
        mActions.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Update(ConstRef<Keyboard> Keyboard, ConstRef<Mouse> Mouse)
    {
        mActions.ForEach([&](UInt64, Ref<Entry> Entry)
        {
            Real32 Digital = 0.0f;
            Real32 Analog  = Entry.Fed;

            // Two keys on one action read as one key held, while mouse movement keeps its full distance.
            for (ConstRef<Binding> Item : Entry.Bindings)
            {
                if (Item.Kind == Source::Axis)
                {
                    Analog += Read(Item, Keyboard, Mouse);
                }
                else
                {
                    Digital += Read(Item, Keyboard, Mouse);
                }
            }

            Entry.WasDown = Entry.Down;
            Entry.Value   = Clamp(Digital, -1.0f, 1.0f) + Analog;
            Entry.Down    = Entry.Value != 0.0f;
            Entry.Fed     = 0.0f;
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Load(JsonObject Root)
    {
        mActions.ForEach([&](UInt64, Ref<Entry> Entry)
        {
            Restore(Entry, Root.GetArray(Entry.Name));
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Save(JsonObject Root) const
    {
        mActions.ForEach([&](UInt64, ConstRef<Entry> Entry)
        {
            Store(Entry, Root.SetArray(Entry.Name));
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Mapping::Entry> Mapping::Define(Text Action)
    {
        Ref<Entry> Result = mActions.FindOrInsert(Identify(Action));

        if (Result.Name.IsEmpty())
        {
            Result.Name = Action;
        }

        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Mapping::Read(ConstRef<Binding> Binding, ConstRef<Keyboard> Keyboard, ConstRef<Mouse> Mouse)
    {
        switch (Binding.Kind)
        {
        case Source::Key:
            return Keyboard.IsKeyHeld(Binding.Positive) ? Binding.Scale : 0.0f;
        case Source::Button:
            return Mouse.IsButtonHeld(Binding.Button) ? Binding.Scale : 0.0f;
        case Source::Pair:
        {
            const Real32 Positive = Keyboard.IsKeyHeld(Binding.Positive) ? 1.0f : 0.0f;
            const Real32 Negative = Keyboard.IsKeyHeld(Binding.Negative) ? 1.0f : 0.0f;
            return (Positive - Negative) * Binding.Scale;
        }
        case Source::Axis:
            switch (Binding.Axis)
            {
            case Axis::MoveX:
                return Mouse.GetDeltaX() * Binding.Scale;
            case Axis::MoveY:
                return Mouse.GetDeltaY() * Binding.Scale;
            case Axis::ScrollX:
                return Mouse.GetScrollX() * Binding.Scale;
            case Axis::ScrollY:
                return Mouse.GetScrollY() * Binding.Scale;
            }
            break;
        }
        return 0.0f;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Restore(Ref<Entry> Entry, JsonArray List)
    {
        if (List.IsNull())
        {
            return;
        }

        Entry.Bindings.Clear();

        for (UInt Index = 0; Index < List.GetSize(); ++Index)
        {
            if (Binding Result; Decode(List.GetObject(Index), Result))
            {
                Entry.Bindings.Append(Result);
            }
            else
            {
                LOG_W("Input: Skipped binding {0} of '{1}', which names no input", Index, Entry.Name);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Store(ConstRef<Entry> Entry, JsonArray List)
    {
        for (ConstRef<Binding> Item : Entry.Bindings)
        {
            JsonObject Output = List.AddObject();

            switch (Item.Kind)
            {
            case Source::Key:
                Output.SetEnum("Key", Item.Positive);
                break;
            case Source::Button:
                Output.SetEnum("Button", Item.Button);
                break;
            case Source::Pair:
                Output.SetEnum("Negative", Item.Negative);
                Output.SetEnum("Positive", Item.Positive);
                break;
            case Source::Axis:
                Output.SetEnum("Axis", Item.Axis);
                break;
            }

            if (Item.Scale != 1.0f)
            {
                Output.SetNumber("Scale", Item.Scale);
            }
        }
    }
}