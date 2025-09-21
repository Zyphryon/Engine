// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Loader.hpp"
#include "Mount.hpp"
#include "Scope.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Central coordinator for content IO, caching, and lifecycle management.
    class Service final : public AbstractService<Service>
    {

        /// \brief The number of worker threads dedicated to IO and parsing.
        constexpr static UInt32 kMaxThreads = 2;

    public:

        /// \brief Constructs the content service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Called once per frame to finalize any resources that completed loading.
        /// 
        /// \param Time The time step for the current frame.
        void OnTick(ConstRef<Time> Time) override;

        /// \brief Shuts down the service and releases its active resources.
        void OnTeardown() override;

        /// \brief Registers a loader for one or more file extensions.
        /// 
        /// \param Loader The loader to register.
        void AddLoader(ConstTracker<Loader> Loader);

        /// \brief Removes the loader associated with a file extension.
        /// 
        /// \param Extension The file extension.
        void RemoveLoader(ConstStr8 Extension);

        /// \brief Registers a mount point for a URI schema.
        /// 
        /// \param Schema The URI schema (e.g., "disk").
        /// \param Mount  The mount to associate with the schema.
        void AddMount(ConstStr8 Schema, ConstTracker<Mount> Mount);

        /// \brief Removes a mount associated with a URI schema.
        /// 
        /// \param Schema The URI schema.
        void RemoveMount(ConstStr8 Schema);

        /// \brief Sets the memory limit for a specific resource type.
        /// 
        /// \tparam Type The resource type.
        /// \param Limit The maximum number of bytes allowed in memory.
        template<typename Type>
        ZYPHRYON_INLINE void SetMemoryLimit(UInt64 Limit)
        {
            Type::GetCache().SetMemoryLimit(Limit);
        }

        /// \brief Gets the memory limit for a specific resource type.
        /// 
        /// \tparam Type The resource type.
        /// \return The maximum number of bytes that can be stored in memory.
        template<typename Type>
        ZYPHRYON_INLINE UInt64 GetMemoryLimit()
        {
            return Type::GetCache().GetMemoryLimit();
        }

        /// \brief Gets the current memory usage for a specific resource type.
        /// 
        /// \tparam Type The resource type.
        /// \return The number of bytes currently used by the resource cache.
        template<typename Type>
        ZYPHRYON_INLINE UInt64 GetMemoryUsage()
        {
            return Type::GetCache().GetMemoryUsage();
        }

        /// \brief Enumerates all file system entries (files and folders) at the specified URI path.
        ///
        /// \param Key The URI of the directory to enumerate. The URI should resolve to a directory path.
        /// \return A vector of \ref Item objects containing metadata for each file and folder.
        Vector<Mount::Item> Enumerate(ConstRef<Uri> Key) const;

        /// \brief Synchronously loads a blob from the appropriate mount.
        /// 
        /// \param Key The URI of the resource.
        /// \return The raw data blob, or an empty blob if the resource cannot be found.
        Blob Find(ConstRef<Uri> Key);

        /// \brief Synchronously loads a blob from the appropriate mount, relative to a scope.
        ///
        /// \param Key   The URI of the resource, which may be relative.
        /// \param Scope The scope providing context for resolution.
        /// \return The raw data blob, or an empty blob if the resource cannot be found.
        ZYPHRYON_INLINE Blob Find(ConstRef<Uri> Key, ConstRef<Scope> Scope)
        {
            return Find(Resolve(Key, &Scope));
        }

        /// \brief Synchronously copies a resource from a source URI to a destination URI.
        ///
        /// \param Source      The URI of the source resource to copy.
        /// \param Destination The URI of the destination where the resource will be copied.
        /// \return `true` if the copy operation succeeded, `false` otherwise.
        Bool Copy(ConstRef<Uri> Source, ConstRef<Uri> Destination);

        /// \brief Synchronously saves a resource to the appropriate mount.
        /// 
        /// \param Key  The URI of the resource.
        /// \param Data The data to write.
        /// \return `true` if the operation succeeded, `false` otherwise.
        Bool Save(ConstRef<Uri> Key, ConstSpan<Byte> Data);

        /// \brief Synchronously saves a string resource to the appropriate mount.
        ///
        /// \param Key  The URI of the resource.
        /// \param Data The string data to write.
        /// \return `true` if the operation succeeded, `false` otherwise.
        ZYPHRYON_INLINE Bool Save(ConstRef<Uri> Key, ConstStr8 Data)
        {
            return Save(Key, ConstSpan<Byte>(reinterpret_cast<ConstPtr<Byte>>(Data.data()), Data.size()));
        }

        /// \brief Synchronously deletes a resource from the appropriate mount.
        /// 
        /// \param Key The URI of the resource.
        /// \return `true` if the operation succeeded, `false` otherwise.
        Bool Delete(ConstRef<Uri> Key);

        /// \brief Requests an asynchronous load of a resource.
        ///
        /// \param Key    The URI of the resource.
        /// \param Parent Optional scope to track this resource as a dependency.
        /// \return A tracker to the requested resource.
        template<typename Type>
        ZYPHRYON_INLINE Tracker<Type> Load(AnyRef<Uri> Key, Ptr<Scope> Parent = nullptr)
        {
            Tracker<Type> Asset = Type::GetCache().GetOrCreate(Resolve(Key, Parent), true);

            if (Asset)
            {
                OnAssetLoad(Asset, Parent);
            }
            return Asset;
        }

        /// \brief Reloads a finished resource in-place, preserving the existing reference.
        /// 
        /// Reloading is only performed if the asset has finished loading successfully.
        ///
        /// \param Asset The resource instance to reload.
        template<typename Type>
        ZYPHRYON_INLINE void Reload(ConstTracker<Type> Asset)
        {
            if (Asset->HasFinished())
            {
                // Prepares the asset for reload by resetting its status to idle.
                Asset->SetStatus(Resource::Status::Idle);

                // Reschedules the resource for loading through the standard pipeline.
                OnAssetLoad(Asset, nullptr);
            }
        }

        /// \brief Unloads a resource and removes it from the cache.
        /// 
        /// \param Asset The resource instance to unload.
        template<typename Type>
        ZYPHRYON_INLINE void Unload(ConstTracker<Type> Asset)
        {
            if (Type::GetCache().Remove(Asset->GetKey()))
            {
                OnAssetDelete(Asset);
            }
        }

        /// \brief Removes unused resources from memory.
        ///
        /// \tparam Type The resource type.
        /// \param Force If `true`, all unused assets are removed regardless of age.
        template<typename Type>
        ZYPHRYON_INLINE void Prune(Bool Force)
        {
            Type::GetCache().Prune(Force, Capture(& Service::OnAssetDelete, this));
        }

    private:

        /// \brief Resolves a potentially context-relative URI into an absolute URI.
        ///
        /// \param Key    The URI that may be relative and require expansion.
        /// \param Parent Optional parent scope providing context.
        /// \return The absolute URI.
        ZYPHRYON_INLINE static constexpr Uri Resolve(ConstRef<Uri> Key, ConstPtr<Scope> Parent)
        {
            return (Parent ? Uri::Expand(Key, Parent->GetResource()->GetKey()) : Key);
        }

        /// \brief Worker thread loop for parsing IO jobs.
        /// 
        /// \param Token A stop token used to signal termination of the worker thread.
        void OnLoaderThread(ConstRef<std::stop_token> Token);

        /// \brief Attempts to parse and prepare a resource from the provided scope.
        /// 
        /// \param Scope The resource scope to parse.
        /// \return `true` if the resource was successfully parsed, `false` otherwise.
        Bool OnLoaderParse(Ref<Scope> Scope);

        /// \brief Enqueues a resource for asynchronous loading.
        /// 
        /// The resource must be in an idle state before it can be queued for loading.
        /// 
        /// \param Asset  The resource to be loaded.
        /// \param Parent An optional parent scope that depends on this resource.
        void OnAssetLoad(ConstTracker<Resource> Asset, Ptr<Scope> Parent);

        /// \brief Finalizes a successfully loaded resource.
        /// 
        /// \param Asset The resource to finalize.
        void OnAssetCreate(Ref<Resource> Asset);

        /// \brief Cleans up a resource upon unloading or pruning.
        /// 
        /// \param Asset The resource to delete.
        void OnAssetDelete(Ref<Resource> Asset);

        /// \brief Registers the engine's default mounts and loaders.
        void RegisterDefaultResources();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        TextTable<Tracker<Loader>> mLoaders;
        TextTable<Tracker<Mount>>  mMounts;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Thread, kMaxThreads> mLoaderThreads;
        Mutex                      mLoaderLatch;
        Vector<Scope>              mLoaderList;
        Condition                  mLoaderCondition;
        Mutex                      mParserLatch;
        Vector<Scope>              mParserList;
    };
}