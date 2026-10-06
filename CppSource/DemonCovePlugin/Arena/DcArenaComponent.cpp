#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/Interfaces/SoundInterface.h>
#include <Core/Utils/Blackboard.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <DemonCovePlugin/Arena/DcArenaComponent.h>
#include <DemonCovePlugin/Player/DcPlayerComponent.h>
#include <GameComponentsPlugin/Effects/Shake/CameraShakeVolumeComponent.h>
#include <GameEngine/Gameplay/GreyBoxComponent.h>
#include <JoltPlugin/Actors/JoltStaticActorComponent.h>
#include <JoltPlugin/Shapes/JoltShapeBoxComponent.h>
#include <MiniAudioPlugin/Components/MiniAudioSoundComponent.h>
#include <MiniAudioPlugin/Resources/MiniAudioSoundResource.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/MeshComponent.h>
#include <RendererCore/Meshes/MeshComponentBase.h>
#include <RmlUiPlugin/Components/RmlUiCanvas2DComponent.h>
#include <RendererCore/Meshes/MeshResource.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(DcArenaComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Squid1Prefab", m_sSquid1Prefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("Squid2Prefab", m_sSquid2Prefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("Skull1Prefab", m_sSkull1Prefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("Skull2Prefab", m_sSkull2Prefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SkeletonPrefab", m_sSkeletonPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("ThiefPrefab", m_sThiefPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("CentipedeHeadPrefab", m_sCentipedeHeadPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("CentipedeBodyPrefab", m_sCentipedeBodyPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("DaggerPrefab", m_sDaggerPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("GemPrefab", m_sGemPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("DeathEffect", m_sDeathEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("HitEffect", m_sHitEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SpawnWarningEffect", m_sSpawnWarningEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("GemPickupEffect", m_sGemPickupEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("DaggerTrailEffect", m_sDaggerTrailEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("ArenaRadius", m_fArenaRadius)->AddAttributes(new ezDefaultValueAttribute(22.0f), new ezClampValueAttribute(5.0f, 200.0f)),
    EZ_MEMBER_PROPERTY("MinArenaRadius", m_fMinArenaRadius)->AddAttributes(new ezDefaultValueAttribute(9.0f), new ezClampValueAttribute(2.0f, 200.0f)),
    EZ_MEMBER_PROPERTY("ShrinkSpeed", m_fShrinkSpeed)->AddAttributes(new ezDefaultValueAttribute(0.08f), new ezClampValueAttribute(0.0f, 10.0f)),
    EZ_MEMBER_PROPERTY("KillHeight", m_fKillHeight)->AddAttributes(new ezDefaultValueAttribute(-8.0f)),
    EZ_MEMBER_PROPERTY("SoundShotgun", m_sSoundShotgun)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package), new ezGroupAttribute("Sounds")),
    EZ_MEMBER_PROPERTY("SoundDaggerStream", m_sSoundDaggerStream)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundDaggerHit", m_sSoundDaggerHit)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundWeakSpotHit", m_sSoundWeakSpotHit)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundGemPickup", m_sSoundGemPickup)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundLevelUp", m_sSoundLevelUp)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundPlayerDeath", m_sSoundPlayerDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundRoundStart", m_sSoundRoundStart)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundDaggerPickup", m_sSoundDaggerPickup)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundSpawnWarning", m_sSoundSpawnWarning)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundSquidSpawn", m_sSoundSquidSpawn)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundSquidSwarm", m_sSoundSquidSwarm)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundSkull2Dash", m_sSoundSkull2Dash)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundThiefSteal", m_sSoundThiefSteal)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundSkull1Death", m_sSoundSkull1Death)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundSkull2Death", m_sSoundSkull2Death)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundSkeletonDeath", m_sSoundSkeletonDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundThiefDeath", m_sSoundThiefDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundSquidDeath", m_sSoundSquidDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SoundCentipedeDeath", m_sSoundCentipedeDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("MusicAmbient", m_sMusicAmbient)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package), new ezGroupAttribute("Music")),
    EZ_MEMBER_PROPERTY("MusicAction", m_sMusicAction)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
  }
  EZ_END_PROPERTIES;

  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("DemonCove"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE
// clang-format on

ezCVarFloat cvar_GemAttractRange("DemonCove.GemAttractRange", 7.0f, ezCVarFlags::Default, "Distance at which gems start flying towards the player.");
extern ezCVarBool cvar_DebugAutoAim;

ezCVarBool cvar_GodMode("DemonCove.GodMode", false, ezCVarFlags::Default, "Debug: enemies can't kill the player and a wall around the arena prevents falling off.");
ezCVarInt cvar_StartStage("DemonCove.StartStage", 1, ezCVarFlags::Default, "Debug: the stage a new round starts at. Stages are numbered from 1, the last one is the endless phase.");
ezCVarInt cvar_StartGems("DemonCove.StartGems", 0, ezCVarFlags::Default, "Debug: the number of gems the player has at the start of a round. Determines the hand level.");
ezCVarInt cvar_DebugSpawn("DemonCove.DebugSpawn", -1, ezCVarFlags::Default, "Debug: set to an enemy type index to spawn one of them (0 = Squid I, 1 = Squid II, 5 = Thief, 6 = Centipede). Resets itself to -1.");
ezCVarInt cvar_DebugStress("DemonCove.DebugStress", 0, ezCVarFlags::Default, "Debug: set to 1 to spawn the worst case number of enemies and gems at once, for performance measurements. Resets itself to 0.");
ezCVarBool cvar_ShowStats("DemonCove.ShowStats", false, ezCVarFlags::Default, "Shows dagger and enemy statistics in the HUD.");

namespace
{
  constexpr const char* s_szInputSet = "Default";

  constexpr float s_fDaggerSpeed = 60.0f;
  constexpr float s_fDaggerLifeTime = 1.0f;
  constexpr float s_fPlayerRadius = 0.45f;
  constexpr ezUInt32 s_uiMaxSkull1 = 50;

  /// Round time at which each stage begins. The last stage is the endless phase that starts after the scripted timeline.
  constexpr float s_StageStartTimes[] = {0.0f, 14.0f, 29.0f, 39.0f, 57.0f, 84.0f};
  constexpr ezUInt32 s_uiNumStages = EZ_ARRAY_SIZE(s_StageStartTimes);

  constexpr ezUInt32 s_uiGemsForLevel2 = 10;
  constexpr ezUInt32 s_uiGemsForLevel3 = 50;

  constexpr float s_fGemCollectRange = 1.2f;

  constexpr float s_fTileSize = 2.0f;
  constexpr float s_fTileWarnDistance = 1.5f;
  constexpr float s_fWeakSpotRadius = 0.45f;
  constexpr float s_fSpawnWarningTime = 1.5f;

  /// Rotates the squid weak spots, so that they sit on the face of the model.
  constexpr float s_fSquidFaceYaw = 0.0f;

  constexpr ezUInt32 s_uiCentipedeSegments = 10;
  constexpr float s_fCentipedeTrailStep = 0.25f; ///< Distance between recorded trail points.
  constexpr ezUInt32 s_uiCentipedeTrailStepsPerSegment = 4;

  struct EnemyConfig
  {
    float m_fHealth;
    float m_fRadius;
    ezUInt32 m_uiGemDrop;
  };

  constexpr EnemyConfig s_EnemyConfig[(int)DcEnemyType::Count] = {
    {12.0f, 1.3f, 2}, // Squid1
    {25.0f, 1.6f, 4}, // Squid2
    {1.0f, 0.4f, 0},  // Skull1
    {5.0f, 0.5f, 1},  // Skull2
    {3.0f, 0.5f, 0},  // Skeleton
    {8.0f, 0.7f, 1},  // Thief, additionally drops all gems it collected
    {25.0f, 0.9f, 5}, // CentipedeHead
    {3.0f, 0.6f, 1},  // CentipedeBody
  };

  const char* s_szEnemyNames[] = {"Squid I", "Squid II", "Skull I", "Skull II", "Skeleton", "Thief", "Centipede", "Centipede segment"};
  static_assert(EZ_ARRAY_SIZE(s_szEnemyNames) == (int)DcEnemyType::Count);

  /// Returns true if the segment from vStart to vEnd passes within fRadius of vCenter.
  bool SegmentHitsSphere(const ezVec3& vStart, const ezVec3& vEnd, const ezVec3& vCenter, float fRadius)
  {
    const ezVec3 vSeg = vEnd - vStart;
    const float fLenSqr = vSeg.GetLengthSquared();
    float t = 0.0f;
    if (fLenSqr > 0.0f)
    {
      t = ezMath::Clamp((vCenter - vStart).Dot(vSeg) / fLenSqr, 0.0f, 1.0f);
    }

    return (vStart + vSeg * t - vCenter).GetLengthSquared() <= fRadius * fRadius;
  }

  ezQuat LookRotation(const ezVec3& vDir)
  {
    if (vDir.IsZero(0.0001f))
      return ezQuat::MakeIdentity();

    return ezQuat::MakeShortestRotation(ezVec3(1, 0, 0), vDir.GetNormalized());
  }

  ezQuat YawRotation(const ezVec3& vDir)
  {
    return ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), ezAngle::MakeFromRadian(ezMath::ATan2(vDir.y, vDir.x).GetRadian()));
  }

  ezQuat YawRotation(float fYaw)
  {
    return ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), ezAngle::MakeFromRadian(fYaw));
  }

  bool IsGroundEnemy(DcEnemyType type)
  {
    return type == DcEnemyType::Skeleton || type == DcEnemyType::Thief;
  }
} // namespace

DcArenaComponent::DcArenaComponent() = default;
DcArenaComponent::~DcArenaComponent() = default;

DcArenaComponent* DcArenaComponent::FindArena(ezWorld* pWorld)
{
  if (pWorld == nullptr)
    return nullptr;

  const DcArenaComponentManager* pManager = pWorld->GetComponentManager<DcArenaComponentManager>();
  if (pManager == nullptr)
    return nullptr;

  for (auto it = pManager->GetComponents(); it.IsValid(); ++it)
  {
    if (it->IsActiveAndSimulating())
      return const_cast<DcArenaComponent*>(&(*it));
  }

  return nullptr;
}

void DcArenaComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  auto loadPrefab = [](const ezString& sFile)
  {
    if (sFile.IsEmpty())
      return ezPrefabResourceHandle();

    return ezResourceManager::LoadResource<ezPrefabResource>(sFile);
  };

  m_hPrefabs[(int)DcEnemyType::Squid1] = loadPrefab(m_sSquid1Prefab);
  m_hPrefabs[(int)DcEnemyType::Squid2] = loadPrefab(m_sSquid2Prefab);
  m_hPrefabs[(int)DcEnemyType::Skull1] = loadPrefab(m_sSkull1Prefab);
  m_hPrefabs[(int)DcEnemyType::Skull2] = loadPrefab(m_sSkull2Prefab);
  m_hPrefabs[(int)DcEnemyType::Skeleton] = loadPrefab(m_sSkeletonPrefab);
  m_hPrefabs[(int)DcEnemyType::Thief] = loadPrefab(m_sThiefPrefab);
  m_hPrefabs[(int)DcEnemyType::CentipedeHead] = loadPrefab(m_sCentipedeHeadPrefab);
  m_hPrefabs[(int)DcEnemyType::CentipedeBody] = loadPrefab(m_sCentipedeBodyPrefab);
  m_hDaggerPrefab = loadPrefab(m_sDaggerPrefab);
  m_hGemPrefab = loadPrefab(m_sGemPrefab);
  m_hDeathEffect = loadPrefab(m_sDeathEffect);
  m_hHitEffect = loadPrefab(m_sHitEffect);
  m_hSpawnWarningEffect = loadPrefab(m_sSpawnWarningEffect);
  m_hGemPickupEffect = loadPrefab(m_sGemPickupEffect);
  m_hDaggerTrailEffect = loadPrefab(m_sDaggerTrailEffect);

  for (const ezPrefabResourceHandle& hPrefab : m_hPrefabs)
    PreloadPrefab(hPrefab);
  PreloadPrefab(m_hDaggerPrefab);
  PreloadPrefab(m_hGemPrefab);
  PreloadPrefab(m_hHitEffect);
  PreloadPrefab(m_hSpawnWarningEffect);
  PreloadPrefab(m_hGemPickupEffect);

  {
    ezInputActionConfig cfg;
    cfg.m_bApplyTimeScaling = false;
    cfg.m_sInputSlotTrigger[0] = ezInputSlot_KeyR;
    cfg.m_sInputSlotTrigger[1] = ezInputSlot_Controller0_ButtonY;
    ezInputManager::SetInputActionConfig(s_szInputSet, "restart", cfg, true);
  }

  // Rough approximation of the opening of the original: a few lone spawners, then pairs, then tougher ones.
  // After the scripted part, waves repeat with increasing pressure (see UpdateTimeline()).
  m_Timeline.Clear();
  m_Timeline.PushBack({3.0f, DcEnemyType::Squid1, 1});
  m_Timeline.PushBack({14.0f, DcEnemyType::Squid1, 1});
  m_Timeline.PushBack({19.0f, DcEnemyType::Squid1, 1});
  m_Timeline.PushBack({29.0f, DcEnemyType::Squid1, 2});
  m_Timeline.PushBack({34.0f, DcEnemyType::Skeleton, 5});
  m_Timeline.PushBack({39.0f, DcEnemyType::Squid2, 1});
  m_Timeline.PushBack({44.0f, DcEnemyType::Thief, 1});
  m_Timeline.PushBack({47.0f, DcEnemyType::Squid1, 2});
  m_Timeline.PushBack({52.0f, DcEnemyType::Skeleton, 7});
  m_Timeline.PushBack({57.0f, DcEnemyType::Squid2, 1});
  m_Timeline.PushBack({62.0f, DcEnemyType::Squid1, 2});
  m_Timeline.PushBack({64.0f, DcEnemyType::CentipedeHead, 1});
  m_Timeline.PushBack({67.0f, DcEnemyType::Skeleton, 9});
  m_Timeline.PushBack({70.0f, DcEnemyType::Thief, 1});
  m_Timeline.PushBack({72.0f, DcEnemyType::Squid2, 2});

  {
    auto createMusic = [this](const ezString& sSound)
    {
      if (sSound.IsEmpty())
        return ezComponentHandle();

      ezGameObjectDesc desc;
      desc.m_sName.Assign("Music");
      desc.m_hParent = GetOwner()->GetHandle();

      ezGameObject* pObject = nullptr;
      GetWorld()->CreateObject(desc, pObject);

      ezMiniAudioSoundComponent* pSound = nullptr;
      const ezComponentHandle hSound = ezMiniAudioSoundComponent::CreateComponent(pObject, pSound);
      pSound->SetSound(ezResourceManager::LoadResource<ezMiniAudioSoundResource>(sSound));
      pSound->SetNoGlobalPitch(true); // the hit pause changes the clock speed
      pSound->SetVolume(0.0f);
      pSound->SetPaused(true);        // UpdateMusic() starts it
      return hSound;
    };

    m_hMusicAmbient = createMusic(m_sMusicAmbient);
    m_hMusicAction = createMusic(m_sMusicAction);
    m_fMusicAction = 0.0f;
  }

  LoadScores();

  // all entries have to exist before the blackboard gets bound to the HUD document
  // global, so that the game state can set 'menuopen' while the world is paused
  m_pHud = ezBlackboard::GetOrCreateGlobal(ezMakeHashedString("hud"));
  m_pHud->SetEntryValue("menuopen", false);
  UpdateHudScores();
  UpdateHudBlackboard();

  BuildFloor();
  RestartRound();
  EnterStartState();
}

void DcArenaComponent::OnDeactivated()
{
  m_Enemies.Clear();
  m_Daggers.Clear();
  m_Gems.Clear();
  m_Tiles.Clear();
  m_TileGrid.Clear();
  m_CentipedeTrails.Clear();
  m_PendingSpawns.Clear();

  if (m_HitPauseEnd.IsPositive())
  {
    GetWorld()->GetClock().SetSpeed(1.0);
    m_HitPauseEnd = ezTime::MakeZero();
  }

  SUPER::OnDeactivated();
}

DcPlayerComponent* DcArenaComponent::GetPlayer() const
{
  DcPlayerComponent* pPlayer = nullptr;
  if (const_cast<ezWorld*>(GetWorld())->TryGetComponent(m_hPlayer, pPlayer))
    return pPlayer;

  const DcPlayerComponentManager* pManager = GetWorld()->GetComponentManager<DcPlayerComponentManager>();
  if (pManager == nullptr)
    return nullptr;

  for (auto it = pManager->GetComponents(); it.IsValid(); ++it)
  {
    if (it->IsActiveAndSimulating())
    {
      pPlayer = const_cast<DcPlayerComponent*>(&(*it));

      auto* pThis = const_cast<DcArenaComponent*>(this);
      pThis->m_hPlayer = pPlayer->GetHandle();
      pThis->m_vPlayerStart = pPlayer->GetOwner()->GetGlobalPosition();
      pThis->m_qPlayerStartRotation = pPlayer->GetOwner()->GetGlobalRotation();
      return pPlayer;
    }
  }

  return nullptr;
}

ezVec3 DcArenaComponent::GetPlayerTarget() const
{
  if (const DcPlayerComponent* pPlayer = GetPlayer())
    return pPlayer->GetBodyCenter() + ezVec3(0, 0, 0.4f);

  return GetOwner()->GetGlobalPosition() + ezVec3(0, 0, 1.5f);
}

ezUInt32 DcArenaComponent::GetHandLevel() const
{
  if (m_uiGems >= s_uiGemsForLevel3)
    return 3;
  if (m_uiGems >= s_uiGemsForLevel2)
    return 2;
  return 1;
}

bool DcArenaComponent::GetClosestEnemyPosition(const ezVec3& vPosition, ezVec3& out_vEnemyPosition) const
{
  float fBestDistSqr = ezMath::MaxValue<float>();

  for (const Enemy& e : m_Enemies)
  {
    const ezVec3 vAim = GetAimPosition(e, vPosition);
    const float fDistSqr = (vAim - vPosition).GetLengthSquared();
    if (fDistSqr < fBestDistSqr)
    {
      fBestDistSqr = fDistSqr;
      out_vEnemyPosition = vAim;
    }
  }

  return !m_Enemies.IsEmpty();
}

float DcArenaComponent::GetEnemyRadius(DcEnemyType type)
{
  return s_EnemyConfig[(int)type].m_fRadius;
}

ezUInt32 DcArenaComponent::GetWeakSpotCount(DcEnemyType type)
{
  switch (type)
  {
    case DcEnemyType::Squid1:
      return 1;
    case DcEnemyType::Squid2:
      return 2;
    default:
      return 0;
  }
}

ezVec3 DcArenaComponent::GetWeakSpotPosition(const Enemy& e, ezUInt32 uiSpot) const
{
  // the gems sit on the front (and back) of the squid, which slowly rotates
  const float fYaw = e.m_fYaw + s_fSquidFaceYaw + uiSpot * ezMath::Pi<float>();
  const float fDist = GetEnemyRadius(e.m_Type) * 0.9f;
  return e.m_vPosition + YawRotation(fYaw) * ezVec3(fDist, 0, 0.2f);
}

ezVec3 DcArenaComponent::GetAimPosition(const Enemy& e, const ezVec3& vFrom) const
{
  const ezUInt32 uiNumSpots = GetWeakSpotCount(e.m_Type);
  if (uiNumSpots == 0)
    return e.m_vPosition;

  ezVec3 vBest = GetWeakSpotPosition(e, 0);
  for (ezUInt32 i = 1; i < uiNumSpots; ++i)
  {
    const ezVec3 vSpot = GetWeakSpotPosition(e, i);
    if ((vSpot - vFrom).GetLengthSquared() < (vBest - vFrom).GetLengthSquared())
      vBest = vSpot;
  }

  return vBest;
}

ezColor DcArenaComponent::GetBaseColor(DcEnemyType type)
{
  if (type == DcEnemyType::CentipedeBody || type == DcEnemyType::CentipedeHead)
    return ezColor(1.0f, 0.35f, 0.3f);

  return ezColor::White;
}

void DcArenaComponent::SetVisualColor(ezGameObjectHandle hObject, const ezColor& color)
{
  ezGameObject* pObject = nullptr;
  if (!GetWorld()->TryGetObject(hObject, pObject))
    return;

  ezHybridArray<ezGameObject*, 16> stack;
  stack.PushBack(pObject);

  ezHybridArray<ezMeshComponentBase*, 4> meshes;
  while (!stack.IsEmpty())
  {
    ezGameObject* pCur = stack.PeekBack();
    stack.PopBack();

    pCur->TryGetComponentsOfBaseType(meshes);
    for (ezMeshComponentBase* pMesh : meshes)
      pMesh->SetColor(color);

    for (auto it = pCur->GetChildren(); it.IsValid(); ++it)
      stack.PushBack(it);
  }
}

//////////////////////////////////////////////////////////////////////////
// Visuals

ezGameObjectHandle DcArenaComponent::CreateVisual(const ezPrefabResourceHandle& hPrefab, const ezVec3& vPosition, const ezQuat& qRotation)
{
  if (!hPrefab.IsValid())
    return {};

  ezGameObjectDesc desc;
  desc.m_bDynamic = true;
  desc.m_LocalPosition = vPosition;
  desc.m_LocalRotation = qRotation;

  ezGameObject* pObject = nullptr;
  ezGameObjectHandle hObject = GetWorld()->CreateObject(desc, pObject);

  ezPrefabInstantiationOptions options;
  options.m_hParent = hObject;
  ezPrefabResource::InstantiatePrefab(hPrefab, true, *GetWorld(), ezTransform::MakeIdentity(), options);

  return hObject;
}

void DcArenaComponent::PreloadPrefab(const ezPrefabResourceHandle& hPrefab)
{
  if (!hPrefab.IsValid())
    return;

  // Instantiate the prefab once and force all meshes it uses to load right away.
  // Otherwise the first instances spawned during the game would show loading placeholders.
  ezDynamicArray<ezGameObject*> rootObjects;
  ezDynamicArray<ezGameObject*> childObjects;
  ezPrefabInstantiationOptions options;
  options.m_pCreatedRootObjectsOut = &rootObjects;
  options.m_pCreatedChildObjectsOut = &childObjects;
  ezPrefabResource::InstantiatePrefab(hPrefab, true, *GetWorld(), ezTransform(ezVec3(0, 0, -1000.0f)), options);

  auto preloadObject = [](ezGameObject* pObject)
  {
    ezHybridArray<ezMeshComponent*, 4> meshes;
    pObject->TryGetComponentsOfBaseType(meshes);

    for (ezMeshComponent* pMesh : meshes)
    {
      if (!pMesh->GetMesh().IsValid())
        continue;

      ezResourceManager::ForceLoadResourceNow(pMesh->GetMesh());

      ezResourceLock<ezMeshResource> pMeshRes(pMesh->GetMesh(), ezResourceAcquireMode::BlockTillLoaded_NeverFail);
      if (pMeshRes.GetAcquireResult() == ezResourceAcquireResult::Final)
      {
        for (const ezMaterialResourceHandle& hMaterial : pMeshRes->GetMaterials())
        {
          if (hMaterial.IsValid())
            ezResourceManager::ForceLoadResourceNow(hMaterial);
        }
      }
    }
  };

  for (ezGameObject* pObject : rootObjects)
    preloadObject(pObject);
  for (ezGameObject* pObject : childObjects)
    preloadObject(pObject);

  for (ezGameObject* pObject : rootObjects)
    GetWorld()->DeleteObjectDelayed(pObject->GetHandle());
}

void DcArenaComponent::DeleteVisual(ezGameObjectHandle hObject)
{
  if (!hObject.IsInvalidated())
  {
    GetWorld()->DeleteObjectDelayed(hObject);
  }
}

void DcArenaComponent::PlaceVisual(ezGameObjectHandle hObject, const ezVec3& vPosition, const ezQuat& qRotation)
{
  ezGameObject* pObject = nullptr;
  if (GetWorld()->TryGetObject(hObject, pObject))
  {
    pObject->SetGlobalPosition(vPosition);
    pObject->SetGlobalRotation(qRotation);
  }
}

//////////////////////////////////////////////////////////////////////////
// Round state

