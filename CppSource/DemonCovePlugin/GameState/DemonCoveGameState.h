#pragma once

#include <DemonCovePlugin/DemonCovePluginDLL.h>
#include <GameEngine/GameState/GameState.h>

class ezMainMenuComponent;

/// The game state is the 'director' of the application, it lives outside of any world.
///
/// The game application picks this class automatically, because it is the only non-fallback game state in the loaded plugins.
/// The base class already does most of the work: it loads the startup scene, spawns the player prefab at the
/// ezPlayerStartPointComponent and sets up the main camera. This class only adds what is specific to this game:
///   - which scene to start with and the loading screen,
///   - ESC opens the main menu of the scene (an ezMainMenuComponent) instead of quitting.
///
/// All gameplay logic is in components (see DcArenaComponent), so that it also works when the scene is simulated in the editor.
class DemonCoveGameState : public ezGameState
{
  EZ_ADD_DYNAMIC_REFLECTION(DemonCoveGameState, ezGameState);

public:
  DemonCoveGameState();
  ~DemonCoveGameState();

  virtual void ProcessInput() override;

  /// ESC in the editor's play mode opens the main menu of the scene instead of stopping the game.
  /// In the game itself ESC is a regular input action, see ConfigureInputActions().
  virtual void RequestQuit(ezStringView sRequestedBy) override;

protected:
  virtual void ConfigureInputActions() override;
  virtual void OnChangedMainWorld(ezWorld* pPrevWorld, ezWorld* pNewWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset) override;
  virtual void GetStartupOptions(ezString& out_sScene, ezString& out_sPreloadCollection) override;

  /// Shows UI/Loading.rml, an RmlUi document with the game's title.
  virtual ezUniquePtr<ezWorld> CreateLoadingScreenWorld(ezStringView sTargetSceneFile) override;

private:
  /// The menu is part of the scene, it is looked up once per world.
  ezMainMenuComponent* GetMainMenu();

  ezComponentHandle m_hMainMenu;
  bool m_bSearchedMainMenu = false;
};
