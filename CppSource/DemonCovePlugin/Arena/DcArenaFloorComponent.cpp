#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <DemonCovePlugin/Arena/DcArenaFloorComponent.h>
#include <DemonCovePlugin/Common/DcUtils.h>
#include <GameEngine/Gameplay/GreyBoxComponent.h>
#include <JoltPlugin/Actors/JoltStaticActorComponent.h>
#include <JoltPlugin/Shapes/JoltShapeBoxComponent.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(DcArenaFloorComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Radius", m_fRadius)->AddAttributes(new ezDefaultValueAttribute(22.0f), new ezClampValueAttribute(5.0f, 200.0f)),
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

namespace
{
  constexpr float s_fTileSize = 2.0f;
  constexpr float s_fTileThickness = 1.5f;
  constexpr float s_fTileWarnDistance = 1.5f; ///< Tiles this close to the crumble radius turn red.
  constexpr float s_fTileFallDepth = 40.0f;   ///< Fallen tiles are deleted this far below the arena.

  const ezColor s_TileColor(0.3f, 0.26f, 0.25f);
  const ezColor s_TileWarnColor(1.0f, 0.3f, 0.2f);
} // namespace

DcArenaFloorComponent::DcArenaFloorComponent() = default;
DcArenaFloorComponent::~DcArenaFloorComponent() = default;

void DcArenaFloorComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  // the grey box is only a stand-in for editing, the game uses tiles that can fall off
  if (ezGreyBoxComponent* pPreview = nullptr; GetOwner()->TryGetComponentOfBaseType(pPreview))
  {
    pPreview->SetActiveFlag(false);
  }

  BuildTiles();
}

void DcArenaFloorComponent::OnDeactivated()
{
  // the tile objects are deleted together with the world, only the bookkeeping has to go
  m_Tiles.Clear();
  m_TileGrid.Clear();
  m_hSafetyWall.Invalidate();

  SUPER::OnDeactivated();
}

void DcArenaFloorComponent::BuildTiles()
{
  const ezVec3 vCenter = GetOwner()->GetGlobalPosition();
  m_iTileGridHalfSize = (ezInt32)ezMath::Ceil(m_fRadius / s_fTileSize);
  const ezInt32 iGridSize = m_iTileGridHalfSize * 2 + 1;

  m_Tiles.Clear();
  m_TileGrid.Clear();
  m_TileGrid.SetCount(iGridSize * iGridSize, ezInvalidIndex);

  for (ezInt32 y = -m_iTileGridHalfSize; y <= m_iTileGridHalfSize; ++y)
  {
    for (ezInt32 x = -m_iTileGridHalfSize; x <= m_iTileGridHalfSize; ++x)
    {
      const ezVec3 vOffset(x * s_fTileSize, y * s_fTileSize, 0);
      const float fDistance = vOffset.GetLength();
      if (fDistance > m_fRadius - s_fTileSize * 0.25f)
        continue;

      m_TileGrid[(y + m_iTileGridHalfSize) * iGridSize + (x + m_iTileGridHalfSize)] = m_Tiles.GetCount();

      Tile& tile = m_Tiles.ExpandAndGetRef();
      tile.m_vPosition = vCenter + vOffset;
      tile.m_fDistance = fDistance;
      tile.m_hObject = CreateTileObject(tile, true);
    }
  }

  m_fCurrentRadius = m_fRadius;
}

ezGameObjectHandle DcArenaFloorComponent::CreateTileObject(const Tile& tile, bool bWithCollision)
{
  // a small gap between the tiles makes the grid visible
  const float fHalfSize = s_fTileSize * 0.5f - 0.02f;

  ezGameObjectDesc desc;
  desc.m_sName.Assign("FloorTile");
  desc.m_hParent = GetOwner()->GetHandle();
  desc.m_LocalPosition = tile.m_vPosition - GetOwner()->GetGlobalPosition();
  desc.m_bDynamic = !bWithCollision; // falling tiles are moved every frame

  ezGameObject* pTile = nullptr;
  const ezGameObjectHandle hTile = GetWorld()->CreateObject(desc, pTile);

  ezGreyBoxComponent* pBox = nullptr;
  ezGreyBoxComponent::CreateComponent(pTile, pBox);
  pBox->SetShape(ezGreyBoxShape::Box);
  pBox->SetSizeNegX(fHalfSize);
  pBox->SetSizePosX(fHalfSize);
  pBox->SetSizeNegY(fHalfSize);
  pBox->SetSizePosY(fHalfSize);
  pBox->SetSizeNegZ(s_fTileThickness);
  pBox->SetSizePosZ(0.0f);
  pBox->SetColor(tile.m_bWarned ? s_TileWarnColor : s_TileColor);

  if (bWithCollision)
  {
    // the shape sits on a child object, because it has to be centered while the tile's origin is at its top
    ezGameObjectDesc shapeDesc;
    shapeDesc.m_hParent = hTile;
    shapeDesc.m_LocalPosition.Set(0, 0, -s_fTileThickness * 0.5f);

    ezGameObject* pShapeObj = nullptr;
    GetWorld()->CreateObject(shapeDesc, pShapeObj);

    ezJoltShapeBoxComponent* pShape = nullptr;
    ezJoltShapeBoxComponent::CreateComponent(pShapeObj, pShape);
    pShape->SetHalfExtents(ezVec3(s_fTileSize * 0.5f, s_fTileSize * 0.5f, s_fTileThickness * 0.5f));

    // the actor collects all shapes on its own object and its children
    ezJoltStaticActorComponent* pActor = nullptr;
    ezJoltStaticActorComponent::CreateComponent(pTile, pActor);
  }

  return hTile;
}