void DcArenaComponent::RestartRound()
{
  LeaveStartState();

  for (const Enemy& e : m_Enemies)
    DeleteVisual(e.m_hObject);
  for (const Dagger& d : m_Daggers)
    DeleteVisual(d.m_hObject);
  for (const Gem& g : m_Gems)
    DeleteVisual(g.m_hObject);

  m_Enemies.Clear();
  m_Daggers.Clear();
  m_Gems.Clear();
  m_CentipedeTrails.Clear();
  ClearPendingSpawns();

  m_fGemPulse = 0.0f;
  m_fLevelUpTime = 0.0f;
  m_uiLastHandLevel = 1;

  ResetFloor();

  const ezUInt32 uiStartStage = (ezUInt32)ezMath::Clamp<ezInt32>(cvar_StartStage, 1, s_uiNumStages);
  const float fStartTime = s_StageStartTimes[uiStartStage - 1];

  // skip all spawns of earlier stages
  m_uiNextTimelineEvent = 0;
  while (m_uiNextTimelineEvent < m_Timeline.GetCount() && m_Timeline[m_uiNextTimelineEvent].m_fTime < fStartTime)
    ++m_uiNextTimelineEvent;

  m_fNextLoopSpawn = s_StageStartTimes[s_uiNumStages - 1];
  m_uiLoopWave = 0;
  m_RoundTime = ezTime::MakeFromSeconds(fStartTime);
  m_RoundStartTime = m_RoundTime;
  m_uiGems = (ezUInt32)ezMath::Max<ezInt32>(cvar_StartGems, 0);
  m_uiKills = 0;
  m_uiDaggersFired = 0;
  m_uiDaggersHit = 0;
  m_bPlayerDead = false;
  m_uiScorePlace = 0;
  m_bDebugRound = false;
  UpdateHudScores();
  m_sDeathReason.Clear();

  if (DcPlayerComponent* pPlayer = GetPlayer())
  {
    pPlayer->Respawn(m_vPlayerStart, m_qPlayerStartRotation);
  }
}

ezUInt32 DcArenaComponent::GetStage() const
{
  const float fTime = m_RoundTime.AsFloatInSeconds();

  ezUInt32 uiStage = 1;
  while (uiStage < s_uiNumStages && s_StageStartTimes[uiStage] <= fTime)
    ++uiStage;

  return uiStage;
}

void DcArenaComponent::KillPlayer(ezStringView sReason)
{
  if (m_bPlayerDead || cvar_GodMode)
    return;

  m_bPlayerDead = true;
  m_sDeathReason = sReason;

  if (const DcPlayerComponent* pPlayer = GetPlayer())
  {
    AddCameraShake(pPlayer->GetBodyCenter(), 5.0f, 1.0f, ezTime::MakeFromMilliseconds(400));
    PlaySound(m_sSoundPlayerDeath, pPlayer->GetBodyCenter());
  }

  if (!m_bDebugRound && IsRegularRound())
  {
    AddScore();
  }

  ezLog::Info("DemonCove: player died after {} seconds ({}), {} kills, {} gems", ezArgF(m_RoundTime.AsFloatInSeconds(), 4), sReason, m_uiKills, m_uiGems);
}

//////////////////////////////////////////////////////////////////////////
// Update

void DcArenaComponent::Update()
{
  UpdateSafetyWall();

  if (ezInputManager::GetInputActionState(s_szInputSet, "restart") == ezKeyState::Pressed)
  {
    RestartRound();
    PlaySound(m_sSoundRoundStart, m_vPlayerStart);
  }

  if (m_HitPauseEnd.IsPositive() && ezTime::Now() >= m_HitPauseEnd)
  {
    GetWorld()->GetClock().SetSpeed(1.0);
    m_HitPauseEnd = ezTime::MakeZero();
  }

  const float fDeltaTime = ezMath::Min(GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds(), 0.1f);

  m_fGemPulse = ezMath::Max(0.0f, m_fGemPulse - fDeltaTime);
  m_fLevelUpTime = ezMath::Max(0.0f, m_fLevelUpTime - fDeltaTime);

  if (GetHandLevel() > m_uiLastHandLevel)
  {
    m_fLevelUpTime = 2.0f;
    PlaySound(m_sSoundLevelUp, GetPlayerTarget());
  }
  m_uiLastHandLevel = GetHandLevel();

  if (m_bWaitingForStart)
  {
    UpdateStartState(fDeltaTime);
  }
  else if (!m_bPlayerDead)
  {
    m_RoundTime += ezTime::MakeFromSeconds(fDeltaTime);
    UpdateTimeline(fDeltaTime);
    UpdatePendingSpawns(fDeltaTime);
  }

  UpdateFloor(fDeltaTime);

  if (!IsRegularRound())
  {
    m_bDebugRound = true;
  }

  if (cvar_DebugStress > 0)
  {
    // about the most that the endless phase gets to: several spawners with full swarms, skeleton groups, two centipedes
    cvar_DebugStress = 0;
    m_bDebugRound = true;

    ezRandom& rng = GetWorld()->GetRandomNumberGenerator();
    const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
    auto randomPos = [&](float fMinHeight, float fMaxHeight)
    {
      const ezAngle angle = ezAngle::MakeFromRadian(rng.FloatMinMax(0.0f, ezMath::Pi<float>() * 2.0f));
      const float fDist = rng.FloatMinMax(6.0f, m_fCurrentRadius - 3.0f);
      return vCenter + ezVec3(ezMath::Cos(angle) * fDist, ezMath::Sin(angle) * fDist, rng.FloatMinMax(fMinHeight, fMaxHeight));
    };

    for (ezUInt32 i = 0; i < 3; ++i)
      SpawnEnemy(DcEnemyType::Squid1, randomPos(3.5f, 6.0f));
    for (ezUInt32 i = 0; i < 3; ++i)
      SpawnEnemy(DcEnemyType::Squid2, randomPos(3.5f, 6.0f));
    for (ezUInt32 i = 0; i < s_uiMaxSkull1; ++i)
      SpawnEnemy(DcEnemyType::Skull1, randomPos(1.0f, 8.0f));
    for (ezUInt32 i = 0; i < 10; ++i)
      SpawnEnemy(DcEnemyType::Skull2, randomPos(2.0f, 5.0f));
    for (ezUInt32 i = 0; i < 20; ++i)
      SpawnEnemy(DcEnemyType::Skeleton, randomPos(0.5f, 0.5f));
    for (ezUInt32 i = 0; i < 2; ++i)
      SpawnEnemy(DcEnemyType::Thief, randomPos(0.7f, 0.7f));
    for (ezUInt32 i = 0; i < 2; ++i)
      SpawnCentipedeAt(randomPos(4.0f, 7.0f) + ezVec3(30, 0, 0), s_uiCentipedeSegments + 4);
    for (ezUInt32 i = 0; i < 60; ++i)
      SpawnGem(randomPos(0.5f, 2.0f));
  }

  if (cvar_DebugSpawn >= 0)
  {
    m_bDebugRound = true;

    if (cvar_DebugSpawn < (int)DcEnemyType::Count)
      SpawnTimelineEnemies((DcEnemyType)(int)cvar_DebugSpawn, 1);

    cvar_DebugSpawn = -1;
  }

  UpdateEnemies(fDeltaTime);
  UpdateDaggers(fDeltaTime);
  UpdateGems(fDeltaTime);

  if (!m_bPlayerDead && !m_bWaitingForStart)
  {
    UpdatePlayer();
  }

  UpdateHud();
  UpdateMusic();
}

void DcArenaComponent::UpdateMusic()
{
  // real time, so that the hit pause doesn't stall the fade
  const float fDeltaTime = ezMath::Min(ezClock::GetGlobalClock()->GetTimeDiff().AsFloatInSeconds(), 0.1f);
  const bool bAction = !m_bWaitingForStart && !m_bPlayerDead;

  ezMiniAudioSoundComponent* pAmbient = nullptr;
  ezMiniAudioSoundComponent* pAction = nullptr;
  if (!GetWorld()->TryGetComponent(m_hMusicAmbient, pAmbient))
    pAmbient = nullptr;
  if (!GetWorld()->TryGetComponent(m_hMusicAction, pAction))
    pAction = nullptr;

  if (bAction)
  {
    if (m_fMusicAction == 0.0f && pAction)
    {
      // starts from the beginning, a previous round's music was faded out
      pAction->SetVolume(0.0f);
      pAction->Play();
    }

    m_fMusicAction = ezMath::Min(1.0f, m_fMusicAction + fDeltaTime / 1.5f);
  }
  else if (m_fMusicAction > 0.0f)
  {
    if (pAction)
    {
      pAction->FadeOut(ezTime::MakeFromSeconds(3.0));
    }

    // the action music fades out on its own, the ambient music comes back slowly
    m_fMusicAction = 0.0f;
  }

  if (pAction && m_fMusicAction > 0.0f)
  {
    pAction->SetVolume(m_fMusicAction);
  }

  if (pAmbient)
  {
    const float fTarget = 1.0f - m_fMusicAction;
    const float fVolume = pAmbient->GetVolume();
    pAmbient->SetVolume(fTarget > fVolume ? ezMath::Min(fTarget, fVolume + fDeltaTime / 4.0f) : fTarget);

    if (pAmbient->GetVolume() > 0.0f)
    {
      if (pAmbient->GetPaused())
        pAmbient->Play();
    }
    else if (!pAmbient->GetPaused())
    {
      pAmbient->Pause();
    }
  }
}

void DcArenaComponent::UpdateTimeline(float fDeltaTime)
{
  const float fTime = m_RoundTime.AsFloatInSeconds();

  while (m_uiNextTimelineEvent < m_Timeline.GetCount() && m_Timeline[m_uiNextTimelineEvent].m_fTime <= fTime)
  {
    const SpawnEvent& ev = m_Timeline[m_uiNextTimelineEvent];
    SpawnTimelineEnemies(ev.m_Type, ev.m_uiCount);

    ++m_uiNextTimelineEvent;

  }

  if (m_uiNextTimelineEvent < m_Timeline.GetCount())
    return;

  // endless phase: alternate between waves of weak and tough spawners, with shrinking intervals
  if (fTime >= m_fNextLoopSpawn)
  {
    ++m_uiLoopWave;

    const ezUInt32 uiExtra = m_uiLoopWave / 4;
    if (m_uiLoopWave % 2 == 0)
    {
      for (ezUInt32 i = 0; i < 1 + uiExtra; ++i)
        SpawnSquid(DcEnemyType::Squid2);
    }
    else
    {
      for (ezUInt32 i = 0; i < 2 + uiExtra; ++i)
        SpawnSquid(DcEnemyType::Squid1);
    }

    if (m_uiLoopWave % 3 == 0)
    {
      SpawnSkeletons(7 + uiExtra);
    }

    if (m_uiLoopWave % 4 == 2)
    {
      SpawnThief();
    }

    if (m_uiLoopWave % 5 == 0)
    {
      SpawnCentipede(s_uiCentipedeSegments + uiExtra * 2);
    }

    m_fNextLoopSpawn = fTime + ezMath::Max(9.0f, 16.0f - m_uiLoopWave * 0.4f);
  }
}

void DcArenaComponent::SpawnTimelineEnemies(DcEnemyType type, ezUInt32 uiCount)
{
  for (ezUInt32 i = 0; i < uiCount; ++i)
  {
    switch (type)
    {
      case DcEnemyType::Skeleton:
        SpawnSkeletons(uiCount);
        return;

      case DcEnemyType::Thief:
        SpawnThief();
        break;

      case DcEnemyType::CentipedeHead:
        SpawnCentipede(s_uiCentipedeSegments);
        break;

      default:
        SpawnSquid(type);
        break;
    }
  }
}

ezVec3 DcArenaComponent::PickFloorSpawnPosition(float fInset, float fMinPlayerDistance) const
{
  ezRandom& rng = const_cast<ezWorld*>(GetWorld())->GetRandomNumberGenerator();
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  const DcPlayerComponent* pPlayer = GetPlayer();
  const float fDist = ezMath::Max(1.0f, m_fCurrentRadius - fInset);

  ezVec3 vPos = vCenter;
  for (ezUInt32 uiTry = 0; uiTry < 32; ++uiTry)
  {
    const ezAngle angle = ezAngle::MakeFromRadian((float)rng.DoubleMinMax(0.0, ezMath::Pi<double>() * 2.0));
    vPos = vCenter + ezVec3(ezMath::Cos(angle) * fDist, ezMath::Sin(angle) * fDist, 0);

    if (pPlayer == nullptr)
      break;

    ezVec3 vToPlayer = pPlayer->GetBodyCenter() - vPos;
    vToPlayer.z = 0;
    if (vToPlayer.GetLength() >= fMinPlayerDistance)
      break;
  }

  return vPos;
}

void DcArenaComponent::SpawnThief()
{
  const ezVec3 vPos = PickFloorSpawnPosition(3.0f, 10.0f) + ezVec3(0, 0, GetEnemyRadius(DcEnemyType::Thief));
  QueueSpawn(DcEnemyType::Thief, vPos, 1);
}

void DcArenaComponent::SpawnCentipede(ezUInt32 uiSegments)
{
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();
  const ezAngle angle = ezAngle::MakeFromRadian((float)rng.DoubleMinMax(0.0, ezMath::Pi<double>() * 2.0));
  const ezVec3 vOutwards(ezMath::Cos(angle), ezMath::Sin(angle), 0);
  const ezVec3 vHeadPos = GetOwner()->GetGlobalPosition() + vOutwards * (m_fArenaRadius + 8.0f) + ezVec3(0, 0, 6.0f);
  QueueSpawn(DcEnemyType::CentipedeHead, vHeadPos, uiSegments);
}

