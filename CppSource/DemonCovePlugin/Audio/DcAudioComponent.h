#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <DemonCovePlugin/DemonCovePluginDLL.h>

using DcAudioComponentManager = ezComponentManagerSimple<class DcAudioComponent, ezComponentUpdateType::WhenSimulating>;

/// All one-shot sounds of the game. Each one maps to a sound asset property of DcAudioComponent.
enum class DcSound : ezUInt8
{
  Shotgun,
  DaggerStream,
  DaggerHit,
  WeakSpotHit,
  GemPickup,
  LevelUp,
  PlayerDeath,
  RoundStart,
  DaggerPickup,
  SpawnWarning,
  SquidSpawn,
  SquidSwarm,
  Skull2Dash,
  ThiefSteal,
  Skull1Death,
  Skull2Death,
  SkeletonDeath,
  ThiefDeath,
  SquidDeath,
  CentipedeDeath,
  None, ///< Plays nothing.
  Count = None
};

/// Plays the game's sound effects and music.
///
/// Gameplay code only says *what* happened (DcSound::GemPickup) and the sound assets are configured here in the editor.
/// That way all audio can be tweaked in one place, without touching the components that trigger it.
///
/// The music crossfades between an ambient loop (no round running) and an action track (round running).
/// Which one is wanted is set from the outside with SetActionMusic(). Lives next to the DcArenaComponent.
class DcAudioComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(DcAudioComponent, ezComponent, DcAudioComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // DcAudioComponent

public:
  DcAudioComponent();
  ~DcAudioComponent();

  /// Plays the sound at the given position. Does nothing if no asset is set for it.
  ///
  /// Each sound plays at most once per 40 ms, so that a burst of hits or kills in one frame doesn't stack up into one very loud sound.
  void Play(DcSound sound, const ezVec3& vPosition);

  /// Fades towards the action music (true) or the ambient music (false).
  /// The action track restarts from the beginning every time it gets selected.
  void SetActionMusic(bool bAction) { m_bActionMusic = bAction; }

  ezString m_sShotgun;        // [ property ]
  ezString m_sDaggerStream;   // [ property ]
  ezString m_sDaggerHit;      // [ property ]
  ezString m_sWeakSpotHit;    // [ property ]
  ezString m_sGemPickup;      // [ property ]
  ezString m_sLevelUp;        // [ property ]
  ezString m_sPlayerDeath;    // [ property ]
  ezString m_sRoundStart;     // [ property ]
  ezString m_sDaggerPickup;   // [ property ]
  ezString m_sSpawnWarning;   // [ property ]
  ezString m_sSquidSpawn;     // [ property ]
  ezString m_sSquidSwarm;     // [ property ]
  ezString m_sSkull2Dash;     // [ property ]
  ezString m_sThiefSteal;     // [ property ]
  ezString m_sSkull1Death;    // [ property ]
  ezString m_sSkull2Death;    // [ property ]
  ezString m_sSkeletonDeath;  // [ property ]
  ezString m_sThiefDeath;     // [ property ]
  ezString m_sSquidDeath;     // [ property ]
  ezString m_sCentipedeDeath; // [ property ]

  ezString m_sMusicAmbient;   // [ property ] loops while no round is running
  ezString m_sMusicAction;    // [ property ] starts from the beginning with every round

private:
  void Update();
  ezStringView GetSoundFile(DcSound sound) const;

  /// Creates a child object with a paused, silent sound component for one music track.
  ezComponentHandle CreateMusicTrack(const ezString& sFile);

  ezTime m_LastPlayed[(int)DcSound::Count];

  ezComponentHandle m_hMusicAmbient;
  ezComponentHandle m_hMusicAction;
  bool m_bActionMusic = false;
  float m_fActionBlend = 0.0f; ///< 0 = only ambient music, 1 = only action music.
};
