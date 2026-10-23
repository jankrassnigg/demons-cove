#pragma once

#include <Foundation/Configuration/CVar.h>

/// \file
/// All console variables of the game in one place.
///
/// CVars can be changed at runtime through the console (F1 in development builds) or set on the command line,
/// e.g. '-DemonCove.GodMode 1'. The debug ones also mark the current round as not eligible for the scoreboard.

extern ezCVarBool cvar_GodMode;
extern ezCVarInt cvar_StartStage;
extern ezCVarInt cvar_StartGems;
extern ezCVarBool cvar_DebugAutoAim;
extern ezCVarInt cvar_DebugSpawn;
extern ezCVarInt cvar_DebugStress;
extern ezCVarBool cvar_ShowStats;
extern ezCVarFloat cvar_GemAttractRange;

/// True if none of the debug CVars that make the game easier are active right now.
bool DcIsRegularRound();
