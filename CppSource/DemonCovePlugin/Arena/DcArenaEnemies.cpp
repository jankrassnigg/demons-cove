#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <DemonCovePlugin/Arena/DcArenaComponent.h>
#include <DemonCovePlugin/Common/DcUtils.h>
#include <DemonCovePlugin/Player/DcPlayerComponent.h>

// Enemy spawning, behaviors and damage. See DcArenaComponent.h for an overview.

namespace
{
  /// How long the spawn warning effect plays before the enemy appears.
  constexpr float s_fSpawnWarningTime = 1.5f;

  constexpr float s_fCentipedeTrailStep = 0.25f; ///< Distance between recorded trail points.
  constexpr ezUInt32 s_uiCentipedeTrailStepsPerSegment = 4;

  const ezColor s_HitFlashColor(6.0f, 3.0f, 2.0f);

  ezAngle RandomAngle(ezRandom& ref_rng)
  {
    return ezAngle::MakeFromRadian((float)ref_rng.DoubleMinMax(0.0, ezMath::Pi<double>() * 2.0));
  }

  /// A horizontal offset of the given length in the direction of the angle.
  ezVec3 CircleOffset(ezAngle angle, float fDistance)
  {
    return ezVec3(ezMath::Cos(angle), ezMath::Sin(angle), 0) * fDistance;
  }

  /// Sine wave for movement noise, phase-shifted per enemy so that they don't move in sync.
  float Wobble(float fTime, float fFrequency, float fPhase)
  {
    return ezMath::Sin(ezAngle::MakeFromRadian(fTime * fFrequency + fPhase));
  }
} // namespace

//////////////////////////////////////////////////////////////////////////
// Spawning

void DcArenaComponent::SpawnTimelineEnemies(DcEnemyType type, ezUInt32 uiCount)
{
  switch (type)
  {
    case DcEnemyType::Skeleton:
      // skeletons always come as one group
      SpawnSkeletons(uiCount);
      break;

    case DcEnemyType::Thief:
      for (ezUInt32 i = 0; i < uiCount; ++i)
        SpawnThief();
      break;

    case DcEnemyType::CentipedeHead:
      for (ezUInt32 i = 0; i < uiCount; ++i)
        SpawnCentipede(CentipedeSegments);
      break;

    default:
      // Squids float in from outside the arena. Skulls are normally released by squids,
      // but come in the same way when spawned through the 'DemonCove.DebugSpawn' CVar.
      for (ezUInt32 i = 0; i < uiCount; ++i)
        SpawnSquid(type);
      break;
  }
}

void DcArenaComponent::SpawnSquid(DcEnemyType type)
{
  // outside the arena, they float in from there (see UpdateSquid())
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();
  const ezVec3 vPos = GetOwner()->GetGlobalPosition() + CircleOffset(RandomAngle(rng), GetFloorRadius() + 6.0f) + ezVec3(0, 0, rng.FloatMinMax(3.5f, 6.0f));
  QueueSpawn(type, vPos, 1);
}

void DcArenaComponent::SpawnSkeletons(ezUInt32 uiCount)
{
  // a group climbs onto the platform at one spot of the edge,
  // far enough away that the spread out group doesn't reach the player either
  const ezVec3 vPos = PickFloorSpawnPosition(3.0f, 12.0f) + ezVec3(0, 0, 1.0f);
  QueueSpawn(DcEnemyType::Skeleton, vPos, uiCount);
}

void DcArenaComponent::SpawnThief()
{
  const ezVec3 vPos = PickFloorSpawnPosition(3.0f, 10.0f) + ezVec3(0, 0, DcGetEnemyTypeInfo(DcEnemyType::Thief).m_fRadius);
  QueueSpawn(DcEnemyType::Thief, vPos, 1);
}

void DcArenaComponent::SpawnCentipede(ezUInt32 uiSegments)
{
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();
  const ezVec3 vHeadPos = GetOwner()->GetGlobalPosition() + CircleOffset(RandomAngle(rng), GetFloorRadius() + 8.0f) + ezVec3(0, 0, 6.0f);
  QueueSpawn(DcEnemyType::CentipedeHead, vHeadPos, uiSegments);
}

