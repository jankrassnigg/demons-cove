#pragma once

#include <GameEngine/GameApplication/GameApplication.h>

/// The stand-alone game executable. All game code is in the DemonCovePlugin, this only locates the project and starts the game state.
class DemonCoveGame : public ezGameApplication
{
public:
  using SUPER = ezGameApplication;

  DemonCoveGame();

protected:
  virtual ezResult BeforeCoreSystemsStartup() override;
  virtual void AfterCoreSystemsStartup() override;

private:
  ezResult TryProjectFolder(ezStringView sPath);
  void DetermineProjectPath();
};