void DcArenaComponent::SpawnCentipedeAt(const ezVec3& vHeadPos, ezUInt32 uiSegments)
{
  ezVec3 vOutwards = vHeadPos - GetOwner()->GetGlobalPosition();
  vOutwards.z = 0;
  vOutwards.NormalizeIfNotZero(ezVec3(1, 0, 0)).IgnoreResult();

  const ezUInt32 uiGroup = m_uiNextCentipede++;

  // the trail initially extends straight outwards behind the head, so that the body trails in from outside the arena
  ezDynamicArray<ezVec3>& trail = m_CentipedeTrails[uiGroup];
  const ezUInt32 uiTrailPoints = (uiSegments + 1) * s_uiCentipedeTrailStepsPerSegment + 1;
  trail.SetCount(uiTrailPoints);
  for (ezUInt32 i = 0; i < uiTrailPoints; ++i)
  {
    trail[uiTrailPoints - 1 - i] = vHeadPos + vOutwards * (i * s_fCentipedeTrailStep);
  }

  SpawnEnemy(DcEnemyType::CentipedeHead, vHeadPos);
  m_Enemies.PeekBack().m_uiGroup = uiGroup;
  m_Enemies.PeekBack().m_vVelocity = -vOutwards * 6.0f;

  for (ezUInt32 i = 1; i <= uiSegments; ++i)
  {
    SpawnEnemy(DcEnemyType::CentipedeBody, trail[uiTrailPoints - 1 - i * s_uiCentipedeTrailStepsPerSegment]);
    m_Enemies.PeekBack().m_uiGroup = uiGroup;
    m_Enemies.PeekBack().m_uiSegment = (ezUInt16)i;
  }
}

void DcArenaComponent::SpawnSkeletons(ezUInt32 uiCount)
{
  // a group climbs onto the platform at one spot of the edge
  // far enough away that the spread out group doesn't reach the player either
  const ezVec3 vPos = PickFloorSpawnPosition(3.0f, 12.0f) + ezVec3(0, 0, 1.0f);
  QueueSpawn(DcEnemyType::Skeleton, vPos, uiCount);
}

void DcArenaComponent::SpawnSkeletonGroup(const ezVec3& vPosition, ezUInt32 uiCount)
{
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();
  const ezVec3 vRel = vPosition - GetOwner()->GetGlobalPosition();
  const float fBaseAngle = ezMath::ATan2(vRel.y, vRel.x).GetRadian();

  for (ezUInt32 i = 0; i < uiCount; ++i)
  {
    const ezAngle angle = ezAngle::MakeFromRadian(fBaseAngle + rng.FloatMinMax(-0.25f, 0.25f));
    const float fDist = m_fCurrentRadius - rng.FloatMinMax(2.0f, 4.0f);
    const ezVec3 vPos = GetOwner()->GetGlobalPosition() + ezVec3(ezMath::Cos(angle) * fDist, ezMath::Sin(angle) * fDist, GetEnemyRadius(DcEnemyType::Skeleton));
    SpawnEnemy(DcEnemyType::Skeleton, vPos);
  }
}

void DcArenaComponent::SpawnSquid(DcEnemyType type)
{
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();
  const float fAngle = (float)rng.DoubleMinMax(0.0, ezMath::Pi<double>() * 2.0);
  const float fDist = m_fArenaRadius + 6.0f;

  const ezVec3 vPos = GetOwner()->GetGlobalPosition() + ezVec3(ezMath::Cos(ezAngle::MakeFromRadian(fAngle)) * fDist, ezMath::Sin(ezAngle::MakeFromRadian(fAngle)) * fDist, rng.FloatMinMax(3.5f, 6.0f));
  QueueSpawn(type, vPos, 1);
}

void DcArenaComponent::QueueSpawn(DcEnemyType type, const ezVec3& vPosition, ezUInt32 uiCount)
{
  PendingSpawn& spawn = m_PendingSpawns.ExpandAndGetRef();
  spawn.m_Type = type;
  spawn.m_uiCount = uiCount;
  spawn.m_vPosition = vPosition;
  spawn.m_fTimeLeft = s_fSpawnWarningTime;
  spawn.m_hWarning = CreateVisual(m_hSpawnWarningEffect, vPosition, ezQuat::MakeIdentity());

  PlaySound(m_sSoundSpawnWarning, vPosition);
}

void DcArenaComponent::UpdatePendingSpawns(float fDeltaTime)
{
  for (ezUInt32 i = 0; i < m_PendingSpawns.GetCount();)
  {
    PendingSpawn& spawn = m_PendingSpawns[i];
    spawn.m_fTimeLeft -= fDeltaTime;

    if (spawn.m_fTimeLeft > 0.0f)
    {
      ++i;
      continue;
    }

    // ground enemies wait until the player has left the spot, the warning keeps playing meanwhile
    if (IsGroundEnemy(spawn.m_Type))
    {
      if (const DcPlayerComponent* pPlayer = GetPlayer())
      {
        ezVec3 vToPlayer = pPlayer->GetBodyCenter() - spawn.m_vPosition;
        vToPlayer.z = 0;
        if (vToPlayer.GetLength() < 5.0f)
        {
          spawn.m_fTimeLeft = 0.25f;
          ++i;
          continue;
        }
      }
    }

    // copy, spawning may add more pending spawns
    const PendingSpawn due = spawn;
    m_PendingSpawns.RemoveAtAndSwap(i);
    DeleteVisual(due.m_hWarning);

    switch (due.m_Type)
    {
      case DcEnemyType::Skeleton:
        SpawnSkeletonGroup(due.m_vPosition, due.m_uiCount);
        break;

      case DcEnemyType::CentipedeHead:
        SpawnCentipedeAt(due.m_vPosition, due.m_uiCount);
        break;

      default:
        SpawnEnemy(due.m_Type, due.m_vPosition);
        break;
    }
  }
}

void DcArenaComponent::ClearPendingSpawns()
{
  for (const PendingSpawn& spawn : m_PendingSpawns)
    DeleteVisual(spawn.m_hWarning);

  m_PendingSpawns.Clear();
}

void DcArenaComponent::AddCameraShake(const ezVec3& vPosition, float fRadius, float fStrength, ezTime duration)
{
  // a burst shake volume, the camera shake component on the player samples these
  ezGameObjectDesc desc;
  desc.m_LocalPosition = vPosition;

  ezGameObject* pObject = nullptr;
  GetWorld()->CreateObject(desc, pObject);

  ezCameraShakeVolumeSphereComponent* pShake = nullptr;
  GetWorld()->GetOrCreateComponentManager<ezCameraShakeVolumeSphereComponentManager>()->CreateComponent(pObject, pShake);
  pShake->SetRadius(fRadius);
  pShake->m_fStrength = fStrength;
  pShake->m_BurstDuration = duration;
  pShake->m_OnFinishedAction = ezOnComponentFinishedAction::DeleteGameObject;
}

void DcArenaComponent::PlaySound(const ezString& sSound, const ezVec3& vPosition)
{
  if (sSound.IsEmpty())
    return;

  const ezTime tNow = ezTime::Now();
  ezTime& tLast = m_LastSoundTime[&sSound];
  if (tNow - tLast < ezTime::MakeFromMilliseconds(40))
    return;

  tLast = tNow;
  ezSoundInterface::PlaySound(GetWorld(), sSound, ezTransform(vPosition)).IgnoreResult();
}

void DcArenaComponent::HitPause()
{
  GetWorld()->GetClock().SetSpeed(0.05);
  m_HitPauseEnd = ezTime::Now() + ezTime::MakeFromMilliseconds(60);
}

void DcArenaComponent::SpawnEnemy(DcEnemyType type, const ezVec3& vPosition)
{
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();

  Enemy& e = m_Enemies.ExpandAndGetRef();
  e.m_Type = type;
  e.m_vPosition = vPosition;
  e.m_vVelocity.SetZero();
  e.m_fHealth = s_EnemyConfig[(int)type].m_fHealth;
  e.m_fPhase = rng.FloatMinMax(0.0f, 100.0f);
  e.m_fYaw = rng.FloatMinMax(0.0f, ezMath::Pi<float>() * 2.0f);
  e.m_uiState = 0;

  switch (type)
  {
    case DcEnemyType::Squid1:
    case DcEnemyType::Squid2:
      e.m_fTimer = 3.0f;
      PlaySound(m_sSoundSquidSpawn, vPosition);
      break;

    case DcEnemyType::Skull1:
      e.m_vVelocity = ezVec3(rng.FloatMinMax(-1, 1), rng.FloatMinMax(-1, 1), rng.FloatMinMax(0.2f, 1.0f)).GetNormalized() * 4.0f;
      break;

    case DcEnemyType::Skull2:
      e.m_fTimer = rng.FloatMinMax(2.0f, 4.0f);
      break;

    default:
      break;
  }

  e.m_hObject = CreateVisual(m_hPrefabs[(int)type], vPosition, ezQuat::MakeIdentity());

  if (GetBaseColor(type) != ezColor::White)
  {
    SetVisualColor(e.m_hObject, GetBaseColor(type));
  }

  // the weak spots are gems attached to the visual
  const ezUInt32 uiNumSpots = GetWeakSpotCount(type);
  if (uiNumSpots > 0 && m_hGemPrefab.IsValid() && !e.m_hObject.IsInvalidated())
  {
    for (ezUInt32 i = 0; i < uiNumSpots; ++i)
    {
      ezGameObjectDesc desc;
      desc.m_bDynamic = true;
      desc.m_hParent = e.m_hObject;
      // relative to the visual, which gets rotated by the enemy's yaw
      desc.m_LocalPosition = YawRotation(-e.m_fYaw) * (GetWeakSpotPosition(e, i) - e.m_vPosition);
      desc.m_LocalUniformScaling = 1.6f;

      ezGameObject* pSpot = nullptr;
      const ezGameObjectHandle hSpot = GetWorld()->CreateObject(desc, pSpot);

      ezPrefabInstantiationOptions options;
      options.m_hParent = hSpot;
      ezPrefabResource::InstantiatePrefab(m_hGemPrefab, true, *GetWorld(), ezTransform::MakeIdentity(), options);
    }
  }
}

