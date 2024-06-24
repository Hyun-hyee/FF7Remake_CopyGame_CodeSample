#include "PhysX_Utility.h"
#include "MeshContainer.h"
#include "Transform.h"
#include "Engine_Defines.h"
#include "PhysX_Manager.h"

void ENGINE_DLL Engine::PhysX_Utility::Convert_PxVec3FromMeshData(PxVec3* In_PxVec3, FMeshData* pMeshData)
{
	_uint iNumVertices(pMeshData->iNumVertices);
	for (_uint i(0); i < iNumVertices; ++i)
	{
		memcpy(&In_PxVec3[i], &pMeshData->vecVertices[i], sizeof(PxVec3));
	}

}

void ENGINE_DLL Engine::PhysX_Utility::Convert_PxVec3FromMeshDataWithTransformMatrix(PxVec3* In_PxVec3, FMeshData* pMeshData, FXMMATRIX In_TransformMatrix)
{
	_uint iNumVertices(pMeshData->iNumVertices);
	_vector vPosition(XMVectorSet(0.f, 0.f, 0.f, 0.f));

	for (_uint i(0); i < iNumVertices; ++i)
	{		
		vPosition = XMVector3TransformCoord(XMLoadFloat3(&pMeshData-> vecVertices[i]), In_TransformMatrix);
		memcpy(&In_PxVec3[i], &vPosition, sizeof(PxVec3));		
	}
}

PxExtendedVec3 ENGINE_DLL Engine::PhysX_Utility::Convert_PxExtendedVec3(FXMVECTOR In_Vector)
{
	return PxExtendedVec3(XMVectorGetX(In_Vector), XMVectorGetY(In_Vector), XMVectorGetZ(In_Vector));
}

PxExtendedVec3 ENGINE_DLL Engine::PhysX_Utility::Convert_PxExtendedVec3(PxVec3 In_Vector)
{
	return PxExtendedVec3(In_Vector.x, In_Vector.y, In_Vector.z);
}

PxVec3 ENGINE_DLL Engine::PhysX_Utility::Convert_PxVec3(FXMVECTOR In_Vector)
{
	return PxVec3(XMVectorGetX(In_Vector), XMVectorGetY(In_Vector), XMVectorGetZ(In_Vector));
}

PxVec3 ENGINE_DLL Engine::PhysX_Utility::Convert_PxVec3(const XMFLOAT3& In_Float3)
{
	return PxVec3(In_Float3.x, In_Float3.y, In_Float3.z);
}

PxVec3 ENGINE_DLL Engine::PhysX_Utility::Convert_PxVec3(PxExtendedVec3 In_Vector)
{
	return PxVec3(_float(In_Vector.x), _float(In_Vector.y), _float(In_Vector.z));
}

XMVECTOR ENGINE_DLL Engine::PhysX_Utility::Convert_Vector(const PxVec3& In_PxVec3)
{
	return XMVectorSet(_float(In_PxVec3.x), _float(In_PxVec3.y), _float(In_PxVec3.z), 0.f);
}

XMVECTOR ENGINE_DLL Engine::PhysX_Utility::Convert_Vector(const PxVec4& In_PxVec4)
{
	return XMVectorSet(_float(In_PxVec4.x), _float(In_PxVec4.y), _float(In_PxVec4.z), _float(In_PxVec4.w));
}

XMVECTOR ENGINE_DLL Engine::PhysX_Utility::Convert_PxExtendedVec3ToVector(const PxExtendedVec3& In_PxVec3)
{
	return XMVectorSet(_float(In_PxVec3.x), _float(In_PxVec3.y), _float(In_PxVec3.z), 0.f);
}

PxQuat ENGINE_DLL Engine::PhysX_Utility::Convert_PxQuat(FXMVECTOR In_Vector)
{
	return PxQuat(In_Vector.m128_f32[0], In_Vector.m128_f32[1], In_Vector.m128_f32[2], In_Vector.m128_f32[3]);
}

