#pragma once

#include <Core/Prefabs/PrefabResource.h>
#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <DemonCovePlugin/DemonCovePluginDLL.h>

class DcPlayerComponent;
class ezBlackboard;

using DcArenaComponentManager = ezComponentManagerSimple<class DcArenaComponent, ezComponentUpdateType::WhenSimulating>;

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

/// Runs one round of the game: the enemy spawn timeline, all enemies, daggers and gems, player death and restart.
///
/// Exactly one of these is expected per scene. Enemies, daggers and gems are simulated here as plain data,
/// their game objects only serve as visuals and are positioned by this component every frame.
/// Collision checks are simple sphere tests and don't use the physics engine.
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

  /// Fires a single dagger. Called by the player.
  void SpawnDagger(const ezVec3& vPosition, const ezVec3& vDirection);

  /// Returns the position of the enemy closest to the given position, or false if there are no enemies.
  bool GetClosestEnemyPosition(const ezVec3& vPosition, ezVec3& out_vEnemyPosition) const;

  /// The hand level determines fire rate, shotgun size and homing. Ranges from 1 to 3.
  ezUInt32 GetHandLevel() const;

  bool IsPlayerDead() const { return m_bPlayerDead; }
  ezTime GetRoundTime() const { return m_RoundTime; }

  /// The current stage of the spawn timeline, starting at 1. Can be skipped ahead with the 'DemonCove.StartStage' CVar.
  ezUInt32 GetStage() const;

  /// Kills the player and ends the round. The round time stops.
  void KillPlayer(ezStringView sReason);

  /// Removes all enemies, daggers and gems and resets the player to the start position. The round starts right away.
  void RestartRound();

  /// True before the first round, until the player picks up the dagger floating in the arena.
  bool IsWaitingForStart() const { return m_bWaitingForStart; }

  const ezPrefabResourceHandle& GetDaggerPrefab() const { return m_hDaggerPrefab; }

  /// Shakes the camera of a player that is within fRadius of vPosition, stronger closer to the center.
  void AddCameraShake(const ezVec3& vPosition, float fRadius, float fStrength, ezTime duration);

  /// Plays one of the sound properties below at the given position. Does nothing if the property is empty.
  ///
  /// Each sound plays at most once per 40 ms, so that a burst of hits or kills in one frame doesn't stack up into one very loud sound.
  void PlaySound(const ezString& sSound, const ezVec3& vPosition);

  /// Crossfades between the ambient and the action music, depending on whether a round is running.
  void UpdateMusic();

  ezString m_sSquid1Prefab; // [ property ]
  ezString m_sSquid2Prefab; // [ property ]
  ezString m_sSkull1Prefab; // [ property ]
  ezString m_sSkull2Prefab; // [ property ]
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

  float m_fArenaRadius = 22.0f;    // [ property ]
  float m_fMinArenaRadius = 9.0f;  // [ property ]
  float m_fShrinkSpeed = 0.08f;    // [ property ] meters per second during the endless phase
  float m_fKillHeight = -8.0f;     // [ property ]

  ezString m_sSoundShotgun;        // [ property ]
  ezString m_sSoundDaggerStream;   // [ property ]
  ezString m_sSoundDaggerHit;      // [ property ]
  ezString m_sSoundWeakSpotHit;    // [ property ]
  ezString m_sSoundGemPickup;      // [ property ]
  ezString m_sSoundLevelUp;        // [ property ]
  ezString m_sSoundPlayerDeath;    // [ property ]
  ezString m_sSoundRoundStart;     // [ property ]
  ezString m_sSoundDaggerPickup;   // [ property ]
  ezString m_sSoundSpawnWarning;   // [ property ]
  ezString m_sSoundSquidSpawn;     // [ property ]
  ezString m_sSoundSquidSwarm;     // [ property ]
  ezString m_sSoundSkull2Dash;     // [ property ]
  ezString m_sSoundThiefSteal;     // [ property ]
  ezString m_sSoundSkull1Death;    // [ property ]
  ezString m_sSoundSkull2Death;    // [ property ]
  ezString m_sSoundSkeletonDeath;  // [ property ]
  ezString m_sSoundThiefDeath;     // [ property ]
  ezString m_sSoundSquidDeath;     // [ property ]
  ezString m_sSoundCentipedeDeath; // [ property ]

  ezString m_sMusicAmbient;        // [ property ] loops while no round is running
  ezString m_sMusicAction;         // [ property ] starts from the beginning with every round

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

  /// One floor tile. The floor is built from tiles, so that the arena can crumble at the edge.
  struct Tile
  {
    ezVec3 m_vPosition;
    float m_fDistance = 0.0f; ///< Distance of the tile center to the arena center.
    float m_fFallSpeed = 0.0f;
    bool m_bWarned = false;
    bool m_bFallen = false;
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

  struct SpawnEvent
  {
    EZ_DECLARE_POD_TYPE();

    float m_fTime;
    DcEnemyType m_Type;
    ezUInt8 m_uiCount;
  };

  void Update();
  void UpdateTimeline(float fDeltaTime);
  void UpdateEnemies(float fDeltaTime);
  void UpdateDaggers(float fDeltaTime);
  void UpdateGems(float fDeltaTime);
  void UpdatePlayer();
  void UpdateHud();

  /// Places the dagger that has to be picked up to start the first round. Nothing happens until then.
  void EnterStartState();
  void UpdateStartState(float fDeltaTime);
  void LeaveStartState();

  /// Writes the round state into the 'hud' blackboard, which the RmlUi HUD document displays.
  /// Returns false if there is no HUD canvas in the scene, then the HUD is drawn with debug text instead.
  bool UpdateHudBlackboard();

  /// The best times are stored in the user's app data, but only for rounds without debug settings.
  void LoadScores();
  void SaveScores();
  /// True if none of the debug CVars are active right now.
  static bool IsRegularRound();

  /// Inserts the finished round into the scoreboard if it is good enough, and stores the place it got.
  void AddScore();

  /// Writes the scoreboard into the HUD blackboard. Only needed when it changes.
  void UpdateHudScores();

  /// Creates or removes the invisible wall around the arena, depending on the god mode CVar.
  void UpdateSafetyWall();

  void BuildFloor();
  void ResetFloor();
  void UpdateFloor(float fDeltaTime);
  ezGameObjectHandle CreateTileObject(const Tile& tile, bool bWithCollision);
  bool IsOverFloor(const ezVec3& vPosition) const;

  void SpawnEnemy(DcEnemyType type, const ezVec3& vPosition);
  void SpawnSquid(DcEnemyType type);
  void SpawnSkeletons(ezUInt32 uiCount);
  void SpawnThief();
  void SpawnCentipede(ezUInt32 uiSegments);

  /// Shows the spawn warning effect at the position and spawns the enemy a moment later.
  void QueueSpawn(DcEnemyType type, const ezVec3& vPosition, ezUInt32 uiCount);
  void UpdatePendingSpawns(float fDeltaTime);
  void ClearPendingSpawns();
  void SpawnSkeletonGroup(const ezVec3& vPosition, ezUInt32 uiCount);

  /// A random position on the floor, fInset meters inside the edge, that is at least fMinPlayerDistance away from the player.
  ezVec3 PickFloorSpawnPosition(float fInset, float fMinPlayerDistance) const;
  void SpawnCentipedeAt(const ezVec3& vHeadPosition, ezUInt32 uiSegments);

  /// Slows down the world for a moment, to give big kills more weight.
  void HitPause();
  void UpdateCentipedeHead(Enemy& e, float fDeltaTime, const ezVec3& vTarget);
  void UpdateCentipedeBody(Enemy& e);
  void UpdateThief(Enemy& e, float fDeltaTime, const ezVec3& vTarget);
  void SpawnTimelineEnemies(DcEnemyType type, ezUInt32 uiCount);
  void SpawnGem(const ezVec3& vPosition);
  void KillEnemy(ezUInt32 uiIndex);

  /// Removes the enemy without any reward, e.g. when it hits the player in god mode.
  void RemoveEnemy(ezUInt32 uiIndex);
  void DamageEnemy(ezUInt32 uiIndex, float fDamage, const ezVec3& vHitPosition);

  /// Kills all body segments of the given centipede.
  void KillCentipedeBody(ezUInt32 uiGroup);

  /// Squids can only be damaged at their gems. Returns how many weak spots an enemy type has, 0 means it can be hit anywhere.
  static ezUInt32 GetWeakSpotCount(DcEnemyType type);
  ezVec3 GetWeakSpotPosition(const Enemy& e, ezUInt32 uiSpot) const;

  /// The position daggers should aim at for this enemy, the closest weak spot or the center.
  ezVec3 GetAimPosition(const Enemy& e, const ezVec3& vFrom) const;

  void SetVisualColor(ezGameObjectHandle hObject, const ezColor& color);
  static ezColor GetBaseColor(DcEnemyType type);

  ezGameObjectHandle CreateVisual(const ezPrefabResourceHandle& hPrefab, const ezVec3& vPosition, const ezQuat& qRotation);
  void PreloadPrefab(const ezPrefabResourceHandle& hPrefab);
  void DeleteVisual(ezGameObjectHandle hObject);
  void PlaceVisual(ezGameObjectHandle hObject, const ezVec3& vPosition, const ezQuat& qRotation);

  DcPlayerComponent* GetPlayer() const;
  ezVec3 GetPlayerTarget() const;

  static float GetEnemyRadius(DcEnemyType type);

  ezPrefabResourceHandle m_hPrefabs[(int)DcEnemyType::Count];
  ezPrefabResourceHandle m_hDaggerPrefab;
  ezPrefabResourceHandle m_hGemPrefab;
  ezPrefabResourceHandle m_hDeathEffect;
  ezPrefabResourceHandle m_hHitEffect;
  ezPrefabResourceHandle m_hSpawnWarningEffect;
  ezPrefabResourceHandle m_hGemPickupEffect;
  ezPrefabResourceHandle m_hDaggerTrailEffect;

  /// When each sound was played last, keyed by its property.
  ezHashTable<const ezString*, ezTime> m_LastSoundTime;

  ezDynamicArray<Enemy> m_Enemies;
  ezDynamicArray<Dagger> m_Daggers;
  ezDynamicArray<Gem> m_Gems;
  ezDynamicArray<SpawnEvent> m_Timeline;
  ezDynamicArray<PendingSpawn> m_PendingSpawns;

  ezTime m_HitPauseEnd;      ///< Real time at which the world runs at normal speed again.
  float m_fGemPulse = 0.0f;  ///< The HUD gem counter pulses while this is positive.
  float m_fLevelUpTime = 0.0f; ///< The HUD announces a new hand level while this is positive.
  ezUInt32 m_uiLastHandLevel = 1;
  ezDynamicArray<Tile> m_Tiles;
  ezDynamicArray<ezUInt32> m_TileGrid; ///< Tile index per grid cell, ezInvalidIndex where there is no tile.
  ezInt32 m_iTileGridHalfSize = 0;      ///< The grid spans from -m_iTileGridHalfSize to +m_iTileGridHalfSize cells in X and Y.

  /// The recent positions of each centipede head, newest last. The body segments are placed along it.
  ezMap<ezUInt32, ezDynamicArray<ezVec3>> m_CentipedeTrails;
  ezUInt32 m_uiNextCentipede = 1;
  float m_fCurrentRadius = 22.0f;

  ezUInt32 m_uiNextTimelineEvent = 0;
  float m_fNextLoopSpawn = 0.0f;
  ezUInt32 m_uiLoopWave = 0;

  ezTime m_RoundTime;
  ezTime m_RoundStartTime;
  ezGameObjectHandle m_hSafetyWall;
  ezUInt32 m_uiGems = 0;
  ezUInt32 m_uiKills = 0;
  ezUInt32 m_uiDaggersFired = 0;
  ezUInt32 m_uiDaggersHit = 0;
  bool m_bPlayerDead = false;
  ezString m_sDeathReason;

  struct Score
  {
    float m_fTime = 0.0f;
    ezUInt32 m_uiKills = 0;
    ezUInt32 m_uiGems = 0;
  };

  /// The best rounds, best first. At most s_uiMaxScores entries.
  ezHybridArray<Score, 10> m_Scores;

  /// The place of the last round on the scoreboard, starting at 1. 0 if it didn't make it.
  ezUInt32 m_uiScorePlace = 0;

  /// Set when any debug CVar was active at some point during the round, such a round doesn't go on the scoreboard.
  bool m_bDebugRound = false;

  bool m_bWaitingForStart = false;
  float m_fStartStateTime = 0.0f;
  ezVec3 m_vStartDaggerPosition = ezVec3::MakeZero();
  ezVec3 m_vLastPlayerBody = ezVec3::MakeZero();
  ezGameObjectHandle m_hStartDagger;

  ezSharedPtr<ezBlackboard> m_pHud;
  bool m_bHudBound = false;

  ezComponentHandle m_hPlayer;

  ezComponentHandle m_hMusicAmbient;
  ezComponentHandle m_hMusicAction;
  float m_fMusicAction = 0.0f; ///< 0 = only ambient music, 1 = only action music.
  ezVec3 m_vPlayerStart = ezVec3::MakeZero();
  ezQuat m_qPlayerStartRotation = ezQuat::MakeIdentity();
};
