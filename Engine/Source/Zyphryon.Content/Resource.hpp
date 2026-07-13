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

#include "Cache.hpp"
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Base class for content resources managed by the content service.
    class Resource : public Retainable<Resource>
    {
    public:

        /// \brief Enumerates the possible loading states of a resource.
        enum class Status : UInt8
        {
            Idle,       ///< Resource has not been requested or is inactive.
            Queued,     ///< Resource has been submitted to the I/O pipeline and is awaiting load or parse.
            Completed,  ///< Resource was loaded, parsed, and initialized successfully.
            Failed,     ///< Resource failed to load, parse, or initialize.
        };

        /// \brief Enumerates the cache ownership policies of a resource.
        enum class Policy : UInt8
        {
            Managed,    ///< Lifetime is controlled by the content service; may be pruned automatically.
            Exclusive,  ///< Lifetime is controlled by a specific owner; never pruned automatically.
        };

    public:

        /// \brief Constructs a resource with the given URI key.
        ///
        /// \param Key The URI that identifies this resource.
        ZY_INLINE explicit Resource(AnyRef<Uri> Key)
            : mKey       { Move(Key) },
              mStatus    { Status::Idle },
              mPolicy    { Policy::Managed },
              mFootprint { 0 }
        {
        }

        /// \brief Destructor for proper cleanup in derived classes.
        virtual ~Resource() = default;

        /// \brief Gets the URI key that identifies this resource.
        ///
        /// \return The URI key of this resource.
        ZY_INLINE Uri GetKey() const
        {
            return mKey;
        }

        /// \brief Atomically transitions the resource status if it matches the expected value.
        ///
        /// \param Comparison The expected current status.
        /// \param Status     The new status to set if the comparison matches.
        /// \return `true` if the transition was successful, otherwise `false`.
        ZY_INLINE Bool Transition(Status Comparison, Status Status)
        {
            return mStatus.compare_exchange_strong(Comparison, Status, std::memory_order_acq_rel);
        }

        /// \brief Sets the loading status of the resource.
        ///
        /// \param Status The new status to set.
        ZY_INLINE void SetStatus(Status Status)
        {
            mStatus.store(Status, std::memory_order_release);
        }
        
        /// \brief Gets the current loading status of the resource.
        ///
        /// \return The current status of the resource.
        ZY_INLINE Status GetStatus() const
        {
            return mStatus.load(std::memory_order_acquire);
        }

        /// \brief Checks whether the resource has finished loading (successfully or with failure).
        ///
        /// \return `true` if the resource has completed or failed, otherwise `false`.
        ZY_INLINE Bool HasFinished() const
        {
            const Status Current = mStatus.load(std::memory_order_acquire);
            return (Current == Status::Failed || Current == Status::Completed);
        }

        /// \brief Checks whether the resource failed to load.
        ///
        /// \return `true` if the resource failed, otherwise `false`.
        ZY_INLINE Bool HasFailed() const
        {
            return mStatus.load(std::memory_order_acquire) == Status::Failed;
        }

        /// \brief Checks whether the resource loaded successfully.
        ///
        /// \return `true` if the resource completed successfully, otherwise `false`.
        ZY_INLINE Bool HasCompleted() const
        {
            return mStatus.load(std::memory_order_acquire) == Status::Completed;
        }

        /// \brief Sets the memory footprint of the resource.
        ///
        /// \param Footprint The memory footprint in bytes.
        ZY_INLINE void SetFootprint(UInt64 Footprint)
        {
            mFootprint = Footprint;
        }

        /// \brief Gets the memory footprint of the resource.
        ///
        /// \return The memory footprint in bytes.
        ZY_INLINE UInt64 GetFootprint() const
        {
            return mFootprint;
        }

        /// \brief Sets the cache ownership policy of the resource.
        ///
        /// \param Policy The cache ownership policy to set.
        ZY_INLINE void SetPolicy(Policy Policy)
        {
            mPolicy = Policy;
        }

        /// \brief Gets the cache ownership policy of the resource.
        ///
        /// \return The current cache ownership policy.
        ZY_INLINE Policy GetPolicy() const
        {
            return mPolicy;
        }

    public:

        /// \brief Called when the resource is being loaded.
        ///
        /// \param Host The service host providing access to engine services.
        /// \return `true` if the resource initialized successfully, otherwise `false`.
        virtual Bool OnCreate(Ref<Engine::Subsystem::Host> Host)
        {
            return true;
        }

        /// \brief Called when the resource is being unloaded.
        ///
        /// \param Host The service host providing access to engine services.
        virtual void OnDelete(Ref<Engine::Subsystem::Host> Host)
        {

        }

    private:

        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-
        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-

        Uri            mKey;
        Atomic<Status> mStatus;
        Policy         mPolicy;
        UInt64         mFootprint;
    };

    /// \brief Abstract base class for typed resources with an associated cache.
    ///
    /// \tparam Type The concrete resource type derived from Resource.
    template<typename Type>
    class AbstractResource : public Resource
    {
    public:

        /// \brief Type alias for the cache managing resources of this type.
        using Cache = Cache<Type>;

    public:

        /// \brief Constructs an abstract resource with the given URI key.
        ///
        /// \param Key The URI that identifies this resource.
        ZY_INLINE explicit AbstractResource(AnyRef<Uri> Key)
            : Resource { Move(Key) }
        {
        }

    public:

        /// \brief Gets the singleton cache instance for this resource type.
        ///
        /// \return A reference to the cache managing resources of this type.
        ZY_INLINE static Ref<Cache> GetCache()
        {
            static Cache kUniqueResourceCache;
            return kUniqueResourceCache;
        }
    };
}