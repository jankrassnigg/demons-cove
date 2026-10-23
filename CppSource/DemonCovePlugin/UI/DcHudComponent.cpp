#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <Core/Utils/Blackboard.h>
#include <DemonCovePlugin/Arena/DcArenaComponent.h>
#include <DemonCovePlugin/Common/DcCVars.h>
#include <DemonCovePlugin/UI/DcHudComponent.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(DcHudComponent, 1, ezComponentMode::Static)
{
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
  /// Strings have to be stored as owning ezString variants, the data binding doesn't support string views.
  ezVariant FormatSeconds(float fSeconds)
  {
    ezStringBuilder sTmp;
    sTmp.SetFormat("{}", ezArgF(fSeconds, 4));
    return ezString(sTmp);
  }
} // namespace

DcHudComponent::DcHudComponent() = default;
DcHudComponent::~DcHudComponent() = default;

void DcHudComponent::Initialize()
{
  SUPER::Initialize();

  ezHashedString sName;
  sName.Assign(BlackboardName);
  m_pBoard = ezBlackboard::GetOrCreateGlobal(sName);

  // All entries have to exist before the canvas binds the blackboard, the data model is built from the entries present at that time.
  // The canvas makes sure that this function ran before it binds, see ezRmlUiCanvasComponentBase::UpdateAutobinding().
  m_pBoard->SetEntryValue("time", FormatSeconds(0.0f));
  m_pBoard->SetEntryValue("accuracy", ezString("0"));
  m_pBoard->SetEntryValue("stage", 1);
  m_pBoard->SetEntryValue("gems", 0);
  m_pBoard->SetEntryValue("hand", ezString("I"));
  m_pBoard->SetEntryValue("kills", 0);
  m_pBoard->SetEntryValue("dead", false);
  m_pBoard->SetEntryValue("reason", ezString());
  m_pBoard->SetEntryValue("godmode", false);
  m_pBoard->SetEntryValue("waiting", false);
  m_pBoard->SetEntryValue("gempulse", false);
  m_pBoard->SetEntryValue("levelup", false);
  m_pBoard->SetEntryValue("menuopen", false);
  m_pBoard->SetEntryValue("scores", ezVariantArray());
  m_pBoard->SetEntryValue("place", 0);
  m_pBoard->SetEntryValue("hasscores", false);
}

void DcHudComponent::Update()
{
  const DcArenaComponent* pArena = DcArenaComponent::FindArena(GetWorld());
  if (pArena == nullptr || m_pBoard == nullptr)
    return;

  // the visual feedback for collected gems and hand levels is purely a HUD concern, so it is tracked here
  const ezTime tNow = GetWorld()->GetClock().GetAccumulatedTime();
  const ezUInt32 uiHandLevel = pArena->GetHandLevel();

  if (pArena->GetGems() > m_uiLastGems)
    m_GemPulseEnd = tNow + ezTime::MakeFromMilliseconds(150);

  if (uiHandLevel > m_uiLastHandLevel)
    m_LevelUpEnd = tNow + ezTime::MakeFromSeconds(2);

  m_uiLastGems = pArena->GetGems();
  m_uiLastHandLevel = uiHandLevel;

  const char* szHand[] = {"I", "II", "III"};
  ezStringBuilder sAccuracy;
  sAccuracy.SetFormat("{}", ezArgF(pArena->GetAccuracy() * 100.0f, 1));

  // setting an unchanged value is cheap, the blackboard only notifies the UI about actual changes
  m_pBoard->SetEntryValue("time", FormatSeconds(pArena->GetRoundTime().AsFloatInSeconds()));
  m_pBoard->SetEntryValue("accuracy", ezString(sAccuracy));
  m_pBoard->SetEntryValue("stage", (ezInt32)pArena->GetStage());
  m_pBoard->SetEntryValue("gems", (ezInt32)pArena->GetGems());
  m_pBoard->SetEntryValue("hand", ezString(szHand[ezMath::Clamp<ezUInt32>(uiHandLevel, 1, 3) - 1]));
  m_pBoard->SetEntryValue("kills", (ezInt32)pArena->GetKills());
  m_pBoard->SetEntryValue("dead", pArena->IsPlayerDead());
  m_pBoard->SetEntryValue("reason", pArena->GetDeathReason());
  m_pBoard->SetEntryValue("godmode", (bool)cvar_GodMode);
  m_pBoard->SetEntryValue("waiting", pArena->IsWaitingForStart());
  m_pBoard->SetEntryValue("gempulse", tNow < m_GemPulseEnd);
  m_pBoard->SetEntryValue("levelup", tNow < m_LevelUpEnd);

  UpdateScores(*pArena);
}

void DcHudComponent::UpdateScores(const DcArenaComponent& arena)
{
  const DcScoreboard& scoreboard = arena.GetScoreboard();
  if (scoreboard.GetChangeCounter() == m_uiScoreboardChangeCounter && arena.GetScoreboardPlace() == m_uiScoreboardPlace)
    return;

  m_uiScoreboardChangeCounter = scoreboard.GetChangeCounter();
  m_uiScoreboardPlace = arena.GetScoreboardPlace();

  // an array of dictionaries becomes a list of structs in the data model, see 'data-for' in Hud.rml
  ezVariantArray scores;
  for (ezUInt32 i = 0; i < scoreboard.GetEntries().GetCount(); ++i)
  {
    const DcScoreboard::Entry& entry = scoreboard.GetEntries()[i];

    ezVariantDictionary score;
    score["rank"] = (ezInt32)(i + 1);
    score["time"] = FormatSeconds(entry.m_fTime);
    score["kills"] = (ezInt32)entry.m_uiKills;
    score["gems"] = (ezInt32)entry.m_uiGems;
    score["current"] = (i + 1 == m_uiScoreboardPlace);
    scores.PushBack(score);
  }

  m_pBoard->SetEntryValue("scores", scores);
  m_pBoard->SetEntryValue("place", (ezInt32)m_uiScoreboardPlace);
  m_pBoard->SetEntryValue("hasscores", !scores.IsEmpty());
}
