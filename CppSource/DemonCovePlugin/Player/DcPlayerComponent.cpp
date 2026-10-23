#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <DemonCovePlugin/Arena/DcArenaComponent.h>
#include <DemonCovePlugin/Common/DcCVars.h>
#include <DemonCovePlugin/Common/DcUtils.h>
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

namespace
{
  /// The actions of this input set are configured in RuntimeConfigs/InputConfig.ddl (editor: Project > Input Configuration).
  constexpr const char* s_szInputSet = "Default";

  struct HandLevelConfig
  {
    float m_fStreamRate;       ///< Daggers per second while holding fire.
    ezUInt32 m_uiShotgunCount; ///< Daggers per click.
    float m_fLightIntensity;   ///< The hand's light glows brighter with every level.
    ezColor m_LightColor;
  };

  const HandLevelConfig s_HandLevels[] = {
    {12.0f, 10, 3.0f, ezColor(1.0f, 0.6f, 0.3f)},
    {20.0f, 16, 8.0f, ezColor(1.0f, 0.5f, 0.2f)},
    {24.0f, 20, 18.0f, ezColor(1.0f, 0.25f, 0.15f)},
  };

  const HandLevelConfig& GetHandLevelConfig(ezUInt32 uiLevel)
  {
    return s_HandLevels[ezMath::Clamp<ezUInt32>(uiLevel, 1, EZ_ARRAY_SIZE(s_HandLevels)) - 1];
  }

  /// Holding the button this long switches from the shotgun burst to the stream.
  constexpr ezTime s_StreamDelay = ezTime::MakeFromMilliseconds(180);
  constexpr ezTime s_ShotgunCooldown = ezTime::MakeFromMilliseconds(120);
  constexpr float s_fShotgunSpreadDeg = 9.0f;
  constexpr float s_fStreamSpreadDeg = 1.5f;

  /// The analog value of an action, e.g. how far the stick is pushed or how far the mouse moved.
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

  FindChildObjects();
}

void DcPlayerComponent::FindChildObjects()
{
  // look up the child objects once and keep their handles, searching by name every frame would be wasteful
  if (ezGameObject* pCamera = GetOwner()->FindChildByName("Camera", true))
  {
    m_hCamera = pCamera->GetHandle();
  }

  if (ezGameObject* pHeadBoneObj = GetOwner()->FindChildByName("HeadBone", true))
  {
    if (ezHeadBoneComponent* pHeadBone = nullptr; pHeadBoneObj->TryGetComponentOfBaseType(pHeadBone))
    {
      m_hHeadBone = pHeadBone->GetHandle();
    }
  }
}

ezGameObject* DcPlayerComponent::GetCamera() const
{
  ezGameObject* pCamera = nullptr;
  return const_cast<ezWorld*>(GetWorld())->TryGetObject(m_hCamera, pCamera) ? pCamera : nullptr;
}

ezHeadBoneComponent* DcPlayerComponent::GetHeadBone() const
{
  ezHeadBoneComponent* pHeadBone = nullptr;
  return const_cast<ezWorld*>(GetWorld())->TryGetComponent(m_hHeadBone, pHeadBone) ? pHeadBone : nullptr;
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
  // the arena may call this before this component's simulation started
  if (m_hHeadBone.IsInvalidated())
  {
    FindChildObjects();
  }

  // the character controller has to be told about the teleport, otherwise it would sweep its shape to the new position
  if (ezJoltDefaultCharacterComponent* pCharacter = nullptr; GetOwner()->TryGetComponentOfBaseType(pCharacter))
  {
    pCharacter->TeleportCharacter(vFootPosition);
  }
  else
  {
    GetOwner()->SetGlobalPosition(vFootPosition);
  }

  GetOwner()->SetGlobalRotation(qRotation);

  if (ezHeadBoneComponent* pHeadBone = GetHeadBone())
  {
    pHeadBone->SetVerticalRotation(0.0f);
  }

  m_bFireHeld = false;
  m_fStreamAccumulator = 0.0f;
}

void DcPlayerComponent::Update()
{
  UpdateHand(GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds());

  const DcArenaComponent* pArena = DcArenaComponent::FindArena(GetWorld());
  if (pArena && pArena->IsPlayerDead())
    return;

  UpdateMovement();
  UpdateFiring();
}

void DcPlayerComponent::UpdateMovement()
{
  // the character controller component reacts to this message, it doesn't read input itself
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

  if (ezHeadBoneComponent* pHeadBone = GetHeadBone())
  {
    pHeadBone->ChangeVerticalRotation((GetInput("turn_down") - GetInput("turn_up")) * m_fMouseSensitivity);
  }
}

