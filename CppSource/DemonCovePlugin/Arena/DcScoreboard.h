#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Time/Time.h>

/// The best rounds, stored as a text file in the user's app data folder (one 'time kills gems' line per round).
///
/// This is a plain class and not a component: it has no presence in the world, it is just data that the DcArenaComponent owns.
class DcScoreboard
{
public:
  static constexpr ezUInt32 MaxEntries = 10;

  struct Entry
  {
    float m_fTime = 0.0f;
    ezUInt32 m_uiKills = 0;
    ezUInt32 m_uiGems = 0;
  };

  /// Reads the scoreboard file. Missing or broken lines are skipped.
  void Load();

  /// Inserts the round if it is good enough and saves the file.
  /// Returns the place it got, starting at 1, or 0 if it didn't make it onto the board.
  ezUInt32 AddRound(const Entry& round);

  /// Best round first.
  const ezHybridArray<Entry, MaxEntries>& GetEntries() const { return m_Entries; }

  /// Incremented whenever the entries change, so that displays know when to update.
  ezUInt32 GetChangeCounter() const { return m_uiChangeCounter; }

private:
  void Save() const;

  ezHybridArray<Entry, MaxEntries> m_Entries;
  ezUInt32 m_uiChangeCounter = 0;
};
