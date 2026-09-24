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
                (Item.Kind == Source::Axis ? Analog : Digital) += Read(Item, Keyboard, Mouse);
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
            const JsonArray List = Root.GetArray(Entry.Name);

            if (List.IsNull())
            {
                return;
            }

            Entry.Bindings.Clear();

            for (UInt Index = 0; Index < List.GetSize(); ++Index)
            {
                const JsonObject Item = List.GetObject(Index);

                Binding Result;
                Result.Scale = Item.GetNumber<Real32>("Scale", 1.0f);

                Bool Valid = false;

                if (Item.Contains("Key"))
                {
                    Result.Kind = Source::Key;
                    Valid       = Parse(Item, "Key", Result.Positive);
                }
                else if (Item.Contains("Button"))
                {
                    Result.Kind = Source::Button;
                    Valid       = Parse(Item, "Button", Result.Button);
                }
                else if (Item.Contains("Axis"))
                {
                    Result.Kind = Source::Axis;
                    Valid       = Parse(Item, "Axis", Result.Axis);
                }
                else if (Item.Contains("Negative") && Item.Contains("Positive"))
                {
                    Result.Kind = Source::Pair;
                    Valid       = Parse(Item, "Negative", Result.Negative) && Parse(Item, "Positive", Result.Positive);
                }

                if (Valid)
                {
                    Entry.Bindings.Append(Result);
                }
                else
                {
                    LOG_W("Input: Skipped binding {0} of '{1}', which names no input", Index, Entry.Name);
                }
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mapping::Save(JsonObject Root) const
    {
        mActions.ForEach([&](UInt64, ConstRef<Entry> Entry)
        {
            JsonArray List = Root.SetArray(Entry.Name);

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
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Mapping::Entry> Mapping::Define(Text Action)
    {
        const UInt64 Identifier = Identify(Action);

        if (const Ptr<Entry> Existing = mActions.Find(Identifier))
        {
            return (* Existing);
        }

        Entry Created;
        Created.Name = Action;
        mActions.Assign(Identifier, Move(Created));

        return (* mActions.Find(Identifier));
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
}