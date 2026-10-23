#pragma once

// Precompiled header: frequently used engine headers that rarely change. Every .cpp file of the plugin includes this first.
// Each file still includes the headers it needs itself, so that it doesn't depend on what happens to be in here.

#include <Foundation/Basics.h>

#include <Core/Prefabs/PrefabResource.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Core/World/World.h>
#include <DemonCovePlugin/DemonCovePluginDLL.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
