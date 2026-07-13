 // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
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
#include "Zyphryon.Engine/Locator.hpp"
#include "Zyphryon.Engine/Subsystem.hpp"
#include "Zyphryon.Job/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Central coordinator for content I/O, caching, and lifecycle management.
    class Service final : public Engine::Subsystem, public Engine::Locator<Job::Service>
    {
    public:

        /// \brief A one-shot delegate invoked when a specific resource finishes loading or fails.
        using Callback = Delegate<void(Ref<Resource>)>;

        /// \brief Fired when a resource has finished loading and is ready for use.
        MulticastDelegate<void(Ref<Resource>)> OnAssetCompleted;

        /// \brief Fired when a resource has failed to load.
        MulticastDelegate<void(Ref<Resource>)> OnAssetFailed;

    public:

        /// \brief Constructs the content service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Called once per frame to finalize any resources that completed loading.
        ///
        /// \param Delta The elapsed time in seconds since the last tick.
        void OnTick(Real64 Delta) override;

        /// \brief Registers a loader for the specified file extensions.
        ///
        /// \param Types  The file extensions this loader handles.
        /// \param Loader The loader instance to associate with the given extensions.
        void AddLoader(ConstSpan<Text> Types, ConstRetainer<Loader> Loader);

        /// \brief Unregisters the loader associated with the specified file extensions.
        ///
        /// \param Types The file extensions to unregister.
        void RemoveLoader(ConstSpan<Text> Types);

        /// \brief Registers a mount point for the given URI schema.
        ///
        /// \param Schema The URI schema to mount.
        /// \param Mount  The mount implementation to handle the schema.
        void AddMount(Text Schema, ConstRetainer<Mount> Mount);

        /// \brief Removes the mount point for the given URI schema.
        ///
        /// \param Schema The URI schema whose mount should be removed.
        void RemoveMount(Text Schema);

        /// \brief Sets the memory limit for the cache of the specified resource type.
        ///
        /// \tparam Type  The resource type whose cache limit to set.
        /// \param  Limit The maximum memory in bytes.
        template<typename Type>
        ZY_INLINE void SetMemoryLimit(UInt64 Limit)
        {
            Type::GetCache().SetMemoryLimit(Limit);
        }

        /// \brief Returns the memory limit for the cache of the specified resource type.
        ///
        /// \tparam Type The resource type whose cache limit to query.
        /// \return The maximum memory in bytes.
        template<typename Type>
        ZY_INLINE UInt64 GetMemoryLimit()
        {
            return Type::GetCache().GetMemoryLimit();
        }

        /// \brief Returns the current memory usage for the cache of the specified resource type.
        ///
        /// \tparam Type The resource type whose cache usage to query.
        /// \return The current memory usage in bytes.
        template<typename Type>
        ZY_INLINE UInt64 GetMemoryUsage()
        {
            return Type::GetCache().GetMemoryUsage();
        }

        /// \brief Enumerates the entries at the specified URI path.
        ///
        /// \param Key      The URI key identifying the mount and path to enumerate.
        /// \param Callback The callback invoked for each entry found; return `true` to continue, `false` to stop.
        void Enumerate(ConstRef<Uri> Key, AnyRef<Mount::OnEnumerate> Callback);

        /// \brief Deletes the file or directory at the specified URI.
        ///
        /// \param Key      The URI key identifying the resource to delete.
        /// \param Callback The callback invoked with the result of the delete operation.
        void Delete(ConstRef<Uri> Key, AnyRef<Mount::OnResult> Callback);

        /// \brief Copies a file from the source URI to the destination URI.
        ///
        /// \param Source      The URI of the file to copy.
        /// \param Destination The URI where the file should be copied to.
        /// \param Callback    The callback invoked with the result of the copy operation.
        void Copy(ConstRef<Uri> Source, ConstRef<Uri> Destination, AnyRef<Mount::OnResult> Callback);

        /// \brief Reads the contents of the file at the specified URI.
        ///
        /// \param Key      The URI key identifying the file to read.
        /// \param Callback The callback invoked with the file's contents if the read succeeds.
        void Read(ConstRef<Uri> Key, AnyRef<Mount::OnRead> Callback);

        /// \brief Writes the specified bytes to a file at the given URI, creating or overwriting it as necessary.
        ///
        /// \param Key      The URI key identifying the file to write.
        /// \param Data     The raw binary data to write to the file.
        /// \param Callback The callback invoked with the result of the write operation.
        void Write(ConstRef<Uri> Key, AnyRef<Blob> Data, AnyRef<Mount::OnResult> Callback);

        /// \brief Loads a resource of the specified type from the given URI.
        ///
        /// \tparam Type   The resource type to load.
        /// \param  Key    The URI of the resource to load.
        /// \param  Parent The parent scope for resolving relative URIs, or nullptr.
        /// \return A handle to the loaded resource, or an empty handle on failure.
        template<typename Type>
        ZY_INLINE Retainer<Type> Load(ConstRef<Uri> Key, Ptr<Scope> Parent = nullptr)
        {
            Retainer<Type> Asset = Type::GetCache().GetOrCreate(Resolve(Key, Parent), true);

            if (Asset)
            {
                OnAssetRequest(Asset, Parent);
            }
            return Asset;
        }

        /// \brief Reloads a previously loaded resource.
        ///
        /// \tparam Type  The resource type to reload.
        /// \param  Asset The resource to reload.
        template<typename Type>
        ZY_INLINE void Reload(ConstRetainer<Type> Asset)
        {
            if (Asset->HasFinished())
            {
                // Prepares the asset for reload by resetting its status to idle.
                Asset->SetStatus(Resource::Status::Idle);

                // Reschedules the resource for loading through the standard pipeline.
                OnAssetRequest(Asset, nullptr);
            }
        }

        /// \brief Unloads a resource, removing it from the cache.
        ///
        /// \tparam Type  The resource type to unload.
        /// \param  Asset The resource to unload.
        template<typename Type>
        ZY_INLINE void Unload(ConstRetainer<Type> Asset)
        {
            if (Type::GetCache().Remove(Asset->GetKey()))
            {
                OnAssetDelete(Asset);
            }
        }

        /// \brief Prunes the cache of the specified resource type.
        ///
        /// \tparam Type   The resource type whose cache to prune.
        /// \param  Force  If `true`, prunes all eligible resources immediately.
        template<typename Type>
        ZY_INLINE void Prune(Bool Force)
        {
            Type::GetCache().Prune(Force, Capture<& Service::OnAssetDelete>(this));
        }

        /// \brief Subscribes a callback to be invoked when a resource at the given URI finishes loading.
        ///
        /// \param Key      The URI to subscribe to.
        /// \param Function The callback to invoke.
        void Subscribe(ConstRef<Uri> Key, AnyRef<Callback> Function);

        /// \brief Unsubscribes any callback associated with the given URI.
        ///
        /// \param Key The URI to unsubscribe from.
        void Unsubscribe(ConstRef<Uri> Key);

        /// \brief Gets the number of asset load operations currently in flight.
        ///
        /// \return The number of assets that have been queued but not yet finalized.
        ZY_INLINE UInt32 GetPending() const
        {
            return mParserPending.load(std::memory_order_acquire);
        }

    private:

        /// \brief Resolves a URI key, expanding it against a parent scope if provided.
        ///
        /// \param Key    The URI key to resolve.
        /// \param Parent The parent scope for relative URI resolution, or nullptr.
        /// \return The resolved URI.
        ZY_INLINE static Uri Resolve(ConstRef<Uri> Key, ConstPtr<Scope> Parent)
        {
            return (Parent ? Uri::Expand(Key, Parent->GetResource()->GetKey()) : Key);
        }

        /// \brief Initializes the loading process for a resource, scheduling it for asynchronous loading.
        ///
        /// \param Asset  The resource to load.
        /// \param Parent The parent scope for dependency tracking, or nullptr.
        void OnAssetRequest(ConstRetainer<Resource> Asset, Ptr<Scope> Parent);

        /// \brief Called when a resource's raw data has been parsed from the mount.
        ///
        /// \param Result The result of the read operation from the mount.
        /// \param Asset  The resource being parsed.
        /// \param Data   The raw binary data read from the mount.
        void OnAssetParse(Filesystem::Result Result, ConstRetainer<Resource> Asset, AnyRef<Blob> Data);

        /// \brief Called when a new resource is created.
        ///
        /// \param Asset The newly created resource.
        void OnAssetCreate(Ref<Resource> Asset);

        /// \brief Called when a resource is being deleted.
        ///
        /// \param Asset The resource being deleted.
        void OnAssetDelete(Ref<Resource> Asset);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<UInt64, Retainer<Loader>> mLoaders;
        Table<UInt64, Retainer<Mount>>  mMounts;
        Table<UInt64, Callback>         mSubscriptions;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Mutex                           mParserMutex;
        Sequence<Scope>                 mParserList;
        Atomic<UInt32>                  mParserPending;
    };
}