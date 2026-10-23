#pragma once

#include <DemonCovePlugin/Audio/DcAudioComponent.h>
#include <Foundation/Math/Color.h>

enum class DcEnemyType : ezUInt8
{
  Squid1,        ///< Slow spawner, releases a swarm of Skull1 periodically. Only takes damage at its gem.
  Squid2,        ///< Tougher spawner with two gems, additionally releases a Skull2 with every swarm.
  Skull1,        ///< Weak swarm enemy, homes in on the player with limited turn rate.
  Skull2,        ///< Circles the player and periodically dashes at them.
  Skeleton,      ///< Runs along the ground towards the player. Spawns in groups at the arena edge.
  Thief,         ///< Crawls along the floor and collects gems lying around. Drops them again when killed.
  CentipedeHead, ///< Flies through the arena, its body segments follow its path.
  CentipedeBody, ///< One segment of a centipede. Dies together with the head.
  Count
};

/// The fixed properties of an enemy type. Everything about an enemy type that isn't behavior lives in this table.
struct DcEnemyTypeInfo
{
  const char* m_szName;   ///< For the death reason and debug output.
  float m_fHealth;        ///< One dagger does 1 damage.
  float m_fRadius;        ///< Collision sphere for daggers and the player.
  ezUInt32 m_uiGemDrop;   ///< Gems dropped on death. A thief additionally drops the gems it collected.
  ezUInt32 m_uiWeakSpots; ///< Squids can only be damaged at their gems. 0 means the enemy can be hit anywhere.
  bool m_bWalksOnGround;  ///< Stays on the floor tiles and falls when there is none, instead of flying.
  bool m_bBigKill;        ///< Killing it shakes the camera and briefly slows down time.
  DcSound m_DeathSound;   ///< Centipede body segments die with the head, its sound covers them.
  ezColor m_Tint;         ///< Multiplied onto the prefab's meshes.
};

/// Returns the properties of the given enemy type.
const DcEnemyTypeInfo& DcGetEnemyTypeInfo(DcEnemyType type);