ezVec3 DcPlayerComponent::UpdateAutoAim(const ezVec3& vHand, const ezVec3& vDir)
{
  const DcArenaComponent* pArena = DcArenaComponent::FindArena(GetWorld());

  ezVec3 vEnemyPos;
  if (pArena == nullptr || !pArena->GetClosestEnemyPosition(vHand, vEnemyPos))
    return vDir;

  const ezVec3 vAimDir = (vEnemyPos - vHand).GetNormalized();

  // also look at the target, so that screenshots show what is being shot at
  GetOwner()->SetGlobalRotation(DcUtils::YawRotation(vAimDir));

  if (ezHeadBoneComponent* pHeadBone = GetHeadBone())
  {
    const float fHorizontal = ezVec3(vAimDir.x, vAimDir.y, 0).GetLength();
    pHeadBone->SetVerticalRotation(ezMath::ATan2(-vAimDir.z, fHorizontal).GetRadian());
  }

  return vAimDir;
}

void DcPlayerComponent::UpdateFiring()
{
  DcArenaComponent* pArena = DcArenaComponent::FindArena(GetWorld());
  if (pArena == nullptr)
    return;

  const ezTime tNow = GetWorld()->GetClock().GetAccumulatedTime();
  const float fDeltaTime = GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds();
  const HandLevelConfig& cfg = GetHandLevelConfig(pArena->GetHandLevel());

  const ezKeyState::Enum fireState = ezInputManager::GetInputActionState(s_szInputSet, "shoot");

  const ezVec3 vHand = GetHandPosition();
  ezVec3 vDir = GetViewDirection();

  if (cvar_DebugAutoAim)
  {
    vDir = UpdateAutoAim(vHand, vDir);
  }

  const ezGameObject* pViewObj = GetCamera() ? GetCamera() : GetOwner();
  const ezVec3 vRight = pViewObj->GetGlobalDirRight();
  const ezVec3 vUp = pViewObj->GetGlobalDirUp();
  ezRandom& rng = GetWorld()->GetRandomNumberGenerator();

  // a random direction within a cone around vDir, uniformly distributed over the cone's cross section
  auto makeSpreadDir = [&](float fMaxSpreadDeg)
  {
    const ezAngle angle = ezAngle::MakeFromRadian((float)rng.DoubleZeroToOneExclusive() * ezMath::Pi<float>() * 2.0f);
    const float fAmount = ezMath::Sqrt((float)rng.DoubleZeroToOneExclusive()) * ezMath::Tan(ezAngle::MakeFromDegree(fMaxSpreadDeg));
    return (vDir + vRight * ezMath::Cos(angle) * fAmount + vUp * ezMath::Sin(angle) * fAmount).GetNormalized();
  };

  // effects only when there actually are daggers, the arena ignores them before the round starts
  const bool bCanFire = !pArena->IsWaitingForStart();

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
        pArena->SpawnDagger(vHand, makeSpreadDir(s_fShotgunSpreadDeg));
      }

      if (bCanFire)
      {
        pArena->PlaySound(DcSound::Shotgun, vHand);
        pArena->AddCameraShake(GetOwner()->GetGlobalPosition(), 3.0f, 0.15f, ezTime::MakeFromMilliseconds(80));
        m_fRecoil = 1.0f;
      }
    }
  }
  else if (fireState == ezKeyState::Down && m_bFireHeld)
  {
    if (tNow - m_FireHeldSince >= s_StreamDelay)
    {
      // accumulate fractional daggers, so that the rate is independent of the frame rate
      m_fStreamAccumulator += fDeltaTime * cfg.m_fStreamRate;

      while (m_fStreamAccumulator >= 1.0f)
      {
        m_fStreamAccumulator -= 1.0f;
        pArena->SpawnDagger(vHand, makeSpreadDir(s_fStreamSpreadDeg));
        m_fRecoil = ezMath::Max(m_fRecoil, 0.3f);

        if (bCanFire)
        {
          pArena->PlaySound(DcSound::DaggerStream, vHand);
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
  const DcArenaComponent* pArena = DcArenaComponent::FindArena(GetWorld());
  ezGameObject* pCamera = GetCamera();
  if (pArena == nullptr || pCamera == nullptr || !pArena->GetDaggerPrefab().IsValid())
    return;

  ezGameObject* pHand = nullptr;
  if (!GetWorld()->TryGetObject(m_hHand, pHand))
  {
    // the same prefab as the flying daggers, attached to the camera
    m_hHand = DcUtils::SpawnPrefab(*GetWorld(), pArena->GetDaggerPrefab(), ezVec3::MakeZero(), ezQuat::MakeIdentity(), pCamera->GetHandle());
    if (!GetWorld()->TryGetObject(m_hHand, pHand))
      return;

    pHand->SetName("Hand");
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

  const ezUInt32 uiLevel = pArena->GetHandLevel();
  if (uiLevel == m_uiHandLevel)
    return;

  m_uiHandLevel = uiLevel;

  // the dagger prefab contains a point light, which glows brighter with every hand level
  const HandLevelConfig& cfg = GetHandLevelConfig(uiLevel);
  DcUtils::VisitHierarchy(pHand, [&](ezGameObject* pObject)
    {
      if (ezPointLightComponent* pLight = nullptr; pObject->TryGetComponentOfBaseType(pLight))
      {
        pLight->SetIntensity(cfg.m_fLightIntensity);
        pLight->SetLightColor(cfg.m_LightColor);
      } });
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
