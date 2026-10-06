#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <DemonCovePlugin/Arena/DcArenaComponent.h>
#include <DemonCovePlugin/Player/DcPlayerComponent.h>
#include <GameComponentsPlugin/Gameplay/HeadBoneComponent.h>
#include <GameEngine/Physics/CharacterControllerComponent.h>
#include <JoltPlugin/Character/JoltDefaultCharacterComponent.h>
#include <RendererCore/Lights/PointLightComponent.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(DcPlayerComponent, 1, ezComponentMode::Dynamic)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("MouseSensitivity", m_fMouseSensitivity)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.01f, 10.0f)),
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

ezCVarBool cvar_DebugAutoAim("DemonCove.DebugAutoAim", false, ezCVarFlags::Default, "Debug: daggers fly towards the closest enemy instead of the view direction, and the view turns towards it. Used for automated tests.");

namespace
{
  constexpr const char* s_szInputSet = "Default";

  struct HandLevelConfig
  {
    float m_fStreamRate;       ///< Daggers per second while holding fire.
    ezUInt32 m_uiShotgunCount; ///< Daggers per click.
  };

  constexpr HandLevelConfig s_HandLevels[] = {
    {12.0f, 10},
    {20.0f, 16},
    {24.0f, 20},
  };

  constexpr ezTime s_StreamDelay = ezTime::MakeFromMilliseconds(180);
  constexpr ezTime s_ShotgunCooldown = ezTime::MakeFromMilliseconds(120);

  float GetInput(const char* szAction)
  {
    float fValue = 0.0f;
    ezInputManager::GetInputActionState(s_szInputSet, szAction, &fValue);
    return fValue;
  }
} // namespace

DcPlayerComponent::DcPlayerComponent() = default;
DcPlayerComponent::~DcPlayerComponent() = default;

void DcPlayerComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();
}

ezGameObject* DcPlayerComponent::GetCamera() const
{
  return const_cast<ezGameObject*>(GetOwner())->FindChildByName("Camera", true);
}

ezVec3 DcPlayerComponent::GetEyePosition() const
{
  if (const ezGameObject* pCamera = GetCamera())
    return pCamera->GetGlobalPosition();

  return GetOwner()->GetGlobalPosition() + ezVec3(0, 0, 1.6f);
}

ezVec3 DcPlayerComponent::GetViewDirection() const
{
  if (const ezGameObject* pCamera = GetCamera())
    return pCamera->GetGlobalDirForwards();

  return GetOwner()->GetGlobalDirForwards();
}

ezVec3 DcPlayerComponent::GetHandPosition() const
{
  const ezGameObject* pCamera = GetCamera();
  if (pCamera == nullptr)
    return GetEyePosition();

  return pCamera->GetGlobalPosition() + pCamera->GetGlobalDirForwards() * 0.3f + pCamera->GetGlobalDirRight() * 0.2f - pCamera->GetGlobalDirUp() * 0.15f;
}

ezVec3 DcPlayerComponent::GetBodyCenter() const
{
  return GetOwner()->GetGlobalPosition() + ezVec3(0, 0, 0.8f);
}

void DcPlayerComponent::Respawn(const ezVec3& vFootPosition, const ezQuat& qRotation)
{
  if (ezJoltDefaultCharacterComponent* pCharacter = nullptr; GetOwner()->TryGetComponentOfBaseType(pCharacter))
  {
    pCharacter->TeleportCharacter(vFootPosition);
  }
  else
  {
    GetOwner()->SetGlobalPosition(vFootPosition);
  }

  GetOwner()->SetGlobalRotation(qRotation);

  if (ezGameObject* pHeadBone = GetOwner()->FindChildByName("HeadBone", true))
  {
    if (ezHeadBoneComponent* pHeadBoneComp = nullptr; pHeadBone->TryGetComponentOfBaseType(pHeadBoneComp))
    {
      pHeadBoneComp->SetVerticalRotation(0.0f);
    }
  }

  m_bFireHeld = false;
  m_fStreamAccumulator = 0.0f;
}

void DcPlayerComponent::Update()
{
  UpdateHand(GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds());

  DcArenaComponent* pArena = DcArenaComponent::FindArena(GetWorld());
  if (pArena && pArena->IsPlayerDead())
    return;

  UpdateMovement();
  UpdateFiring();
}