void ENGINE_DLL Engine::PhysX_Utility::PhysXColliderDesc::Setting_StaticCollider(PHYSXCOLLIDER_DESC& Out_Desc, PHYSXCOLLIDER_TYPE eColliderType, _uint iFilterType, shared_ptr<CTransform> pTransform, const _bool bTrigger, shared_ptr<CCommonModelComp> pModel, _float3 vMaterial)
{
	Out_Desc.eShape = eColliderType;
	Out_Desc.eActorType = PHYSXACTOR_TYPE::ACTOR_STATIC;
	Out_Desc.iFilterType = iFilterType;
	Out_Desc.fDensity = 5.f;
	Out_Desc.vAngles = XMLoadFloat3(&Extract_PitchYawRollFromRotationMatrix(Get_RotationMatrix(pTransform->Get_WorldMatrix())));
	Out_Desc.vPosition = pTransform->Get_State(CTransform::STATE_POSITION);
	Out_Desc.vScale = Get_ScaleFromMatrix(pTransform->Get_WorldFloat4x4());
	PxMaterial* pMaterial = nullptr;
	GET_SINGLE(CPhysX_Manager)->Create_Material(vMaterial.x, vMaterial.y, vMaterial.z,&pMaterial);
	Out_Desc.pMaterial = pMaterial;
	Out_Desc.bTrigger = bTrigger;
	Out_Desc.bKinematic = false;
	Out_Desc.pModelCom = pModel.get();
}

void ENGINE_DLL Engine::PhysX_Utility::PhysXColliderDesc::Setting_DynamicCollider(PHYSXCOLLIDER_DESC& Out_Desc, PHYSXCOLLIDER_TYPE eColliderType, _uint iFilterType, shared_ptr<CTransform> pTransform, const _bool bTrigger,shared_ptr<CCommonModelComp> pModel, _bool bKinematic, _float3 vMaterial, _float fMass)
{
	Out_Desc.eShape = eColliderType;
	Out_Desc.eActorType = PHYSXACTOR_TYPE::ACTOR_DYNAMIC;
	Out_Desc.iFilterType = iFilterType;
	Out_Desc.fDensity = 5.f;
	Out_Desc.vAngles = XMLoadFloat3(&Extract_PitchYawRollFromRotationMatrix(Get_RotationMatrix(pTransform->Get_WorldMatrix())));
	Out_Desc.vPosition = pTransform->Get_State(CTransform::STATE_POSITION);
	Out_Desc.vScale = Get_ScaleFromMatrix(pTransform->Get_WorldFloat4x4());
	PxMaterial* pMaterial = nullptr;
	GET_SINGLE(CPhysX_Manager)->Create_Material(vMaterial.x, vMaterial.y, vMaterial.z, &pMaterial);
	Out_Desc.pMaterial = pMaterial;
	Out_Desc.bTrigger = bTrigger;
	Out_Desc.bKinematic = bKinematic;
	Out_Desc.pModelCom = pModel.get();
	Out_Desc.fMass = fMass;
}

void ENGINE_DLL Engine::PhysX_Utility::PhysXColliderDesc::Setting_StaticCollider_WithScale(PHYSXCOLLIDER_DESC& Out_Desc, PHYSXCOLLIDER_TYPE eColliderType, _uint iFilterType, shared_ptr<CTransform> pTransform, _float3 vScale, const _bool bTrigger, shared_ptr<CCommonModelComp> pModel, _float3 vMaterial)
{
	Out_Desc.eShape = eColliderType;
	Out_Desc.eActorType = PHYSXACTOR_TYPE::ACTOR_STATIC;
	Out_Desc.iFilterType = iFilterType;
	Out_Desc.fDensity = 5.f;
	Out_Desc.vAngles = XMLoadFloat3(&Extract_PitchYawRollFromRotationMatrix(Get_RotationMatrix(pTransform->Get_WorldMatrix())));
	Out_Desc.vPosition = pTransform->Get_State(CTransform::STATE_POSITION);
	Out_Desc.vScale = XMLoadFloat3(&vScale);
	PxMaterial* pMaterial = nullptr;
	GET_SINGLE(CPhysX_Manager)->Create_Material(vMaterial.x, vMaterial.y, vMaterial.z, &pMaterial);
	Out_Desc.pMaterial = pMaterial;
	Out_Desc.bTrigger = bTrigger;
	Out_Desc.bKinematic = false;
	Out_Desc.pModelCom = pModel.get();
}