void DcArenaComponent::UpdateEnemies(float fDeltaTime)
{
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  const ezVec3 vTarget = GetPlayerTarget();
  const float fTime = GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds();
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();

  // enemies may be appended while iterating (spawners), so iterate by index over the count at the start
  const ezUInt32 uiNumEnemies = m_Enemies.GetCount();
  for (ezUInt32 i = 0; i < uiNumEnemies; ++i)
  {
    {
      Enemy& e = m_Enemies[i];

      switch (e.m_Type)
      {
        case DcEnemyType::Squid1:
        case DcEnemyType::Squid2:
        {
          ezVec3 vToCenter = vCenter - e.m_vPosition;
          vToCenter.z = 0;
          const float fDist = vToCenter.GetLength();

          if (e.m_uiState == 0)
          {
            // float in from outside the arena
            e.m_vVelocity = vToCenter.GetNormalized() * 2.0f;
            if (fDist < m_fCurrentRadius * 0.7f)
              e.m_uiState = 1;
          }
          else
          {
            // slowly circle around the arena center
            ezVec3 vTangent = vToCenter.CrossRH(ezVec3(0, 0, 1)).GetNormalized();
            e.m_vVelocity = vTangent * 0.8f + vToCenter.GetNormalized() * (fDist - m_fCurrentRadius * 0.7f) * 0.2f;
          }

          e.m_vVelocity.z = ezMath::Sin(ezAngle::MakeFromRadian(fTime * 0.7f + e.m_fPhase)) * 0.4f;
          e.m_vPosition += e.m_vVelocity * fDeltaTime;

          // the slow rotation hides the gems from time to time
          e.m_fYaw += fDeltaTime * ((e.m_Type == DcEnemyType::Squid1) ? 0.45f : -0.35f);

          e.m_fTimer -= fDeltaTime;
          if (e.m_fTimer <= 0.0f && !m_bPlayerDead)
          {
            e.m_fTimer = (e.m_Type == DcEnemyType::Squid1) ? 14.0f : 16.0f;

            // copy, the spawns below may reallocate the array
            const ezVec3 vSpawnPos = e.m_vPosition;
            const DcEnemyType spawnerType = e.m_Type;

            // limit the swarm size, otherwise later stages become a wall of skulls
            ezUInt32 uiNumSkull1 = 0;
            for (const Enemy& other : m_Enemies)
              uiNumSkull1 += (other.m_Type == DcEnemyType::Skull1) ? 1 : 0;

            const ezUInt32 uiSwarmSize = ezMath::Min<ezUInt32>(7, s_uiMaxSkull1 - ezMath::Min(uiNumSkull1, s_uiMaxSkull1));

            for (ezUInt32 s = 0; s < uiSwarmSize; ++s)
            {
              const ezVec3 vOffset = ezVec3(rng.FloatMinMax(-1, 1), rng.FloatMinMax(-1, 1), rng.FloatMinMax(-0.5f, 1.0f)) * 1.5f;
              SpawnEnemy(DcEnemyType::Skull1, vSpawnPos + vOffset);
            }

            if (spawnerType == DcEnemyType::Squid2)
            {
              SpawnEnemy(DcEnemyType::Skull2, vSpawnPos + ezVec3(0, 0, 2.0f));
            }

            if (uiSwarmSize > 0 || spawnerType == DcEnemyType::Squid2)
            {
              PlaySound(m_sSoundSquidSwarm, vSpawnPos);
            }
          }

          break;
        }

        case DcEnemyType::Skull1:
        {
          // home in on the player with a limited turn rate and some wobble, so they can be dodged
          ezVec3 vToTarget = vTarget - e.m_vPosition;
          ezVec3 vDesired = vToTarget.GetNormalized();
          const ezVec3 vSide = vDesired.CrossRH(ezVec3(0, 0, 1)).GetNormalized();
          vDesired += vSide * ezMath::Sin(ezAngle::MakeFromRadian(fTime * 2.3f + e.m_fPhase)) * 0.6f;
          vDesired.z += ezMath::Sin(ezAngle::MakeFromRadian(fTime * 1.7f + e.m_fPhase * 1.3f)) * 0.4f;
          vDesired.NormalizeIfNotZero(ezVec3(1, 0, 0)).IgnoreResult();

          const float fSpeed = 6.0f + ezMath::Sin(ezAngle::MakeFromRadian(e.m_fPhase)) * 1.0f;
          e.m_vVelocity = ezMath::Lerp(e.m_vVelocity, vDesired * fSpeed, ezMath::Min(1.0f, fDeltaTime * 1.4f));
          e.m_vPosition += e.m_vVelocity * fDeltaTime;
          break;
        }

        case DcEnemyType::Skeleton:
        {
          // run along the floor, turning towards the player at a limited rate, so that it can be outrun with sharp turns
          ezVec3 vToTarget = vTarget - e.m_vPosition;
          vToTarget.z = 0;
          const ezVec3 vDesired = vToTarget.GetNormalized() * (5.0f + ezMath::Sin(ezAngle::MakeFromRadian(e.m_fPhase)) * 0.7f);
          e.m_vVelocity = ezMath::Lerp(e.m_vVelocity, vDesired, ezMath::Min(1.0f, fDeltaTime * 2.5f));
          e.m_vVelocity.z = 0;
          e.m_vPosition += e.m_vVelocity * fDeltaTime;
          break;
        }

        case DcEnemyType::Thief:
          UpdateThief(e, fDeltaTime, vTarget);
          break;

        case DcEnemyType::CentipedeHead:
          UpdateCentipedeHead(e, fDeltaTime, vTarget);
          break;

        case DcEnemyType::CentipedeBody:
          UpdateCentipedeBody(e);
          break;

        case DcEnemyType::Skull2:
        {
          ezVec3 vToTarget = vTarget - e.m_vPosition;
          e.m_fTimer -= fDeltaTime;

          if (e.m_uiState == 0)
          {
            // circle the player at a distance
            const float fAngle = fTime * 0.6f + e.m_fPhase;
            const ezVec3 vOrbit = vTarget + ezVec3(ezMath::Cos(ezAngle::MakeFromRadian(fAngle)) * 7.0f, ezMath::Sin(ezAngle::MakeFromRadian(fAngle)) * 7.0f, 2.5f);
            const ezVec3 vDesired = (vOrbit - e.m_vPosition).GetNormalized() * 5.0f;
            e.m_vVelocity = ezMath::Lerp(e.m_vVelocity, vDesired, ezMath::Min(1.0f, fDeltaTime * 2.0f));

            if (e.m_fTimer <= 0.0f)
            {
              e.m_uiState = 1;
              e.m_fTimer = 0.9f;
              e.m_vVelocity = vToTarget.GetNormalized() * 13.0f;
              PlaySound(m_sSoundSkull2Dash, e.m_vPosition);
            }
          }
          else
          {
            if (e.m_fTimer <= 0.0f)
            {
              e.m_uiState = 0;
              e.m_fTimer = rng.FloatMinMax(2.5f, 4.0f);
            }
          }

          e.m_vPosition += e.m_vVelocity * fDeltaTime;
          break;
        }

        default:
          break;
      }
    }

    // fetch again, the squid spawns above may have reallocated the array
    Enemy& e = m_Enemies[i];

    // keep flying enemies above the floor
    if (e.m_Type == DcEnemyType::Skull1 || e.m_Type == DcEnemyType::Skull2 || e.m_Type == DcEnemyType::CentipedeHead)
    {
      const float fMinHeight = vCenter.z + GetEnemyRadius(e.m_Type);
      if (e.m_vPosition.z < fMinHeight)
      {
        e.m_vPosition.z = fMinHeight;
        e.m_vVelocity.z = ezMath::Max(e.m_vVelocity.z, 0.0f);
      }
    }

    // ground enemies walk on the floor tiles and fall when there is none
    if (IsGroundEnemy(e.m_Type))
    {
      if (e.m_fFallSpeed == 0.0f && IsOverFloor(e.m_vPosition))
      {
        e.m_vPosition.z = vCenter.z + GetEnemyRadius(e.m_Type);
      }
      else
      {
        e.m_fFallSpeed += 9.81f * fDeltaTime;
        e.m_vPosition.z -= e.m_fFallSpeed * fDeltaTime;

        if (e.m_vPosition.z < m_fKillHeight)
        {
          ezLog::Debug("DemonCove: {} fell off the arena", s_szEnemyNames[(int)e.m_Type]);
          e.m_bRemove = true;
        }
      }
    }

    if (e.m_fHitFlash > 0.0f)
    {
      e.m_fHitFlash = ezMath::Max(0.0f, e.m_fHitFlash - fDeltaTime * 6.0f);
      SetVisualColor(e.m_hObject, ezMath::Lerp(GetBaseColor(e.m_Type), ezColor(6.0f, 3.0f, 2.0f), e.m_fHitFlash));
    }

    ezGameObject* pObject = nullptr;
    if (GetWorld()->TryGetObject(e.m_hObject, pObject))
    {
      pObject->SetGlobalPosition(e.m_vPosition);

      switch (e.m_Type)
      {
        case DcEnemyType::Squid1:
        case DcEnemyType::Squid2:
          pObject->SetGlobalRotation(YawRotation(e.m_fYaw));
          break;

        case DcEnemyType::Skull1:
        case DcEnemyType::CentipedeHead:
        case DcEnemyType::CentipedeBody:
          pObject->SetGlobalRotation(LookRotation(e.m_vVelocity));
          break;

        case DcEnemyType::Skeleton:
        case DcEnemyType::Thief:
          pObject->SetGlobalRotation(YawRotation(e.m_vVelocity));
          break;

        default:
          pObject->SetGlobalRotation(YawRotation(vTarget - e.m_vPosition));
          break;
      }
    }
  }

  for (ezUInt32 i = m_Enemies.GetCount(); i > 0; --i)
  {
    if (m_Enemies[i - 1].m_bRemove)
      RemoveEnemy(i - 1);
  }
}

void DcArenaComponent::UpdateThief(Enemy& e, float fDeltaTime, const ezVec3& vTarget)
{
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();

  // head for the closest gem lying on the floor
  float fBestDistSqr = ezMath::MaxValue<float>();
  ezUInt32 uiBestGem = ezInvalidIndex;

  for (ezUInt32 g = 0; g < m_Gems.GetCount(); ++g)
  {
    const Gem& gem = m_Gems[g];
    if (gem.m_fAge < 1.0f || gem.m_vPosition.z > vCenter.z + 1.0f)
      continue;

    const float fDistSqr = (gem.m_vPosition - e.m_vPosition).GetLengthSquared();
    if (fDistSqr < fBestDistSqr)
    {
      fBestDistSqr = fDistSqr;
      uiBestGem = g;
    }
  }

  ezVec3 vGoal;
  float fSpeed = 3.5f;

  if (uiBestGem != ezInvalidIndex)
  {
    if (fBestDistSqr < 1.0f)
    {
      ++e.m_uiCarriedGems;
      PlaySound(m_sSoundThiefSteal, e.m_vPosition);
      DeleteVisual(m_Gems[uiBestGem].m_hObject);
      m_Gems.RemoveAtAndSwap(uiBestGem);
      return;
    }

    vGoal = m_Gems[uiBestGem].m_vPosition;
  }
  else
  {
    // nothing to collect, lurk around the player at a distance
    const float fAngle = e.m_fPhase + GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds() * 0.3f;
    vGoal = vTarget + ezVec3(ezMath::Cos(ezAngle::MakeFromRadian(fAngle)), ezMath::Sin(ezAngle::MakeFromRadian(fAngle)), 0) * 6.0f;
    fSpeed = 2.5f;

    // stay away from the edge
    ezVec3 vFromCenter = vGoal - vCenter;
    vFromCenter.z = 0;
    if (vFromCenter.GetLength() > m_fCurrentRadius - 2.0f)
      vGoal = vCenter + vFromCenter.GetNormalized() * (m_fCurrentRadius - 2.0f);
  }

  ezVec3 vToGoal = vGoal - e.m_vPosition;
  vToGoal.z = 0;
  e.m_vVelocity = ezMath::Lerp(e.m_vVelocity, vToGoal.GetNormalized() * fSpeed, ezMath::Min(1.0f, fDeltaTime * 3.0f));
  e.m_vVelocity.z = 0;
  e.m_vPosition += e.m_vVelocity * fDeltaTime;
}

void DcArenaComponent::UpdateCentipedeHead(Enemy& e, float fDeltaTime, const ezVec3& vTarget)
{
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();

  e.m_fTimer -= fDeltaTime;
  if (e.m_fTimer <= 0.0f || (e.m_vGoal - e.m_vPosition).GetLengthSquared() < 9.0f)
  {
    // alternate between sweeping through the arena and going for the player
    e.m_fTimer = rng.FloatMinMax(4.0f, 7.0f);
    if (rng.Bool())
    {
      e.m_vGoal = vTarget + ezVec3(0, 0, 0.5f);
    }
    else
    {
      const ezAngle angle = ezAngle::MakeFromRadian((float)rng.DoubleMinMax(0.0, ezMath::Pi<double>() * 2.0));
      const float fDist = rng.FloatMinMax(0.0f, m_fCurrentRadius * 0.8f);
      e.m_vGoal = vCenter + ezVec3(ezMath::Cos(angle) * fDist, ezMath::Sin(angle) * fDist, rng.FloatMinMax(2.0f, 8.0f));
    }
  }

  ezVec3 vDesired = (e.m_vGoal - e.m_vPosition).GetNormalized();

  // weave from side to side
  const ezVec3 vSide = vDesired.CrossRH(ezVec3(0, 0, 1)).GetNormalized();
  const float fTime = GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds();
  vDesired += vSide * ezMath::Sin(ezAngle::MakeFromRadian(fTime * 2.0f + e.m_fPhase)) * 0.7f;
  vDesired.NormalizeIfNotZero(ezVec3(1, 0, 0)).IgnoreResult();

  e.m_vVelocity = ezMath::Lerp(e.m_vVelocity, vDesired * 7.0f, ezMath::Min(1.0f, fDeltaTime * 1.5f));
  e.m_vPosition += e.m_vVelocity * fDeltaTime;
  e.m_vPosition.z = ezMath::Min(e.m_vPosition.z, vCenter.z + 10.0f);

  ezDynamicArray<ezVec3>* pTrail = nullptr;
  if (!m_CentipedeTrails.TryGetValue(e.m_uiGroup, pTrail))
    return;

  if ((e.m_vPosition - pTrail->PeekBack()).GetLengthSquared() >= s_fCentipedeTrailStep * s_fCentipedeTrailStep)
  {
    // the trail only needs to be as long as the body, drop the oldest point
    pTrail->RemoveAtAndCopy(0);
    pTrail->PushBack(e.m_vPosition);
  }
}