ezVec3 DcArenaComponent::PickFloorSpawnPosition(float fInset, float fMinPlayerDistance) const
{
  ezRandom& rng = const_cast<ezWorld*>(GetWorld())->GetRandomNumberGenerator();
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  const DcPlayerComponent* pPlayer = GetPlayer();
  const float fDist = ezMath::Max(1.0f, GetCurrentFloorRadius() - fInset);

  // a few tries are enough to find a spot away from the player, otherwise just take the last one
  ezVec3 vPos = vCenter;
  for (ezUInt32 uiTry = 0; uiTry < 32; ++uiTry)
  {
    vPos = vCenter + CircleOffset(RandomAngle(rng), fDist);

    if (pPlayer == nullptr)
      break;

    ezVec3 vToPlayer = pPlayer->GetBodyCenter() - vPos;
    vToPlayer.z = 0;
    if (vToPlayer.GetLength() >= fMinPlayerDistance)
      break;
  }

  return vPos;
}

void DcArenaComponent::QueueSpawn(DcEnemyType type, const ezVec3& vPosition, ezUInt32 uiCount)
{
  PendingSpawn& spawn = m_PendingSpawns.ExpandAndGetRef();
  spawn.m_Type = type;
  spawn.m_uiCount = uiCount;
  spawn.m_vPosition = vPosition;
  spawn.m_fTimeLeft = s_fSpawnWarningTime;
  spawn.m_hWarning = DcUtils::SpawnPrefab(*GetWorld(), m_hSpawnWarningEffect, vPosition, ezQuat::MakeIdentity());

  PlaySound(DcSound::SpawnWarning, vPosition);
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
    if (DcGetEnemyTypeInfo(spawn.m_Type).m_bWalksOnGround)
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
    DcUtils::DeleteObject(*GetWorld(), due.m_hWarning);

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
    DcUtils::DeleteObject(*GetWorld(), spawn.m_hWarning);

  m_PendingSpawns.Clear();
}

void DcArenaComponent::SpawnEnemy(DcEnemyType type, const ezVec3& vPosition)
{
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();
  const DcEnemyTypeInfo& info = DcGetEnemyTypeInfo(type);

  Enemy& e = m_Enemies.ExpandAndGetRef();
  e.m_Type = type;
  e.m_vPosition = vPosition;
  e.m_fHealth = info.m_fHealth;
  e.m_fPhase = rng.FloatMinMax(0.0f, 100.0f);
  e.m_fYaw = rng.FloatMinMax(0.0f, ezMath::Pi<float>() * 2.0f);

  switch (type)
  {
    case DcEnemyType::Squid1:
    case DcEnemyType::Squid2:
      e.m_fTimer = 3.0f; // first swarm
      PlaySound(DcSound::SquidSpawn, vPosition);
      break;

    case DcEnemyType::Skull1:
      // burst out of the spawner in a random direction
      e.m_vVelocity = ezVec3(rng.FloatMinMax(-1, 1), rng.FloatMinMax(-1, 1), rng.FloatMinMax(0.2f, 1.0f)).GetNormalized() * 4.0f;
      break;

    case DcEnemyType::Skull2:
      e.m_fTimer = rng.FloatMinMax(2.0f, 4.0f); // first dash
      break;

    default:
      break;
  }

  e.m_hObject = DcUtils::SpawnPrefab(*GetWorld(), m_hEnemyPrefabs[(int)type], vPosition, ezQuat::MakeIdentity());

  if (info.m_Tint != ezColor::White)
  {
    DcUtils::SetMeshColor(*GetWorld(), e.m_hObject, info.m_Tint);
  }

  // the weak spots are gems attached to the visual, which rotates with the enemy's yaw
  for (ezUInt32 i = 0; i < info.m_uiWeakSpots && !e.m_hObject.IsInvalidated(); ++i)
  {
    const ezVec3 vLocalPos = DcUtils::YawRotation(-e.m_fYaw) * (GetWeakSpotPosition(e, i) - e.m_vPosition);
    const ezGameObjectHandle hSpot = DcUtils::SpawnPrefab(*GetWorld(), m_hGemPrefab, vLocalPos, ezQuat::MakeIdentity(), e.m_hObject);

    ezGameObject* pSpot = nullptr;
    if (GetWorld()->TryGetObject(hSpot, pSpot))
    {
      pSpot->SetLocalUniformScaling(1.6f);
    }
  }
}

