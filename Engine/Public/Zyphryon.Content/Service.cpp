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
#include "Mount/Embedded.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : AbstractService { Host }
    {
        for (Ref<Thread> Worker : mLoaderThreads)
        {
            Worker = Thread(Capture<& Service::OnLoaderThread>(this));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Time Time)
    {
        ZYPHRYON_PROFILE_SCOPE("Content::Tick");

        Guard Guard(mParserMutex);

        // Iterates through all pending scopes in the loader list.
        UInt32 Completed = 0;

        for (UInt32 Element = 0; Element < mParserList.size();)
        {
            // If the scope's dependencies are all resolved, finalize the associated resource.
            if (Ref<Scope> Scope = mParserList[Element]; Scope.Poll())
            {
                // Proceed to load the asset.
                OnAssetCreate(* Scope.GetResource());

                // Remove the completed scope from the list via swap-remove for efficiency.
                mParserList[Element] = Move(mParserList[mParserList.size() - 1]);
                mParserList.pop_back();

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

    void Service::OnTeardown()
    {
        ZYPHRYON_PROFILE_SCOPE("Content::Teardown");

        for (Ref<Thread> Worker : mLoaderThreads)
        {
            Worker.request_stop();
        }

        mLoaderGate.notify_all();

        for (Ref<Thread> Worker : mLoaderThreads)
        {
            Worker.join();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::AddLoader(ConstTracker<Loader> Loader)
    {
        for (const ConstStr8 Extension : Loader->GetExtensions())
        {
            mLoaders.try_emplace(Hash(Extension), Loader);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RemoveLoader(ConstStr8 Extension)
    {
        mLoaders.erase(Hash(Extension));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::AddMount(ConstStr8 Schema, ConstTracker<Mount> Mount)
    {
        mMounts.try_emplace(Hash(Schema), Mount);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RemoveMount(ConstStr8 Schema)
    {
        mMounts.erase(Hash(Schema));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector<Mount::Item> Service::Enumerate(ConstRef<Uri> Key) const
    {
        if (const auto Iterator = mMounts.find(Hash(Key.GetSchema())); Iterator != mMounts.end())
        {
            return Iterator->second->Enumerate(Key.GetPath());
        }
        return Vector<Mount::Item>();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Service::Find(ConstRef<Uri> Key)
    {
        if (const auto Iterator = mMounts.find(Hash(Key.GetSchema())); Iterator != mMounts.end())
        {
            return Iterator->second->Read(Key.GetPath());
        }
        return Blob();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Copy(ConstRef<Uri> Source, ConstRef<Uri> Destination)
    {
        if (const Blob Data = Find(Source); Data)
        {
            return Save(Destination, Data);
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Save(ConstRef<Uri> Key, ConstSpan<Byte> Data)
    {
        if (const auto Iterator = mMounts.find(Hash(Key.GetSchema())); Iterator != mMounts.end())
        {
            Iterator->second->Write(Key.GetPath(), Data);
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Delete(ConstRef<Uri> Key)
    {
        if (const auto Iterator = mMounts.find(Hash(Key.GetSchema())); Iterator != mMounts.end())
        {
            Iterator->second->Delete(Key.GetPath());
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Subscribe(ConstRef<Uri> Key, AnyRef<AssetDelegate> Delegate)
    {
        mSubscriptions.try_emplace(Hash(Key), Move(Delegate));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Unsubscribe(ConstRef<Uri> Key)
    {
        mSubscriptions.erase(Hash(Key));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnLoaderThread(ConstRef<std::stop_token> Token)
    {
        ZYPHRYON_PROFILE_THREAD("I/O Thread");

        LOG_INFO("Content: I/O Thread initialized.");

        while (!Token.stop_requested())
        {
            ZYPHRYON_PROFILE_SCOPE("Content::I/O");

            // Wait for new work or stop signal
            Scope Scope;

            {
                Lock Lock(mLoaderMutex);

                mLoaderGate.wait(Lock, [&]
                {
                    return !mLoaderList.empty() || Token.stop_requested();
                });

                if (Token.stop_requested() || mLoaderList.empty())
                {
                    continue;
                }

                Scope = Move(mLoaderList.back());
                mLoaderList.pop_back();
            }

            // Do work outside lock
            if (OnLoaderParse(Scope))
            {
                // Push parsed scope into parser list
                {
                    Guard Guard(mParserMutex);
                    mParserList.push_back(Move(Scope));
                }
            }
            else
            {
                // Mark the resource as failed.
                Scope.GetResource()->SetStatus(Resource::Status::Failed);

                // Decrement the count of pending parser jobs.
                mParserPending.fetch_sub(1, std::memory_order_release);
            }
        }

        LOG_INFO("Content: I/O Thread stopped.");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::OnLoaderParse(Ref<Scope> Scope)
    {
        ConstRef<Uri> Key = Scope.GetResource()->GetKey();

        if (const auto Iterator = mLoaders.find(Hash(Key.GetExtension())); Iterator != mLoaders.end())
        {
            ConstTracker<Loader> Loader = Iterator->second;

            if (Blob File = Find(Key); File)
            {
                if (Loader->Load(* this, Scope, Move(File)))
                {
                    return true;
                }
                LOG_WARNING("Content: Failed to parse '{}'", Key.GetUrl());
            }
            else
            {
                LOG_WARNING("Content: '{}' not found.", Key.GetUrl());
            }
        }
        else
        {
            LOG_WARNING("Content: Unknown file format '{}'", Key.GetExtension());
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetLoad(ConstTracker<Resource> Asset, Ptr<Scope> Parent)
    {
        // If a parent Scope is provided, register this resource as a dependency of it.
        if (Parent)
        {
            Parent->Mark(Asset);
        }

        // Atomically transitions the resource's status from Idle to Queued.
        if (Asset->Transition(Resource::Status::Idle, Resource::Status::Queued))
        {

            // Queues the resource.
            {
                Guard Guard(mLoaderMutex);
                mLoaderList.emplace_back(Asset);
            }

            // Increment the count of pending parser jobs.
            mParserPending.fetch_add(1, std::memory_order_release);

            // Wake up one loader thread to process the resource
            mLoaderGate.notify_one();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetCreate(Ref<Resource> Asset)
    {
        LOG_DEBUG("Content: Loading '{}'", Asset.GetKey().GetUrl());

        Asset.Create(GetHost());

        // Notify listeners that the asset has finished loading.
        OnAssetComplete.Broadcast(Asset);

        // Invoke subscription delegate if one exists for this asset.
        if (const auto Iterator = mSubscriptions.find(Hash(Asset.GetKey())); Iterator != mSubscriptions.end())
        {
            Iterator->second(Asset);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetDelete(Ref<Resource> Asset)
    {
        LOG_DEBUG("Content: Unloading '{}'", Asset.GetKey().GetUrl());

        Asset.Delete(GetHost());

        // Invoke subscription delegate if one exists for this asset.
        if (const auto Iterator = mSubscriptions.find(Hash(Asset.GetKey())); Iterator != mSubscriptions.end())
        {
            Iterator->second(Asset);
        }
    }
}