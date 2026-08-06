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

#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Reason Translate(UInt16 Code)
    {
        switch (Code)
        {
        case 1000:
        case 1001:
        case 1005:
        case 1006:
        case 1011:
            return Reason::Closed;
        case 1002:
        case 1003:
        case 1007:
        case 1009:
            return Reason::Protocol;
        default:
            return Reason::Unreachable;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    struct Proactor::Backend final
    {
        /// \brief One socket as the browser holds it, and everything still outstanding on it.
        struct Session final
        {
            /// The socket the browser opened, which every call about it names.
            EMSCRIPTEN_WEBSOCKET_T Handle = 0;

            /// The connection whatever finishes on it is reported under.
            Connection             Link;

            /// The backend a callback reports into, since the browser threads only one pointer of its own through.
            Ptr<Backend>           Owner    = nullptr;

            /// What the browser has handed over and no read has taken yet.
            Sequence<Byte>         Inbound;

            /// The buffer a read waits to be filled with, which stands only while one waits.
            Span<Byte>             Buffer;

            /// Why the socket ended, kept so a read posted after the fact is answered rather than left waiting.
            Reason                 Cause    = Reason::None;

            /// Whether an attempt to reach the peer is still outstanding.
            Bool                   Reaching = false;

            /// Whether a read is still outstanding.
            Bool                   Reading  = false;

            /// Whether the peer ever answered, which tells a socket that failed to reach from one that was cut.
            Bool                   Opened   = false;
        };

        /// The sockets the browser holds, kept behind a pointer each so a callback's own never moves under it.
        Table<UInt64, Unique<Session>> Arena;

        /// What has finished since the last wait, which the browser adds to whenever it runs the callbacks.
        Sequence<Event>                Ready;

        /// What the last wait found, kept so the span handed out survives until the next one.
        Sequence<Event>                Events;

        /// \brief Writes down that something finished on a socket, for the next wait to hand over.
        ///
        /// \param Entry The socket the operation was posted on.
        /// \param Kind  The operation that finished.
        /// \param Cause Why it failed, or \ref Reason::None when it did not.
        /// \param Size  The number of bytes it moved.
        void Post(ConstRef<Session> Entry, Operation Kind, Reason Cause, UInt32 Size)
        {
            Ref<Event> Report = Ready.Append();
            Report.Link  = Entry.Link;
            Report.Kind  = Kind;
            Report.Cause = Cause;
            Report.Size  = Size;
        }

        /// \brief Fills whatever read waits on a socket from what the browser has already handed over.
        ///
        /// \param Entry The socket to take from.
        void Pump(Ref<Session> Entry)
        {
            if (!Entry.Reading || Entry.Inbound.IsEmpty())
            {
                return;
            }

            // Taken a read's worth at a time, since the browser hands over whole messages and one of them may be
            // wider than the buffer offered for it, leaving the rest for the read that follows.
            const UInt Size = Min(Entry.Buffer.GetSize(), Entry.Inbound.GetSize());

            Blit(Entry.Buffer.GetData(), Size, Entry.Inbound.GetData());
            Entry.Inbound.Remove(0, Size);

            Entry.Reading = false;
            Post(Entry, Operation::Receive, Reason::None, static_cast<UInt32>(Size));
        }

        /// \brief Gives back everything a socket still had outstanding, which one that ends has to before it goes.
        ///
        /// \param Entry The socket that ended.
        /// \param Cause The reason it ended.
        void Settle(Ref<Session> Entry, Reason Cause)
        {
            Entry.Cause = Cause;

            if (Entry.Reaching)
            {
                Entry.Reaching = false;
                Post(Entry, Operation::Connect, Cause, 0);
            }

            if (Entry.Reading)
            {
                Entry.Reading = false;
                Post(Entry, Operation::Receive, Cause, 0);
            }
        }

        /// \brief Finds the socket a handle names.
        ///
        /// \param Handle The socket to look for.
        /// \return The socket, or `nullptr` when none was ever taken on under that handle.
        Ptr<Session> Find(ConstRef<Socket> Handle)
        {
            const Ptr<Unique<Session>> Slot = Arena.Find(Handle.GetHandle());
            return Slot ? Slot->Grab() : nullptr;
        }

        /// \brief Acts on the browser saying that a socket has reached its peer.
        ///
        /// \param Kind    The event the browser named, which is the one this was registered for.
        /// \param Event   What the browser had to say about it.
        /// \param Context The socket it happened on.
        /// \return `true` always, since the browser only asks so a page may swallow the event.
        static EM_BOOL OnOpen(SInt32 Kind, ConstPtr<EmscriptenWebSocketOpenEvent> Event, Ptr<void> Context)
        {
            Ref<Session> Entry = * static_cast<Ptr<Session>>(Context);
            Entry.Opened = true;

            if (Entry.Reaching)
            {
                Entry.Reaching = false;
                Entry.Owner->Post(Entry, Operation::Connect, Reason::None, 0);
            }
            return EM_TRUE;
        }

        /// \brief Acts on the browser handing over a message a peer sent.
        ///
        /// \param Kind    The event the browser named, which is the one this was registered for.
        /// \param Event   What the browser had to say about it.
        /// \param Context The socket it arrived on.
        /// \return `true` always, since the browser only asks so a page may swallow the event.
        static EM_BOOL OnMessage(SInt32 Kind, ConstPtr<EmscriptenWebSocketMessageEvent> Event, Ptr<void> Context)
        {
            Ref<Session> Entry = * static_cast<Ptr<Session>>(Context);

            if (Event->isText)
            {
                LOG_E("Network: a peer sent text where this build frames everything it says in bytes");
                return EM_TRUE;
            }

            // Kept rather than handed straight over, since the browser decides when a message arrives and the
            // endpoint decides when it offers somewhere to put one, and the two rarely line up.
            Entry.Inbound.Append(ConstSpan(Event->data, Event->numBytes));

            Entry.Owner->Pump(Entry);
            return EM_TRUE;
        }

        /// \brief Acts on the browser saying that a socket went wrong.
        ///
        /// \param Kind    The event the browser named, which is the one this was registered for.
        /// \param Event   What the browser had to say about it.
        /// \param Context The socket it happened on.
        /// \return `true` always, since the browser only asks so a page may swallow the event.
        static EM_BOOL OnError(SInt32 Kind, ConstPtr<EmscriptenWebSocketErrorEvent> Event, Ptr<void> Context)
        {
            return EM_TRUE;
        }

        /// \brief Acts on the browser saying that a socket has ended.
        ///
        /// \param Kind    The event the browser named, which is the one this was registered for.
        /// \param Event   What the browser had to say about it.
        /// \param Context The socket that ended.
        /// \return `true` always, since the browser only asks so a page may swallow the event.
        static EM_BOOL OnClose(SInt32 Kind, ConstPtr<EmscriptenWebSocketCloseEvent> Event, Ptr<void> Context)
        {
            Ref<Session> Entry = * static_cast<Ptr<Session>>(Context);

            Entry.Owner->Settle(Entry, Entry.Opened ? Translate(Event->code) : Reason::Unreachable);
            return EM_TRUE;
        }
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Proactor::Proactor()
        : mBackend { Unique<Backend>::Create() }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Proactor::~Proactor()
    {
        Stop();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Start()
    {
        mBackend->Ready.Reserve(kMaxHarvest);
        mBackend->Events.Reserve(kMaxHarvest);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Proactor::Stop()
    {
        ZY_ASSERT(mBackend->Arena.IsEmpty(), "The completion queue is being closed while sockets are still on it");

        mBackend->Arena.Clear();
        mBackend->Ready.Clear();
        mBackend->Events.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Attach(ConstRef<Socket> Handle)
    {
        if (!Handle.IsValid())
        {
            LOG_E("Network: a socket that was never opened cannot be put under the completion queue");
            return false;
        }

        Ref<Unique<Backend::Session>> Slot = mBackend->Arena.FindOrInsert(Handle.GetHandle());
        Slot = Unique<Backend::Session>::Create();

        const Ptr<Backend::Session> Entry = Slot.Grab();
        Entry->Handle = static_cast<EMSCRIPTEN_WEBSOCKET_T>(Handle.GetHandle());
        Entry->Owner  = mBackend.Grab();

        emscripten_websocket_set_onopen_callback(Entry->Handle, Entry, Backend::OnOpen);
        emscripten_websocket_set_onmessage_callback(Entry->Handle, Entry, Backend::OnMessage);
        emscripten_websocket_set_onerror_callback(Entry->Handle, Entry, Backend::OnError);
        emscripten_websocket_set_onclose_callback(Entry->Handle, Entry, Backend::OnClose);

        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Proactor::Cancel(ConstRef<Socket> Handle)
    {
        if (const Ptr<Backend::Session> Entry = mBackend->Find(Handle))
        {
            emscripten_websocket_set_onopen_callback(Entry->Handle, nullptr, nullptr);
            emscripten_websocket_set_onmessage_callback(Entry->Handle, nullptr, nullptr);
            emscripten_websocket_set_onerror_callback(Entry->Handle, nullptr, nullptr);
            emscripten_websocket_set_onclose_callback(Entry->Handle, nullptr, nullptr);

            emscripten_websocket_close(Entry->Handle, 1000, nullptr);

            mBackend->Settle(* Entry, Reason::Closed);
            mBackend->Arena.Erase(Handle.GetHandle());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Accept(ConstRef<Socket> Handle, Connection Link, ConstRef<Endpoint> Family)
    {
        LOG_E("Network: the browser accepts no peers, so nothing can be posted to take one on");
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Connect(ConstRef<Socket> Handle, Connection Link, ConstRef<Endpoint> Address)
    {
        if (const Ptr<Backend::Session> Entry = mBackend->Find(Handle); Entry != nullptr)
        {
            Entry->Link     = Link;
            Entry->Reaching = true;
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Receive(ConstRef<Socket> Handle, Connection Link, Span<Byte> Buffer)
    {
        if (const Ptr<Backend::Session> Entry = mBackend->Find(Handle); Entry != nullptr)
        {
            Entry->Link    = Link;
            Entry->Buffer  = Buffer;
            Entry->Reading = true;

            if (Entry->Cause == Reason::None)
            {
                mBackend->Pump(* Entry);
            }
            else
            {
                mBackend->Settle(* Entry, Entry->Cause);
            }
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::ReceiveFrom(ConstRef<Socket> Handle, Connection Link, Span<Byte> Buffer)
    {
        LOG_E("Network: the browser carries no datagrams, so nothing can be read from one address at a time");
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Send(ConstRef<Socket> Handle, Connection Link, ConstSpan<Byte> Buffer)
    {
        if (const Ptr<Backend::Session> Entry = mBackend->Find(Handle); Entry != nullptr)
        {
            Entry->Link = Link;

            const EMSCRIPTEN_RESULT Result = emscripten_websocket_send_binary(
                Entry->Handle,
                const_cast<Ptr<Byte>>(Buffer.GetData()),
                static_cast<UInt32>(Buffer.GetSize()));

            if (Result != EMSCRIPTEN_RESULT_SUCCESS)
            {
                LOG_E("Network: a write could not be posted ({0})", Result);
                return false;
            }

            mBackend->Post(* Entry, Operation::Send, Reason::None, static_cast<UInt32>(Buffer.GetSize()));
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Proactor::Send(ConstRef<Socket> Handle, Connection Link, ConstSpan<Byte> Buffer, ConstRef<Endpoint> Address)
    {
        LOG_E("Network: the browser carries no datagrams, so nothing can be written to one address at a time");
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Proactor::Rouse()
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstSpan<Proactor::Event> Proactor::Wait(UInt32 Timeout)
    {
        mBackend->Events.Clear();
        Base::Swap(mBackend->Events, mBackend->Ready);

        return mBackend->Events;
    }
}