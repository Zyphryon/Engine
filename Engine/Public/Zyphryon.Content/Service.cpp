    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Service.hpp"
#include "Mount/EmbeddedMount.hpp"

#ifdef    ZYPHRYON_LOADER_MP3
    #include "Zyphryon.Content/Sound/MP3/Loader.hpp"
#endif // ZYPHRYON_LOADER_MP3
#ifdef    ZYPHRYON_LOADER_WAV
    #include "Zyphryon.Content/Sound/WAV/Loader.hpp"
#endif // ZYPHRYON_LOADER_WAV
#ifdef    ZYPHRYON_LOADER_STB
    #include "Zyphryon.Content/Texture/STB/Loader.hpp"
#endif // ZYPHRYON_LOADER_STB
#ifdef    ZYPHRYON_LOADER_MATERIAL
    #include "Zyphryon.Content/Material/Loader.hpp"
#endif // ZYPHRYON_LOADER_MATERIAL
#ifdef    ZYPHRYON_LOADER_GLTF
    #include "Zyphryon.Content/Model/GLTF/Loader.hpp"
#endif // ZYPHRYON_LOADER_GLTF
#ifdef    ZYPHRYON_LOADER_EFFECT
    #include "Zyphryon.Graphic/Service.hpp"
    #include "Zyphryon.Content/Pipeline/Loader.hpp"
#endif // ZYPHRYON_LOADER_EFFECT
#ifdef    ZYPHRYON_LOADER_ARTERY
    #include "Zyphryon.Content/Font/Artery/Loader.hpp"
#endif // ZYPHRYON_LOADER_ARTERY

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

        RegisterDefaultResources();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(ConstRef<Time> Time)
    {
        ZYPHRYON_PROFILE_SCOPE("Content::Tick");

        Lock<> Guard(mParserLatch);

        // Iterates through all pending scopes in the loader list.
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
            }
            else
            {
                ++Element;
            }
        }
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

        {
            Lock<true> Guard(mLoaderLatch);
            mLoaderCondition.notify_all();
        }

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
            mLoaders.try_emplace(Extension, Loader);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RemoveLoader(ConstStr8 Extension)
    {
        mLoaders.erase(Extension);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::AddMount(ConstStr8 Schema, ConstTracker<Mount> Mount)
    {
        mMounts.try_emplace(Schema, Mount);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RemoveMount(ConstStr8 Schema)
    {
        mMounts.erase(Schema);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector<Mount::Item> Service::Enumerate(ConstRef<Uri> Key) const
    {
        if (const auto Iterator = mMounts.find(Key.GetSchema()); Iterator != mMounts.end())
        {
            return Iterator->second->Enumerate(Key.GetPath());
        }
        return Vector<Mount::Item>();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Service::Find(ConstRef<Uri> Key)
    {
        if (const auto Iterator = mMounts.find(Key.GetSchema()); Iterator != mMounts.end())
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
        if (const auto Iterator = mMounts.find(Key.GetSchema()); Iterator != mMounts.end())
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
        if (const auto Iterator = mMounts.find(Key.GetSchema()); Iterator != mMounts.end())
        {
            Iterator->second->Delete(Key.GetPath());
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnLoaderThread(ConstRef<std::stop_token> Token)
    {
        ZYPHRYON_THREAD_NAME("I/O Thread");

        LOG_INFO("Content: I/O Thread initialized.");

        while (!Token.stop_requested())
        {
            ZYPHRYON_PROFILE_SCOPE("Content::I/O");

            // Wait for new work or stop signal
            Scope Scope;

            {
                Lock<true> Guard(mLoaderLatch);

                mLoaderCondition.wait(Guard, [&]
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
                    Lock<> Guard(mParserLatch);
                    mParserList.push_back(Move(Scope));
                }
            }
            else
            {
                Scope.GetResource()->SetStatus(Resource::Status::Failed);
            }
        }

        LOG_INFO("Content: I/O Thread stopped.");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::OnLoaderParse(Ref<Scope> Scope)
    {
        ConstRef<Uri> Key = Scope.GetResource()->GetKey();

        if (const auto Iterator = mLoaders.find(Key.GetExtension()); Iterator != mLoaders.end())
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
                Lock<> Guard(mLoaderLatch);
                mLoaderList.emplace_back(Asset);
            }

            // Wake up one loader thread to process the resource
            mLoaderCondition.notify_one();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetCreate(Ref<Resource> Asset)
    {
        LOG_DEBUG("Content: Loading '{}'", Asset.GetKey().GetUrl());

        Asset.Create(GetHost());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnAssetDelete(Ref<Resource> Asset)
    {
        LOG_DEBUG("Content: Unloading '{}'", Asset.GetKey().GetUrl());

        Asset.Delete(GetHost());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RegisterDefaultResources()
    {
        AddMount("Engine", Tracker<EmbeddedMount>::Create());

        if (IsClientMode())
        {
#ifdef    ZYPHRYON_LOADER_MP3
            AddLoader(Tracker<MP3Loader>::Create());
#endif // ZYPHRYON_LOADER_MP3

#ifdef    ZYPHRYON_LOADER_WAV
            AddLoader(Tracker<WAVLoader>::Create());
#endif // ZYPHRYON_LOADER_WAV

#ifdef    ZYPHRYON_LOADER_STB
            AddLoader(Tracker<STBLoader>::Create());
#endif // ZYPHRYON_LOADER_STB

#ifdef    ZYPHRYON_LOADER_MATERIAL
            AddLoader(Tracker<MaterialLoader>::Create());
#endif // ZYPHRYON_LOADER_MATERIAL

#ifdef    ZYPHRYON_LOADER_GLTF
            AddLoader(Tracker<GLTFLoader>::Create());
#endif // ZYPHRYON_LOADER_GLTF

#ifdef    ZYPHRYON_LOADER_EFFECT
            Graphic::Capabilities Capabilities = GetService<Graphic::Service>()->GetCapabilities();
            AddLoader(Tracker<PipelineLoader>::Create(Capabilities.Backend, Capabilities.Language));
#endif // ZYPHRYON_LOADER_EFFECT
        }

#ifdef    ZYPHRYON_LOADER_ARTERY
        AddLoader(Tracker<ArteryFontLoader>::Create());
#endif // ZYPHRYON_LOADER_ARTERY
    }
}