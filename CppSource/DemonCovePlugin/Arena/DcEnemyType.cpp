#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <DemonCovePlugin/Arena/DcEnemyType.h>

namespace
{
  const ezColor s_NoTint(1.0f, 1.0f, 1.0f);
  const ezColor s_CentipedeTint(1.0f, 0.35f, 0.3f);

  // clang-format off
  const DcEnemyTypeInfo s_EnemyTypes[] = {
    // name                 health radius gems spots ground big    death sound                 tint
    {"Squid I",             12.0f, 1.3f,  2,   1,    false, true,  DcSound::SquidDeath,     s_NoTint},
    {"Squid II",            25.0f, 1.6f,  4,   2,    false, true,  DcSound::SquidDeath,     s_NoTint},
    {"Skull I",             1.0f,  0.4f,  0,   0,    false, false, DcSound::Skull1Death,    s_NoTint},
    {"Skull II",            5.0f,  0.5f,  1,   0,    false, false, DcSound::Skull2Death,    s_NoTint},
    {"Skeleton",            3.0f,  0.5f,  0,   0,    true,  false, DcSound::SkeletonDeath,  s_NoTint},
    {"Thief",               8.0f,  0.7f,  1,   0,    true,  false, DcSound::ThiefDeath,     s_NoTint},
    {"Centipede",           25.0f, 0.9f,  5,   0,    false, true,  DcSound::CentipedeDeath, s_CentipedeTint},
    {"Centipede segment",   3.0f,  0.6f,  1,   0,    false, false, DcSound::None,           s_CentipedeTint},
  };
  // clang-format on

  static_assert(EZ_ARRAY_SIZE(s_EnemyTypes) == (int)DcEnemyType::Count, "one entry per enemy type");
} // namespace

const DcEnemyTypeInfo& DcGetEnemyTypeInfo(DcEnemyType type)
{
  return s_EnemyTypes[(int)type];
}