void DcPlayerComponent::UpdateMovement()
{
  ezMsgMoveCharacterController msg;
  msg.m_fMoveForwards = GetInput("move_forwards");
  msg.m_fMoveBackwards = GetInput("move_backwards");
  msg.m_fStrafeLeft = GetInput("move_left");
  msg.m_fStrafeRight = GetInput("move_right");
  msg.m_fRotateLeft = GetInput("turn_left") * m_fMouseSensitivity;
  msg.m_fRotateRight = GetInput("turn_right") * m_fMouseSensitivity;
  msg.m_bJump = GetInput("jump") > 0.5f;
  msg.m_bRun = true;

  GetOwner()->SendMessage(msg);

  if (ezGameObject* pHeadBone = GetOwner()->FindChildByName("HeadBone", true))
  {
    if (ezHeadBoneComponent* pHeadBoneComp = nullptr; pHeadBone->TryGetComponentOfBaseType(pHeadBoneComp))
    {
      const float fUp = GetInput("turn_up");
      const float fDown = GetInput("turn_down");
      pHeadBoneComp->ChangeVerticalRotation((fDown - fUp) * m_fMouseSensitivity);
    }
  }
}

void DcPlayerComponent::UpdateFiring()
{
  DcArenaComponent* pArena = DcArenaComponent::FindArena(GetWorld());
  if (pArena == nullptr)
    return;

  const ezTime tNow = GetWorld()->GetClock().GetAccumulatedTime();
  const float fDeltaTime = GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds();
  const HandLevelConfig& cfg = s_HandLevels[ezMath::Clamp<ezUInt32>(pArena->GetHandLevel(), 1, EZ_ARRAY_SIZE(s_HandLevels)) - 1];

  const ezKeyState::Enum fireState = ezInputManager::GetInputActionState(s_szInputSet, "shoot");

  const ezVec3 vHand = GetHandPosition();
  ezVec3 vDir = GetViewDirection();

  if (ezVec3 vEnemyPos; cvar_DebugAutoAim && pArena->GetClosestEnemyPosition(vHand, vEnemyPos))
  {
    vDir = (vEnemyPos - vHand).GetNormalized();

    // also look at the target, so that screenshots show what is being shot at
    GetOwner()->SetGlobalRotation(ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), ezAngle::MakeFromRadian(ezMath::ATan2(vDir.y, vDir.x).GetRadian())));

    if (ezGameObject* pHeadBone = GetOwner()->FindChildByName("HeadBone", true))
    {
      if (ezHeadBoneComponent* pHeadBoneComp = nullptr; pHeadBone->TryGetComponentOfBaseType(pHeadBoneComp))
      {
        const float fHorizontal = ezVec3(vDir.x, vDir.y, 0).GetLength();
        pHeadBoneComp->SetVerticalRotation(ezMath::ATan2(-vDir.z, fHorizontal).GetRadian());
      }
    }
  }

  const ezGameObject* pViewObj = GetCamera() ? GetCamera() : GetOwner();
  const ezVec3 vRight = pViewObj->GetGlobalDirRight();
  const ezVec3 vUp = pViewObj->GetGlobalDirUp();
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();

  auto makeSpreadDir = [&](float fMaxSpreadDeg)
  {
    const float fAngle = (float)rng.DoubleZeroToOneExclusive() * ezMath::Pi<float>() * 2.0f;
    const float fAmount = ezMath::Sqrt((float)rng.DoubleZeroToOneExclusive()) * ezMath::Tan(ezAngle::MakeFromDegree(fMaxSpreadDeg));
    return (vDir + vRight * ezMath::Cos(ezAngle::MakeFromRadian(fAngle)) * fAmount + vUp * ezMath::Sin(ezAngle::MakeFromRadian(fAngle)) * fAmount).GetNormalized();
  };

  if (fireState == ezKeyState::Pressed)
  {
    m_bFireHeld = true;
    m_FireHeldSince = tNow;
    m_fStreamAccumulator = 0.0f;

    if (tNow - m_LastShotgun >= s_ShotgunCooldown)
    {
      m_LastShotgun = tNow;

      for (ezUInt32 i = 0; i < cfg.m_uiShotgunCount; ++i)
      {
        pArena->SpawnDagger(vHand, makeSpreadDir(9.0f));
      }

      if (!pArena->IsWaitingForStart())
      {
        pArena->PlaySound(pArena->m_sSoundShotgun, vHand);
        m_fRecoil = 1.0f;
        pArena->AddCameraShake(GetOwner()->GetGlobalPosition(), 3.0f, 0.15f, ezTime::MakeFromMilliseconds(80));
      }
    }
  }
  else if (fireState == ezKeyState::Down && m_bFireHeld)
  {
    if (tNow - m_FireHeldSince >= s_StreamDelay)
    {
      m_fStreamAccumulator += fDeltaTime * cfg.m_fStreamRate;

      while (m_fStreamAccumulator >= 1.0f)
      {
        m_fStreamAccumulator -= 1.0f;
        pArena->SpawnDagger(vHand, makeSpreadDir(1.5f));
        m_fRecoil = ezMath::Max(m_fRecoil, 0.3f);

        if (!pArena->IsWaitingForStart())
        {
          pArena->PlaySound(pArena->m_sSoundDaggerStream, vHand);
        }
      }
    }
  }
  else
  {
    m_bFireHeld = false;
  }
}