void ENGINE_DLL Engine::PhysX_Utility::PhysXColliderDesc::Setting_DynamicCollider_WithScale(PHYSXCOLLIDER_DESC& Out_Desc, PHYSXCOLLIDER_TYPE eColliderType, _uint iFilterType, shared_ptr<CTransform> pTransform, _float3 vScale, const _bool bTrigger,  shared_ptr<CCommonModelComp> pModel,_bool bKinematic, _float3 vMaterial ,_float fMass)
{
	Out_Desc.eShape = eColliderType;
	Out_Desc.eActorType = PHYSXACTOR_TYPE::ACTOR_DYNAMIC;
	Out_Desc.iFilterType = iFilterType;
	Out_Desc.fDensity = 5.f;
	Out_Desc.vAngles = XMLoadFloat3(&Extract_PitchYawRollFromRotationMatrix(Get_RotationMatrix(pTransform->Get_WorldMatrix())));
	Out_Desc.vPosition = pTransform->Get_State(CTransform::STATE_POSITION);
	Out_Desc.vScale = XMLoadFloat3(&vScale);
	PxMaterial* pMaterial = nullptr;
	GET_SINGLE(CPhysX_Manager)->Create_Material(vMaterial.x, vMaterial.y, vMaterial.z, &pMaterial);
	Out_Desc.pMaterial = pMaterial;
	Out_Desc.bTrigger = bTrigger;
	Out_Desc.bKinematic = bKinematic;
	Out_Desc.pModelCom = pModel.get();
	Out_Desc.fMass = fMass;
}

void ENGINE_DLL PhysX_Utility::PhysXControllerDesc::Setting_Controller(PHYSXCONTROLLER_DESC& Out_Desc, shared_ptr<CTransform> pTransform, _uint iFilterType, _float fHeight, _float fHalfWidthX , _float fHalfWidthZ , PxControllerShapeType::Enum eShapeType, _float3 vMaterial)
{
	Out_Desc.iFilterType = iFilterType;
	PxMaterial* pMaterial = nullptr;
	GET_SINGLE(CPhysX_Manager)->Create_Material(vMaterial.x, vMaterial.y, vMaterial.z, &pMaterial);
	Out_Desc.eShapeType = eShapeType;
	if (eShapeType == PxControllerShapeType::eCAPSULE)
	{
		Out_Desc.CapsuleControllerDesc.height = fHeight;
		Out_Desc.CapsuleControllerDesc.radius = fHalfWidthX;
		Out_Desc.CapsuleControllerDesc.material = pMaterial;
		Out_Desc.CapsuleControllerDesc.position = Convert_PxExtendedVec3(pTransform->Get_State(CTransform::STATE_POSITION));
		Out_Desc.CapsuleControllerDesc.stepOffset = 0.01f;
	}
	else
	{
		Out_Desc.BoxControllerDesc.halfHeight = fHeight * 0.5f;
		Out_Desc.BoxControllerDesc.halfSideExtent = fHalfWidthX;
		Out_Desc.BoxControllerDesc.halfForwardExtent = fHalfWidthZ;
		Out_Desc.BoxControllerDesc.material = pMaterial;
		Out_Desc.BoxControllerDesc.position = Convert_PxExtendedVec3(pTransform->Get_State(CTransform::STATE_POSITION));
		Out_Desc.BoxControllerDesc.stepOffset = 0.01f;
	}
}
