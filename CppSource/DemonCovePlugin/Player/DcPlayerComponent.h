#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <DemonCovePlugin/DemonCovePluginDLL.h>

class ezHeadBoneComponent;

using DcPlayerComponentManager = ezComponentManagerSimple<class DcPlayerComponent, ezComponentUpdateType::WhenSimulating>;

/// Translates input into character movement, mouse look and dagger fire.
///
/// Part of the player prefab (Prefabs/Player/Player.ezPrefab), which the game state spawns at the ezPlayerStartPointComponent.
/// Expects to be attached next to an ezJoltDefaultCharacterComponent, which does the actual movement: this component only
/// sends it an ezMsgMoveCharacterController every frame. Vertical look is done by an ezHeadBoneComponent on a child
/// named 'HeadBone', the camera is a child named 'Camera'.
///
/// The player doesn't simulate daggers itself, it hands them to the DcArenaComponent, which owns all gameplay objects.
/// Firing follows the hand from the original: a click fires a shotgun burst, holding the button fires a continuous stream.
class DcPlayerComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(DcPlayerComponent, ezComponent, DcPlayerComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // DcPlayerComponent

public:
  DcPlayerComponent();
  ~DcPlayerComponent();

  /// Position the daggers emerge from, slightly below and to the right of the eye.
  ezVec3 GetHandPosition() const;

  ezVec3 GetEyePosition() const;
  ezVec3 GetViewDirection() const;

  /// Center of the character's body, used for enemy collision checks.
  ezVec3 GetBodyCenter() const;

  /// Moves the character to the given foot position and resets the view.
  void Respawn(const ezVec3& vFootPosition, const ezQuat& qRotation);

  float m_fMouseSensitivity = 1.0f; // [ property ]

private:
  void Update();
  void UpdateMovement();
  void UpdateFiring();

  /// Turns the view towards the closest enemy, for the 'DemonCove.DebugAutoAim' CVar. Returns the direction to fire in.
  ezVec3 UpdateAutoAim(const ezVec3& vHand, const ezVec3& vDir);

  /// The hand is a dagger at the bottom right of the view. It glows brighter with every hand level and recoils when firing.
  void UpdateHand(float fDeltaTime);

  void FindChildObjects();
  ezGameObject* GetCamera() const;
  ezHeadBoneComponent* GetHeadBone() const;

  ezGameObjectHandle m_hCamera;
  ezComponentHandle m_hHeadBone;

  ezTime m_FireHeldSince;
  bool m_bFireHeld = false;
  float m_fStreamAccumulator = 0.0f;
  ezTime m_LastShotgun;

  ezGameObjectHandle m_hHand;
  float m_fRecoil = 0.0f;
  ezUInt32 m_uiHandLevel = 0;
};
