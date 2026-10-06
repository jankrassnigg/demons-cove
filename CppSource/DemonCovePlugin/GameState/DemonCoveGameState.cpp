#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <Core/Input/InputManager.h>
#include <GameEngine/UI/MainMenuComponent.h>
#include <Core/System/Window.h>
#include <Core/World/World.h>
#include <DemonCovePlugin/GameState/DemonCoveGameState.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Logging/Log.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/MeshComponent.h>
#include <RmlUiPlugin/Components/RmlUiCanvas2DComponent.h>
#include <RmlUiPlugin/Resources/RmlUiResource.h>


EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(DemonCoveGameState, 1, ezRTTIDefaultAllocator<DemonCoveGameState>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

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
    out_sScene = "AssetCache/Common/Scenes/Arena.ezBinScene";
  }

  // a collection with the same name as the scene lists the assets to load while the loading screen is shown

  ezStringBuilder sPreloadCollection = out_sScene;
  sPreloadCollection.ChangeFileExtension("ezBinCollection");
  if (ezFileSystem::ExistsFile(sPreloadCollection))
  {
    out_sPreloadCollection = sPreloadCollection;
  }
}

void DemonCoveGameState::OnActivation(ezWorld* pWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset)
{
  EZ_LOG_BLOCK("GameState::Activate");

  SUPER::OnActivation(pWorld, sStartPosition, startPositionOffset);

  // the main entry point when the game starts
  // could do some setup here, but in a lot of cases it is better to leave this as is
  // and instead override the various other virtual functions that the game state provides
  // see below and see ezGameState for additional details
}

void DemonCoveGameState::AfterWorldUpdate()
{
  SUPER::AfterWorldUpdate();

}

void DemonCoveGameState::BeforeWorldUpdate()
{
  SUPER::BeforeWorldUpdate();

  EZ_LOCK(m_pMainWorld->GetWriteMarker());

  // if you need to modify the world, this is a good place to do it
}

ezResult DemonCoveGameState::SpawnPlayer(ezStringView sStartPosition, const ezTransform& startPositionOffset)
{
  // replace this to create a custom player object or load a prefab
  return SUPER::SpawnPlayer(sStartPosition, startPositionOffset);
}

void DemonCoveGameState::OnChangedMainWorld(ezWorld* pPrevWorld, ezWorld* pNewWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset)
{
  m_hMainMenu.Invalidate();
  m_bSearchedMainMenu = false;

  SUPER::OnChangedMainWorld(pPrevWorld, pNewWorld, sStartPosition, startPositionOffset);

  // called whenever the main world is changed, ie when transitioning between levels
  // may need to update references to the world here or reset some state
}


namespace
{
  constexpr const char* s_szMenuInputSet = "DemonCoveMenu";
  constexpr const char* s_szLoadingScreenDocument = "{ 5a3316e9-67ac-4904-af63-57f0183eebaa }"; // UI/Loading.ezRmlUiAsset
} // namespace

ezUniquePtr<ezWorld> DemonCoveGameState::CreateLoadingScreenWorld(ezStringView sTargetSceneFile)
{
  ezUniquePtr<ezWorld> pWorld = SUPER::CreateLoadingScreenWorld(sTargetSceneFile);

  EZ_LOCK(pWorld->GetWriteMarker());

  ezGameObjectDesc desc;
  desc.m_sName.Assign("LoadingScreen");

  ezGameObject* pObject = nullptr;
  pWorld->CreateObject(desc, pObject);

  ezRmlUiCanvas2DComponent* pCanvas = nullptr;
  pWorld->GetOrCreateComponentManager<ezRmlUiCanvas2DComponentManager>()->CreateComponent(pObject, pCanvas);
  pCanvas->SetPassInput(false);
  pCanvas->SetRmlResource(ezResourceManager::LoadResource<ezRmlUiResource>(s_szLoadingScreenDocument));

  return pWorld;
}

void DemonCoveGameState::ConfigureInputActions()
{
  if (auto pApp = ezGameApplication::GetGameApplicationInstance())
  {
    // ESC opens the menu instead of closing the application, the developer shortcuts are only there in development builds.
    // F12 screenshots are kept in shipping builds, they are useful for players as well.
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    ezBitflags<ezGameApplicationInputFlags> flags = ezGameApplicationInputFlags::All;
    flags.Remove(ezGameApplicationInputFlags::Dev_EscapeToClose);
    pApp->RegisterGameApplicationInputActions(flags);
#else
    ezBitflags<ezGameApplicationInputFlags> flags = ezGameApplicationInputFlags::Regular;
    flags.Add(ezGameApplicationInputFlags::Dev_Screenshot);
    pApp->RegisterGameApplicationInputActions(flags);
#endif
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

  // the HUD hides the death screen while the menu is open, the two would overlap
  if (ezSharedPtr<ezBlackboard> pHud = ezBlackboard::FindGlobal(ezTempHashedString("hud")))
  {
    pHud->SetEntryValue("menuopen", bMenuOpen);
  }

  // while the menu is open, the game doesn't get any input
  if (bMenuOpen)
  {
    ezInputManager::SetExclusiveInputSet("ezMainMenu");
  }
  else if (ezInputManager::GetExclusiveInputSet() == "ezMainMenu")
  {
    ezInputManager::SetExclusiveInputSet("");
  }
}

void DemonCoveGameState::ConfigureMainCamera()
{
  SUPER::ConfigureMainCamera();

  // do custom camera setup here
}
