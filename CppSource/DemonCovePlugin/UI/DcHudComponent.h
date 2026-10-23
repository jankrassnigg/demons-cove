#pragma once

#include <Core/World/ComponentManager.h>
#include <DemonCovePlugin/DemonCovePluginDLL.h>
#include <RendererCore/Components/BlackboardComponent.h>

using DcHudComponentManager = ezComponentManagerSimple<class DcHudComponent, ezComponentUpdateType::WhenSimulating>;

/// Feeds the round state of the DcArenaComponent into the RmlUi HUD document (UI/Hud.rml).
///
/// This is a blackboard component: it provides the global blackboard named 'hud' and fills it every frame.
/// Place it next to an ezRmlUiCanvas2DComponent with 'AutobindBlackboards' enabled. The canvas then binds the blackboard
/// before it loads the document, and in the RML document it is available as the data model 'hud',
/// so '{{gems}}' displays the blackboard entry 'gems'.
///
/// The HUD is part of the player prefab, so it only exists when the game is played, not while editing the level.
/// The blackboard is global, so that the game state can set 'menuopen' while the world is paused.
class DcHudComponent : public ezBlackboardComponent
{
  EZ_DECLARE_COMPONENT_TYPE(DcHudComponent, ezBlackboardComponent, DcHudComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

protected:
  virtual void Initialize() override;

  //////////////////////////////////////////////////////////////////////////
  // DcHudComponent

public:
  DcHudComponent();
  ~DcHudComponent();

  /// The name of the global blackboard, and thus of the data model in the RML document.
  static constexpr const char* BlackboardName = "hud";

private:
  void Update();

  /// Writes the scoreboard into the blackboard. Only done when it changed, building the array isn't free.
  void UpdateScores(const class DcArenaComponent& arena);

  ezUInt32 m_uiScoreboardChangeCounter = ezInvalidIndex;
  ezUInt32 m_uiScoreboardPlace = ezInvalidIndex;

  ezUInt32 m_uiLastGems = 0;
  ezUInt32 m_uiLastHandLevel = 1;
  ezTime m_GemPulseEnd; ///< The gem counter pulses briefly whenever a gem is collected.
  ezTime m_LevelUpEnd;  ///< A new hand level is announced in the center of the screen for a moment.
};
