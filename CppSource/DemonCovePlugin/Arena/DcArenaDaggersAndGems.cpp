#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <DemonCovePlugin/Arena/DcArenaComponent.h>
#include <DemonCovePlugin/Common/DcCVars.h>
#include <DemonCovePlugin/Common/DcUtils.h>
#include <DemonCovePlugin/Player/DcPlayerComponent.h>

// Dagger flight and hits, gem drops and pickups. See DcArenaComponent.h for an overview.

namespace
{
  constexpr float s_fDaggerSpeed = 60.0f;
  constexpr float s_fDaggerLifeTime = 1.0f;

  /// Daggers are thin, but hitting should be forgiving. This is added to every target radius.
  constexpr float s_fDaggerHitTolerance = 0.1f;
  constexpr float s_fWeakSpotRadius = 0.45f;

  /// Homing daggers (hand level 3) only steer towards enemies within this distance and roughly in front of them.
  constexpr float s_fHomingRange = 25.0f;
  constexpr float s_fHomingMinCosAngle = 0.85f;

  constexpr float s_fGemCollectRange = 1.2f;
  constexpr float s_fGemAttractSpeed = 18.0f;
} // namespace

//////////////////////////////////////////////////////////////////////////
// Daggers

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
  d.m_hObject = DcUtils::SpawnPrefab(*GetWorld(), m_hDaggerPrefab, d.m_vPosition, DcUtils::LookRotation(vDirection));

  // the trail is attached here and not in the dagger prefab, because that one is also used for the player's hand
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
      // steer towards the enemy that is most in front of the dagger
      const ezVec3 vDir = d.m_vVelocity.GetNormalized();
      float fBestScore = s_fHomingMinCosAngle;
      ezVec3 vBestDir = vDir;

      for (const Enemy& e : m_Enemies)
      {
        ezVec3 vToEnemy = GetAimPosition(e, d.m_vPosition) - d.m_vPosition;
        const float fDist = vToEnemy.GetLengthAndNormalize();
        if (fDist > s_fHomingRange)
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

    // daggers that run out of time or hit the floor are gone
    const bool bRemove = d.m_fLifeTime <= 0.0f || (vEnd.z < vCenter.z && IsOverFloor(vEnd)) || CheckDaggerHit(vStart, vEnd);

    if (bRemove)
    {
      DcUtils::DeleteObject(*GetWorld(), m_Daggers[i].m_hObject);
      m_Daggers.RemoveAtAndSwap(i);
      continue;
    }

    d.m_vPosition = vEnd;
    DcUtils::PlaceObject(*GetWorld(), d.m_hObject, d.m_vPosition, DcUtils::LookRotation(d.m_vVelocity));
    ++i;
  }
}

bool DcArenaComponent::CheckDaggerHit(const ezVec3& vStart, const ezVec3& vEnd)
{
  for (ezUInt32 e = 0; e < m_Enemies.GetCount(); ++e)
  {
    const Enemy& enemy = m_Enemies[e];
    const DcEnemyTypeInfo& info = DcGetEnemyTypeInfo(enemy.m_Type);

    // squids only take damage at their gems
    for (ezUInt32 s = 0; s < info.m_uiWeakSpots; ++s)
    {
      const ezVec3 vSpot = GetWeakSpotPosition(enemy, s);
      if (DcUtils::SegmentHitsSphere(vStart, vEnd, vSpot, s_fWeakSpotRadius + s_fDaggerHitTolerance))
      {
        ++m_uiDaggersHit;
        PlaySound(DcSound::WeakSpotHit, vSpot);
        DamageEnemy(e, 1.0f, vSpot);
        return true;
      }
    }

    if (DcUtils::SegmentHitsSphere(vStart, vEnd, enemy.m_vPosition, info.m_fRadius + s_fDaggerHitTolerance))
    {
      // the body of an enemy with weak spots just blocks the dagger
      if (info.m_uiWeakSpots == 0)
      {
        ++m_uiDaggersHit;
        PlaySound(DcSound::DaggerHit, enemy.m_vPosition);
        DamageEnemy(e, 1.0f, enemy.m_vPosition);
      }

      return true;
    }
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////
// Gems

void DcArenaComponent::SpawnGem(const ezVec3& vPosition)
{
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();

  // pop out of the enemy in a random direction
  Gem& g = m_Gems.ExpandAndGetRef();
  g.m_vPosition = vPosition;
  g.m_vVelocity = ezVec3(rng.FloatMinMax(-2, 2), rng.FloatMinMax(-2, 2), rng.FloatMinMax(2, 5));
  g.m_hObject = DcUtils::SpawnPrefab(*GetWorld(), m_hGemPrefab, vPosition, ezQuat::MakeIdentity());
}

void DcArenaComponent::UpdateGems(float fDeltaTime)
{
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  const DcPlayerComponent* pPlayer = m_bPlayerDead ? nullptr : GetPlayer();

  for (ezUInt32 i = 0; i < m_Gems.GetCount();)
  {
    Gem& g = m_Gems[i];
    g.m_fAge += fDeltaTime;

    // fly towards the player when close enough, after a short delay so that the drop is visible
    bool bAttracted = false;
    if (pPlayer && g.m_fAge > 0.4f)
    {
      const ezVec3 vToPlayer = pPlayer->GetBodyCenter() - g.m_vPosition;
      const float fDist = vToPlayer.GetLength();

      if (fDist < s_fGemCollectRange)
      {
        ++m_uiGems;
        PlaySound(DcSound::GemPickup, g.m_vPosition);

        if (m_hGemPickupEffect.IsValid())
        {
          ezPrefabResource::InstantiatePrefab(m_hGemPickupEffect, false, *GetWorld(), ezTransform(g.m_vPosition));
        }

        DcUtils::DeleteObject(*GetWorld(), g.m_hObject);
        m_Gems.RemoveAtAndSwap(i);
        continue;
      }

      if (fDist < cvar_GemAttractRange)
      {
        bAttracted = true;
        g.m_vVelocity = vToPlayer.GetNormalized() * s_fGemAttractSpeed;
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
      // fall and slide to a halt
      g.m_vVelocity.z -= 9.81f * fDeltaTime;
      g.m_vVelocity.x *= ezMath::Max(0.0f, 1.0f - fDeltaTime);
      g.m_vVelocity.y *= ezMath::Max(0.0f, 1.0f - fDeltaTime);
    }

    g.m_vPosition += g.m_vVelocity * fDeltaTime;

    // bounce off the floor
    const float fFloor = vCenter.z + 0.3f;
    if (!bAttracted && g.m_vPosition.z < fFloor && g.m_vPosition.z > vCenter.z - 0.5f && IsOverFloor(g.m_vPosition))
    {
      g.m_vPosition.z = fFloor;
      g.m_vVelocity.z = ezMath::Abs(g.m_vVelocity.z) * 0.3f;
    }

    if (g.m_vPosition.z < m_fKillHeight)
    {
      DcUtils::DeleteObject(*GetWorld(), g.m_hObject);
      m_Gems.RemoveAtAndSwap(i);
      continue;
    }

    DcUtils::PlaceObject(*GetWorld(), g.m_hObject, g.m_vPosition, DcUtils::YawRotation(g.m_fAge * 3.0f));
    ++i;
  }
}

void DcArenaComponent::RemoveAllDaggersAndGems()
{
  for (const Dagger& d : m_Daggers)
    DcUtils::DeleteObject(*GetWorld(), d.m_hObject);
  for (const Gem& g : m_Gems)
    DcUtils::DeleteObject(*GetWorld(), g.m_hObject);

  m_Daggers.Clear();
  m_Gems.Clear();
}
