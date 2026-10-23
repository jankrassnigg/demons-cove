#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <DemonCovePlugin/DemonCovePluginDLL.h>

using DcArenaFloorComponentManager = ezComponentManagerSimple<class DcArenaFloorComponent, ezComponentUpdateType::WhenSimulating>;

/// The round platform the game takes place on, built from square tiles so that it can crumble away from the edge.
///
/// In the editor, a grey box on the same object shows the shape of the arena. When the simulation starts, this component
/// hides it and builds the tiles instead (render geometry plus a static physics shape each, for the character controller).
///
/// The DcArenaComponent decides how far the floor has crumbled (see SetCrumbleRadius()), this component only does the visuals,
/// physics and IsOverFloor() queries. Tiles turn red shortly before they fall.
class DcArenaFloorComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(DcArenaFloorComponent, ezComponent, DcArenaFloorComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

protected:
  virtual void OnSimulationStarted() override;
  virtual void OnDeactivated() override;

  //////////////////////////////////////////////////////////////////////////
  // DcArenaFloorComponent

public:
  DcArenaFloorComponent();
  ~DcArenaFloorComponent();

  /// True if there is an intact tile below the position (height is ignored).
  bool IsOverFloor(const ezVec3& vPosition) const;

  /// Tiles further away from the center than this fall down. Tiles close to the radius are marked as a warning first.
  /// Passing a value >= Radius doesn't crumble anything.
  void SetCrumbleRadius(float fRadius);

  /// The radius of the intact part of the floor.
  float GetCurrentRadius() const { return m_fCurrentRadius; }

  /// Replaces all fallen and warned tiles with intact ones.
  void Restore();

  /// Surrounds the arena with an invisible wall, so that the player can't fall off. Used by the god mode.
  void SetSafetyWall(bool bEnable);

  float m_fRadius = 22.0f; // [ property ]

private:
  struct Tile
  {
    ezVec3 m_vPosition;
    float m_fDistance = 0.0f; ///< Distance of the tile center to the arena center.
    float m_fFallSpeed = 0.0f;
    bool m_bWarned = false;
    bool m_bFallen = false;
    ezGameObjectHandle m_hObject;
  };

  void Update();
  void BuildTiles();
  ezGameObjectHandle CreateTileObject(const Tile& tile, bool bWithCollision);

  ezDynamicArray<Tile> m_Tiles;
  ezDynamicArray<ezUInt32> m_TileGrid; ///< Tile index per grid cell, ezInvalidIndex where there is no tile.
  ezInt32 m_iTileGridHalfSize = 0;     ///< The grid spans from -m_iTileGridHalfSize to +m_iTileGridHalfSize cells in X and Y.
  float m_fCurrentRadius = 0.0f;

  ezGameObjectHandle m_hSafetyWall;
};
