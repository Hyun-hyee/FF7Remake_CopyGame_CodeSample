#include "PhysX_Collider.h"
#include "GameObject.h"
#include "PhysX_Manager.h"
#include "Transform.h"

#include "MeshContainer.h"
#include "CommonModelComp.h"
#include "PartObject.h"


_uint CPhysX_Collider::m_iNewColliderID = 0;

IMPLEMENT_CREATE(CPhysX_Collider)
IMPLEMENT_CLONE(CPhysX_Collider, CComponent)

CPhysX_Collider::CPhysX_Collider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent(pDevice, pContext)
{
}

CPhysX_Collider::CPhysX_Collider(const CPhysX_Collider& rhs)
	: CComponent(rhs)
{
}

HRESULT CPhysX_Collider::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPhysX_Collider::Initialize(void* pArg)
{
	__super::Initialize(pArg);

	m_iColliderID = m_iNewColliderID++;

	if (nullptr != pArg)
	{
		m_PhysXColliderDesc = *(PHYSXCOLLIDER_DESC*)pArg;
		CreatePhysXActor(m_PhysXColliderDesc);
		GET_SINGLE(CPhysX_Manager)->Register_Collider(static_pointer_cast<CPhysX_Collider>(shared_from_this()));
	}
	return S_OK;
}

void CPhysX_Collider::PhysX_OnCollision_Enter(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
	m_pOwner.lock()->PhysX_OnCollision_Enter(pThisCol, pOtherCol, ContactInfo);
}

void CPhysX_Collider::PhysX_OnCollision_Stay(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
	m_pOwner.lock()->PhysX_OnCollision_Stay(pThisCol, pOtherCol, ContactInfo);
}

void CPhysX_Collider::PhysX_OnCollision_Exit(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
	m_pOwner.lock()->PhysX_OnCollision_Exit(pThisCol, pOtherCol, ContactInfo);
}

void CPhysX_Collider::PhysX_Raycast_Stay(weak_ptr<CGameObject> pOther, _uint iOtherColLayer, _float4 vOrigin, _float4 vDirection, _float4 vColPosition)
{
	m_pOwner.lock()->PhysX_Raycast_Stay(pOther, iOtherColLayer, vOrigin, vDirection, vColPosition);
}

_vector CPhysX_Collider::Get_Position()
{
	PxTransform	Transform;
	if (m_pRigidDynamic)
		Transform = m_pRigidDynamic->getGlobalPose();

	if (m_pRigidStatic)
		Transform = m_pRigidStatic->getGlobalPose();

	return XMVectorSet(Transform.p.x, Transform.p.y, Transform.p.z, 1.f);
}

_vector CPhysX_Collider::Get_Quaternion()
{
	PxTransform	Transform;
	if (m_pRigidDynamic)
		Transform = m_pRigidDynamic->getGlobalPose();

	if (m_pRigidStatic)
		Transform = m_pRigidStatic->getGlobalPose();

	_vector vQuaternion = { Transform.q.x, Transform.q.y, Transform.q.z, Transform.q.w };

	return vQuaternion;
}

_vector CPhysX_Collider::Get_Velocity()
{
	if (m_pRigidDynamic)
	{
		PxVec3 Velocity = m_pRigidDynamic->getLinearVelocity();
		return XMVectorSet(Velocity.x, Velocity.y, Velocity.z, 0.f);
	}

	return _vector{ 0.f, 0.f, 0.f, 0.f };
}

_float CPhysX_Collider::Get_Mess()
{
	assert(m_pRigidDynamic);

	PxReal fMess = m_pRigidDynamic->getMass();
	return fMess;
}

_vector CPhysX_Collider::Get_AngularVelocity()
{
	assert(m_pRigidDynamic);

	PxVec3 vVelocity = m_pRigidDynamic->getAngularVelocity();
	return XMVectorSet(vVelocity.x, vVelocity.y, vVelocity.z, 0.f);
}

_vector CPhysX_Collider::Get_LinearVelocity()
{
	assert(m_pRigidDynamic);

	PxVec3 vVelocity = m_pRigidDynamic->getLinearVelocity();
	return XMVectorSet(vVelocity.x, vVelocity.y, vVelocity.z, 0.f);
}