void DcArenaComponent::UpdateCentipedeBody(Enemy& e)
{
  const ezDynamicArray<ezVec3>* pTrail = nullptr;
  if (!m_CentipedeTrails.TryGetValue(e.m_uiGroup, pTrail))
  {
    // the head is gone
    e.m_bRemove = true;
    return;
  }

  const ezUInt32 uiCount = pTrail->GetCount();
  const ezUInt32 uiIndex = uiCount - 1 - ezMath::Min<ezUInt32>(uiCount - 1, e.m_uiSegment * s_uiCentipedeTrailStepsPerSegment);
  const ezUInt32 uiAhead = ezMath::Min(uiIndex + 1, uiCount - 1);

  e.m_vPosition = (*pTrail)[uiIndex];

  // only used for the orientation
  e.m_vVelocity = (*pTrail)[uiAhead] - e.m_vPosition;
}

void DcArenaComponent::SpawnDagger(const ezVec3& vPosition, const ezVec3& vDirection)
{
  // no daggers before picking up the first one
  if (m_bPlayerDead || m_bWaitingForStart)
    return;

  Dagger& d = m_Daggers.ExpandAndGetRef();
  // start a bit in front of the hand, right at the hand the dagger would fill the screen for a frame
  d.m_vPosition = vPosition + vDirection * 0.6f;
  d.m_vVelocity = vDirection * s_fDaggerSpeed;
  d.m_fLifeTime = s_fDaggerLifeTime;
  d.m_bHoming = GetHandLevel() >= 3;
  d.m_hObject = CreateVisual(m_hDaggerPrefab, d.m_vPosition, LookRotation(vDirection));

  // the trail is attached here and not in the dagger prefab, because that one is also used for the hand
  if (m_hDaggerTrailEffect.IsValid() && !d.m_hObject.IsInvalidated())
  {
    ezPrefabInstantiationOptions options;
    options.m_hParent = d.m_hObject;
    ezPrefabResource::InstantiatePrefab(m_hDaggerTrailEffect, false, *GetWorld(), ezTransform::MakeIdentity(), options);
  }

  ++m_uiDaggersFired;
}

void DcArenaComponent::UpdateDaggers(float fDeltaTime)
{
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();

  for (ezUInt32 i = 0; i < m_Daggers.GetCount();)
  {
    Dagger& d = m_Daggers[i];

    if (d.m_bHoming)
    {
      // steer towards the closest enemy in front of the dagger
      const ezVec3 vDir = d.m_vVelocity.GetNormalized();
      float fBestScore = 0.85f;
      ezVec3 vBestDir = vDir;

      for (const Enemy& e : m_Enemies)
      {
        ezVec3 vToEnemy = GetAimPosition(e, d.m_vPosition) - d.m_vPosition;
        const float fDist = vToEnemy.GetLengthAndNormalize();
        if (fDist > 25.0f)
          continue;

        const float fScore = vToEnemy.Dot(vDir);
        if (fScore > fBestScore)
        {
          fBestScore = fScore;
          vBestDir = vToEnemy;
        }
      }

      d.m_vVelocity = ezMath::Lerp(vDir, vBestDir, ezMath::Min(1.0f, fDeltaTime * 8.0f)).GetNormalized() * s_fDaggerSpeed;
    }

    const ezVec3 vStart = d.m_vPosition;
    const ezVec3 vEnd = vStart + d.m_vVelocity * fDeltaTime;
    d.m_fLifeTime -= fDeltaTime;

    bool bRemove = d.m_fLifeTime <= 0.0f;

    // daggers that hit the floor are gone
    if (vEnd.z < vCenter.z && IsOverFloor(vEnd))
    {
      bRemove = true;
    }

    if (!bRemove)
    {
      for (ezUInt32 e = 0; e < m_Enemies.GetCount(); ++e)
      {
        const Enemy& enemy = m_Enemies[e];

        // squids only take damage at their gems, the body just blocks daggers
        const ezUInt32 uiNumSpots = GetWeakSpotCount(enemy.m_Type);
        bool bWeakSpotHit = false;
        for (ezUInt32 s = 0; s < uiNumSpots; ++s)
        {
          const ezVec3 vSpot = GetWeakSpotPosition(enemy, s);
          if (SegmentHitsSphere(vStart, vEnd, vSpot, s_fWeakSpotRadius + 0.1f))
          {
            ++m_uiDaggersHit;
            PlaySound(m_sSoundWeakSpotHit, vSpot);
            DamageEnemy(e, 1.0f, vSpot);
            bWeakSpotHit = true;
            break;
          }
        }

        if (bWeakSpotHit)
        {
          bRemove = true;
          break;
        }

        if (SegmentHitsSphere(vStart, vEnd, enemy.m_vPosition, GetEnemyRadius(enemy.m_Type) + 0.1f))
        {
          if (uiNumSpots == 0)
          {
            ++m_uiDaggersHit;
            PlaySound(m_sSoundDaggerHit, enemy.m_vPosition);
            DamageEnemy(e, 1.0f, enemy.m_vPosition);
          }

          bRemove = true;
          break;
        }
      }
    }

    if (bRemove)
    {
      DeleteVisual(m_Daggers[i].m_hObject);
      m_Daggers.RemoveAtAndSwap(i);
      continue;
    }

    d.m_vPosition = vEnd;
    PlaceVisual(d.m_hObject, d.m_vPosition, LookRotation(d.m_vVelocity));
    ++i;
  }
}

void DcArenaComponent::DamageEnemy(ezUInt32 uiIndex, float fDamage, const ezVec3& vHitPosition)
{
  Enemy& e = m_Enemies[uiIndex];
  e.m_fHealth -= fDamage;
  e.m_fHitFlash = 1.0f;

  if (m_hHitEffect.IsValid())
  {
    ezPrefabResource::InstantiatePrefab(m_hHitEffect, false, *GetWorld(), ezTransform(vHitPosition));
  }

  if (e.m_fHealth <= 0.0f)
  {
    KillEnemy(uiIndex);
  }
}

void DcArenaComponent::RemoveEnemy(ezUInt32 uiIndex)
{
  if (m_Enemies[uiIndex].m_Type == DcEnemyType::CentipedeHead)
  {
    // the body segments notice that the trail is gone and remove themselves
    m_CentipedeTrails.Remove(m_Enemies[uiIndex].m_uiGroup);
  }

  DeleteVisual(m_Enemies[uiIndex].m_hObject);
  m_Enemies.RemoveAtAndSwap(uiIndex);
}

void DcArenaComponent::KillCentipedeBody(ezUInt32 uiGroup)
{
  // iterate backwards, KillEnemy() moves the last enemy into the freed slot
  for (ezUInt32 i = m_Enemies.GetCount(); i > 0; --i)
  {
    if (m_Enemies[i - 1].m_Type == DcEnemyType::CentipedeBody && m_Enemies[i - 1].m_uiGroup == uiGroup)
      KillEnemy(i - 1);
  }
}

void DcArenaComponent::KillEnemy(ezUInt32 uiIndex)
{
  const Enemy e = m_Enemies[uiIndex];
  m_Enemies.RemoveAtAndSwap(uiIndex);

  DeleteVisual(e.m_hObject);
  ++m_uiKills;

  const ezUInt32 uiNumGems = s_EnemyConfig[(int)e.m_Type].m_uiGemDrop + e.m_uiCarriedGems;
  for (ezUInt32 i = 0; i < uiNumGems; ++i)
  {
    SpawnGem(e.m_vPosition);
  }

  if (m_hDeathEffect.IsValid())
  {
    ezPrefabResource::InstantiatePrefab(m_hDeathEffect, false, *GetWorld(), ezTransform(e.m_vPosition));
  }

  switch (e.m_Type)
  {
    case DcEnemyType::Squid1:
    case DcEnemyType::Squid2:
      PlaySound(m_sSoundSquidDeath, e.m_vPosition);
      break;
    case DcEnemyType::Skull1:
      PlaySound(m_sSoundSkull1Death, e.m_vPosition);
      break;
    case DcEnemyType::Skull2:
      PlaySound(m_sSoundSkull2Death, e.m_vPosition);
      break;
    case DcEnemyType::Skeleton:
      PlaySound(m_sSoundSkeletonDeath, e.m_vPosition);
      break;
    case DcEnemyType::Thief:
      PlaySound(m_sSoundThiefDeath, e.m_vPosition);
      break;
    case DcEnemyType::CentipedeHead:
      PlaySound(m_sSoundCentipedeDeath, e.m_vPosition);
      break;
    default:
      // body segments die together with the head, its sound covers them
      break;
  }

  if (e.m_Type == DcEnemyType::CentipedeHead)
  {
    KillCentipedeBody(e.m_uiGroup);
    m_CentipedeTrails.Remove(e.m_uiGroup);
  }

  // big kills get some weight
  if (e.m_Type == DcEnemyType::Squid1 || e.m_Type == DcEnemyType::Squid2 || e.m_Type == DcEnemyType::CentipedeHead)
  {
    AddCameraShake(e.m_vPosition, 30.0f, 0.6f, ezTime::MakeFromMilliseconds(250));
    HitPause();
  }
}

void DcArenaComponent::SpawnGem(const ezVec3& vPosition)
{
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();

  Gem& g = m_Gems.ExpandAndGetRef();
  g.m_vPosition = vPosition;
  g.m_vVelocity = ezVec3(rng.FloatMinMax(-2, 2), rng.FloatMinMax(-2, 2), rng.FloatMinMax(2, 5));
  g.m_hObject = CreateVisual(m_hGemPrefab, vPosition, ezQuat::MakeIdentity());
}

void DcArenaComponent::UpdateGems(float fDeltaTime)
{
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  const DcPlayerComponent* pPlayer = m_bPlayerDead ? nullptr : GetPlayer();

  for (ezUInt32 i = 0; i < m_Gems.GetCount();)
  {
    Gem& g = m_Gems[i];
    g.m_fAge += fDeltaTime;

    bool bAttracted = false;
    if (pPlayer && g.m_fAge > 0.4f)
    {
      const ezVec3 vToPlayer = pPlayer->GetBodyCenter() - g.m_vPosition;
      const float fDist = vToPlayer.GetLength();

      if (fDist < s_fGemCollectRange)
      {
        ++m_uiGems;
        m_fGemPulse = 0.15f;
        PlaySound(m_sSoundGemPickup, g.m_vPosition);

        if (m_hGemPickupEffect.IsValid())
        {
          ezPrefabResource::InstantiatePrefab(m_hGemPickupEffect, false, *GetWorld(), ezTransform(g.m_vPosition));
        }

        DeleteVisual(g.m_hObject);
        m_Gems.RemoveAtAndSwap(i);
        continue;
      }

      if (fDist < cvar_GemAttractRange)
      {
        bAttracted = true;
        g.m_vVelocity = vToPlayer.GetNormalized() * 18.0f;
      }
    }

    if (!bAttracted && !IsOverFloor(g.m_vPosition) && g.m_vPosition.z > vCenter.z - 2.0f)
    {
      // a gem dropped over the void floats back above the floor instead of being lost, losing it would feel unfair
      ezVec3 vToCenter = vCenter - g.m_vPosition;
      vToCenter.z = 0;
      const ezVec3 vDesired = vToCenter.GetNormalized() * 5.0f + ezVec3(0, 0, (vCenter.z + 1.0f - g.m_vPosition.z) * 2.0f);
      g.m_vVelocity = ezMath::Lerp(g.m_vVelocity, vDesired, ezMath::Min(1.0f, fDeltaTime * 3.0f));
    }
    else if (!bAttracted)
    {
      g.m_vVelocity.z -= 9.81f * fDeltaTime;
      g.m_vVelocity.x *= ezMath::Max(0.0f, 1.0f - fDeltaTime);
      g.m_vVelocity.y *= ezMath::Max(0.0f, 1.0f - fDeltaTime);
    }

    g.m_vPosition += g.m_vVelocity * fDeltaTime;

    const float fFloor = vCenter.z + 0.3f;
    if (!bAttracted && g.m_vPosition.z < fFloor && g.m_vPosition.z > vCenter.z - 0.5f && IsOverFloor(g.m_vPosition))
    {
      g.m_vPosition.z = fFloor;
      g.m_vVelocity.z = ezMath::Abs(g.m_vVelocity.z) * 0.3f;
    }

    if (g.m_vPosition.z < m_fKillHeight)
    {
      DeleteVisual(g.m_hObject);
      m_Gems.RemoveAtAndSwap(i);
      continue;
    }

    const ezQuat qSpin = ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), ezAngle::MakeFromRadian(g.m_fAge * 3.0f));
    PlaceVisual(g.m_hObject, g.m_vPosition, qSpin);
    ++i;
  }
}