void DcArenaComponent::SpawnSkeletonGroup(const ezVec3& vPosition, ezUInt32 uiCount)
{
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  const ezVec3 vRel = vPosition - vCenter;
  const float fBaseAngle = ezMath::ATan2(vRel.y, vRel.x).GetRadian();

  // spread along the edge around the spawn position
  for (ezUInt32 i = 0; i < uiCount; ++i)
  {
    const ezAngle angle = ezAngle::MakeFromRadian(fBaseAngle + rng.FloatMinMax(-0.25f, 0.25f));
    const float fDist = GetCurrentFloorRadius() - rng.FloatMinMax(2.0f, 4.0f);
    SpawnEnemy(DcEnemyType::Skeleton, vCenter + CircleOffset(angle, fDist) + ezVec3(0, 0, DcGetEnemyTypeInfo(DcEnemyType::Skeleton).m_fRadius));
  }
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

//////////////////////////////////////////////////////////////////////////
// Behaviors

void DcArenaComponent::UpdateEnemies(float fDeltaTime)
{
  const ezVec3 vTarget = GetPlayerTarget();

  // Spawners append new enemies while iterating, so iterate by index over the count at the start.
  // The new ones get their first update next frame.
  const ezUInt32 uiNumEnemies = m_Enemies.GetCount();
  for (ezUInt32 i = 0; i < uiNumEnemies; ++i)
  {
    switch (m_Enemies[i].m_Type)
    {
      case DcEnemyType::Squid1:
      case DcEnemyType::Squid2:
        // takes the index, because spawning skulls may reallocate the array and invalidate references
        UpdateSquid(i, fDeltaTime);
        break;
      case DcEnemyType::Skull1:
        UpdateSkull1(m_Enemies[i], fDeltaTime, vTarget);
        break;
      case DcEnemyType::Skull2:
        UpdateSkull2(m_Enemies[i], fDeltaTime, vTarget);
        break;
      case DcEnemyType::Skeleton:
        UpdateSkeleton(m_Enemies[i], fDeltaTime, vTarget);
        break;
      case DcEnemyType::Thief:
        UpdateThief(m_Enemies[i], fDeltaTime, vTarget);
        break;
      case DcEnemyType::CentipedeHead:
        UpdateCentipedeHead(m_Enemies[i], fDeltaTime, vTarget);
        break;
      case DcEnemyType::CentipedeBody:
        UpdateCentipedeBody(m_Enemies[i]);
        break;
      default:
        break;
    }

    FinishEnemyUpdate(m_Enemies[i], fDeltaTime, vTarget);
  }

  // backwards, removing moves the last enemy into the freed slot
  for (ezUInt32 i = m_Enemies.GetCount(); i > 0; --i)
  {
    if (m_Enemies[i - 1].m_bRemove)
      RemoveEnemy(i - 1);
  }
}

void DcArenaComponent::UpdateSquid(ezUInt32 uiIndex, float fDeltaTime)
{
  Enemy& e = m_Enemies[uiIndex];
  const float fTime = GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds();
  const float fCircleRadius = GetCurrentFloorRadius() * 0.7f;

  ezVec3 vToCenter = GetOwner()->GetGlobalPosition() - e.m_vPosition;
  vToCenter.z = 0;
  const float fDist = vToCenter.GetLength();

  if (e.m_uiState == 0)
  {
    // float in from outside the arena
    e.m_vVelocity = vToCenter.GetNormalized() * 2.0f;
    if (fDist < fCircleRadius)
      e.m_uiState = 1;
  }
  else
  {
    // slowly circle around the arena center
    const ezVec3 vTangent = vToCenter.CrossRH(ezVec3(0, 0, 1)).GetNormalized();
    e.m_vVelocity = vTangent * 0.8f + vToCenter.GetNormalized() * (fDist - fCircleRadius) * 0.2f;
  }

  e.m_vVelocity.z = Wobble(fTime, 0.7f, e.m_fPhase) * 0.4f;
  e.m_vPosition += e.m_vVelocity * fDeltaTime;

  // the slow rotation hides the gems from time to time
  e.m_fYaw += fDeltaTime * ((e.m_Type == DcEnemyType::Squid1) ? 0.45f : -0.35f);

  e.m_fTimer -= fDeltaTime;
  if (e.m_fTimer > 0.0f || m_bPlayerDead)
    return;

  e.m_fTimer = (e.m_Type == DcEnemyType::Squid1) ? 14.0f : 16.0f;

  // copy, the spawns below may reallocate the array and 'e' becomes invalid
  const ezVec3 vSpawnPos = e.m_vPosition;
  const DcEnemyType spawnerType = e.m_Type;

  ezUInt32 uiNumSkull1 = 0;
  for (const Enemy& other : m_Enemies)
    uiNumSkull1 += (other.m_Type == DcEnemyType::Skull1) ? 1 : 0;

  const ezUInt32 uiSwarmSize = ezMath::Min<ezUInt32>(7, MaxSkull1 - ezMath::Min(uiNumSkull1, MaxSkull1));

  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();
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
    PlaySound(DcSound::SquidSwarm, vSpawnPos);
  }
}