_matrix CPhysX_Collider::Get_WorldMatrix()
{
	PxTransform	Transform;
	if (m_pRigidDynamic)
		Transform = m_pRigidDynamic->getGlobalPose();

	if (m_pRigidStatic)
		Transform = m_pRigidStatic->getGlobalPose();

	_vector vPos = { Transform.p.x, Transform.p.y, Transform.p.z };
	vPos.m128_f32[3] = 1.f;
	_vector vQuaternion = { Transform.q.x, Transform.q.y, Transform.q.z, Transform.q.w };

	_matrix ResultMatrix = XMMatrixRotationQuaternion(vQuaternion);
	ResultMatrix.r[3] = vPos;

	return ResultMatrix;
}

HRESULT CPhysX_Collider::Set_Position(_vector _vPos, _vector _vQuaternion, _bool bIsSimulation)
{
	if (m_bSimulation)
	{
		PxTransform	Transform;
		PxVec3	vPos(XMVectorGetX(_vPos), XMVectorGetY(_vPos), XMVectorGetZ(_vPos));
		PxQuat	vQuaternion(XMVectorGetX(_vQuaternion), XMVectorGetY(_vQuaternion), XMVectorGetZ(_vQuaternion), XMVectorGetW(_vQuaternion));

		Transform.p = vPos;
		Transform.q = vQuaternion;

		if (m_pRigidDynamic)
		{
			if (m_PhysXColliderDesc.bKinematic && bIsSimulation)
				m_pRigidDynamic->setKinematicTarget(Transform);
			else
				m_pRigidDynamic->setGlobalPose(Transform);
		}

		if (m_pRigidStatic)
			m_pRigidStatic->setGlobalPose(Transform);

	}

	return S_OK;
}

HRESULT CPhysX_Collider::Set_Position(_vector _vPos, _bool bIsSimulation)
{
	if (m_bSimulation)
	{
		PxTransform	Transform;
		PxVec3	vPos(XMVectorGetX(_vPos), XMVectorGetY(_vPos), XMVectorGetZ(_vPos));

		if (m_pRigidDynamic)
			Transform = m_pRigidDynamic->getGlobalPose();

		if (m_pRigidStatic)
			Transform = m_pRigidStatic->getGlobalPose();

		Transform.p = vPos;

		if (m_pRigidDynamic)
		{
			if (m_PhysXColliderDesc.bKinematic && bIsSimulation)
				m_pRigidDynamic->setKinematicTarget(Transform);
			else
				m_pRigidDynamic->setGlobalPose(Transform);
		}

		if (m_pRigidStatic)
			m_pRigidStatic->setGlobalPose(Transform);

	}

	return S_OK;
}

void CPhysX_Collider::Set_ActorFlag(PxActorFlag::Enum eFlag, _bool bState)
{
	if (m_pRigidDynamic)
	{
		m_pRigidDynamic->setActorFlag(eFlag, bState);
	}

	if (m_pRigidStatic)
	{
		m_pRigidStatic->setActorFlag(eFlag, bState);
	}
}


void CPhysX_Collider::Delete_Collider()
{
	if (m_pRigidDynamic)
	{
		m_pRigidDynamic->release();
		m_pRigidDynamic = nullptr;
	}

	if (m_pRigidStatic)
	{
		m_pRigidStatic->release();
		m_pRigidStatic = nullptr;
	}
}

void CPhysX_Collider::Create_Collider()
{
	if (!m_pRigidDynamic && !m_pRigidStatic)
	{
		CreatePhysXActor(m_PhysXColliderDesc);
	}
}

HRESULT CPhysX_Collider::Add_Force(_vector _vForce)
{
	PxVec3	vForce;
	vForce = { XMVectorGetX(_vForce), XMVectorGetY(_vForce), XMVectorGetZ(_vForce) };

	if (m_pRigidDynamic)
		m_pRigidDynamic->addForce(vForce);

	return S_OK;
}

HRESULT CPhysX_Collider::Clear_Force()
{
	if (m_pRigidDynamic)
	{
		m_pRigidDynamic->clearForce(PxForceMode::eACCELERATION);
		m_pRigidDynamic->clearForce(PxForceMode::eFORCE);
		m_pRigidDynamic->clearForce(PxForceMode::eIMPULSE);
		m_pRigidDynamic->clearForce(PxForceMode::eVELOCITY_CHANGE);
	}

	m_pRigidDynamic->putToSleep();
	m_pRigidDynamic->wakeUp();
	return S_OK;
}