void DcArenaComponent::UpdatePlayer()
{
  DcPlayerComponent* pPlayer = GetPlayer();
  if (pPlayer == nullptr)
    return;

  // the character teleport on restart only takes effect with the next physics update
  if (m_RoundTime - m_RoundStartTime < ezTime::MakeFromMilliseconds(250))
    return;

  const ezVec3 vBody = pPlayer->GetBodyCenter();

  if (vBody.z < m_fKillHeight)
  {
    if (cvar_GodMode)
    {
      pPlayer->Respawn(m_vPlayerStart, pPlayer->GetOwner()->GetGlobalRotation());
      return;
    }

    KillPlayer("fell into the abyss");
    return;
  }

  for (ezUInt32 i = 0; i < m_Enemies.GetCount(); ++i)
  {
    const Enemy& e = m_Enemies[i];
    const float fRange = GetEnemyRadius(e.m_Type) + s_fPlayerRadius;

    // treat the player as a vertical capsule
    ezVec3 vClosest = vBody;
    vClosest.z = ezMath::Clamp(e.m_vPosition.z, vBody.z - 0.6f, vBody.z + 0.8f);

    if ((e.m_vPosition - vClosest).GetLengthSquared() < fRange * fRange)
    {
      if (cvar_GodMode)
      {
        // otherwise everything piles up around an invincible player
        RemoveEnemy(i);
        --i;
        continue;
      }

      ezStringBuilder sReason;
      sReason.SetFormat("killed by {}", s_szEnemyNames[(int)e.m_Type]);
      KillPlayer(sReason);
      return;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// Floor

namespace
{
  const ezColor s_TileColor(0.3f, 0.26f, 0.25f);
  const ezColor s_TileWarnColor(1.0f, 0.3f, 0.2f);
} // namespace

void DcArenaComponent::BuildFloor()
{
  // the floor in the scene is only there for editing, the round uses tiles that can fall off
  ezGameObject* pSceneFloor = nullptr;
  if (GetWorld()->TryGetObjectWithGlobalKey("ArenaFloor", pSceneFloor))
  {
    pSceneFloor->SetActiveFlag(false);
  }

  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  m_iTileGridHalfSize = (ezInt32)ezMath::Ceil(m_fArenaRadius / s_fTileSize);
  const ezInt32 iGridSize = m_iTileGridHalfSize * 2 + 1;

  m_Tiles.Clear();
  m_TileGrid.Clear();
  m_TileGrid.SetCount(iGridSize * iGridSize, ezInvalidIndex);

  for (ezInt32 y = -m_iTileGridHalfSize; y <= m_iTileGridHalfSize; ++y)
  {
    for (ezInt32 x = -m_iTileGridHalfSize; x <= m_iTileGridHalfSize; ++x)
    {
      const ezVec3 vOffset(x * s_fTileSize, y * s_fTileSize, 0);
      const float fDistance = vOffset.GetLength();
      if (fDistance > m_fArenaRadius - s_fTileSize * 0.25f)
        continue;

      m_TileGrid[(y + m_iTileGridHalfSize) * iGridSize + (x + m_iTileGridHalfSize)] = m_Tiles.GetCount();

      Tile& tile = m_Tiles.ExpandAndGetRef();
      tile.m_vPosition = vCenter + vOffset;
      tile.m_fDistance = fDistance;
      tile.m_hObject = CreateTileObject(tile, true);
    }
  }

  m_fCurrentRadius = m_fArenaRadius;
}

ezGameObjectHandle DcArenaComponent::CreateTileObject(const Tile& tile, bool bWithCollision)
{
  constexpr float fThickness = 1.5f;
  const float fHalfSize = s_fTileSize * 0.5f - 0.02f;

  ezGameObjectDesc desc;
  desc.m_sName.Assign("FloorTile");
  desc.m_LocalPosition = tile.m_vPosition;
  desc.m_bDynamic = !bWithCollision; // falling tiles are moved every frame

  ezGameObject* pTile = nullptr;
  const ezGameObjectHandle hTile = GetWorld()->CreateObject(desc, pTile);

  ezGreyBoxComponent* pBox = nullptr;
  GetWorld()->GetOrCreateComponentManager<ezGreyBoxComponentManager>()->CreateComponent(pTile, pBox);
  pBox->SetShape(ezGreyBoxShape::Box);
  pBox->SetSizeNegX(fHalfSize);
  pBox->SetSizePosX(fHalfSize);
  pBox->SetSizeNegY(fHalfSize);
  pBox->SetSizePosY(fHalfSize);
  pBox->SetSizeNegZ(fThickness);
  pBox->SetSizePosZ(0.0f);
  pBox->SetColor(tile.m_bWarned ? s_TileWarnColor : s_TileColor);

  if (bWithCollision)
  {
    ezGameObjectDesc shapeDesc;
    shapeDesc.m_hParent = hTile;
    shapeDesc.m_LocalPosition.Set(0, 0, -fThickness * 0.5f);

    ezGameObject* pShapeObj = nullptr;
    GetWorld()->CreateObject(shapeDesc, pShapeObj);

    ezJoltShapeBoxComponent* pShape = nullptr;
    GetWorld()->GetOrCreateComponentManager<ezJoltShapeBoxComponentManager>()->CreateComponent(pShapeObj, pShape);
    pShape->SetHalfExtents(ezVec3(s_fTileSize * 0.5f, s_fTileSize * 0.5f, fThickness * 0.5f));

    ezJoltStaticActorComponent* pActor = nullptr;
    GetWorld()->GetOrCreateComponentManager<ezJoltStaticActorComponentManager>()->CreateComponent(pTile, pActor);
  }

  return hTile;
}

void DcArenaComponent::ResetFloor()
{
  for (Tile& tile : m_Tiles)
  {
    if (!tile.m_bFallen && !tile.m_bWarned)
      continue;

    DeleteVisual(tile.m_hObject);

    tile.m_bFallen = false;
    tile.m_bWarned = false;
    tile.m_fFallSpeed = 0.0f;
    tile.m_hObject = CreateTileObject(tile, true);
  }

  m_fCurrentRadius = m_fArenaRadius;
}

void DcArenaComponent::UpdateFloor(float fDeltaTime)
{
  // the arena shrinks during the endless phase
  const float fShrinkTime = m_RoundTime.AsFloatInSeconds() - s_StageStartTimes[s_uiNumStages - 1];
  if (fShrinkTime > 0.0f)
  {
    m_fCurrentRadius = ezMath::Max(m_fMinArenaRadius, m_fArenaRadius - fShrinkTime * m_fShrinkSpeed);
  }

  for (Tile& tile : m_Tiles)
  {
    if (tile.m_bFallen)
    {
      ezGameObject* pObject = nullptr;
      if (!GetWorld()->TryGetObject(tile.m_hObject, pObject))
        continue;

      tile.m_fFallSpeed += 6.0f * fDeltaTime;
      pObject->SetGlobalPosition(pObject->GetGlobalPosition() - ezVec3(0, 0, tile.m_fFallSpeed * fDeltaTime));

      if (pObject->GetGlobalPosition().z < m_fKillHeight * 4.0f)
      {
        DeleteVisual(tile.m_hObject);
        tile.m_hObject.Invalidate();
      }

      continue;
    }

    if (tile.m_fDistance > m_fCurrentRadius)
    {
      // replace the tile with one without collision, that falls down
      DeleteVisual(tile.m_hObject);
      tile.m_bFallen = true;
      tile.m_fFallSpeed = 0.0f;
      tile.m_hObject = CreateTileObject(tile, false);
      continue;
    }

    if (!tile.m_bWarned && fShrinkTime > 0.0f && tile.m_fDistance > m_fCurrentRadius - s_fTileWarnDistance)
    {
      tile.m_bWarned = true;

      ezGameObject* pObject = nullptr;
      ezGreyBoxComponent* pBox = nullptr;
      if (GetWorld()->TryGetObject(tile.m_hObject, pObject) && pObject->TryGetComponentOfBaseType(pBox))
      {
        pBox->SetColor(s_TileWarnColor);
      }
    }
  }
}

bool DcArenaComponent::IsOverFloor(const ezVec3& vPosition) const
{
  const ezVec3 vRel = vPosition - GetOwner()->GetGlobalPosition();
  const ezInt32 x = (ezInt32)ezMath::Round(vRel.x / s_fTileSize) + m_iTileGridHalfSize;
  const ezInt32 y = (ezInt32)ezMath::Round(vRel.y / s_fTileSize) + m_iTileGridHalfSize;
  const ezInt32 iGridSize = m_iTileGridHalfSize * 2 + 1;

  if (x < 0 || y < 0 || x >= iGridSize || y >= iGridSize)
    return false;

  const ezUInt32 uiTile = m_TileGrid[y * iGridSize + x];
  return uiTile != ezInvalidIndex && !m_Tiles[uiTile].m_bFallen;
}

void DcArenaComponent::EnterStartState()
{
  LeaveStartState();

  // make sure the player start is known
  GetPlayer();

  m_bWaitingForStart = true;
  m_fStartStateTime = 0.0f;

  // a few steps in front of the player, at chest height
  ezVec3 vForward = m_qPlayerStartRotation * ezVec3(1, 0, 0);
  vForward.z = 0;
  vForward.NormalizeIfNotZero(ezVec3(1, 0, 0)).IgnoreResult();
  m_vStartDaggerPosition = m_vPlayerStart + vForward * 5.0f + ezVec3(0, 0, 1.2f);

  m_hStartDagger = CreateVisual(m_hDaggerPrefab, m_vStartDaggerPosition, ezQuat::MakeIdentity());

  // a bigger dagger is easier to spot
  ezGameObject* pDagger = nullptr;
  if (GetWorld()->TryGetObject(m_hStartDagger, pDagger))
  {
    pDagger->SetLocalUniformScaling(2.5f);
  }
}

void DcArenaComponent::LeaveStartState()
{
  m_bWaitingForStart = false;
  DeleteVisual(m_hStartDagger);
  m_hStartDagger.Invalidate();
}

void DcArenaComponent::UpdateStartState(float fDeltaTime)
{
  m_fStartStateTime += fDeltaTime;

  // the dagger points down and slowly turns and bobs
  const ezQuat qPointDown = ezQuat::MakeFromAxisAndAngle(ezVec3(0, 1, 0), ezAngle::MakeFromDegree(90));
  const ezQuat qSpin = ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), ezAngle::MakeFromRadian(m_fStartStateTime * 1.5f));
  const ezVec3 vPos = m_vStartDaggerPosition + ezVec3(0, 0, ezMath::Sin(ezAngle::MakeFromRadian(m_fStartStateTime * 2.0f)) * 0.15f);
  PlaceVisual(m_hStartDagger, vPos, qSpin * qPointDown);

  DcPlayerComponent* pPlayer = GetPlayer();
  if (pPlayer == nullptr)
    return;

  const ezVec3 vBody = pPlayer->GetBodyCenter();

  if (vBody.z < m_fKillHeight)
  {
    pPlayer->Respawn(m_vPlayerStart, m_qPlayerStartRotation);
    return;
  }

  // swept along the movement since the last frame, a long frame could otherwise step right over the dagger
  const ezVec3 vLastBody = m_fStartStateTime > fDeltaTime ? m_vLastPlayerBody : vBody;
  m_vLastPlayerBody = vBody;

  const ezVec3 vDaggerAtBodyHeight(vPos.x, vPos.y, ezMath::Clamp(vPos.z, vBody.z - 0.8f, vBody.z + 0.8f)); // the body is a capsule
  if (SegmentHitsSphere(vLastBody, vBody, vDaggerAtBodyHeight, 1.2f))
  {
    LeaveStartState();
    m_RoundStartTime = m_RoundTime;
    PlaySound(m_sSoundDaggerPickup, vBody);
    PlaySound(m_sSoundRoundStart, vBody);
  }
}

void DcArenaComponent::UpdateSafetyWall()
{
  const bool bHasWall = !m_hSafetyWall.IsInvalidated();
  if (bHasWall == cvar_GodMode)
    return;

  if (bHasWall)
  {
    GetWorld()->DeleteObjectDelayed(m_hSafetyWall);
    m_hSafetyWall.Invalidate();
    return;
  }

  // a ring of boxes along the arena edge
  constexpr ezUInt32 uiNumSegments = 48;
  constexpr float fHeight = 6.0f;
  constexpr float fThickness = 0.5f;
  const float fSegmentHalfLength = m_fArenaRadius * ezMath::Pi<float>() / uiNumSegments * 1.1f;

  ezGameObjectDesc desc;
  desc.m_sName.Assign("SafetyWall");
  desc.m_LocalPosition = GetOwner()->GetGlobalPosition();

  ezGameObject* pRoot = nullptr;
  m_hSafetyWall = GetWorld()->CreateObject(desc, pRoot);

  auto* pShapeManager = GetWorld()->GetOrCreateComponentManager<ezJoltShapeBoxComponentManager>();

  for (ezUInt32 i = 0; i < uiNumSegments; ++i)
  {
    const ezAngle angle = ezAngle::MakeFromDegree(360.0f * i / uiNumSegments);

    ezGameObjectDesc segDesc;
    segDesc.m_hParent = m_hSafetyWall;
    segDesc.m_LocalPosition = ezVec3(ezMath::Cos(angle), ezMath::Sin(angle), 0) * (m_fArenaRadius + fThickness) + ezVec3(0, 0, fHeight * 0.5f);
    segDesc.m_LocalRotation = ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), angle);

    ezGameObject* pSegment = nullptr;
    GetWorld()->CreateObject(segDesc, pSegment);

    ezJoltShapeBoxComponent* pBox = nullptr;
    pShapeManager->CreateComponent(pSegment, pBox);
    pBox->SetHalfExtents(ezVec3(fThickness, fSegmentHalfLength, fHeight * 0.5f));
  }

  ezJoltStaticActorComponent* pActor = nullptr;
  GetWorld()->GetOrCreateComponentManager<ezJoltStaticActorComponentManager>()->CreateComponent(pRoot, pActor);
}