void DcPlayerComponent::UpdateHand(float fDeltaTime)
{
  DcArenaComponent* pArena = DcArenaComponent::FindArena(GetWorld());
  ezGameObject* pCamera = GetCamera();
  if (pArena == nullptr || pCamera == nullptr || !pArena->GetDaggerPrefab().IsValid())
    return;

  ezGameObject* pHand = nullptr;
  if (!GetWorld()->TryGetObject(m_hHand, pHand))
  {
    ezGameObjectDesc desc;
    desc.m_sName.Assign("Hand");
    desc.m_hParent = pCamera->GetHandle();
    desc.m_bDynamic = true;
    m_hHand = GetWorld()->CreateObject(desc, pHand);

    ezPrefabInstantiationOptions options;
    options.m_hParent = m_hHand;
    ezPrefabResource::InstantiatePrefab(pArena->GetDaggerPrefab(), true, *GetWorld(), ezTransform::MakeIdentity(), options);

    m_uiHandLevel = 0;
  }

  // the dagger is only in the hand once it was picked up
  const bool bVisible = !pArena->IsPlayerDead() && !pArena->IsWaitingForStart();
  if (pHand->GetActiveFlag() != bVisible)
  {
    pHand->SetActiveFlag(bVisible);
  }

  if (!bVisible)
    return;

  m_fRecoil = ezMath::Max(0.0f, m_fRecoil - fDeltaTime * 8.0f);

  // bottom right of the view, the blade pointing forwards and a bit inwards and up
  pHand->SetLocalPosition(ezVec3(0.6f - m_fRecoil * 0.1f, 0.3f, -0.26f));
  pHand->SetLocalUniformScaling(0.6f);
  pHand->SetLocalRotation(ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), ezAngle::MakeFromDegree(-12.0f)) *
                          ezQuat::MakeFromAxisAndAngle(ezVec3(0, 1, 0), ezAngle::MakeFromDegree(-10.0f - m_fRecoil * 12.0f)));

  // brighter with every hand level
  const ezUInt32 uiLevel = pArena->GetHandLevel();
  if (uiLevel != m_uiHandLevel)
  {
    m_uiHandLevel = uiLevel;

    constexpr float fIntensity[] = {3.0f, 8.0f, 18.0f};
    const ezColor color[] = {ezColor(1.0f, 0.6f, 0.3f), ezColor(1.0f, 0.5f, 0.2f), ezColor(1.0f, 0.25f, 0.15f)};
    const ezUInt32 uiIndex = ezMath::Clamp<ezUInt32>(uiLevel, 1, 3) - 1;

    ezHybridArray<ezGameObject*, 8> stack;
    stack.PushBack(pHand);
    while (!stack.IsEmpty())
    {
      ezGameObject* pObj = stack.PeekBack();
      stack.PopBack();

      if (ezPointLightComponent* pLight = nullptr; pObj->TryGetComponentOfBaseType(pLight))
      {
        pLight->SetIntensity(fIntensity[uiIndex]);
        pLight->SetLightColor(color[uiIndex]);
      }

      for (auto it = pObj->GetChildren(); it.IsValid(); ++it)
        stack.PushBack(it);
    }
  }
}

void DcPlayerComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  ezReflectionSerializer::WriteObjectToBinary(inout_stream.GetStream(), GetDynamicRTTI(), this);
}

void DcPlayerComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  ezReflectionSerializer::ReadObjectPropertiesFromBinary(inout_stream.GetStream(), *GetDynamicRTTI(), this);
}