void DcArenaFloorComponent::Restore()
{
  for (Tile& tile : m_Tiles)
  {
    if (!tile.m_bFallen && !tile.m_bWarned)
      continue;

    DcUtils::DeleteObject(*GetWorld(), tile.m_hObject);

    tile.m_bFallen = false;
    tile.m_bWarned = false;
    tile.m_fFallSpeed = 0.0f;
    tile.m_hObject = CreateTileObject(tile, true);
  }

  m_fCurrentRadius = m_fRadius;
}

void DcArenaFloorComponent::SetCrumbleRadius(float fRadius)
{
  m_fCurrentRadius = ezMath::Min(fRadius, m_fRadius);

  // only warn while the floor is actually crumbling
  if (m_fCurrentRadius >= m_fRadius)
    return;

  for (Tile& tile : m_Tiles)
  {
    if (tile.m_bFallen)
      continue;

    if (tile.m_fDistance > m_fCurrentRadius)
    {
      // physics objects can't be moved around, replace the tile with one without collision that falls down
      DcUtils::DeleteObject(*GetWorld(), tile.m_hObject);
      tile.m_bFallen = true;
      tile.m_fFallSpeed = 0.0f;
      tile.m_hObject = CreateTileObject(tile, false);
    }
    else if (!tile.m_bWarned && tile.m_fDistance > m_fCurrentRadius - s_fTileWarnDistance)
    {
      tile.m_bWarned = true;

      ezGameObject* pObject = nullptr;
      ezGreyBoxComponent* pBox = nullptr;
      if (GetWorld()->TryGetObject(tile.m_hObject, pObject) && pObject->TryGetComponentOfBaseType(pBox))
      {
        pBox->SetColor(s_TileWarnColor);
      }
    }
  }
}

void DcArenaFloorComponent::Update()
{
  const float fDeltaTime = GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds();
  const float fDeleteHeight = GetOwner()->GetGlobalPosition().z - s_fTileFallDepth;

  for (Tile& tile : m_Tiles)
  {
    if (!tile.m_bFallen)
      continue;

    ezGameObject* pObject = nullptr;
    if (!GetWorld()->TryGetObject(tile.m_hObject, pObject))
      continue;

    tile.m_fFallSpeed += 6.0f * fDeltaTime;
    pObject->SetGlobalPosition(pObject->GetGlobalPosition() - ezVec3(0, 0, tile.m_fFallSpeed * fDeltaTime));

    if (pObject->GetGlobalPosition().z < fDeleteHeight)
    {
      DcUtils::DeleteObject(*GetWorld(), tile.m_hObject);
      tile.m_hObject.Invalidate();
    }
  }
}

bool DcArenaFloorComponent::IsOverFloor(const ezVec3& vPosition) const
{
  const ezVec3 vRel = vPosition - GetOwner()->GetGlobalPosition();
  const ezInt32 x = (ezInt32)ezMath::Round(vRel.x / s_fTileSize) + m_iTileGridHalfSize;
  const ezInt32 y = (ezInt32)ezMath::Round(vRel.y / s_fTileSize) + m_iTileGridHalfSize;
  const ezInt32 iGridSize = m_iTileGridHalfSize * 2 + 1;

  if (x < 0 || y < 0 || x >= iGridSize || y >= iGridSize)
    return false;

  const ezUInt32 uiTile = m_TileGrid[y * iGridSize + x];
  return uiTile != ezInvalidIndex && !m_Tiles[uiTile].m_bFallen;
}

void DcArenaFloorComponent::SetSafetyWall(bool bEnable)
{
  const bool bHasWall = !m_hSafetyWall.IsInvalidated();
  if (bHasWall == bEnable)
    return;

  if (bHasWall)
  {
    DcUtils::DeleteObject(*GetWorld(), m_hSafetyWall);
    m_hSafetyWall.Invalidate();
    return;
  }

  // a ring of boxes along the arena edge, all part of one static actor
  constexpr ezUInt32 uiNumSegments = 48;
  constexpr float fHeight = 6.0f;
  constexpr float fThickness = 0.5f;
  const float fSegmentHalfLength = m_fRadius * ezMath::Pi<float>() / uiNumSegments * 1.1f;

  ezGameObjectDesc desc;
  desc.m_sName.Assign("SafetyWall");
  desc.m_hParent = GetOwner()->GetHandle();

  ezGameObject* pRoot = nullptr;
  m_hSafetyWall = GetWorld()->CreateObject(desc, pRoot);

  for (ezUInt32 i = 0; i < uiNumSegments; ++i)
  {
    const ezAngle angle = ezAngle::MakeFromDegree(360.0f * i / uiNumSegments);

    ezGameObjectDesc segDesc;
    segDesc.m_hParent = m_hSafetyWall;
    segDesc.m_LocalPosition = ezVec3(ezMath::Cos(angle), ezMath::Sin(angle), 0) * (m_fRadius + fThickness) + ezVec3(0, 0, fHeight * 0.5f);
    segDesc.m_LocalRotation = ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), angle);

    ezGameObject* pSegment = nullptr;
    GetWorld()->CreateObject(segDesc, pSegment);

    ezJoltShapeBoxComponent* pBox = nullptr;
    ezJoltShapeBoxComponent::CreateComponent(pSegment, pBox);
    pBox->SetHalfExtents(ezVec3(fThickness, fSegmentHalfLength, fHeight * 0.5f));
  }

  ezJoltStaticActorComponent* pActor = nullptr;
  ezJoltStaticActorComponent::CreateComponent(pRoot, pActor);
}

void DcArenaFloorComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  ezReflectionSerializer::WriteObjectToBinary(inout_stream.GetStream(), GetDynamicRTTI(), this);
}

void DcArenaFloorComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  ezReflectionSerializer::ReadObjectPropertiesFromBinary(inout_stream.GetStream(), *GetDynamicRTTI(), this);
}