namespace
{
  constexpr const char* s_szScoresFile = ":appdata/Scores.txt";
  constexpr ezUInt32 s_uiMaxScores = 10;
} // namespace

bool DcArenaComponent::IsRegularRound()
{
  return !cvar_GodMode && !cvar_DebugAutoAim && cvar_StartStage <= 1 && cvar_StartGems <= 0;
}

void DcArenaComponent::LoadScores()
{
  m_Scores.Clear();

  ezFileReader file;
  if (file.Open(s_szScoresFile).Failed())
    return;

  ezStringBuilder sContent;
  sContent.ReadAll(file);

  // one round per line: time kills gems
  ezHybridArray<ezStringView, 16> lines;
  sContent.Split(false, lines, "\n", "\r");

  for (ezStringView sLine : lines)
  {
    ezHybridArray<ezStringView, 4> parts;
    sLine.Split(false, parts, " ");

    double fTime = 0.0;
    ezInt64 iKills = 0, iGems = 0;
    if (parts.GetCount() != 3 || ezConversionUtils::StringToFloat(parts[0], fTime).Failed() ||
        ezConversionUtils::StringToInt64(parts[1], iKills).Failed() || ezConversionUtils::StringToInt64(parts[2], iGems).Failed())
      continue;

    Score& score = m_Scores.ExpandAndGetRef();
    score.m_fTime = (float)fTime;
    score.m_uiKills = (ezUInt32)iKills;
    score.m_uiGems = (ezUInt32)iGems;

    if (m_Scores.GetCount() == s_uiMaxScores)
      break;
  }

  m_Scores.Sort([](const Score& a, const Score& b)
    { return a.m_fTime > b.m_fTime; });
}

void DcArenaComponent::SaveScores()
{
  ezFileWriter file;
  if (file.Open(s_szScoresFile).Failed())
  {
    ezLog::Warning("DemonCove: could not save the scoreboard to '{}'", s_szScoresFile);
    return;
  }

  ezStringBuilder sContent;
  for (const Score& score : m_Scores)
  {
    sContent.AppendFormat("{} {} {}\n", ezArgF(score.m_fTime, 4), score.m_uiKills, score.m_uiGems);
  }

  file.WriteBytes(sContent.GetData(), sContent.GetElementCount()).IgnoreResult();
}

void DcArenaComponent::AddScore()
{
  const float fTime = m_RoundTime.AsFloatInSeconds();

  ezUInt32 uiIndex = 0;
  while (uiIndex < m_Scores.GetCount() && m_Scores[uiIndex].m_fTime >= fTime)
    ++uiIndex;

  if (uiIndex >= s_uiMaxScores)
    return;

  Score score;
  score.m_fTime = fTime;
  score.m_uiKills = m_uiKills;
  score.m_uiGems = m_uiGems;
  m_Scores.InsertAt(uiIndex, score);

  if (m_Scores.GetCount() > s_uiMaxScores)
    m_Scores.PopBack();

  m_uiScorePlace = uiIndex + 1;

  SaveScores();
  UpdateHudScores();
}

void DcArenaComponent::UpdateHudScores()
{
  if (m_pHud == nullptr)
    return;

  ezVariantArray scores;
  ezStringBuilder sTmp;

  for (ezUInt32 i = 0; i < m_Scores.GetCount(); ++i)
  {
    ezVariantDictionary entry;
    entry["rank"] = (ezInt32)(i + 1);
    sTmp.SetFormat("{}", ezArgF(m_Scores[i].m_fTime, 4));
    entry["time"] = ezString(sTmp);
    entry["kills"] = (ezInt32)m_Scores[i].m_uiKills;
    entry["gems"] = (ezInt32)m_Scores[i].m_uiGems;
    entry["current"] = (i + 1 == m_uiScorePlace);
    scores.PushBack(entry);
  }

  m_pHud->SetEntryValue("scores", scores);
  m_pHud->SetEntryValue("place", (ezInt32)m_uiScorePlace);
  m_pHud->SetEntryValue("hasscores", !m_Scores.IsEmpty());
}

bool DcArenaComponent::UpdateHudBlackboard()
{
  if (m_pHud == nullptr)
    return false;

  const char* szHand[] = {"I", "II", "III"};
  const float fAccuracy = m_uiDaggersFired > 0 ? 100.0f * m_uiDaggersHit / m_uiDaggersFired : 0.0f;

  // strings have to be owning ezString variants, the data binding doesn't support string views
  auto format = [](const ezFormatString& fmt)
  {
    ezStringBuilder sTmp;
    return ezVariant(ezString(fmt.GetText(sTmp)));
  };

  m_pHud->SetEntryValue("time", format(ezFmt("{}", ezArgF(m_RoundTime.AsFloatInSeconds(), 4))));
  m_pHud->SetEntryValue("accuracy", format(ezFmt("{}", ezArgF(fAccuracy, 1))));
  m_pHud->SetEntryValue("stage", (ezInt32)GetStage());
  m_pHud->SetEntryValue("gems", (ezInt32)m_uiGems);
  m_pHud->SetEntryValue("hand", ezString(szHand[ezMath::Clamp<ezUInt32>(GetHandLevel(), 1, 3) - 1]));
  m_pHud->SetEntryValue("kills", (ezInt32)m_uiKills);
  m_pHud->SetEntryValue("dead", m_bPlayerDead);
  m_pHud->SetEntryValue("reason", m_sDeathReason);
  m_pHud->SetEntryValue("godmode", (bool)cvar_GodMode);
  m_pHud->SetEntryValue("waiting", m_bWaitingForStart);
  m_pHud->SetEntryValue("gempulse", m_fGemPulse > 0.0f);
  m_pHud->SetEntryValue("levelup", m_fLevelUpTime > 0.0f);

  if (!m_bHudBound)
  {
    // the HUD is an RmlUi canvas on an object with this global key
    ezGameObject* pHudObject = nullptr;
    ezRmlUiCanvas2DComponent* pCanvas = nullptr;
    if (!GetWorld()->TryGetObjectWithGlobalKey("Hud", pHudObject) || !pHudObject->TryGetComponentOfBaseType(pCanvas))
      return false;

    pCanvas->AddBlackboardBinding(m_pHud);
    m_bHudBound = true;
  }

  return true;
}

void DcArenaComponent::UpdateHud()
{
  const ezDebugRendererContext ctx(GetWorld());
  const bool bHasHud = UpdateHudBlackboard();
  const ezVec2 vRes = ezScriptExtensionClass_Debug::GetResolution();
  const ezInt32 iCenterX = (ezInt32)(vRes.x * 0.5f);
  const ezInt32 iCenterY = (ezInt32)(vRes.y * 0.5f);

  // fallback visualization for anything without a prefab
  for (const Enemy& e : m_Enemies)
  {
    if (!m_hPrefabs[(int)e.m_Type].IsValid())
    {
      ezDebugRenderer::DrawLineSphere(ctx, ezBoundingSphere::MakeFromCenterAndRadius(e.m_vPosition, GetEnemyRadius(e.m_Type)), ezColor::OrangeRed);
    }
  }
  if (!m_hDaggerPrefab.IsValid())
  {
    for (const Dagger& d : m_Daggers)
    {
      const ezDebugRendererLine line(d.m_vPosition, d.m_vPosition - d.m_vVelocity.GetNormalized() * 0.5f);
      ezDebugRenderer::DrawLines(ctx, ezMakeArrayPtr(&line, 1), ezColor::White);
    }
  }
  if (!m_hGemPrefab.IsValid())
  {
    for (const Gem& g : m_Gems)
      ezDebugRenderer::DrawLineSphere(ctx, ezBoundingSphere::MakeFromCenterAndRadius(g.m_vPosition, 0.15f), ezColor::Red);
  }

  ezStringBuilder sText;

  if (!bHasHud)
  {
    sText.SetFormat("{}", ezArgF(m_RoundTime.AsFloatInSeconds(), 4));
    ezDebugRenderer::Draw2DText(ctx, sText.GetView(), ezVec2I32(iCenterX, 20), m_bPlayerDead ? ezColor::Red : ezColor::White, 32, ezDebugTextHAlign::Center);

    sText.SetFormat("Stage {}   Gems: {}   Hand: Lv {}   Enemies: {}{}", GetStage(), m_uiGems, GetHandLevel(), m_Enemies.GetCount(), cvar_GodMode ? "   [GOD MODE]" : "");
    ezDebugRenderer::Draw2DText(ctx, sText.GetView(), ezVec2I32(20, 20), ezColor::IndianRed, 18);
  }

  if (cvar_ShowStats)
  {
    const ezVec3 vPlayerPos = GetPlayer() ? GetPlayer()->GetOwner()->GetGlobalPosition() : ezVec3::MakeZero();
    sText.SetFormat("Daggers fired: {}  hit: {}  alive: {}   Kills: {}   Gems lying: {}   Arena: {}m   Player: {} {} {}", m_uiDaggersFired, m_uiDaggersHit, m_Daggers.GetCount(), m_uiKills, m_Gems.GetCount(), ezArgF(m_fCurrentRadius, 1), ezArgF(vPlayerPos.x, 1), ezArgF(vPlayerPos.y, 1), ezArgF(vPlayerPos.z, 1));
    ezDebugRenderer::Draw2DText(ctx, sText.GetView(), ezVec2I32(20, 160), ezColor::Gray, 16);

    ezUInt32 uiCounts[(int)DcEnemyType::Count] = {};
    for (const Enemy& e : m_Enemies)
      ++uiCounts[(int)e.m_Type];

    sText.Clear();
    for (ezUInt32 t = 0; t < (ezUInt32)DcEnemyType::Count; ++t)
    {
      if (uiCounts[t] > 0)
        sText.AppendFormat("{}: {}   ", s_szEnemyNames[t], uiCounts[t]);
    }
    ezDebugRenderer::Draw2DText(ctx, sText.GetView(), ezVec2I32(20, 180), ezColor::Gray, 16);
  }

  if (bHasHud)
    return;

  if (m_bPlayerDead)
  {
    const float fAccuracy = m_uiDaggersFired > 0 ? 100.0f * m_uiDaggersHit / m_uiDaggersFired : 0.0f;
    sText.SetFormat("YOU DIED - {}\nTime: {}s   Best: {}s\nKills: {}   Gems: {}   Accuracy: {}%%\n\nPress R to restart", m_sDeathReason, ezArgF(m_RoundTime.AsFloatInSeconds(), 4), ezArgF(m_Scores.IsEmpty() ? 0.0f : m_Scores[0].m_fTime, 4), m_uiKills, m_uiGems, ezArgF(fAccuracy, 1));
    ezDebugRenderer::Draw2DText(ctx, sText.GetView(), ezVec2I32(iCenterX, iCenterY - 60), ezColor::Red, 28, ezDebugTextHAlign::Center);
  }
  else
  {
    ezDebugRenderer::Draw2DText(ctx, "+", ezVec2I32(iCenterX, iCenterY), ezColor::White, 20, ezDebugTextHAlign::Center, ezDebugTextVAlign::Center);
  }
}

void DcArenaComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  ezReflectionSerializer::WriteObjectToBinary(inout_stream.GetStream(), GetDynamicRTTI(), this);
}

void DcArenaComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  ezReflectionSerializer::ReadObjectPropertiesFromBinary(inout_stream.GetStream(), *GetDynamicRTTI(), this);
}
