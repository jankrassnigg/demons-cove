#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <DemonCovePlugin/Common/DcCVars.h>

ezCVarBool cvar_GodMode("DemonCove.GodMode", false, ezCVarFlags::Default, "Debug: enemies can't kill the player and a wall around the arena prevents falling off.");
ezCVarInt cvar_StartStage("DemonCove.StartStage", 1, ezCVarFlags::Default, "Debug: the stage a new round starts at. Stages are numbered from 1, the last one is the endless phase.");
ezCVarInt cvar_StartGems("DemonCove.StartGems", 0, ezCVarFlags::Default, "Debug: the number of gems the player has at the start of a round. Determines the hand level.");
ezCVarBool cvar_DebugAutoAim("DemonCove.DebugAutoAim", false, ezCVarFlags::Default, "Debug: daggers fly towards the closest enemy instead of the view direction, and the view turns towards it. Used for automated tests.");
ezCVarInt cvar_DebugSpawn("DemonCove.DebugSpawn", -1, ezCVarFlags::Default, "Debug: set to an enemy type index to spawn one of them (0 = Squid I, 1 = Squid II, 5 = Thief, 6 = Centipede). Resets itself to -1.");
ezCVarInt cvar_DebugStress("DemonCove.DebugStress", 0, ezCVarFlags::Default, "Debug: set to 1 to spawn the worst case number of enemies and gems at once, for performance measurements. Resets itself to 0.");
ezCVarBool cvar_ShowStats("DemonCove.ShowStats", false, ezCVarFlags::Default, "Shows dagger and enemy statistics on screen.");
ezCVarFloat cvar_GemAttractRange("DemonCove.GemAttractRange", 7.0f, ezCVarFlags::Default, "Distance at which gems start flying towards the player.");

bool DcIsRegularRound()
{
  return !cvar_GodMode && !cvar_DebugAutoAim && cvar_StartStage <= 1 && cvar_StartGems <= 0;
}