HRESULT CPhysX_Collider::Clear_Velocity()
{
	m_pRigidDynamic->setLinearVelocity(PxVec3(0.f));
	return S_OK;
}

HRESULT CPhysX_Collider::Add_LinearVelocityResistance(_vector vResistanceRate)
{
	PxVec3	vResistRate = { XMVectorGetX(vResistanceRate), XMVectorGetY(vResistanceRate), XMVectorGetZ(vResistanceRate) };
	PxVec3	vVelocity = m_pRigidDynamic->getLinearVelocity();
	vVelocity.x *= vResistRate.x;
	vVelocity.y *= vResistRate.y;
	vVelocity.z *= vResistRate.z;

	m_pRigidDynamic->setLinearVelocity(vVelocity);

	return S_OK;
}

void CPhysX_Collider::Init_MeshCollider(FMeshData* pMeshData)
{
	PxU32 iNumVertices;
	PxU32 iNumIndices;
	PxVec3* pVertices;
	_uint* pIndices;
	PxTriangleMeshDesc meshDesc;

	{
		iNumVertices = pMeshData->iNumVertices;
		iNumIndices = pMeshData->iNumIndices;
		pVertices = new PxVec3[iNumVertices];
		pIndices = new _uint[pMeshData->iNumIndices];

		PhysX_Utility::Convert_PxVec3FromMeshData(pVertices, pMeshData);

		meshDesc.points.count = iNumVertices;
		meshDesc.points.stride = sizeof(PxVec3);
		meshDesc.points.data = pVertices;

		for (PxU32 i = 0; i < iNumIndices; ++i)
		{
			memcpy(&pIndices[i], &pMeshData->vecIndices[i], sizeof(_uint));
		}

		meshDesc.triangles.count = iNumIndices / 3.f;
		meshDesc.triangles.stride = 3 * sizeof(PxU32);
		meshDesc.triangles.data = pIndices;

		m_TriangleMesh.push_back(nullptr);

		GET_SINGLE(CPhysX_Manager)->Create_MeshFromTriangles(meshDesc, &m_TriangleMesh.back());

		if (m_TriangleMesh.back() == nullptr)
		{
			assert(false);
		}

		Safe_Delete_Array(pVertices);
		Safe_Delete_Array(pIndices);
	}

}


void CPhysX_Collider::Init_ModelCollider(CCommonModelComp* pModelData)
{
	if (pModelData == nullptr)
	{
		assert(false);
	}

	const vector<shared_ptr<CMeshComp>> vecMeshComps = pModelData->Get_MeshComps();

	for (_uint i = 0; i < vecMeshComps.size(); ++i)
	{
		Init_MeshCollider(vecMeshComps[i]->Get_MeshData());
	}
}



void CPhysX_Collider::Synchronize_Transform(weak_ptr<CTransform> pTransform, _fvector In_vLocalOffset, _fvector In_vWorldOffset)
{
	/*Non-Kinematic만 적용*/
	if (m_PhysXColliderDesc.bKinematic)
		return;

	PxTransform	Transform;
	if (m_pRigidDynamic)
		Transform = m_pRigidDynamic->getGlobalPose();

	if (m_pRigidStatic)
		Transform = m_pRigidStatic->getGlobalPose();

	_vector vPos = { Transform.p.x, Transform.p.y, Transform.p.z };

	_vector vLocalOffset = XMVector3TransformCoord(In_vLocalOffset, Get_RotationMatrix(pTransform.lock()->Get_WorldMatrix()));
	vPos += In_vWorldOffset + vLocalOffset;
	vPos.m128_f32[3] = 1.f;
	_vector vQuaternion = { Transform.q.x, Transform.q.y, Transform.q.z, Transform.q.w };
	pTransform.lock()->Set_State(CTransform::STATE_POSITION, vPos);
	pTransform.lock()->Rotation_Quaternion(vQuaternion);
}

void CPhysX_Collider::Synchronize_Transform_Position(weak_ptr<CTransform> pTransform)
{
	PxTransform	Transform;
	if (m_pRigidDynamic)
		Transform = m_pRigidDynamic->getGlobalPose();

	if (m_pRigidStatic)
		Transform = m_pRigidStatic->getGlobalPose();

	_vector vPos = { Transform.p.x, Transform.p.y, Transform.p.z };
	pTransform.lock()->Set_State(CTransform::STATE_POSITION, vPos);
}

