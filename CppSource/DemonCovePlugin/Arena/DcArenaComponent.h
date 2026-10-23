#pragma once

#include <Core/Prefabs/PrefabResource.h>
#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <DemonCovePlugin/Arena/DcEnemyType.h>
#include <DemonCovePlugin/Arena/DcScoreboard.h>
#include <DemonCovePlugin/Audio/DcAudioComponent.h>
#include <DemonCovePlugin/DemonCovePluginDLL.h>

class DcPlayerComponent;
class DcArenaFloorComponent;

using DcArenaComponentManager = ezComponentManagerSimple<class DcArenaComponent, ezComponentUpdateType::WhenSimulating>;

/// The game rules: runs one round with its enemy spawn timeline, all enemies, daggers and gems, player death and restart.
///
/// Exactly one of these is expected per scene. It works together with two components on the same object:
///   - DcArenaFloorComponent: the tile floor, which this component tells how far to crumble.
///   - DcAudioComponent: all sound effects and music.
/// The player (DcPlayerComponent) and the HUD (DcHudComponent) find this component through FindArena().
///
/// Enemies, daggers and gems are simulated here as plain data in arrays, not as components with their own update.
/// Their game objects (prefab instances) are only visuals and are moved by this component every frame.
/// With hundreds of objects that all need to know about each other (daggers vs. enemies, thieves vs. gems), this is
/// simpler and faster than many small components that look each other up. Collision checks are simple sphere tests
/// and don't use the physics engine. Only the player and the floor use physics.
///
/// The implementation is split by topic:
///   - DcArenaComponent.cpp: setup, round flow, spawn timeline, player death
///   - DcArenaEnemies.cpp: spawning, enemy behaviors, damage
///   - DcArenaDaggersAndGems.cpp: dagger flight and hits, gem pickups
class DcArenaComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(DcArenaComponent, ezComponent, DcArenaComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

protected:
  virtual void OnSimulationStarted() override;
  virtual void OnDeactivated() override;

  //////////////////////////////////////////////////////////////////////////
  // DcArenaComponent

public:
  DcArenaComponent();
  ~DcArenaComponent();

  /// Returns the active arena in the world, if any.
  static DcArenaComponent* FindArena(ezWorld* pWorld);

  /// \name Round state, used by the player and the HUD
  ///@{

  /// True before the first round, until the player picks up the dagger floating in the arena.
  bool IsWaitingForStart() const { return m_bWaitingForStart; }
  bool IsPlayerDead() const { return m_bPlayerDead; }
  ezStringView GetDeathReason() const { return m_sDeathReason; }

  ezTime GetRoundTime() const { return m_RoundTime; }

  /// The current stage of the spawn timeline, starting at 1. Can be skipped ahead with the 'DemonCove.StartStage' CVar.
  ezUInt32 GetStage() const;

  ezUInt32 GetGems() const { return m_uiGems; }
  ezUInt32 GetKills() const { return m_uiKills; }

  /// The hand level determines fire rate, shotgun size and homing. Ranges from 1 to 3 and depends on the collected gems.
  ezUInt32 GetHandLevel() const;

  /// Fraction of the daggers fired this round that hit an enemy, 0 to 1.
  float GetAccuracy() const;

  const DcScoreboard& GetScoreboard() const { return m_Scoreboard; }

  /// The place of the last round on the scoreboard, starting at 1. 0 if it didn't make it.
  ezUInt32 GetScoreboardPlace() const { return m_uiScoreboardPlace; }

  ///@}
  /// \name Actions
  ///@{

  /// Fires a single dagger. Called by the player. Does nothing before the round started or after the player died.
  void SpawnDagger(const ezVec3& vPosition, const ezVec3& vDirection);

  /// Returns the position daggers should aim at for the enemy closest to vPosition, or false if there are no enemies.
  bool GetClosestEnemyPosition(const ezVec3& vPosition, ezVec3& out_vEnemyPosition) const;

  /// Kills the player and ends the round. The round time stops.
  void KillPlayer(ezStringView sReason);

  /// Removes all enemies, daggers and gems and resets the player to the start position. The round starts right away.
  void RestartRound();

  /// Shakes the camera of a player that is within fRadius of vPosition, stronger closer to the center.
  void AddCameraShake(const ezVec3& vPosition, float fRadius, float fStrength, ezTime duration);

  /// Plays a sound through the DcAudioComponent next to this one.
  void PlaySound(DcSound sound, const ezVec3& vPosition);

  ///@}

  const ezPrefabResourceHandle& GetDaggerPrefab() const { return m_hDaggerPrefab; }

  ezString m_sSquid1Prefab;        // [ property ]
  ezString m_sSquid2Prefab;        // [ property ]
  ezString m_sSkull1Prefab;        // [ property ]
  ezString m_sSkull2Prefab;        // [ property ]
  ezString m_sSkeletonPrefab;      // [ property ]
  ezString m_sThiefPrefab;         // [ property ]
  ezString m_sCentipedeHeadPrefab; // [ property ]
  ezString m_sCentipedeBodyPrefab; // [ property ]
  ezString m_sDaggerPrefab;        // [ property ]
  ezString m_sGemPrefab;           // [ property ]
  ezString m_sDeathEffect;         // [ property ]
  ezString m_sHitEffect;           // [ property ]
  ezString m_sSpawnWarningEffect;  // [ property ]
  ezString m_sGemPickupEffect;     // [ property ]
  ezString m_sDaggerTrailEffect;   // [ property ]

  float m_fMinArenaRadius = 9.0f;  // [ property ] the floor stops crumbling at this radius
  float m_fShrinkSpeed = 0.08f;    // [ property ] meters per second during the endless phase
  float m_fKillHeight = -8.0f;     // [ property ] anything below this height is lost

private:
  struct Enemy
  {
    DcEnemyType m_Type = DcEnemyType::Skull1;
    ezVec3 m_vPosition = ezVec3::MakeZero();
    ezVec3 m_vVelocity = ezVec3::MakeZero();
    float m_fHealth = 1.0f;
    ezVec3 m_vGoal = ezVec3::MakeZero(); ///< Behavior specific target position.
    float m_fTimer = 0.0f;               ///< Behavior specific countdown (spawn interval, dash timer).
    float m_fPhase = 0.0f;               ///< Random offset to desynchronize movement noise.
    float m_fYaw = 0.0f;                 ///< Rotation around the up axis, for enemies that don't face their movement direction.
    float m_fHitFlash = 0.0f;            ///< Fades from 1 to 0 after taking damage.
    float m_fFallSpeed = 0.0f;           ///< Ground enemies fall when the floor below them is gone.
    ezUInt32 m_uiGroup = 0;              ///< The centipede this head or body segment belongs to.
    ezUInt16 m_uiSegment = 0;            ///< Index of a centipede body segment, starting at 1 behind the head.
    ezUInt16 m_uiCarriedGems = 0;        ///< Gems collected by a thief.
    ezUInt8 m_uiState = 0;               ///< Behavior specific state.
    bool m_bRemove = false;              ///< Removed without reward after the update.
    ezGameObjectHandle m_hObject;
  };

  struct Dagger
  {
    ezVec3 m_vPosition;
    ezVec3 m_vVelocity;
    float m_fLifeTime = 0.0f;
    bool m_bHoming = false;
    ezGameObjectHandle m_hObject;
  };

  struct Gem
  {
    ezVec3 m_vPosition;
    ezVec3 m_vVelocity;
    float m_fAge = 0.0f;
    ezGameObjectHandle m_hObject;
  };

  /// An enemy that appears once its warning effect has played for a moment.
  struct PendingSpawn
  {
    DcEnemyType m_Type;
    ezUInt32 m_uiCount = 1; ///< Skeletons spawn as a group, centipedes use this as the number of segments.
    ezVec3 m_vPosition;
    float m_fTimeLeft = 0.0f;
    ezGameObjectHandle m_hWarning;
  };

  /// One entry of the scripted part of the spawn timeline.
  struct SpawnEvent
  {
    EZ_DECLARE_POD_TYPE();

    float m_fTime;
    DcEnemyType m_Type;
    ezUInt8 m_uiCount;
  };

  /// Swarms of Skull1 stop growing beyond this, otherwise later stages become a wall of skulls.
  static constexpr ezUInt32 MaxSkull1 = 50;
  static constexpr ezUInt32 CentipedeSegments = 10;

  //////////////////////////////////////////////////////////////////////////
  // DcArenaComponent.cpp

  void Update();
  void UpdateTimeline();

  /// Kills the player on contact with an enemy or when falling off.
  void UpdatePlayerCollision();

  /// Places the dagger that has to be picked up to start the first round. Nothing happens until then.
  void EnterStartState();
  void UpdateStartState(float fDeltaTime);
  void LeaveStartState();

  /// Handles the debug CVars that spawn things.
  void UpdateDebugSpawns();
  void DrawDebugStats();

  /// Slows down the world for a moment, to give big kills more weight.
  void HitPause();

  DcPlayerComponent* GetPlayer() const;

  /// Where enemies aim at: the player's chest, or the arena center if there is no player.
  ezVec3 GetPlayerTarget() const;

  DcArenaFloorComponent* GetFloor() const;
  bool IsOverFloor(const ezVec3& vPosition) const;
  float GetFloorRadius() const;
  float GetCurrentFloorRadius() const;

  //////////////////////////////////////////////////////////////////////////
  // DcArenaEnemies.cpp

  /// Spawns enemies from the timeline. They show a spawn warning first, see QueueSpawn().
  void SpawnTimelineEnemies(DcEnemyType type, ezUInt32 uiCount);
  void SpawnSquid(DcEnemyType type);
  void SpawnSkeletons(ezUInt32 uiCount);
  void SpawnThief();
  void SpawnCentipede(ezUInt32 uiSegments);

  /// Shows the spawn warning effect at the position and spawns the enemy a moment later.
  void QueueSpawn(DcEnemyType type, const ezVec3& vPosition, ezUInt32 uiCount);
  void UpdatePendingSpawns(float fDeltaTime);
  void ClearPendingSpawns();

  /// Spawns an enemy right away. The new enemy is m_Enemies.PeekBack().
  void SpawnEnemy(DcEnemyType type, const ezVec3& vPosition);
  void SpawnSkeletonGroup(const ezVec3& vPosition, ezUInt32 uiCount);
  void SpawnCentipedeAt(const ezVec3& vHeadPosition, ezUInt32 uiSegments);

  /// A random position on the floor, fInset meters inside the edge, that is at least fMinPlayerDistance away from the player.
  ezVec3 PickFloorSpawnPosition(float fInset, float fMinPlayerDistance) const;

  void UpdateEnemies(float fDeltaTime);
  void UpdateSquid(ezUInt32 uiIndex, float fDeltaTime);
  void UpdateSkull1(Enemy& e, float fDeltaTime, const ezVec3& vTarget);
  void UpdateSkull2(Enemy& e, float fDeltaTime, const ezVec3& vTarget);
  void UpdateSkeleton(Enemy& e, float fDeltaTime, const ezVec3& vTarget);
  void UpdateThief(Enemy& e, float fDeltaTime, const ezVec3& vTarget);
  void UpdateCentipedeHead(Enemy& e, float fDeltaTime, const ezVec3& vTarget);
  void UpdateCentipedeBody(Enemy& e);

  /// Applies gravity and floor contact to ground enemies, keeps flying ones above the floor, and moves the visual.
  void FinishEnemyUpdate(Enemy& e, float fDeltaTime, const ezVec3& vTarget);

  void DamageEnemy(ezUInt32 uiIndex, float fDamage, const ezVec3& vHitPosition);

  /// Kills the enemy with all rewards and effects. Note that this moves the last enemy into the freed slot.
  void KillEnemy(ezUInt32 uiIndex);

  /// Removes the enemy without any reward, e.g. when it hits the player in god mode.
  void RemoveEnemy(ezUInt32 uiIndex);
  void RemoveAllEnemies();

  /// Kills all body segments of the given centipede.
  void KillCentipedeBody(ezUInt32 uiGroup);

  ezVec3 GetWeakSpotPosition(const Enemy& e, ezUInt32 uiSpot) const;

  /// The position daggers should aim at for this enemy, the closest weak spot or the center.
  ezVec3 GetAimPosition(const Enemy& e, const ezVec3& vFrom) const;

  //////////////////////////////////////////////////////////////////////////
  // DcArenaDaggersAndGems.cpp

  void UpdateDaggers(float fDeltaTime);

  /// Checks a dagger's path for hits. Returns true if the dagger hit something and is used up.
  bool CheckDaggerHit(const ezVec3& vStart, const ezVec3& vEnd);

  void SpawnGem(const ezVec3& vPosition);
  void UpdateGems(float fDeltaTime);
  void RemoveAllDaggersAndGems();

  //////////////////////////////////////////////////////////////////////////
  // Data

  ezPrefabResourceHandle m_hEnemyPrefabs[(int)DcEnemyType::Count];
  ezPrefabResourceHandle m_hDaggerPrefab;
  ezPrefabResourceHandle m_hGemPrefab;
  ezPrefabResourceHandle m_hDeathEffect;
  ezPrefabResourceHandle m_hHitEffect;
  ezPrefabResourceHandle m_hSpawnWarningEffect;
  ezPrefabResourceHandle m_hGemPickupEffect;
  ezPrefabResourceHandle m_hDaggerTrailEffect;

  ezComponentHandle m_hFloor;
  ezComponentHandle m_hAudio;
  ezComponentHandle m_hPlayer;
  ezVec3 m_vPlayerStart = ezVec3::MakeZero();
  ezQuat m_qPlayerStartRotation = ezQuat::MakeIdentity();

  ezDynamicArray<Enemy> m_Enemies;
  ezDynamicArray<Dagger> m_Daggers;
  ezDynamicArray<Gem> m_Gems;
  ezDynamicArray<PendingSpawn> m_PendingSpawns;

  /// The recent positions of each centipede head, newest last. The body segments are placed along it.
  ezMap<ezUInt32, ezDynamicArray<ezVec3>> m_CentipedeTrails;
  ezUInt32 m_uiNextCentipede = 1;

  ezDynamicArray<SpawnEvent> m_Timeline;
  ezUInt32 m_uiNextTimelineEvent = 0;
  float m_fNextEndlessWave = 0.0f;
  ezUInt32 m_uiEndlessWave = 0;

  ezTime m_RoundTime;
  ezTime m_RoundStartTime;
  ezTime m_HitPauseEnd; ///< Real time at which the world runs at normal speed again.
  ezUInt32 m_uiGems = 0;
  ezUInt32 m_uiKills = 0;
  ezUInt32 m_uiDaggersFired = 0;
  ezUInt32 m_uiDaggersHit = 0;
  ezUInt32 m_uiLastHandLevel = 1;
  bool m_bPlayerDead = false;
  ezString m_sDeathReason;

  DcScoreboard m_Scoreboard;
  ezUInt32 m_uiScoreboardPlace = 0;

  /// Set when any debug CVar was active at some point during the round, such a round doesn't go on the scoreboard.
  bool m_bDebugRound = false;

  bool m_bWaitingForStart = false;
  float m_fStartStateTime = 0.0f;
  ezVec3 m_vStartDaggerPosition = ezVec3::MakeZero();
  ezVec3 m_vLastPlayerBody = ezVec3::MakeZero();
  ezGameObjectHandle m_hStartDagger;
};
