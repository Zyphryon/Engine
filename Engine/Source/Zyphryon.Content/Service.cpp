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

#include "Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyContent
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host },
          Locator   { Host },
          mBacklog  { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Real64 Delta)
    {
        ZY_PROFILE_SCOPE("Content::Tick");

        if (mBacklog.load(std::memory_order_relaxed) == 0)
        {
            return;
        }

        {
            const Guard Lock(mParserMutex);

            mParserList.DrainIf(mParserReady, [](Ref<Scope> Scope)
            {
                return Scope.Poll();
            });
        }

        mBacklog.fetch_sub(static_cast<UInt32>(mParserReady.GetSize()), std::memory_order_relaxed);

        for (ConstRef<Scope> Scope : mParserReady)
        {
            OnAssetCreate(* Scope.GetResource());
        }
        mParserReady.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::AddLoader(ConstSpan<Text> Types, ConstRetainer<Loader> Loader)
    {
        for (const Text Extension : Types)
        {
            mLoaders.Assign(Digest(Hash(Extension)), Loader);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RemoveLoader(ConstSpan<Text> Types)
    {
        for (const Text Extension : Types)
        {
            mLoaders.Erase(Digest(Hash(Extension)));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::AddMount(Text Schema, ConstRetainer<Mount> Mount)
    {
        mMounts.Assign(Digest(Hash(Schema)), Mount);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RemoveMount(Text Schema)
    {
        mMounts.Erase(Digest(Hash(Schema)));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Enumerate(ConstRef<Uri> Key, AnyRef<Mount::OnEnumerate> Callback)
    {
        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Digest(Hash(Key.GetSchema()))))
        {
            Route(Mount, [Key, Mount, Callback = Move(Callback)] mutable
            {
                Mount->Enumerate(Key.GetPath(), Move(Callback));
            });
        }
        else
        {
            Callback(Filesystem::Result::Invalid, {});
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Delete(ConstRef<Uri> Key, AnyRef<Mount::OnResult> Callback)
    {
        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Digest(Hash(Key.GetSchema()))))
        {
            Route(Mount, [Key, Mount, Callback = Move(Callback)] mutable
            {
                Mount->Delete(Key.GetPath(), Move(Callback));
            });
        }
        else
        {
            Callback(Filesystem::Result::Invalid);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Copy(ConstRef<Uri> Source, ConstRef<Uri> Destination, AnyRef<Mount::OnResult> Callback)
    {
        ZY_ASSERT(Source.GetSchema() == Destination.GetSchema(), "Source and destination schemas must match");

        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Digest(Hash(Source.GetSchema()))))
        {
            Route(Mount, [Source, Destination, Mount, Callback = Move(Callback)] mutable
            {
                Mount->Copy(Source.GetPath(), Destination.GetPath(), Move(Callback));
            });
        }
        else
        {
            Callback(Filesystem::Result::Invalid);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Read(ConstRef<Uri> Key, AnyRef<Mount::OnRead> Callback)
    {
        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Digest(Hash(Key.GetSchema()))))
        {
            Route(Mount, [Key, Mount, Callback = Move(Callback)] mutable
            {
                Mount->Read(Key.GetPath(), Move(Callback));
            });
        }
        else
        {
            Callback(Filesystem::Result::Invalid, Blob());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Read(ConstRef<Uri> Key, ZyJob::Lane Lane, AnyRef<Mount::OnRead> Callback)
    {
        mBacklog.fetch_add(1, std::memory_order_relaxed);

        Read(Key, [this, Lane, Callback = Move(Callback)](Filesystem::Result Result, Blob Data) mutable
        {
            Deliver(Lane, Result, Move(Data), Move(Callback));
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Write(ConstRef<Uri> Key, AnyRef<Blob> Data, AnyRef<Mount::OnResult> Callback)
    {
        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Digest(Hash(Key.GetSchema()))))
        {
            Route(Mount, [Key, Mount, Data = Move(Data), Callback = Move(Callback)] mutable
            {
                Mount->Write(Key.GetPath(), Move(Data), Move(Callback));
            });
        }
        else
        {
            Callback(Filesystem::Result::Invalid);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Subscribe(ConstRef<Uri> Key, AnyRef<Callback> Function)
    {
        mSubscriptions.FindOrInsert(Digest(Hash(Key))).Append(Move(Function));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Unsubscribe(ConstRef<Uri> Key)
    {
        mSubscriptions.Erase(Digest(Hash(Key)));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Deliver(
        ZyJob::Lane           Lane,
        Filesystem::Result    Result,
        AnyRef<Blob>          Data,
        AnyRef<Mount::OnRead> Callback)
    {
        GetService<ZyJob::Service>().Dispatch(Lane,
            [this, Result, Data = Move(Data), Callback = Move(Callback)] mutable
        {
            Callback(Result, Move(Data));

            mBacklog.fetch_sub(1, std::memory_order_release);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetRequest(ConstRetainer<Resource> Asset, Ptr<Scope> Parent)
    {
        if (Parent)
        {
            Parent->DependsOn(Asset);
        }

        if (Asset->Transition(Resource::Status::Idle, Resource::Status::Queued))
        {
            mBacklog.fetch_add(1, std::memory_order_relaxed);

            Read(Asset->GetKey(), [this, Asset](Filesystem::Result Result, Blob Data)
            {
                OnAssetParse(Result, Asset, Move(Data));
            });
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetReload(ConstRetainer<Resource> Asset)
    {
        // Cascade first, so the references the resource still holds are refreshed before it drops them.
        Asset->OnReload(GetHost());

        // Release what the previous load created, which also resets the status back to idle.
        OnAssetDelete(* Asset);

        OnAssetRequest(Asset, nullptr);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetParse(Filesystem::Result Result, ConstRetainer<Resource> Asset, AnyRef<Blob> Data)
    {
        Scope Scope(Asset);
        Bool  Successful = false;

        ConstRef<Uri> Key = Asset->GetKey();

        if (Result == Filesystem::Result::Success)
        {
            if (ConstRetainer<Loader> Loader = mLoaders.FindOrDefault(Digest(Hash(Key.GetExtension()))))
            {
                Successful = Loader->Load(* this, Scope, Move(Data));

                if (!Successful)
                {
                    LOG_W("Content: Failed to parse '{0}'", Key.GetFilename());
                }
            }
            else
            {
                LOG_W("Content: Unknown file format '{0}'", Key.GetExtension());
            }
        }
        else
        {
            LOG_W("Content: Failed to read asset '{0}' = '{1}'", Key.GetPath(), ZyEnum::GetName(Result));
        }

        if (!Successful)
        {
            Asset->SetStatus(Resource::Status::Failed);
        }

        const Guard Lock(mParserMutex);
        mParserList.Append(Move(Scope));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetCreate(Ref<Resource> Asset)
    {
        Bool Complete = !Asset.HasFailed();

        if (Complete)
        {
            LOG_D("Content: Loading '{0}'", Asset.GetKey());

            Complete = Asset.OnCreate(GetHost());

            Asset.SetStatus(Complete ? Resource::Status::Completed : Resource::Status::Failed);
        }

        if (Complete)
        {
            OnAssetCompleted.Broadcast(Asset);
        }
        else
        {
            OnAssetFailed.Broadcast(Asset);
        }

        // Subscriptions fire once, so they are taken out before they run.
        if (Sequence<Callback> Listeners; mSubscriptions.Extract(Digest(Hash(Asset.GetKey())), Listeners))
        {
            for (Ref<Callback> Listener : Listeners)
            {
                Listener(Asset);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetDelete(Ref<Resource> Asset)
    {
        LOG_D("Content: Unloading '{0}'", Asset.GetKey());

        Asset.OnDelete(GetHost());

        Asset.SetStatus(Resource::Status::Idle);
    }
}