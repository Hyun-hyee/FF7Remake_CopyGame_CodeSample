#pragma once

#include "Engine_Defines.h"

BEGIN(Engine)
class FMeshData;
class CCommonModelComp;
class CTransform;

namespace PhysX_Utility
{
	struct PHYSXCOLLIDER_DESC //PhysX_Collider 정보
	{
		PHYSXCOLLIDER_DESC()
		{
			ZeroMemory(this, sizeof(PHYSXCOLLIDER_DESC));
		}

		CCommonModelComp*	pModelCom;
		PxMaterial*			pMaterial;
		PHYSXACTOR_TYPE		eActorType; 
		XMVECTOR			vPosition; 
		XMVECTOR			vAngles;
		PHYSXCOLLIDER_TYPE	eShape;
		_uint				iFilterType;
		XMVECTOR			vScale;
		float				fDensity;
		_float				fMass;
		_bool				bTrigger;
		_bool				bKinematic;	
	};

	struct PHYSXCONTROLLER_DESC //PhysX_Controller 정보
	{
		PxControllerShapeType::Enum	eShapeType;
		PxBoxControllerDesc			BoxControllerDesc;
		PxCapsuleControllerDesc		CapsuleControllerDesc;
		_uint iFilterType;
	};

	void			ENGINE_DLL Convert_PxVec3FromMeshData(PxVec3* In_PxVec3, FMeshData*  pMeshData);
	void			ENGINE_DLL Convert_PxVec3FromMeshDataWithTransformMatrix(PxVec3* In_PxVec3, FMeshData* pMeshData, FXMMATRIX In_TransformMatrix);
	PxExtendedVec3	ENGINE_DLL Convert_PxExtendedVec3(FXMVECTOR In_Vector);
	PxExtendedVec3	ENGINE_DLL Convert_PxExtendedVec3(PxVec3 In_Vector);
	PxVec3			ENGINE_DLL Convert_PxVec3(FXMVECTOR In_Vector);
	PxVec3			ENGINE_DLL Convert_PxVec3(const XMFLOAT3& In_Float3);
	PxVec3			ENGINE_DLL Convert_PxVec3(PxExtendedVec3 In_Vector);
	XMVECTOR		ENGINE_DLL Convert_Vector(const PxVec3& In_PxVec3);
	XMVECTOR		ENGINE_DLL Convert_Vector(const PxVec4& In_PxVec4);
	XMVECTOR		ENGINE_DLL Convert_PxExtendedVec3ToVector(const PxExtendedVec3& In_PxVec3);
	PxQuat			ENGINE_DLL Convert_PxQuat(FXMVECTOR In_Vector);
	
	/*PHYSXCOLLIDER_DESC 설정 함수*/
	namespace PhysXColliderDesc
	{
		// Transform의 Scale정보 반영해서 PHYSXCOLLIDER_DESC 설정
		void ENGINE_DLL Setting_StaticCollider(PHYSXCOLLIDER_DESC& Out_Desc, PHYSXCOLLIDER_TYPE eColliderType, _uint iFilterType, shared_ptr< CTransform> pTransform, const _bool bTrigger = false, shared_ptr<CCommonModelComp> pModel = nullptr, _float3 vMaterial = { 0.5f, 0.5f, 0.3f });
		void ENGINE_DLL Setting_DynamicCollider(PHYSXCOLLIDER_DESC& Out_Desc, PHYSXCOLLIDER_TYPE eColliderType, _uint iFilterType, shared_ptr< CTransform> pTransform, const _bool bTrigger = false,shared_ptr<CCommonModelComp> pModel = nullptr, _bool bKinematic = false, _float3 vMaterial = { 0.7f, 0.7f, 0.3f }, _float fMass = { 0.f });

		// 입력한 Scale값 반영해서 PHYSXCOLLIDER_DESC 설정
		void ENGINE_DLL Setting_StaticCollider_WithScale(PHYSXCOLLIDER_DESC& Out_Desc, PHYSXCOLLIDER_TYPE eColliderType, _uint iFilterType, shared_ptr< CTransform> pTransform, _float3 vScale, const _bool bTrigger = false,  shared_ptr<CCommonModelComp> pModel = nullptr, _float3 vMaterial = { 0.5f, 0.5f, 0.3f });
		void ENGINE_DLL Setting_DynamicCollider_WithScale(PHYSXCOLLIDER_DESC& Out_Desc, PHYSXCOLLIDER_TYPE eColliderType, _uint iFilterType, shared_ptr< CTransform> pTransform, _float3 vScale, const _bool bTrigger = false, shared_ptr<CCommonModelComp> pModel = nullptr, _bool bKinematic = false, _float3 vMaterial = { 0.7f, 0.7f, 0.3f }, _float fMass = {0.f});
	}

	/*PHYSXCONTROLLER_DESC 설정 함수*/
	namespace PhysXControllerDesc
	{
		void ENGINE_DLL Setting_Controller(PHYSXCONTROLLER_DESC& Out_Desc, shared_ptr< CTransform> pTransform, _uint iFilterType, _float fHeight = 1.5f, _float fHalfWidthX = 0.5f, _float fHalfWidthZ = 0.5f, PxControllerShapeType::Enum eShapeType = PxControllerShapeType::Enum::eCAPSULE, _float3 vMaterial = {0.5f, 0.5f, 0.5f});
	}
}
using namespace PhysX_Utility;

END

