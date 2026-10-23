#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/Utils/Blackboard.h>
#include <DemonCovePlugin/GameState/DemonCoveGameState.h>
#include <DemonCovePlugin/UI/DcHudComponent.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/UI/MainMenuComponent.h>
#include <RmlUiPlugin/Components/RmlUiCanvas2DComponent.h>
#include <RmlUiPlugin/Resources/RmlUiResource.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(DemonCoveGameState, 1, ezRTTIDefaultAllocator<DemonCoveGameState>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

namespace
{
  /// A separate input set, so that the menu key keeps working while the menu makes its own input set exclusive.
  constexpr const char* s_szMenuInputSet = "DemonCoveMenu";

  /// The input set that ezMainMenuComponent uses for navigating the menu.
  constexpr const char* s_szMainMenuInputSet = "ezMainMenu";

  constexpr const char* s_szLoadingScreenDocument = "{ 5a3316e9-67ac-4904-af63-57f0183eebaa }"; // UI/Loading.ezRmlUiAsset
} // namespace

DemonCoveGameState::DemonCoveGameState() = default;
DemonCoveGameState::~DemonCoveGameState() = default;

void DemonCoveGameState::GetStartupOptions(ezString& out_sScene, ezString& out_sPreloadCollection)
{
  // a "-scene" argument comes from the editor or from testing, otherwise the game always starts in the arena
  if (ezCommandLineUtils::GetGlobalInstance()->HasOption("-scene"))
  {
    out_sScene = ezCommandLineUtils::GetGlobalInstance()->GetStringOption("-scene");
  }
  else
  {
    out_sScene = "AssetCache/Common/Scenes/Main.ezBinScene";
  }

  // a collection with the same name as the scene lists the assets to load while the loading screen is shown
  ezStringBuilder sPreloadCollection = out_sScene;
  sPreloadCollection.ChangeFileExtension("ezBinCollection");
  if (ezFileSystem::ExistsFile(sPreloadCollection))
  {
    out_sPreloadCollection = sPreloadCollection;
  }
}

void DemonCoveGameState::OnChangedMainWorld(ezWorld* pPrevWorld, ezWorld* pNewWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset)
{
  // the menu of the previous world is gone
  m_hMainMenu.Invalidate();
  m_bSearchedMainMenu = false;

  SUPER::OnChangedMainWorld(pPrevWorld, pNewWorld, sStartPosition, startPositionOffset);
}

ezUniquePtr<ezWorld> DemonCoveGameState::CreateLoadingScreenWorld(ezStringView sTargetSceneFile)
{
  ezUniquePtr<ezWorld> pWorld = SUPER::CreateLoadingScreenWorld(sTargetSceneFile);

  EZ_LOCK(pWorld->GetWriteMarker());

  ezGameObjectDesc desc;
  desc.m_sName.Assign("LoadingScreen");

  ezGameObject* pObject = nullptr;
  pWorld->CreateObject(desc, pObject);

  ezRmlUiCanvas2DComponent* pCanvas = nullptr;
  ezRmlUiCanvas2DComponent::CreateComponent(pObject, pCanvas);
  pCanvas->SetPassInput(false);
  pCanvas->SetRmlResource(ezResourceManager::LoadResource<ezRmlUiResource>(s_szLoadingScreenDocument));

  return pWorld;
}

void DemonCoveGameState::ConfigureInputActions()
{
  // The gameplay actions are configured in RuntimeConfigs/InputConfig.ddl, which the game application loads.
  // Only the application level shortcuts and the menu key are set up in code.

  if (auto pApp = ezGameApplication::GetGameApplicationInstance())
  {
    // ESC opens the menu instead of closing the application, the developer shortcuts are only there in development builds.
    // F12 screenshots are kept in shipping builds, they are useful for players as well.
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezBitflags<ezGameApplicationInputFlags> flags = ezGameApplicationInputFlags::All;
    flags.Remove(ezGameApplicationInputFlags::Dev_EscapeToClose);
#else
    ezBitflags<ezGameApplicationInputFlags> flags = ezGameApplicationInputFlags::Regular;
    flags.Add(ezGameApplicationInputFlags::Dev_Screenshot);
#endif
    pApp->RegisterGameApplicationInputActions(flags);
  }

  ezInputActionConfig cfg;
  cfg.m_bApplyTimeScaling = false;
  cfg.m_sInputSlotTrigger[0] = ezInputSlot_KeyEscape;
  cfg.m_sInputSlotTrigger[1] = ezInputSlot_Controller0_ButtonStart;
  ezInputManager::SetInputActionConfig(s_szMenuInputSet, "open", cfg, true);
}

ezMainMenuComponent* DemonCoveGameState::GetMainMenu()
{
  if (m_pMainWorld == nullptr)
    return nullptr;

  if (!m_bSearchedMainMenu)
  {
    m_bSearchedMainMenu = true;
    m_hMainMenu = ezMainMenuComponent::FindInWorld(*m_pMainWorld);
  }

  ezMainMenuComponent* pMenu = nullptr;
  if (!m_pMainWorld->TryGetComponent(m_hMainMenu, pMenu))
    return nullptr;

  return pMenu;
}

void DemonCoveGameState::RequestQuit(ezStringView sRequestedBy)
{
  if (m_pMainWorld != nullptr && sRequestedBy == "editor-esc")
  {
    EZ_LOCK(m_pMainWorld->GetWriteMarker());

    if (ezMainMenuComponent* pMenu = GetMainMenu())
    {
      pMenu->OpenMenu();
      return;
    }
  }

  SUPER::RequestQuit(sRequestedBy);
}

void DemonCoveGameState::ProcessInput()
{
  SUPER::ProcessInput();

  if (m_pMainWorld == nullptr)
    return;

  // the game state runs outside of the world update, so accessing the world requires locking it
  bool bMenuOpen = false;
  {
    EZ_LOCK(m_pMainWorld->GetWriteMarker());
    ezMainMenuComponent* pMenu = GetMainMenu();
    bMenuOpen = pMenu != nullptr && pMenu->IsMenuOpen();
  }

  if (!bMenuOpen && ezInputManager::GetInputActionState(s_szMenuInputSet, "open") == ezKeyState::Pressed)
  {
    EZ_LOCK(m_pMainWorld->GetWriteMarker());

    if (ezMainMenuComponent* pMenu = GetMainMenu())
    {
      pMenu->OpenMenu();
      bMenuOpen = true;
    }
    else
    {
      // without a menu in the scene, ESC quits like in any other ez application
      RequestQuit("menu-key");
    }
  }

  // The HUD hides the death screen while the menu is open, the two would overlap.
  // The menu pauses the world, so the HUD component can't find out itself, it doesn't get updated then.
  if (ezSharedPtr<ezBlackboard> pHud = ezBlackboard::FindGlobal(ezTempHashedString(DcHudComponent::BlackboardName)))
  {
    pHud->SetEntryValue("menuopen", bMenuOpen);
  }

  // while the menu is open, the game doesn't get any input
  if (bMenuOpen)
  {
    ezInputManager::SetExclusiveInputSet(s_szMainMenuInputSet);
  }
  else if (ezInputManager::GetExclusiveInputSet() == s_szMainMenuInputSet)
  {
    ezInputManager::SetExclusiveInputSet("");
  }
}
