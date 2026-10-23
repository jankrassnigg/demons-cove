#include <DemonCovePlugin/DemonCovePluginPCH.h>

#include <DemonCovePlugin/Common/DcUtils.h>
#include <RendererCore/Meshes/MeshComponent.h>
#include <RendererCore/Meshes/MeshResource.h>

namespace DcUtils
{
  ezQuat LookRotation(const ezVec3& vDir)
  {
    if (vDir.IsZero(0.0001f))
      return ezQuat::MakeIdentity();

    return ezQuat::MakeShortestRotation(ezVec3(1, 0, 0), vDir.GetNormalized());
  }

  ezQuat YawRotation(const ezVec3& vDir)
  {
    return ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), ezMath::ATan2(vDir.y, vDir.x));
  }

  ezQuat YawRotation(float fYawRadian)
  {
    return ezQuat::MakeFromAxisAndAngle(ezVec3(0, 0, 1), ezAngle::MakeFromRadian(fYawRadian));
  }

  bool SegmentHitsSphere(const ezVec3& vStart, const ezVec3& vEnd, const ezVec3& vCenter, float fRadius)
  {
    const ezVec3 vSeg = vEnd - vStart;
    const float fLenSqr = vSeg.GetLengthSquared();
    float t = 0.0f;
    if (fLenSqr > 0.0f)
    {
      t = ezMath::Clamp((vCenter - vStart).Dot(vSeg) / fLenSqr, 0.0f, 1.0f);
    }

    return (vStart + vSeg * t - vCenter).GetLengthSquared() <= fRadius * fRadius;
  }

  ezGameObjectHandle SpawnPrefab(ezWorld& ref_world, const ezPrefabResourceHandle& hPrefab, const ezVec3& vPosition, const ezQuat& qRotation, ezGameObjectHandle hParent)
  {
    if (!hPrefab.IsValid())
      return {};

    ezGameObjectDesc desc;
    desc.m_bDynamic = true;
    desc.m_hParent = hParent;
    desc.m_LocalPosition = vPosition;
    desc.m_LocalRotation = qRotation;

    ezGameObject* pObject = nullptr;
    const ezGameObjectHandle hObject = ref_world.CreateObject(desc, pObject);

    ezPrefabInstantiationOptions options;
    options.m_hParent = hObject;
    ezPrefabResource::InstantiatePrefab(hPrefab, true, ref_world, ezTransform::MakeIdentity(), options);

    return hObject;
  }

  void PreloadPrefab(ezWorld& ref_world, const ezPrefabResourceHandle& hPrefab)
  {
    if (!hPrefab.IsValid())
      return;

    ezDynamicArray<ezGameObject*> rootObjects;
    ezPrefabInstantiationOptions options;
    options.m_pCreatedRootObjectsOut = &rootObjects;
    ezPrefabResource::InstantiatePrefab(hPrefab, true, ref_world, ezTransform(ezVec3(0, 0, -1000.0f)), options);

    for (ezGameObject* pRoot : rootObjects)
    {
      VisitHierarchy(pRoot, [](ezGameObject* pObject)
        {
          ezHybridArray<ezMeshComponent*, 4> meshes;
          pObject->TryGetComponentsOfBaseType(meshes);

          for (ezMeshComponent* pMesh : meshes)
          {
            if (!pMesh->GetMesh().IsValid())
              continue;

            ezResourceManager::ForceLoadResourceNow(pMesh->GetMesh());

            ezResourceLock<ezMeshResource> pMeshRes(pMesh->GetMesh(), ezResourceAcquireMode::BlockTillLoaded_NeverFail);
            if (pMeshRes.GetAcquireResult() != ezResourceAcquireResult::Final)
              continue;

            for (const ezMaterialResourceHandle& hMaterial : pMeshRes->GetMaterials())
            {
              if (hMaterial.IsValid())
                ezResourceManager::ForceLoadResourceNow(hMaterial);
            }
          } });

      ref_world.DeleteObjectDelayed(pRoot->GetHandle());
    }
  }

  ezPrefabResourceHandle LoadPrefab(ezStringView sFile, ezStringView sPropertyName)
  {
    if (sFile.IsEmpty())
    {
      ezLog::Warning("DemonCove: no prefab set for '{}'", sPropertyName);
      return {};
    }

    return ezResourceManager::LoadResource<ezPrefabResource>(sFile);
  }

  void SetMeshColor(ezWorld& ref_world, ezGameObjectHandle hObject, const ezColor& color)
  {
    ezGameObject* pRoot = nullptr;
    if (!ref_world.TryGetObject(hObject, pRoot))
      return;

    ezHybridArray<ezMeshComponentBase*, 4> meshes;
    VisitHierarchy(pRoot, [&](ezGameObject* pObject)
      {
        pObject->TryGetComponentsOfBaseType(meshes);
        for (ezMeshComponentBase* pMesh : meshes)
          pMesh->SetColor(color); });
  }

  void PlaceObject(ezWorld& ref_world, ezGameObjectHandle hObject, const ezVec3& vPosition, const ezQuat& qRotation)
  {
    ezGameObject* pObject = nullptr;
    if (ref_world.TryGetObject(hObject, pObject))
    {
      pObject->SetGlobalPosition(vPosition);
      pObject->SetGlobalRotation(qRotation);
    }
  }
} // namespace DcUtils
