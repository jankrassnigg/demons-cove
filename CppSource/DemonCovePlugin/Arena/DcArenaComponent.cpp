#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <DemonCovePlugin/Arena/DcArenaComponent.h>
#include <DemonCovePlugin/Arena/DcArenaFloorComponent.h>
#include <DemonCovePlugin/Common/DcCVars.h>
#include <DemonCovePlugin/Common/DcUtils.h>
#include <DemonCovePlugin/Player/DcPlayerComponent.h>
#include <GameComponentsPlugin/Effects/Shake/CameraShakeVolumeComponent.h>
#include <RendererCore/Debug/DebugRenderer.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(DcArenaComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Squid1Prefab", m_sSquid1Prefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package), new ezGroupAttribute("Enemies")),
    EZ_MEMBER_PROPERTY("Squid2Prefab", m_sSquid2Prefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("Skull1Prefab", m_sSkull1Prefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("Skull2Prefab", m_sSkull2Prefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SkeletonPrefab", m_sSkeletonPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("ThiefPrefab", m_sThiefPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("CentipedeHeadPrefab", m_sCentipedeHeadPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("CentipedeBodyPrefab", m_sCentipedeBodyPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("DaggerPrefab", m_sDaggerPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package), new ezGroupAttribute("Objects")),
    EZ_MEMBER_PROPERTY("GemPrefab", m_sGemPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("DeathEffect", m_sDeathEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package), new ezGroupAttribute("Effects")),
    EZ_MEMBER_PROPERTY("HitEffect", m_sHitEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SpawnWarningEffect", m_sSpawnWarningEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("GemPickupEffect", m_sGemPickupEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("DaggerTrailEffect", m_sDaggerTrailEffect)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("MinArenaRadius", m_fMinArenaRadius)->AddAttributes(new ezDefaultValueAttribute(9.0f), new ezClampValueAttribute(2.0f, 200.0f), new ezGroupAttribute("Rules")),
    EZ_MEMBER_PROPERTY("ShrinkSpeed", m_fShrinkSpeed)->AddAttributes(new ezDefaultValueAttribute(0.08f), new ezClampValueAttribute(0.0f, 10.0f)),
    EZ_MEMBER_PROPERTY("KillHeight", m_fKillHeight)->AddAttributes(new ezDefaultValueAttribute(-8.0f)),
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

namespace
{
  /// The 'restart' action is configured in RuntimeConfigs/InputConfig.ddl, so it can be rebound like any other action.
  constexpr const char* s_szInputSet = "Default";

  /// Round time at which each stage begins. The last stage is the endless phase that starts after the scripted timeline.
  constexpr float s_StageStartTimes[] = {0.0f, 14.0f, 29.0f, 39.0f, 57.0f, 84.0f};
  constexpr ezUInt32 s_uiNumStages = EZ_ARRAY_SIZE(s_StageStartTimes);
  constexpr float s_fEndlessPhaseStart = s_StageStartTimes[s_uiNumStages - 1];

  constexpr ezUInt32 s_uiGemsForLevel2 = 10;
  constexpr ezUInt32 s_uiGemsForLevel3 = 50;

  /// The player is treated as a vertical capsule with this radius for enemy contact.
  constexpr float s_fPlayerRadius = 0.45f;
} // namespace

DcArenaComponent::DcArenaComponent() = default;
DcArenaComponent::~DcArenaComponent() = default;

DcArenaComponent* DcArenaComponent::FindArena(ezWorld* pWorld)
{
  if (pWorld == nullptr)
    return nullptr;

  // the component manager knows all components of its type, no need to search the scene graph
  DcArenaComponentManager* pManager = pWorld->GetComponentManager<DcArenaComponentManager>();
  if (pManager == nullptr)
    return nullptr;

  for (auto it = pManager->GetComponents(); it.IsValid(); ++it)
  {
    if (it->IsActiveAndSimulating())
      return &(*it);
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Setup

void DcArenaComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  // load the prefabs once, spawning just instantiates them
  m_hEnemyPrefabs[(int)DcEnemyType::Squid1] = DcUtils::LoadPrefab(m_sSquid1Prefab, "Squid1Prefab");
  m_hEnemyPrefabs[(int)DcEnemyType::Squid2] = DcUtils::LoadPrefab(m_sSquid2Prefab, "Squid2Prefab");
  m_hEnemyPrefabs[(int)DcEnemyType::Skull1] = DcUtils::LoadPrefab(m_sSkull1Prefab, "Skull1Prefab");
  m_hEnemyPrefabs[(int)DcEnemyType::Skull2] = DcUtils::LoadPrefab(m_sSkull2Prefab, "Skull2Prefab");
  m_hEnemyPrefabs[(int)DcEnemyType::Skeleton] = DcUtils::LoadPrefab(m_sSkeletonPrefab, "SkeletonPrefab");
  m_hEnemyPrefabs[(int)DcEnemyType::Thief] = DcUtils::LoadPrefab(m_sThiefPrefab, "ThiefPrefab");
  m_hEnemyPrefabs[(int)DcEnemyType::CentipedeHead] = DcUtils::LoadPrefab(m_sCentipedeHeadPrefab, "CentipedeHeadPrefab");
  m_hEnemyPrefabs[(int)DcEnemyType::CentipedeBody] = DcUtils::LoadPrefab(m_sCentipedeBodyPrefab, "CentipedeBodyPrefab");
  m_hDaggerPrefab = DcUtils::LoadPrefab(m_sDaggerPrefab, "DaggerPrefab");
  m_hGemPrefab = DcUtils::LoadPrefab(m_sGemPrefab, "GemPrefab");
  m_hDeathEffect = DcUtils::LoadPrefab(m_sDeathEffect, "DeathEffect");
  m_hHitEffect = DcUtils::LoadPrefab(m_sHitEffect, "HitEffect");
  m_hSpawnWarningEffect = DcUtils::LoadPrefab(m_sSpawnWarningEffect, "SpawnWarningEffect");
  m_hGemPickupEffect = DcUtils::LoadPrefab(m_sGemPickupEffect, "GemPickupEffect");
  m_hDaggerTrailEffect = DcUtils::LoadPrefab(m_sDaggerTrailEffect, "DaggerTrailEffect");

  // everything that appears in the middle of a round
  for (const ezPrefabResourceHandle& hPrefab : m_hEnemyPrefabs)
    DcUtils::PreloadPrefab(*GetWorld(), hPrefab);
  DcUtils::PreloadPrefab(*GetWorld(), m_hDaggerPrefab);
  DcUtils::PreloadPrefab(*GetWorld(), m_hGemPrefab);
  DcUtils::PreloadPrefab(*GetWorld(), m_hHitEffect);
  DcUtils::PreloadPrefab(*GetWorld(), m_hSpawnWarningEffect);
  DcUtils::PreloadPrefab(*GetWorld(), m_hGemPickupEffect);

  // The sibling components are looked up once and referenced by handle. Handles stay safe to use when a component gets
  // deleted, pointers would dangle.
  if (DcArenaFloorComponent* pFloor = nullptr; GetOwner()->TryGetComponentOfBaseType(pFloor))
  {
    m_hFloor = pFloor->GetHandle();

    // the floor builds its tiles when its simulation starts, make sure that happened before the first round
    pFloor->EnsureSimulationStarted();
  }
  else
  {
    ezLog::Error("DcArenaComponent: there is no DcArenaFloorComponent on object '{}'", GetOwner()->GetName());
  }

  if (DcAudioComponent* pAudio = nullptr; GetOwner()->TryGetComponentOfBaseType(pAudio))
  {
    m_hAudio = pAudio->GetHandle();
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

  m_Scoreboard.Load();

  RestartRound();
  EnterStartState();
}

void DcArenaComponent::OnDeactivated()
{
  // the game objects are deleted together with the world, only the bookkeeping has to go
  m_Enemies.Clear();
  m_Daggers.Clear();
  m_Gems.Clear();
  m_CentipedeTrails.Clear();
  m_PendingSpawns.Clear();

  if (m_HitPauseEnd.IsPositive())
  {
    GetWorld()->GetClock().SetSpeed(1.0);
    m_HitPauseEnd = ezTime::MakeZero();
  }

  SUPER::OnDeactivated();
}

//////////////////////////////////////////////////////////////////////////
// Accessors

DcPlayerComponent* DcArenaComponent::GetPlayer() const
{
  // const_cast: looking something up doesn't modify the world, but the world only hands out non-const components to non-const callers
  ezWorld* pWorld = const_cast<ezWorld*>(GetWorld());

  DcPlayerComponent* pPlayer = nullptr;
  if (pWorld->TryGetComponent(m_hPlayer, pPlayer))
    return pPlayer;

  // The player is spawned by the game state from a prefab, so it can't be referenced in the scene.
  // Find it once and remember its handle and where it started.
  DcPlayerComponentManager* pManager = pWorld->GetComponentManager<DcPlayerComponentManager>();
  if (pManager == nullptr)
    return nullptr;

  for (auto it = pManager->GetComponents(); it.IsValid(); ++it)
  {
    if (!it->IsActiveAndSimulating())
      continue;

    pPlayer = &(*it);

    auto* pThis = const_cast<DcArenaComponent*>(this);
    pThis->m_hPlayer = pPlayer->GetHandle();
    pThis->m_vPlayerStart = pPlayer->GetOwner()->GetGlobalPosition();
    pThis->m_qPlayerStartRotation = pPlayer->GetOwner()->GetGlobalRotation();
    return pPlayer;
  }

  return nullptr;
}

ezVec3 DcArenaComponent::GetPlayerTarget() const
{
  if (const DcPlayerComponent* pPlayer = GetPlayer())
    return pPlayer->GetBodyCenter() + ezVec3(0, 0, 0.4f);

  return GetOwner()->GetGlobalPosition() + ezVec3(0, 0, 1.5f);
}

DcArenaFloorComponent* DcArenaComponent::GetFloor() const
{
  DcArenaFloorComponent* pFloor = nullptr;
  return const_cast<ezWorld*>(GetWorld())->TryGetComponent(m_hFloor, pFloor) ? pFloor : nullptr;
}

bool DcArenaComponent::IsOverFloor(const ezVec3& vPosition) const
{
  const DcArenaFloorComponent* pFloor = GetFloor();
  return pFloor != nullptr && pFloor->IsOverFloor(vPosition);
}

float DcArenaComponent::GetFloorRadius() const
{
  const DcArenaFloorComponent* pFloor = GetFloor();
  return pFloor ? pFloor->m_fRadius : 20.0f;
}

float DcArenaComponent::GetCurrentFloorRadius() const
{
  const DcArenaFloorComponent* pFloor = GetFloor();
  return pFloor ? pFloor->GetCurrentRadius() : 20.0f;
}

ezUInt32 DcArenaComponent::GetHandLevel() const
{
  if (m_uiGems >= s_uiGemsForLevel3)
    return 3;
  if (m_uiGems >= s_uiGemsForLevel2)
    return 2;
  return 1;
}

float DcArenaComponent::GetAccuracy() const
{
  return m_uiDaggersFired > 0 ? (float)m_uiDaggersHit / m_uiDaggersFired : 0.0f;
}

ezUInt32 DcArenaComponent::GetStage() const
{
  const float fTime = m_RoundTime.AsFloatInSeconds();

  ezUInt32 uiStage = 1;
  while (uiStage < s_uiNumStages && s_StageStartTimes[uiStage] <= fTime)
    ++uiStage;

  return uiStage;
}

void DcArenaComponent::PlaySound(DcSound sound, const ezVec3& vPosition)
{
  DcAudioComponent* pAudio = nullptr;
  if (GetWorld()->TryGetComponent(m_hAudio, pAudio))
  {
    pAudio->Play(sound, vPosition);
  }
}

void DcArenaComponent::AddCameraShake(const ezVec3& vPosition, float fRadius, float fStrength, ezTime duration)
{
  // A shake volume in burst mode. The ezCameraShakeComponent on the player's camera samples all volumes it is inside of.
  // The volume deletes itself once the burst is over.
  ezGameObjectDesc desc;
  desc.m_LocalPosition = vPosition;

  ezGameObject* pObject = nullptr;
  GetWorld()->CreateObject(desc, pObject);

  ezCameraShakeVolumeSphereComponent* pShake = nullptr;
  ezCameraShakeVolumeSphereComponent::CreateComponent(pObject, pShake);
  pShake->SetRadius(fRadius);
  pShake->m_fStrength = fStrength;
  pShake->m_BurstDuration = duration;
  pShake->m_OnFinishedAction = ezOnComponentFinishedAction::DeleteGameObject;
}

void DcArenaComponent::HitPause()
{
  // slowing down the world clock affects everything that uses it: physics, animations, particles and this component
  GetWorld()->GetClock().SetSpeed(0.05);
  m_HitPauseEnd = ezTime::Now() + ezTime::MakeFromMilliseconds(60);
}

//////////////////////////////////////////////////////////////////////////
// Round flow

void DcArenaComponent::RestartRound()
{
  LeaveStartState();
  RemoveAllEnemies();
  RemoveAllDaggersAndGems();
  ClearPendingSpawns();

  if (DcArenaFloorComponent* pFloor = GetFloor())
  {
    pFloor->Restore();
  }

  const ezUInt32 uiStartStage = (ezUInt32)ezMath::Clamp<ezInt32>(cvar_StartStage, 1, s_uiNumStages);
  const float fStartTime = s_StageStartTimes[uiStartStage - 1];

  // skip all spawns of earlier stages
  m_uiNextTimelineEvent = 0;
  while (m_uiNextTimelineEvent < m_Timeline.GetCount() && m_Timeline[m_uiNextTimelineEvent].m_fTime < fStartTime)
    ++m_uiNextTimelineEvent;

  m_fNextEndlessWave = s_fEndlessPhaseStart;
  m_uiEndlessWave = 0;
  m_RoundTime = ezTime::MakeFromSeconds(fStartTime);
  m_RoundStartTime = m_RoundTime;
  m_uiGems = (ezUInt32)ezMath::Max<ezInt32>(cvar_StartGems, 0);
  m_uiLastHandLevel = GetHandLevel();
  m_uiKills = 0;
  m_uiDaggersFired = 0;
  m_uiDaggersHit = 0;
  m_bPlayerDead = false;
  m_sDeathReason.Clear();
  m_uiScoreboardPlace = 0;
  m_bDebugRound = false;

  if (DcPlayerComponent* pPlayer = GetPlayer())
  {
    pPlayer->Respawn(m_vPlayerStart, m_qPlayerStartRotation);
  }
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
    PlaySound(DcSound::PlayerDeath, pPlayer->GetBodyCenter());
  }

  if (!m_bDebugRound && DcIsRegularRound())
  {
    DcScoreboard::Entry round;
    round.m_fTime = m_RoundTime.AsFloatInSeconds();
    round.m_uiKills = m_uiKills;
    round.m_uiGems = m_uiGems;
    m_uiScoreboardPlace = m_Scoreboard.AddRound(round);
  }

  ezLog::Info("DemonCove: player died after {} seconds ({}), {} kills, {} gems", ezArgF(m_RoundTime.AsFloatInSeconds(), 4), sReason, m_uiKills, m_uiGems);
}

void DcArenaComponent::Update()
{
  if (DcArenaFloorComponent* pFloor = GetFloor())
  {
    pFloor->SetSafetyWall(cvar_GodMode);
  }

  if (ezInputManager::GetInputActionState(s_szInputSet, "restart") == ezKeyState::Pressed)
  {
    RestartRound();
    PlaySound(DcSound::RoundStart, m_vPlayerStart);
  }

  if (m_HitPauseEnd.IsPositive() && ezTime::Now() >= m_HitPauseEnd)
  {
    GetWorld()->GetClock().SetSpeed(1.0);
    m_HitPauseEnd = ezTime::MakeZero();
  }

  // clamped, so that a hitch doesn't make everything jump
  const float fDeltaTime = ezMath::Min(GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds(), 0.1f);

  if (GetHandLevel() > m_uiLastHandLevel)
  {
    PlaySound(DcSound::LevelUp, GetPlayerTarget());
  }
  m_uiLastHandLevel = GetHandLevel();

  if (m_bWaitingForStart)
  {
    UpdateStartState(fDeltaTime);
  }
  else if (!m_bPlayerDead)
  {
    m_RoundTime += ezTime::MakeFromSeconds(fDeltaTime);
    UpdateTimeline();
    UpdatePendingSpawns(fDeltaTime);
  }

  // the arena crumbles during the endless phase
  if (DcArenaFloorComponent* pFloor = GetFloor())
  {
    const float fShrinkTime = m_RoundTime.AsFloatInSeconds() - s_fEndlessPhaseStart;
    if (fShrinkTime > 0.0f)
    {
      pFloor->SetCrumbleRadius(ezMath::Max(m_fMinArenaRadius, pFloor->m_fRadius - fShrinkTime * m_fShrinkSpeed));
    }
  }

  UpdateDebugSpawns();

  UpdateEnemies(fDeltaTime);
  UpdateDaggers(fDeltaTime);
  UpdateGems(fDeltaTime);

  if (!m_bPlayerDead && !m_bWaitingForStart)
  {
    UpdatePlayerCollision();
  }

  if (DcAudioComponent* pAudio = nullptr; GetWorld()->TryGetComponent(m_hAudio, pAudio))
  {
    pAudio->SetActionMusic(!m_bWaitingForStart && !m_bPlayerDead);
  }

  if (cvar_ShowStats)
  {
    DrawDebugStats();
  }
}

void DcArenaComponent::UpdateTimeline()
{
  const float fTime = m_RoundTime.AsFloatInSeconds();

  while (m_uiNextTimelineEvent < m_Timeline.GetCount() && m_Timeline[m_uiNextTimelineEvent].m_fTime <= fTime)
  {
    const SpawnEvent& ev = m_Timeline[m_uiNextTimelineEvent];
    SpawnTimelineEnemies(ev.m_Type, ev.m_uiCount);
    ++m_uiNextTimelineEvent;
  }

  if (m_uiNextTimelineEvent < m_Timeline.GetCount() || fTime < m_fNextEndlessWave)
    return;

  // endless phase: alternate between waves of weak and tough spawners, with shrinking intervals
  ++m_uiEndlessWave;

  const ezUInt32 uiExtra = m_uiEndlessWave / 4;
  if (m_uiEndlessWave % 2 == 0)
  {
    for (ezUInt32 i = 0; i < 1 + uiExtra; ++i)
      SpawnSquid(DcEnemyType::Squid2);
  }
  else
  {
    for (ezUInt32 i = 0; i < 2 + uiExtra; ++i)
      SpawnSquid(DcEnemyType::Squid1);
  }

  if (m_uiEndlessWave % 3 == 0)
    SpawnSkeletons(7 + uiExtra);

  if (m_uiEndlessWave % 4 == 2)
    SpawnThief();

  if (m_uiEndlessWave % 5 == 0)
    SpawnCentipede(CentipedeSegments + uiExtra * 2);

  m_fNextEndlessWave = fTime + ezMath::Max(9.0f, 16.0f - m_uiEndlessWave * 0.4f);
}

void DcArenaComponent::UpdatePlayerCollision()
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
    const DcEnemyTypeInfo& info = DcGetEnemyTypeInfo(e.m_Type);
    const float fRange = info.m_fRadius + s_fPlayerRadius;

    // closest point on the player's capsule axis
    ezVec3 vClosest = vBody;
    vClosest.z = ezMath::Clamp(e.m_vPosition.z, vBody.z - 0.6f, vBody.z + 0.8f);

    if ((e.m_vPosition - vClosest).GetLengthSquared() >= fRange * fRange)
      continue;

    if (cvar_GodMode)
    {
      // otherwise everything piles up around an invincible player
      RemoveEnemy(i);
      --i;
      continue;
    }

    ezStringBuilder sReason;
    sReason.SetFormat("killed by {}", info.m_szName);
    KillPlayer(sReason);
    return;
  }
}

//////////////////////////////////////////////////////////////////////////
// Start state

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

  m_hStartDagger = DcUtils::SpawnPrefab(*GetWorld(), m_hDaggerPrefab, m_vStartDaggerPosition, ezQuat::MakeIdentity());

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
  DcUtils::DeleteObject(*GetWorld(), m_hStartDagger);
  m_hStartDagger.Invalidate();
}

void DcArenaComponent::UpdateStartState(float fDeltaTime)
{
  m_fStartStateTime += fDeltaTime;

  // the dagger points down and slowly turns and bobs
  const ezQuat qPointDown = ezQuat::MakeFromAxisAndAngle(ezVec3(0, 1, 0), ezAngle::MakeFromDegree(90));
  const ezQuat qSpin = ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), ezAngle::MakeFromRadian(m_fStartStateTime * 1.5f));
  const ezVec3 vPos = m_vStartDaggerPosition + ezVec3(0, 0, ezMath::Sin(ezAngle::MakeFromRadian(m_fStartStateTime * 2.0f)) * 0.15f);
  DcUtils::PlaceObject(*GetWorld(), m_hStartDagger, vPos, qSpin * qPointDown);

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
  if (DcUtils::SegmentHitsSphere(vLastBody, vBody, vDaggerAtBodyHeight, 1.2f))
  {
    LeaveStartState();
    m_RoundStartTime = m_RoundTime;
    PlaySound(DcSound::DaggerPickup, vBody);
    PlaySound(DcSound::RoundStart, vBody);
  }
}