void DcArenaComponent::UpdateSkull1(Enemy& e, float fDeltaTime, const ezVec3& vTarget)
{
  const float fTime = GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds();

  // home in on the player with a limited turn rate and some wobble, so they can be dodged
  ezVec3 vDesired = (vTarget - e.m_vPosition).GetNormalized();
  const ezVec3 vSide = vDesired.CrossRH(ezVec3(0, 0, 1)).GetNormalized();
  vDesired += vSide * Wobble(fTime, 2.3f, e.m_fPhase) * 0.6f;
  vDesired.z += Wobble(fTime, 1.7f, e.m_fPhase * 1.3f) * 0.4f;
  vDesired.NormalizeIfNotZero(ezVec3(1, 0, 0)).IgnoreResult();

  const float fSpeed = 6.0f + ezMath::Sin(ezAngle::MakeFromRadian(e.m_fPhase)); // each skull has a slightly different speed
  e.m_vVelocity = ezMath::Lerp(e.m_vVelocity, vDesired * fSpeed, ezMath::Min(1.0f, fDeltaTime * 1.4f));
  e.m_vPosition += e.m_vVelocity * fDeltaTime;
}

void DcArenaComponent::UpdateSkull2(Enemy& e, float fDeltaTime, const ezVec3& vTarget)
{
  e.m_fTimer -= fDeltaTime;

  if (e.m_uiState == 0)
  {
    // circle the player at a distance
    const float fTime = GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds();
    const ezAngle angle = ezAngle::MakeFromRadian(fTime * 0.6f + e.m_fPhase);
    const ezVec3 vOrbit = vTarget + CircleOffset(angle, 7.0f) + ezVec3(0, 0, 2.5f);
    const ezVec3 vDesired = (vOrbit - e.m_vPosition).GetNormalized() * 5.0f;
    e.m_vVelocity = ezMath::Lerp(e.m_vVelocity, vDesired, ezMath::Min(1.0f, fDeltaTime * 2.0f));

    if (e.m_fTimer <= 0.0f)
    {
      // dash in a straight line towards where the player is now
      e.m_uiState = 1;
      e.m_fTimer = 0.9f;
      e.m_vVelocity = (vTarget - e.m_vPosition).GetNormalized() * 13.0f;
      PlaySound(DcSound::Skull2Dash, e.m_vPosition);
    }
  }
  else if (e.m_fTimer <= 0.0f)
  {
    e.m_uiState = 0;
    e.m_fTimer = GetWorld()->GetRandomNumberGenerator().FloatMinMax(2.5f, 4.0f);
  }

  e.m_vPosition += e.m_vVelocity * fDeltaTime;
}

void DcArenaComponent::UpdateSkeleton(Enemy& e, float fDeltaTime, const ezVec3& vTarget)
{
  // run along the floor, turning towards the player at a limited rate, so that it can be outrun with sharp turns
  ezVec3 vToTarget = vTarget - e.m_vPosition;
  vToTarget.z = 0;
  const float fSpeed = 5.0f + ezMath::Sin(ezAngle::MakeFromRadian(e.m_fPhase)) * 0.7f;
  const ezVec3 vDesired = vToTarget.GetNormalized() * fSpeed;
  e.m_vVelocity = ezMath::Lerp(e.m_vVelocity, vDesired, ezMath::Min(1.0f, fDeltaTime * 2.5f));
  e.m_vVelocity.z = 0;
  e.m_vPosition += e.m_vVelocity * fDeltaTime;
}

