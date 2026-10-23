#pragma once

#include <Core/Prefabs/PrefabResource.h>
#include <Core/World/World.h>

/// Small helpers that are shared by several components.
namespace DcUtils
{
  /// Rotation that turns the +X axis (the forward axis of all models in this project) towards vDir.
  ezQuat LookRotation(const ezVec3& vDir);

  /// Rotation around the up axis only, so that +X points along vDir projected onto the ground plane.
  ezQuat YawRotation(const ezVec3& vDir);
  ezQuat YawRotation(float fYawRadian);

  /// Returns true if the segment from vStart to vEnd passes within fRadius of vCenter.
  ///
  /// Fast moving objects (daggers) are tested along their path, otherwise they could skip over small targets in one frame.
  bool SegmentHitsSphere(const ezVec3& vStart, const ezVec3& vEnd, const ezVec3& vCenter, float fRadius);

  /// Creates an empty dynamic object at the given transform and instantiates the prefab as its child.
  ///
  /// The extra parent object gives the caller a single handle to move and delete, independent of how the prefab is structured.
  /// Returns an invalid handle if the prefab handle is invalid.
  ezGameObjectHandle SpawnPrefab(ezWorld& ref_world, const ezPrefabResourceHandle& hPrefab, const ezVec3& vPosition, const ezQuat& qRotation, ezGameObjectHandle hParent = {});

  /// Instantiates the prefab once far away and forces all meshes and materials it uses to load right away.
  ///
  /// Without this, the first instances spawned during the game would briefly show loading placeholders.
  void PreloadPrefab(ezWorld& ref_world, const ezPrefabResourceHandle& hPrefab);

  /// Loads a prefab resource, or returns an invalid handle and logs a warning if no file is set.
  ezPrefabResourceHandle LoadPrefab(ezStringView sFile, ezStringView sPropertyName);

  /// Sets the color of all mesh components on the object and all of its children.
  void SetMeshColor(ezWorld& ref_world, ezGameObjectHandle hObject, const ezColor& color);

  /// Calls func for the object and all of its children, depth first.
  template <typename Func>
  void VisitHierarchy(ezGameObject* pRoot, Func func)
  {
    ezHybridArray<ezGameObject*, 16> stack;
    stack.PushBack(pRoot);

    while (!stack.IsEmpty())
    {
      ezGameObject* pObject = stack.PeekBack();
      stack.PopBack();

      func(pObject);

      for (auto it = pObject->GetChildren(); it.IsValid(); ++it)
        stack.PushBack(it);
    }
  }

  /// Deletes the object at the end of the frame, if the handle is valid.
  inline void DeleteObject(ezWorld& ref_world, ezGameObjectHandle hObject)
  {
    if (!hObject.IsInvalidated())
      ref_world.DeleteObjectDelayed(hObject);
  }

  /// Sets the global position and rotation of the object, if it still exists.
  void PlaceObject(ezWorld& ref_world, ezGameObjectHandle hObject, const ezVec3& vPosition, const ezQuat& qRotation);
} // namespace DcUtils