//////////////////////////////////////////////////////////////////////////
// Debugging

void DcArenaComponent::UpdateDebugSpawns()
{
  if (!DcIsRegularRound())
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
    const float fRadius = GetCurrentFloorRadius();
    auto randomPos = [&](float fMinHeight, float fMaxHeight)
    {
      const ezAngle angle = ezAngle::MakeFromRadian(rng.FloatMinMax(0.0f, ezMath::Pi<float>() * 2.0f));
      const float fDist = rng.FloatMinMax(6.0f, fRadius - 3.0f);
      return vCenter + ezVec3(ezMath::Cos(angle) * fDist, ezMath::Sin(angle) * fDist, rng.FloatMinMax(fMinHeight, fMaxHeight));
    };

    for (ezUInt32 i = 0; i < 3; ++i)
      SpawnEnemy(DcEnemyType::Squid1, randomPos(3.5f, 6.0f));
    for (ezUInt32 i = 0; i < 3; ++i)
      SpawnEnemy(DcEnemyType::Squid2, randomPos(3.5f, 6.0f));
    for (ezUInt32 i = 0; i < MaxSkull1; ++i)
      SpawnEnemy(DcEnemyType::Skull1, randomPos(1.0f, 8.0f));
    for (ezUInt32 i = 0; i < 10; ++i)
      SpawnEnemy(DcEnemyType::Skull2, randomPos(2.0f, 5.0f));
    for (ezUInt32 i = 0; i < 20; ++i)
      SpawnEnemy(DcEnemyType::Skeleton, randomPos(0.5f, 0.5f));
    for (ezUInt32 i = 0; i < 2; ++i)
      SpawnEnemy(DcEnemyType::Thief, randomPos(0.7f, 0.7f));
    for (ezUInt32 i = 0; i < 2; ++i)
      SpawnCentipedeAt(randomPos(4.0f, 7.0f) + ezVec3(30, 0, 0), CentipedeSegments + 4);
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
}