void DcArenaComponent::UpdateThief(Enemy& e, float fDeltaTime, const ezVec3& vTarget)
{
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();

  // head for the closest gem lying on the floor, gems still in the air don't count
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
      // collected, dropped again when the thief dies
      ++e.m_uiCarriedGems;
      PlaySound(DcSound::ThiefSteal, e.m_vPosition);
      DcUtils::DeleteObject(*GetWorld(), m_Gems[uiBestGem].m_hObject);
      m_Gems.RemoveAtAndSwap(uiBestGem);
      return;
    }

    vGoal = m_Gems[uiBestGem].m_vPosition;
  }
  else
  {
    // nothing to collect, lurk around the player at a distance
    const float fTime = GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds();
    vGoal = vTarget + CircleOffset(ezAngle::MakeFromRadian(e.m_fPhase + fTime * 0.3f), 6.0f);
    fSpeed = 2.5f;

    // stay away from the edge
    const float fMaxDist = GetCurrentFloorRadius() - 2.0f;
    ezVec3 vFromCenter = vGoal - vCenter;
    vFromCenter.z = 0;
    if (vFromCenter.GetLength() > fMaxDist)
      vGoal = vCenter + vFromCenter.GetNormalized() * fMaxDist;
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
      const float fDist = rng.FloatMinMax(0.0f, GetCurrentFloorRadius() * 0.8f);
      e.m_vGoal = vCenter + CircleOffset(RandomAngle(rng), fDist) + ezVec3(0, 0, rng.FloatMinMax(2.0f, 8.0f));
    }
  }

  ezVec3 vDesired = (e.m_vGoal - e.m_vPosition).GetNormalized();

  // weave from side to side
  const float fTime = GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds();
  const ezVec3 vSide = vDesired.CrossRH(ezVec3(0, 0, 1)).GetNormalized();
  vDesired += vSide * Wobble(fTime, 2.0f, e.m_fPhase) * 0.7f;
  vDesired.NormalizeIfNotZero(ezVec3(1, 0, 0)).IgnoreResult();

  e.m_vVelocity = ezMath::Lerp(e.m_vVelocity, vDesired * 7.0f, ezMath::Min(1.0f, fDeltaTime * 1.5f));
  e.m_vPosition += e.m_vVelocity * fDeltaTime;
  e.m_vPosition.z = ezMath::Min(e.m_vPosition.z, vCenter.z + 10.0f);

  // record the path, the body segments follow it
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

void DcArenaComponent::FinishEnemyUpdate(Enemy& e, float fDeltaTime, const ezVec3& vTarget)
{
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  const DcEnemyTypeInfo& info = DcGetEnemyTypeInfo(e.m_Type);

  if (info.m_bWalksOnGround)
  {
    // walk on the floor tiles and fall when there is none
    if (e.m_fFallSpeed == 0.0f && IsOverFloor(e.m_vPosition))
    {
      e.m_vPosition.z = vCenter.z + info.m_fRadius;
    }
    else
    {
      e.m_fFallSpeed += 9.81f * fDeltaTime;
      e.m_vPosition.z -= e.m_fFallSpeed * fDeltaTime;

      if (e.m_vPosition.z < m_fKillHeight)
      {
        ezLog::Debug("DemonCove: {} fell off the arena", info.m_szName);
        e.m_bRemove = true;
      }
    }
  }
  else if (e.m_Type == DcEnemyType::Skull1 || e.m_Type == DcEnemyType::Skull2 || e.m_Type == DcEnemyType::CentipedeHead)
  {
    // keep enemies that chase the player above the floor
    const float fMinHeight = vCenter.z + info.m_fRadius;
    if (e.m_vPosition.z < fMinHeight)
    {
      e.m_vPosition.z = fMinHeight;
      e.m_vVelocity.z = ezMath::Max(e.m_vVelocity.z, 0.0f);
    }
  }

  if (e.m_fHitFlash > 0.0f)
  {
    e.m_fHitFlash = ezMath::Max(0.0f, e.m_fHitFlash - fDeltaTime * 6.0f);
    DcUtils::SetMeshColor(*GetWorld(), e.m_hObject, ezMath::Lerp(info.m_Tint, s_HitFlashColor, e.m_fHitFlash));
  }

  ezQuat qRotation;
  switch (e.m_Type)
  {
    case DcEnemyType::Squid1:
    case DcEnemyType::Squid2:
      qRotation = DcUtils::YawRotation(e.m_fYaw);
      break;

    case DcEnemyType::Skull1:
    case DcEnemyType::CentipedeHead:
    case DcEnemyType::CentipedeBody:
      qRotation = DcUtils::LookRotation(e.m_vVelocity);
      break;

    case DcEnemyType::Skeleton:
    case DcEnemyType::Thief:
      qRotation = DcUtils::YawRotation(e.m_vVelocity);
      break;

    default:
      // always watch the player
      qRotation = DcUtils::YawRotation(vTarget - e.m_vPosition);
      break;
  }

  DcUtils::PlaceObject(*GetWorld(), e.m_hObject, e.m_vPosition, qRotation);
}

