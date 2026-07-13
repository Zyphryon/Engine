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

namespace Content
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem  { Host },
          Locator    { Host }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Real64 Delta)
    {
        ZY_PROFILE_SCOPE("Content::Tick");

        Guard Guard(mParserMutex);

        // Iterates through all pending scopes in the loader list.
        UInt32 Completed = 0;

        for (UInt Element = 0; Element < mParserList.GetSize();)
        {
            // If the scope's dependencies are all resolved, finalize the associated resource.
            if (Ref<Scope> Scope = mParserList[Element]; Scope.Poll())
            {
                // Proceed to load the asset.
                OnAssetCreate(* Scope.GetResource());

                // Remove the completed scope from the list via swap-remove for efficiency.
                mParserList.RemoveFast(Element);

                // Increment the count of completed parsers.
                ++Completed;
            }
            else
            {
                ++Element;
            }
        }

        // Update the count of pending parsers.
        mParserPending.fetch_sub(Completed, std::memory_order_release);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::AddLoader(ConstSpan<Text> Types, ConstRetainer<Loader> Loader)
    {
        for (const Text Extension : Types)
        {
            mLoaders.Assign(Hash(Extension), Loader);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RemoveLoader(ConstSpan<Text> Types)
    {
        for (const Text Extension : Types)
        {
            mLoaders.Erase(Hash(Extension));
        }
    }

    // -=-=-=-=-=-=-=-=1-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::AddMount(Text Schema, ConstRetainer<Mount> Mount)
    {
        mMounts.Assign(Hash(Schema), Mount);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RemoveMount(Text Schema)
    {
        mMounts.Erase(Hash(Schema));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Enumerate(ConstRef<Uri> Key, AnyRef<Filesystem::OnEnumerate> Callback)
    {
        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Hash(Key.GetSchema())))
        {
            if (Mount->IsAsynchronous())
            {
                Mount->Enumerate(Key.GetPath(), Move(Callback));
            }
            else
            {
                GetService<Job::Service>().SubmitOnBackground([Key, Mount, Callback = Move(Callback)] mutable
                {
                    Mount->Enumerate(Key.GetPath(), Move(Callback));
                });
            }
        }
        else
        {
            Callback({});
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Delete(ConstRef<Uri> Key, AnyRef<Mount::OnResult> Callback)
    {
        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Hash(Key.GetSchema())))
        {
            if (Mount->IsAsynchronous())
            {
                Mount->Delete(Key.GetPath(), Move(Callback));
            }
            else
            {
                GetService<Job::Service>().SubmitOnBackground([Key, Mount, Callback = Move(Callback)] mutable
                {
                    Mount->Delete(Key.GetPath(), Move(Callback));
                });
            }
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

        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Hash(Source.GetSchema())))
        {
            if (Mount->IsAsynchronous())
            {
                Mount->Copy(Source.GetPath(), Destination.GetPath(), Move(Callback));
            }
            else
            {
                GetService<Job::Service>().SubmitOnBackground([Source, Destination, Mount, Callback = Move(Callback)] mutable
                {
                    Mount->Copy(Source.GetPath(), Destination.GetPath(), Move(Callback));
                });
            }
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
        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Hash(Key.GetSchema())))
        {
            if (Mount->IsAsynchronous())
            {
                Mount->Read(Key.GetPath(), Move(Callback));
            }
            else
            {
                GetService<Job::Service>().SubmitOnBackground([Key, Mount, Callback = Move(Callback)] mutable
                {
                    Mount->Read(Key.GetPath(), Move(Callback));
                });
            }
        }
        else
        {
            Callback(Filesystem::Result::Invalid, Blob());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Write(ConstRef<Uri> Key, ConstSpan<Byte> Data, AnyRef<Mount::OnResult> Callback)
    {
        if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Hash(Key.GetSchema())))
        {
            Mount->Write(Key.GetPath(), Data, Move(Callback));  // TODO: Use Blob for Async?
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
        mSubscriptions.Assign(Hash(Key.GetPath()), Move(Function));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Unsubscribe(ConstRef<Uri> Key)
    {
        mSubscriptions.Erase(Hash(Key.GetPath()));
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
            // Increment the count of pending parser jobs.
            mParserPending.fetch_add(1, std::memory_order_release);

            if (ConstRetainer<Mount> Mount = mMounts.FindOrDefault(Hash(Asset->GetKey().GetSchema())))
            {
                if (Mount->IsAsynchronous())
                {
                    Mount->Read(Asset->GetKey().GetPath(), [this, Asset](Filesystem::Result Result, AnyRef<Blob> Data)
                    {
                        OnAssetParse(Result, Asset, Move(Data));
                    });
                }
                else
                {
                    GetService<Job::Service>().SubmitOnBackground([this, Mount, Asset]
                    {
                        Mount->Read(Asset->GetKey().GetPath(), [this, Asset](Filesystem::Result Result, AnyRef<Blob> Data)
                        {
                            OnAssetParse(Result, Asset, Move(Data));
                        });
                    });
                }
            }
            else
            {
                OnAssetParse(Filesystem::Result::Inexistent, Asset, Blob());
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetParse(Filesystem::Result Result, ConstRetainer<Resource> Asset, AnyRef<Blob> Data)
    {
        Scope Scope(Asset);
        Bool  Successful = false;

        if (Result == Filesystem::Result::Success)
        {
            const Uri Key = Scope.GetResource()->GetKey();

            if (ConstRetainer<Loader> Loader = mLoaders.FindOrDefault(Hash(Key.GetExtension())))
            {
                if (Loader->Load(* this, Scope, Move(Data)))
                {
                    Successful = true;
                }
                else
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
            LOG_W("Failed to read asset '{0}' = '{1}'", Asset->GetKey().GetPath(), Enum::GetName(Result));
        }

        if (!Successful)
        {
            Asset->SetStatus(Resource::Status::Failed);
        }

        Guard Guard(mParserMutex);
        mParserList.Append(Move(Scope));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetCreate(Ref<Resource> Asset)
    {
        Bool Complete = !Asset.HasFailed();

        if (Complete)
        {
            LOG_D("Content: Loading '{0}'", Asset.GetKey().GetUrl());

            // Invoke the resource's creation hook to allow it to realize any system-side state.
            Complete = Asset.OnCreate(GetHost());

            if (Complete)
            {
                Asset.SetStatus(Resource::Status::Completed);
            }
            else
            {
                Asset.SetStatus(Resource::Status::Failed);
            }
        }

        // Notify listeners based on the final status of the asset.
        if (Complete)
        {
            OnAssetCompleted.Broadcast(Asset);
        }
        else
        {
            OnAssetFailed.Broadcast(Asset);
        }

        // Consume the one-shot subscription delegate if one exists for this asset.
        if (Callback Delegate; mSubscriptions.Extract(Hash(Asset.GetKey().GetPath()), Delegate))
        {
            Delegate(Asset);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetDelete(Ref<Resource> Asset)
    {
        LOG_D("Content: Unloading '{0}'", Asset.GetKey().GetUrl());

        // Invoke the resource's deletion hook to release any system-side state acquired during creation.
        Asset.OnDelete(GetHost());

        // Reset the resource's status to reflect that it is no longer loaded.
        Asset.SetStatus(Resource::Status::Idle);
    }
}