void CPhysX_Collider::Synchronize_Transform_Rotation(weak_ptr<CTransform> pTransform)
{
	PxTransform	Transform;
	if (m_pRigidDynamic)
		Transform = m_pRigidDynamic->getGlobalPose();

	if (m_pRigidStatic)
		Transform = m_pRigidStatic->getGlobalPose();

	_vector vQuaternion = { Transform.q.x, Transform.q.y, Transform.q.z, Transform.q.w };
	pTransform.lock()->Rotation_Quaternion(vQuaternion);
}

_matrix CPhysX_Collider::Synchronize_Matrix(_fmatrix In_WorldMatrix)
{
	_matrix ResultMatrix = Get_ScaleMatrix(In_WorldMatrix);

	ResultMatrix *= Get_WorldMatrix();

	return ResultMatrix;
}

void CPhysX_Collider::Synchronize_Collider(weak_ptr<CTransform> pTransform, _fvector In_vLocalOffset, _fvector In_vWorldOffset, _bool bIsSimulation)
{
	if (m_bSimulation)
	{
		_vector vLocalOffset = XMVector3TransformCoord(In_vLocalOffset, Get_RotationMatrix(pTransform.lock()->Get_WorldMatrix()));
		_vector vPos = pTransform.lock()->Get_State(CTransform::STATE_POSITION);
		vPos += In_vWorldOffset + vLocalOffset;
		vPos.m128_f32[3] = 1.f;
		_vector vQuaternion = XMQuaternionRotationMatrix(Get_RotationMatrix(pTransform.lock()->Get_WorldMatrix()));
		Set_Position(vPos, vQuaternion, bIsSimulation);

		m_vLocalOffset = vLocalOffset;
		m_vWorldOffset = In_vWorldOffset;
	}
}

void CPhysX_Collider::Synchronize_Collider(_float4x4 In_WorldMatrix, _fvector In_vLocalOffset, _fvector In_vWorldOffset, _bool bIsSimulation)
{
	if (m_bSimulation)
	{
		_vector vLocalOffset = XMVector3TransformCoord(In_vLocalOffset, Get_RotationMatrix(In_WorldMatrix));
		_vector vPos = XMVectorSet(In_WorldMatrix._41, In_WorldMatrix._42, In_WorldMatrix._43, 1.f);
		vPos += In_vWorldOffset + vLocalOffset;
		vPos.m128_f32[3] = 1.f;
		_vector vQuaternion = XMQuaternionRotationMatrix(Get_RotationMatrix(In_WorldMatrix));
		Set_Position(vPos, vQuaternion, bIsSimulation);

		m_vLocalOffset = vLocalOffset;
		m_vWorldOffset = In_vWorldOffset;
	}
}

void CPhysX_Collider::PutToSleep()
{
	if (m_bSimulation == true)
	{
		m_bSimulation = false;
		if (m_pRigidDynamic)
		{
			if (!m_PhysXColliderDesc.bKinematic)
				m_pRigidDynamic->putToSleep();
			else
				GET_SINGLE(CPhysX_Manager)->Remove_Collider(static_pointer_cast<CPhysX_Collider>(shared_from_this()), false);
		}
		else if (m_pRigidStatic)
		{
			GET_SINGLE(CPhysX_Manager)->Remove_Collider(static_pointer_cast<CPhysX_Collider>(shared_from_this()), false);
		}

	}
}

void CPhysX_Collider::WakeUp()
{
	if (m_bSimulation == false)
	{
		m_bSimulation = true;
		if (m_pRigidDynamic)
		{
			if (!m_PhysXColliderDesc.bKinematic)
				m_pRigidDynamic->wakeUp();
			else
				GET_SINGLE(CPhysX_Manager)->Register_Collider(static_pointer_cast<CPhysX_Collider>(shared_from_this()), false);

			shared_ptr<CPartObject> pPartOwner = dynamic_pointer_cast<CPartObject>(m_pOwner.lock());
			if (pPartOwner != nullptr)
				Synchronize_Collider(pPartOwner->Get_WorldMatrix(), pPartOwner->Get_PhysXColliderLocalOffset(), pPartOwner->Get_PhysXColliderWorldOffset(), false);
			else
				Synchronize_Collider(m_pOwner.lock()->Get_TransformCom().lock()->Get_WorldFloat4x4(), m_pOwner.lock()->Get_PhysXColliderLocalOffset(), m_pOwner.lock()->Get_PhysXColliderWorldOffset(), false);
		}
		else if (m_pRigidStatic)
		{
			GET_SINGLE(CPhysX_Manager)->Register_Collider(static_pointer_cast<CPhysX_Collider>(shared_from_this()), false);
		}
	}
}

