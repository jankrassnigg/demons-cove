#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <DemonCovePlugin/Arena/DcScoreboard.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Strings/StringConversion.h>

namespace
{
  /// ':appdata' is a writable data directory in the user's app data folder, mounted by ezGameApplicationBase.
  constexpr const char* s_szScoresFile = ":appdata/Scores.txt";
} // namespace

void DcScoreboard::Load()
{
  m_Entries.Clear();
  ++m_uiChangeCounter;

  ezFileReader file;
  if (file.Open(s_szScoresFile).Failed())
    return;

  ezStringBuilder sContent;
  sContent.ReadAll(file);

  ezHybridArray<ezStringView, 16> lines;
  sContent.Split(false, lines, "\n", "\r");

  for (ezStringView sLine : lines)
  {
    ezHybridArray<ezStringView, 4> parts;
    sLine.Split(false, parts, " ");

    double fTime = 0.0;
    ezInt64 iKills = 0, iGems = 0;
    if (parts.GetCount() != 3 || ezConversionUtils::StringToFloat(parts[0], fTime).Failed() ||
        ezConversionUtils::StringToInt64(parts[1], iKills).Failed() || ezConversionUtils::StringToInt64(parts[2], iGems).Failed())
      continue;

    Entry& entry = m_Entries.ExpandAndGetRef();
    entry.m_fTime = (float)fTime;
    entry.m_uiKills = (ezUInt32)iKills;
    entry.m_uiGems = (ezUInt32)iGems;

    if (m_Entries.GetCount() == MaxEntries)
      break;
  }

  m_Entries.Sort([](const Entry& a, const Entry& b)
    { return a.m_fTime > b.m_fTime; });
}

void DcScoreboard::Save() const
{
  ezFileWriter file;
  if (file.Open(s_szScoresFile).Failed())
  {
    ezLog::Warning("DemonCove: could not save the scoreboard to '{}'", s_szScoresFile);
    return;
  }

  ezStringBuilder sContent;
  for (const Entry& entry : m_Entries)
  {
    sContent.AppendFormat("{} {} {}\n", ezArgF(entry.m_fTime, 4), entry.m_uiKills, entry.m_uiGems);
  }

  file.WriteBytes(sContent.GetData(), sContent.GetElementCount()).IgnoreResult();
}

ezUInt32 DcScoreboard::AddRound(const Entry& round)
{
  ezUInt32 uiIndex = 0;
  while (uiIndex < m_Entries.GetCount() && m_Entries[uiIndex].m_fTime >= round.m_fTime)
    ++uiIndex;

  if (uiIndex >= MaxEntries)
    return 0;

  m_Entries.InsertAt(uiIndex, round);

  if (m_Entries.GetCount() > MaxEntries)
    m_Entries.PopBack();

  ++m_uiChangeCounter;
  Save();

  return uiIndex + 1;
}
