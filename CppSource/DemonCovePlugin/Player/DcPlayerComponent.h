#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <DemonCovePlugin/DemonCovePluginDLL.h>

using DcPlayerComponentManager = ezComponentManagerSimple<class DcPlayerComponent, ezComponentUpdateType::WhenSimulating>;

/// Translates input into character movement, mouse look and dagger fire.
///
/// Expects to be attached next to an ezJoltDefaultCharacterComponent. The camera is looked up as a child object named 'Camera',
/// the head bone (for vertical look) as a child named 'HeadBone'. Daggers are handed to the DcArenaComponent in the scene.
///
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

  float m_fMouseSensitivity = 1.0f;  // [ property ]

private:
  void Update();
  void UpdateMovement();
  void UpdateFiring();

  /// The hand is a dagger at the bottom right of the view. It glows brighter with every hand level and recoils when firing.
  void UpdateHand(float fDeltaTime);

  ezGameObject* GetCamera() const;

  ezTime m_FireHeldSince;
  bool m_bFireHeld = false;
  float m_fStreamAccumulator = 0.0f;
  ezTime m_LastShotgun;

  ezGameObjectHandle m_hHand;
  float m_fRecoil = 0.0f;
  ezUInt32 m_uiHandLevel = 0;
};