void CPhysX_Collider::Attach_Shape(PxShape* pShape)
{
	if (m_pRigidDynamic)
		m_pRigidDynamic->attachShape(*pShape);
}

void CPhysX_Collider::Detach_Shape(PxShape* pShape)
{
	if (m_pRigidDynamic)
		m_pRigidDynamic->detachShape(*pShape);
}

void CPhysX_Collider::CreatePhysXActor(PHYSXCOLLIDER_DESC& PhysXColliderDesc)
{
	m_PhysXColliderDesc = PhysXColliderDesc;

	PxTransform	Transform;
	Transform.p = PxVec3(
		XMVectorGetX(PhysXColliderDesc.vPosition),
		XMVectorGetY(PhysXColliderDesc.vPosition),
		XMVectorGetZ(PhysXColliderDesc.vPosition));

	_float4 vQuaternion;
	XMStoreFloat4(&vQuaternion, XMQuaternionRotationRollPitchYawFromVector(PhysXColliderDesc.vAngles));
	Transform.q = PxQuat(
		vQuaternion.x,
		vQuaternion.y,
		vQuaternion.z,
		vQuaternion.w);


	switch (PhysXColliderDesc.eActorType)
	{
	case PHYSXACTOR_TYPE::ACTOR_DYNAMIC:
	case PHYSXACTOR_TYPE::ACTOR_YFIXED_DYNAMIC:
		Create_DynamicActor(PhysXColliderDesc, Transform);
		break;
	case PHYSXACTOR_TYPE::ACTOR_STATIC:
		Create_StaticActor(PhysXColliderDesc, Transform);
		break;
	default:
		assert(false);
		break;
	}

}

PxRigidActor* CPhysX_Collider::Add_PhysXActorAtScene()
{
	if (m_pRigidDynamic && m_pRigidStatic)
	{
		// 둘 다 존재하면 안된다.
		assert(false);
	}
	else if (m_pRigidDynamic)
	{
		return m_pRigidDynamic;
	}
	else if (m_pRigidStatic)
	{
		return m_pRigidStatic;
	}
	else
	{
		assert(false);
	}

	if (m_pGeometry.size() != 0)
	{
		for (auto& elem : m_pGeometry)
		{
			Safe_Delete(elem);
		}
	}

	return nullptr;
}

void CPhysX_Collider::Create_Geometry()
{
	switch (m_PhysXColliderDesc.eShape)
	{
	case PHYSXCOLLIDER_TYPE::SPHERE:
		m_pGeometry.push_back(new PxSphereGeometry(XMVectorGetX(m_PhysXColliderDesc.vScale)));
		break;

	case PHYSXCOLLIDER_TYPE::BOX:
		m_pGeometry.push_back(new PxBoxGeometry(XMVectorGetX(m_PhysXColliderDesc.vScale) * 0.5f,
			XMVectorGetY(m_PhysXColliderDesc.vScale) * 0.5f,
			XMVectorGetZ(m_PhysXColliderDesc.vScale) * 0.5f));
		break;

	case PHYSXCOLLIDER_TYPE::CYLINDER:
	{
		PxCustomGeometryExt::CylinderCallbacks* cylinder = new PxCustomGeometryExt::CylinderCallbacks(XMVectorGetY(m_PhysXColliderDesc.vScale), XMVectorGetX(m_PhysXColliderDesc.vScale), 0, 0.0f);
		PxShape* shape = PxRigidActorExt::createExclusiveShape(*m_pRigidDynamic, PxCustomGeometry(*cylinder), *m_PhysXColliderDesc.pMaterial);
		m_pShape.push_back(shape);
	}
	break;

	case PHYSXCOLLIDER_TYPE::CONE:
	{
		PxCustomGeometryExt::ConeCallbacks* cone = new PxCustomGeometryExt::ConeCallbacks(XMVectorGetY(m_PhysXColliderDesc.vScale), XMVectorGetX(m_PhysXColliderDesc.vScale), 0, 0.0f);
		PxShape* shape = PxRigidActorExt::createExclusiveShape(*m_pRigidDynamic, PxCustomGeometry(*cone), *m_PhysXColliderDesc.pMaterial);
		m_pShape.push_back(shape);
	}
	break;

	case PHYSXCOLLIDER_TYPE::MODEL:
	{
		Init_ModelCollider(m_PhysXColliderDesc.pModelCom);

		for (size_t i = 0; i < m_TriangleMesh.size(); ++i)
		{
			PxTriangleMeshGeometry* PxGeometry = new PxTriangleMeshGeometry();
			PxGeometry->triangleMesh = m_TriangleMesh[i];
			PxMeshScale	vScale;
			vScale.scale.x = XMVectorGetX(m_PhysXColliderDesc.vScale);
			vScale.scale.y = XMVectorGetY(m_PhysXColliderDesc.vScale);
			vScale.scale.z = XMVectorGetZ(m_PhysXColliderDesc.vScale);
			PxGeometry->scale = vScale;
			m_pGeometry.push_back(PxGeometry);
		}

		m_PhysXColliderDesc.pModelCom = nullptr;

		break;
	}

	default:
		assert(false);
		break;
	}
}