void DcArenaComponent::DrawDebugStats()
{
  const ezDebugRendererContext ctx(GetWorld());
  const ezVec3 vPlayerPos = GetPlayer() ? GetPlayer()->GetOwner()->GetGlobalPosition() : ezVec3::MakeZero();

  ezStringBuilder sText;
  sText.SetFormat("Daggers fired: {}  hit: {}  alive: {}   Kills: {}   Gems lying: {}   Arena: {}m   Player: {} {} {}", m_uiDaggersFired, m_uiDaggersHit, m_Daggers.GetCount(), m_uiKills, m_Gems.GetCount(), ezArgF(GetCurrentFloorRadius(), 1), ezArgF(vPlayerPos.x, 1), ezArgF(vPlayerPos.y, 1), ezArgF(vPlayerPos.z, 1));
  ezDebugRenderer::Draw2DText(ctx, sText.GetView(), ezVec2I32(20, 160), ezColor::Gray, 16);

  ezUInt32 uiCounts[(int)DcEnemyType::Count] = {};
  for (const Enemy& e : m_Enemies)
    ++uiCounts[(int)e.m_Type];

  sText.Clear();
  for (ezUInt32 t = 0; t < (ezUInt32)DcEnemyType::Count; ++t)
  {
    if (uiCounts[t] > 0)
      sText.AppendFormat("{}: {}   ", DcGetEnemyTypeInfo((DcEnemyType)t).m_szName, uiCounts[t]);
  }
  ezDebugRenderer::Draw2DText(ctx, sText.GetView(), ezVec2I32(20, 180), ezColor::Gray, 16);
}

//////////////////////////////////////////////////////////////////////////
// Serialization

void DcArenaComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  // writes all reflected properties, so this doesn't need to change when properties are added or removed
  ezReflectionSerializer::WriteObjectToBinary(inout_stream.GetStream(), GetDynamicRTTI(), this);
}

void DcArenaComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  ezReflectionSerializer::ReadObjectPropertiesFromBinary(inout_stream.GetStream(), *GetDynamicRTTI(), this);
}
