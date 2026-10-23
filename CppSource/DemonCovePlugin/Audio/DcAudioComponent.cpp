#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <Core/Interfaces/SoundInterface.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <DemonCovePlugin/Audio/DcAudioComponent.h>
#include <MiniAudioPlugin/Components/MiniAudioSoundComponent.h>
#include <MiniAudioPlugin/Resources/MiniAudioSoundResource.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(DcAudioComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Shotgun", m_sShotgun)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package), new ezGroupAttribute("Player")),
    EZ_MEMBER_PROPERTY("DaggerStream", m_sDaggerStream)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("DaggerHit", m_sDaggerHit)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("WeakSpotHit", m_sWeakSpotHit)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("GemPickup", m_sGemPickup)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("LevelUp", m_sLevelUp)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("PlayerDeath", m_sPlayerDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("RoundStart", m_sRoundStart)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("DaggerPickup", m_sDaggerPickup)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SpawnWarning", m_sSpawnWarning)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package), new ezGroupAttribute("Enemies")),
    EZ_MEMBER_PROPERTY("SquidSpawn", m_sSquidSpawn)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SquidSwarm", m_sSquidSwarm)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("Skull2Dash", m_sSkull2Dash)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("ThiefSteal", m_sThiefSteal)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("Skull1Death", m_sSkull1Death)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("Skull2Death", m_sSkull2Death)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SkeletonDeath", m_sSkeletonDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("ThiefDeath", m_sThiefDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("SquidDeath", m_sSquidDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("CentipedeDeath", m_sCentipedeDeath)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
    EZ_MEMBER_PROPERTY("MusicAmbient", m_sMusicAmbient)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package), new ezGroupAttribute("Music")),
    EZ_MEMBER_PROPERTY("MusicAction", m_sMusicAction)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_MiniAudio_Sound", ezDependencyFlags::Package)),
  }
  EZ_END_PROPERTIES;

  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("DemonCove"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE
// clang-format on

DcAudioComponent::DcAudioComponent() = default;
DcAudioComponent::~DcAudioComponent() = default;

void DcAudioComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  m_hMusicAmbient = CreateMusicTrack(m_sMusicAmbient);
  m_hMusicAction = CreateMusicTrack(m_sMusicAction);
  m_fActionBlend = 0.0f;
}

ezComponentHandle DcAudioComponent::CreateMusicTrack(const ezString& sFile)
{
  if (sFile.IsEmpty())
    return {};

  ezGameObjectDesc desc;
  desc.m_sName.Assign("Music");
  desc.m_hParent = GetOwner()->GetHandle();

  ezGameObject* pObject = nullptr;
  GetWorld()->CreateObject(desc, pObject);

  ezMiniAudioSoundComponent* pSound = nullptr;
  const ezComponentHandle hSound = ezMiniAudioSoundComponent::CreateComponent(pObject, pSound);
  pSound->SetSound(ezResourceManager::LoadResource<ezMiniAudioSoundResource>(sFile));
  pSound->SetNoGlobalPitch(true); // the arena's hit pause slows down the world clock, the music should not be affected
  pSound->SetVolume(0.0f);
  pSound->SetPaused(true);        // Update() starts it
  return hSound;
}

ezStringView DcAudioComponent::GetSoundFile(DcSound sound) const
{
  switch (sound)
  {
    case DcSound::Shotgun:
      return m_sShotgun;
    case DcSound::DaggerStream:
      return m_sDaggerStream;
    case DcSound::DaggerHit:
      return m_sDaggerHit;
    case DcSound::WeakSpotHit:
      return m_sWeakSpotHit;
    case DcSound::GemPickup:
      return m_sGemPickup;
    case DcSound::LevelUp:
      return m_sLevelUp;
    case DcSound::PlayerDeath:
      return m_sPlayerDeath;
    case DcSound::RoundStart:
      return m_sRoundStart;
    case DcSound::DaggerPickup:
      return m_sDaggerPickup;
    case DcSound::SpawnWarning:
      return m_sSpawnWarning;
    case DcSound::SquidSpawn:
      return m_sSquidSpawn;
    case DcSound::SquidSwarm:
      return m_sSquidSwarm;
    case DcSound::Skull2Dash:
      return m_sSkull2Dash;
    case DcSound::ThiefSteal:
      return m_sThiefSteal;
    case DcSound::Skull1Death:
      return m_sSkull1Death;
    case DcSound::Skull2Death:
      return m_sSkull2Death;
    case DcSound::SkeletonDeath:
      return m_sSkeletonDeath;
    case DcSound::ThiefDeath:
      return m_sThiefDeath;
    case DcSound::SquidDeath:
      return m_sSquidDeath;
    case DcSound::CentipedeDeath:
      return m_sCentipedeDeath;
    default:
      return {};
  }
}

void DcAudioComponent::Play(DcSound sound, const ezVec3& vPosition)
{
  const ezStringView sFile = GetSoundFile(sound);
  if (sFile.IsEmpty())
    return;

  // real time, the world clock may be slowed down
  const ezTime tNow = ezTime::Now();
  ezTime& tLast = m_LastPlayed[(int)sound];
  if (tNow - tLast < ezTime::MakeFromMilliseconds(40))
    return;

  tLast = tNow;

  // fire and forget: the sound interface creates a temporary sound source that deletes itself when done
  ezSoundInterface::PlaySound(GetWorld(), sFile, ezTransform(vPosition)).IgnoreResult();
}

void DcAudioComponent::Update()
{
  // real time, so that the hit pause doesn't stall the fade
  const float fDeltaTime = ezMath::Min(ezClock::GetGlobalClock()->GetTimeDiff().AsFloatInSeconds(), 0.1f);

  ezMiniAudioSoundComponent* pAmbient = nullptr;
  ezMiniAudioSoundComponent* pAction = nullptr;
  if (!GetWorld()->TryGetComponent(m_hMusicAmbient, pAmbient))
    pAmbient = nullptr;
  if (!GetWorld()->TryGetComponent(m_hMusicAction, pAction))
    pAction = nullptr;

  if (m_bActionMusic)
  {
    if (m_fActionBlend == 0.0f && pAction)
    {
      // start from the beginning, a previous round's track was faded out
      pAction->SetVolume(0.0f);
      pAction->Play();
    }

    m_fActionBlend = ezMath::Min(1.0f, m_fActionBlend + fDeltaTime / 1.5f);

    if (pAction)
      pAction->SetVolume(m_fActionBlend);
  }
  else if (m_fActionBlend > 0.0f)
  {
    // the action track fades out on its own, the ambient music comes back slowly below
    if (pAction)
      pAction->FadeOut(ezTime::MakeFromSeconds(3.0));

    m_fActionBlend = 0.0f;
  }

  if (pAmbient)
  {
    const float fTarget = 1.0f - m_fActionBlend;
    const float fVolume = pAmbient->GetVolume();
    pAmbient->SetVolume(fTarget > fVolume ? ezMath::Min(fTarget, fVolume + fDeltaTime / 4.0f) : fTarget);

    // pause the silent track instead of playing it at zero volume
    if (pAmbient->GetVolume() > 0.0f)
    {
      if (pAmbient->GetPaused())
        pAmbient->Play();
    }
    else if (!pAmbient->GetPaused())
    {
      pAmbient->Pause();
    }
  }
}

void DcAudioComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  ezReflectionSerializer::WriteObjectToBinary(inout_stream.GetStream(), GetDynamicRTTI(), this);
}

void DcAudioComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  ezReflectionSerializer::ReadObjectPropertiesFromBinary(inout_stream.GetStream(), *GetDynamicRTTI(), this);
}