void CPhysX_Collider::Create_DynamicActor(PHYSXCOLLIDER_DESC& PhysXColliderDesc, PxTransform Transform)
{
	m_pRigidDynamic = GET_SINGLE(CPhysX_Manager)->Create_DynamicActor(Transform);
	Create_Geometry();

	// 지오메트리 개수만큼 셰이프를 만든다.

	if (PhysXColliderDesc.eShape == PHYSXCOLLIDER_TYPE::CYLINDER || PhysXColliderDesc.eShape == PHYSXCOLLIDER_TYPE::CONE)
	{
		for (size_t i = 0; i < m_pShape.size(); ++i)
		{
			if (PhysXColliderDesc.bTrigger)
			{
				m_pShape[i]->setFlag(PxShapeFlag::eVISUALIZATION, true);
				m_pShape[i]->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
				m_pShape[i]->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				m_FilterData.word2 = 1;
			}
			else
			{
				m_pShape[i]->setFlag(PxShapeFlag::eVISUALIZATION, true);
				m_pShape[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
				m_pShape[i]->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				m_FilterData.word2 = 1;
			}

			m_FilterData.word0 = (1 << m_PhysXColliderDesc.iFilterType);
			m_FilterData.word1 = GET_SINGLE(CPhysX_Manager)->Get_PhysXFilterGroup(m_PhysXColliderDesc.iFilterType);
			m_FilterData.word3 = m_PhysXColliderDesc.iFilterType;

			m_pShape[i]->setSimulationFilterData(m_FilterData);

			m_pShape[i]->userData = shared_from_this().get();
			m_pRigidDynamic->userData = shared_from_this().get();
			m_pRigidDynamic->attachShape(*m_pShape[i]);

			if (m_PhysXColliderDesc.bKinematic)
			{
				m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
				m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eFORCE_KINE_KINE_NOTIFICATIONS, true);
				m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eFORCE_STATIC_KINE_NOTIFICATIONS, true);
				//m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			}
			else if (m_pRigidDynamic)
			{
				if (m_PhysXColliderDesc.fMass != 0.f)
				{
					m_pRigidDynamic->setMass(m_PhysXColliderDesc.fMass);
					PxRigidBodyExt::updateMassAndInertia(*m_pRigidDynamic, 10.f);
				}
				m_pRigidDynamic->setAngularDamping(0.5f);
				m_pRigidDynamic->setLinearVelocity(PxVec3(0.f));
				m_pRigidDynamic->setAngularVelocity(PxVec3(5.f));
				//m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			}
		}
	}
	else
	{
		for (size_t i = 0; i < m_pGeometry.size(); ++i)
		{
			PxShapeFlags shapeFlags;

			if (PhysXColliderDesc.bTrigger)
			{
				shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE;
				m_FilterData.word2 = 1;
			}
			else
			{
				shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE;
				m_FilterData.word2 = 1;
			}

			PxShape* pShape = nullptr;

			GET_SINGLE(CPhysX_Manager)->Create_Shape(*m_pGeometry[i], m_PhysXColliderDesc.pMaterial, true, shapeFlags, &pShape);
			m_FilterData.word0 = (1 << m_PhysXColliderDesc.iFilterType);
			m_FilterData.word1 = GET_SINGLE(CPhysX_Manager)->Get_PhysXFilterGroup(m_PhysXColliderDesc.iFilterType);
			m_FilterData.word3 = m_PhysXColliderDesc.iFilterType;

			if (!pShape)
			{
				assert(false);
			}

			pShape->setSimulationFilterData(m_FilterData);

			pShape->userData = shared_from_this().get();
			m_pRigidDynamic->userData = shared_from_this().get();
			m_pRigidDynamic->attachShape(*pShape);

			if (m_PhysXColliderDesc.bKinematic)
			{
				m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
				m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eFORCE_KINE_KINE_NOTIFICATIONS, true);
				m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eFORCE_STATIC_KINE_NOTIFICATIONS, true);
				//m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			}
			else if (m_pRigidDynamic)
			{
				if (m_PhysXColliderDesc.fMass != 0.f)
				{
					m_pRigidDynamic->setMass(m_PhysXColliderDesc.fMass);
					PxRigidBodyExt::updateMassAndInertia(*m_pRigidDynamic, 10.f);
				}
				m_pRigidDynamic->setAngularDamping(0.5f);
				m_pRigidDynamic->setLinearVelocity(PxVec3(0.f));
				m_pRigidDynamic->setAngularVelocity(PxVec3(5.f));
				//m_pRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			}

			m_pShape.push_back(pShape);
		}

	}

}

void CPhysX_Collider::Create_StaticActor(PHYSXCOLLIDER_DESC& PhysXColliderDesc, PxTransform Transform)
{
	m_pRigidStatic = GET_SINGLE(CPhysX_Manager)->Create_StaticActor(Transform);
	Create_Geometry();

	// 지오메트리 개수만큼 셰이프를 만든다.
	for (size_t i = 0; i < m_pGeometry.size(); ++i)
	{

		PxShapeFlags shapeFlags;

		if (PhysXColliderDesc.bTrigger)
		{
			shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE;
			m_FilterData.word2 = 1;
		}
		else
		{
			shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE;
			m_FilterData.word2 = 1;
		}

		PxShape* pShape = nullptr;

		GET_SINGLE(CPhysX_Manager)->Create_Shape(*m_pGeometry[i], m_PhysXColliderDesc.pMaterial, true, shapeFlags, &pShape);

		m_FilterData.word0 = (1 << m_PhysXColliderDesc.iFilterType);
		m_FilterData.word1 = GET_SINGLE(CPhysX_Manager)->Get_PhysXFilterGroup(m_PhysXColliderDesc.iFilterType);
		m_FilterData.word3 = m_PhysXColliderDesc.iFilterType;

		if (!pShape)
		{
			// Shape가 생성되지 않음.
			assert(false);
		}

		pShape->setSimulationFilterData(m_FilterData);

		pShape->userData = shared_from_this().get();
		m_pRigidStatic->userData = shared_from_this().get();
		m_pShape.push_back(pShape);
		m_pRigidStatic->attachShape(*pShape);
	}
}

void CPhysX_Collider::Write_Json(json& Out_Json)
{
	__super::Write_Json(Out_Json);
}

void CPhysX_Collider::Load_Json(const json& In_Json)
{
	__super::Load_Json(In_Json);
}

void CPhysX_Collider::Set_Simulation(_bool bSimulation)
{
	m_bSimulation = bSimulation;
	if (m_bSimulation)
	{
		if (m_pRigidDynamic)
			m_pRigidDynamic->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, false);
		if (m_pRigidStatic)
			m_pRigidStatic->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, false);
	}
	else
	{
		if (m_pRigidDynamic)
			m_pRigidDynamic->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
		if (m_pRigidStatic)
			m_pRigidStatic->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
	}
}

void CPhysX_Collider::Free()
{
	if (m_pRigidDynamic)
		m_pRigidDynamic->release();
	if (m_pRigidStatic)
		m_pRigidStatic->release();

	for (auto iter : m_pGeometry)
		Safe_Delete(iter);
	for (auto iter : m_ConvexMeshes)
		iter->release();
	for (auto iter : m_TriangleMesh)
		iter->release();

	__super::Free();
}