//////////////////////////////////////////////////////////////////////////
// Weak spots

ezVec3 DcArenaComponent::GetWeakSpotPosition(const Enemy& e, ezUInt32 uiSpot) const
{
  // the gems sit on the front (and back) of the squid, which slowly rotates
  const float fYaw = e.m_fYaw + uiSpot * ezMath::Pi<float>();
  const float fDist = DcGetEnemyTypeInfo(e.m_Type).m_fRadius * 0.9f;
  return e.m_vPosition + DcUtils::YawRotation(fYaw) * ezVec3(fDist, 0, 0.2f);
}

ezVec3 DcArenaComponent::GetAimPosition(const Enemy& e, const ezVec3& vFrom) const
{
  const ezUInt32 uiNumSpots = DcGetEnemyTypeInfo(e.m_Type).m_uiWeakSpots;
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

//////////////////////////////////////////////////////////////////////////
// Damage

void DcArenaComponent::DamageEnemy(ezUInt32 uiIndex, float fDamage, const ezVec3& vHitPosition)
{
  Enemy& e = m_Enemies[uiIndex];
  e.m_fHealth -= fDamage;
  e.m_fHitFlash = 1.0f;

  // effects are fire and forget: the particle effect deletes its object when it is done
  if (m_hHitEffect.IsValid())
  {
    ezPrefabResource::InstantiatePrefab(m_hHitEffect, false, *GetWorld(), ezTransform(vHitPosition));
  }

  if (e.m_fHealth <= 0.0f)
  {
    KillEnemy(uiIndex);
  }
}

void DcArenaComponent::KillEnemy(ezUInt32 uiIndex)
{
  // copy, the slot gets reused
  const Enemy e = m_Enemies[uiIndex];
  const DcEnemyTypeInfo& info = DcGetEnemyTypeInfo(e.m_Type);
  m_Enemies.RemoveAtAndSwap(uiIndex);

  DcUtils::DeleteObject(*GetWorld(), e.m_hObject);
  ++m_uiKills;

  const ezUInt32 uiNumGems = info.m_uiGemDrop + e.m_uiCarriedGems;
  for (ezUInt32 i = 0; i < uiNumGems; ++i)
  {
    SpawnGem(e.m_vPosition);
  }

  if (m_hDeathEffect.IsValid())
  {
    ezPrefabResource::InstantiatePrefab(m_hDeathEffect, false, *GetWorld(), ezTransform(e.m_vPosition));
  }

  PlaySound(info.m_DeathSound, e.m_vPosition);

  if (e.m_Type == DcEnemyType::CentipedeHead)
  {
    KillCentipedeBody(e.m_uiGroup);
    m_CentipedeTrails.Remove(e.m_uiGroup);
  }

  if (info.m_bBigKill)
  {
    AddCameraShake(e.m_vPosition, 30.0f, 0.6f, ezTime::MakeFromMilliseconds(250));
    HitPause();
  }
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

void DcArenaComponent::RemoveEnemy(ezUInt32 uiIndex)
{
  if (m_Enemies[uiIndex].m_Type == DcEnemyType::CentipedeHead)
  {
    // the body segments notice that the trail is gone and remove themselves
    m_CentipedeTrails.Remove(m_Enemies[uiIndex].m_uiGroup);
  }

  DcUtils::DeleteObject(*GetWorld(), m_Enemies[uiIndex].m_hObject);
  m_Enemies.RemoveAtAndSwap(uiIndex);
}

void DcArenaComponent::RemoveAllEnemies()
{
  for (const Enemy& e : m_Enemies)
    DcUtils::DeleteObject(*GetWorld(), e.m_hObject);

  m_Enemies.Clear();
  m_CentipedeTrails.Clear();
}
