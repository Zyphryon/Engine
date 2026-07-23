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

#include "Zyphryon.Base/Compression/LZ4.hpp"
#include "Zyphryon.Base/Container/Adaptor.hpp"
#include "Zyphryon.Base/Container/Array.hpp"
#include "Zyphryon.Base/Container/Bag.hpp"
#include "Zyphryon.Base/Container/Bitset.hpp"
#include "Zyphryon.Base/Container/Freelist.hpp"
#include "Zyphryon.Base/Container/Pool.hpp"
#include "Zyphryon.Base/Container/Sequence.hpp"
#include "Zyphryon.Base/Container/Span.hpp"
#include "Zyphryon.Base/Container/Table.hpp"
#include "Zyphryon.Base/Container/Variant.hpp"
#include "Zyphryon.Base/Functional/Delegate.hpp"
#include "Zyphryon.Base/Functional/Journal.hpp"
#include "Zyphryon.Base/Lexical/Format/Processor.hpp"
#include "Zyphryon.Base/Lexical/Algorithm.hpp"
#include "Zyphryon.Base/Lexical/Symbol.hpp"
#include "Zyphryon.Base/Marshal/Binary/Archive.hpp"
#include "Zyphryon.Base/Marshal/Json/JsonArray.hpp"
#include "Zyphryon.Base/Marshal/Json/JsonDocument.hpp"
#include "Zyphryon.Base/Marshal/Json/JsonObject.hpp"
#include "Zyphryon.Base/Memory/Blob.hpp"
#include "Zyphryon.Base/Memory/Retainable.hpp"
#include "Zyphryon.Base/Memory/Retainer.hpp"
#include "Zyphryon.Base/Memory/Unique.hpp"
#include "Zyphryon.Base/Pattern/Enumerator.hpp"
#include "Zyphryon.Base/Pattern/Switchable.hpp"
#include "Zyphryon.Base/Platform/Filesystem/Filesystem.hpp"
#include "Zyphryon.Base/Platform/Telemetry/Log.hpp"
#include "Zyphryon.Base/Platform/Telemetry/Profile.hpp"
#include "Zyphryon.Base/Thread/Atomic.hpp"
#include "Zyphryon.Base/Thread/Gate.hpp"
#include "Zyphryon.Base/Thread/Mutex.hpp"
#include "Zyphryon.Base/Thread/Ring.hpp"
#include "Zyphryon.Base/Thread/Thread.hpp"