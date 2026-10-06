#pragma once

#include <Core/Input/Declarations.h>
#include <Core/World/Declarations.h>
#include <DemonCovePlugin/DemonCovePluginDLL.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/FallbackGameState.h>
#include <GameEngine/GameState/GameState.h>

// the ezFallbackGameState adds a free flying camera and a scene switching menu, so can be useful in the very beginning
// but generally it's better to use ezGameState instead
// using DemonCoveGameStateBase = ezFallbackGameState;
using DemonCoveGameStateBase = ezGameState;

class DemonCoveGameState : public DemonCoveGameStateBase
{
  EZ_ADD_DYNAMIC_REFLECTION(DemonCoveGameState, DemonCoveGameStateBase);

public:
  DemonCoveGameState();
  ~DemonCoveGameState();

  virtual void ProcessInput() override;

  /// ESC in the editor's play mode opens the main menu of the scene (an ezMainMenuComponent) instead of stopping the game.
  /// In the game itself ESC is a regular input action, see ConfigureInputActions().
  virtual void RequestQuit(ezStringView sRequestedBy) override;

protected:
  virtual void ConfigureInputActions() override;
  virtual void ConfigureMainCamera() override;
  virtual ezResult SpawnPlayer(ezStringView sStartPosition, const ezTransform& startPositionOffset) override;
  virtual void OnChangedMainWorld(ezWorld* pPrevWorld, ezWorld* pNewWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset) override;
  virtual void GetStartupOptions(ezString& out_sScene, ezString& out_sPreloadCollection) override;

  /// Shows UI/Loading.rml, an RmlUi document with the game's title.
  virtual ezUniquePtr<ezWorld> CreateLoadingScreenWorld(ezStringView sTargetSceneFile) override;

private:
  virtual void OnActivation(ezWorld* pWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset) override;
  virtual void BeforeWorldUpdate() override;
  virtual void AfterWorldUpdate() override;

  class ezMainMenuComponent* GetMainMenu();

  ezComponentHandle m_hMainMenu;
  bool m_bSearchedMainMenu = false;
};
