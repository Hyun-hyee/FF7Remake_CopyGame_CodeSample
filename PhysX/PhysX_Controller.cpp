#include "PhysX_Controller.h"
#include "PhysX_Manager.h"
#include "GameObject.h"
#include "Transform.h"
#include "PhysX_Collider.h"

IMPLEMENT_CREATE(CPhysX_Controller)
IMPLEMENT_CLONE(CPhysX_Controller, CComponent)

_uint CPhysX_Controller::m_iNewControllerID = 0;

CPhysX_Controller::CPhysX_Controller(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent(pDevice, pContext)
{
}

CPhysX_Controller::CPhysX_Controller(const CPhysX_Controller& rhs)
	: CComponent(rhs)
{
}

HRESULT CPhysX_Controller::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPhysX_Controller::Initialize(void* pArg)
{
	__super::Initialize(pArg);

	m_iControllerID = m_iNewControllerID++;

	PHYSXCONTROLLER_DESC* pDesc = (PHYSXCONTROLLER_DESC*)pArg;
	Create_Controller(*pDesc);

	GET_SINGLE(CPhysX_Manager)->Register_Controller(static_pointer_cast<CPhysX_Controller>(shared_from_this()));

	return S_OK;
}

void CPhysX_Controller::Create_Controller(const PHYSXCONTROLLER_DESC& pDesc)
{
	m_pControllerDesc = pDesc;
	m_FilterData.word0 = (1 << pDesc.iFilterType);
	m_FilterData.word1 = GET_SINGLE(CPhysX_Manager)->Get_PhysXFilterGroup(pDesc.iFilterType); 
	
	if (m_pControllerDesc.eShapeType == PxControllerShapeType::eCAPSULE)
	{
		m_fHalfWidth = m_pControllerDesc.CapsuleControllerDesc.radius;
		m_pControllerDesc.CapsuleControllerDesc.reportCallback = this;
		m_pControllerDesc.CapsuleControllerDesc.userData = shared_from_this().get();
		GET_SINGLE(CPhysX_Manager)->Create_Controller(m_pControllerDesc.CapsuleControllerDesc, &m_pController);
	}
	else
	{
		m_fHalfWidth = m_pControllerDesc.BoxControllerDesc.halfForwardExtent;
		m_pControllerDesc.BoxControllerDesc.reportCallback = this;
		m_pControllerDesc.BoxControllerDesc.userData = shared_from_this().get();
		GET_SINGLE(CPhysX_Manager)->Create_Controller(m_pControllerDesc.BoxControllerDesc, &m_pController);
	}
	m_pController->setUserData(shared_from_this().get());
	m_pController->getActor()->userData = shared_from_this().get();
}

bool CPhysX_Controller::filter(const PxController& a, const PxController& b)
{
	CPhysX_Controller* pLeftControllerCom = (CPhysX_Controller*)a.getUserData();
	CPhysX_Controller* pRightControllerCom = (CPhysX_Controller*)b.getUserData();

	if (!pLeftControllerCom || !pRightControllerCom)
		return false;

	if (pLeftControllerCom->Is_EnableSimulation() && pRightControllerCom->Is_EnableSimulation())
		return true;

	return false;
}

void CPhysX_Controller::onShapeHit(const PxControllerShapeHit& hit)
{
	if (hit.actor->is<PxRigidDynamic>())
	{
		CPhysX_Collider* pCollider = (CPhysX_Collider*)(hit.actor->userData);
		if (!pCollider->Get_ColliderDesc().bKinematic)
		{
			_float3 vForce = Convert_Vector(hit.dir * 500.f);
			vForce.y += 10.f;
			pCollider->Add_Force(vForce);
		}
		
	}
	else if (hit.actor->is<PxRigidStatic>())
	{
		m_bGround = true;
	}
}

void CPhysX_Controller::onControllerHit(const PxControllersHit& hit)
{
	CPhysX_Controller* pOtherController = (CPhysX_Controller*)hit.other->getUserData();

	m_pOwner.lock()->onControllerHit(pOtherController,hit);
}

void CPhysX_Controller::onObstacleHit(const PxControllerObstacleHit& hit)
{
}

PxQueryHitType::Enum CPhysX_Controller::preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)
{
	if (!m_pOwner.lock()->IsState(OBJSTATE::Active))
		return PxQueryHitType::eNONE;

	if (!m_EnableColliderSimulation)
	{
		return PxQueryHitType::eNONE;
	}

	PxFilterData OtherFilter = shape->getSimulationFilterData();

	if ((m_FilterData.word0 & OtherFilter.word1) && (OtherFilter.word0 & m_FilterData.word1))
	{
		if (actor->is<PxRigidDynamic>())
		{
			CPhysX_Collider* pCollider = (CPhysX_Collider*)(actor->userData);
			if (pCollider->Get_ColliderDesc().bKinematic)
			{
				return PxQueryHitType::eNONE;
			}
		}
		m_bIsBlocking = true;
		return PxQueryHitType::eBLOCK;
	}

	return PxQueryHitType::eNONE;
}

PxQueryHitType::Enum CPhysX_Controller::postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor)
{
	if (!m_pOwner.lock()->IsState(OBJSTATE::Active))
		return PxQueryHitType::eNONE;

	if (!m_EnableColliderSimulation)
	{
		return PxQueryHitType::eNONE;
	}

	PxFilterData OtherFilter = shape->getSimulationFilterData();

	if ((filterData.word0 & OtherFilter.word1) && (OtherFilter.word0 & filterData.word1))
	{
		if (actor->is<PxRigidDynamic>())
		{
			CPhysX_Collider* pCollider = (CPhysX_Collider*)(actor->userData);
			if (pCollider->Get_ColliderDesc().bKinematic)
			{
				return PxQueryHitType::eNONE;
			}
		}
		return PxQueryHitType::eBLOCK;
	}

	return PxQueryHitType::eNONE;
}

void CPhysX_Controller::Synchronize_Transform(weak_ptr<CTransform> pTransform, _fvector In_vLocalOffset, _fvector In_vWorldOffset )
{
	if (!m_pController)
		return;
	PxExtendedVec3 vPosFromPx = m_pController->getFootPosition();
	_vector vPos = { (_float)vPosFromPx.x, (_float)vPosFromPx.y, (_float)vPosFromPx.z };
	_vector vLocalOffset = XMVector3TransformCoord(In_vLocalOffset, Get_RotationMatrix(pTransform.lock()->Get_WorldMatrix()));
	vPos += In_vWorldOffset + vLocalOffset;
	vPos.m128_f32[3] = 1.f;
	pTransform.lock()->Set_State(CTransform::STATE_POSITION, vPos);
}

void CPhysX_Controller::Synchronize_Controller(weak_ptr<CTransform> pTransform, _fvector In_vLocalOffset, _fvector In_vWorldOffset)
{
	_vector vPos = pTransform.lock()->Get_State(CTransform::STATE_POSITION);
	_vector vLocalOffset = XMVector3TransformCoord(In_vLocalOffset, Get_RotationMatrix(pTransform.lock()->Get_WorldMatrix()));
	vPos += In_vWorldOffset + vLocalOffset;
	vPos.m128_f32[3] = 1.f;

	m_pController->setFootPosition(PhysX_Utility::Convert_PxExtendedVec3(pTransform.lock()->Get_State(CTransform::STATE_POSITION)));
}

PxControllerCollisionFlags CPhysX_Controller::Synchronize_Controller(weak_ptr<CTransform> pTransform, PxF32 elapsedTime, PxControllerFilters& filters, _fvector In_vOffset)
{
	_vector vPos = pTransform.lock()->Get_State(CTransform::STATE_POSITION);
	vPos += In_vOffset;
	vPos.m128_f32[3] = 1.f;

	m_pController->setFootPosition(PhysX_Utility::Convert_PxExtendedVec3(pTransform.lock()->Get_State(CTransform::STATE_POSITION)));

	Bind_FilterOptions(filters);

	return m_pController->move({ 0.f, 0.f, 0.f }, 0.f, elapsedTime, filters);
}

PxControllerCollisionFlags CPhysX_Controller::Set_Position(_fvector In_vPosition, PxF32 elapsedTime, PxControllerFilters& filters)
{
	m_pController->setFootPosition(PhysX_Utility::Convert_PxExtendedVec3(In_vPosition));

	Bind_FilterOptions(filters);
	return m_pController->move({ 0.f, 0.f, 0.f }, 0.f, elapsedTime, filters);
}

_bool CPhysX_Controller::Set_Position(_fvector In_vPosition)
{
	_vector vPos = In_vPosition;
	_vector vLocalOffset = XMVector3TransformCoord(m_vPhysXControllerLocalOffset, Get_RotationMatrix(m_pOwner.lock()->Get_TransformCom().lock()->Get_WorldMatrix()));
	vPos -= m_vPhysXControllerWorldOffset + vLocalOffset;
	vPos.m128_f32[3] = 1.f;

	return m_pController->setFootPosition(Convert_PxExtendedVec3(vPos));
}

_vector CPhysX_Controller::Get_Position()
{
	PxExtendedVec3 vPosFromPx = m_pController->getFootPosition();

	return { (_float)vPosFromPx.x, (_float)vPosFromPx.y, (_float)vPosFromPx.z, (_float)1.f };
}

PxControllerCollisionFlags CPhysX_Controller::Move(_fvector disp, PxF32 minDist, PxF32 elapsedTime, PxControllerFilters& filters, const PxObstacleContext* obstacles)
{
	if (!m_pOwner.lock()->IsState(OBJSTATE::Active))
		return PxControllerCollisionFlags();
	
	PxVec3 vPositionFromPx = PhysX_Utility::Convert_PxVec3(disp);

	Bind_FilterOptions(filters);

	return m_pController->move(vPositionFromPx, minDist, elapsedTime, filters, obstacles);
}

PxControllerCollisionFlags CPhysX_Controller::MoveGravity(const _float fDeltaTime, PxControllerFilters& filters)
{
	if (!m_pOwner.lock()->IsState(OBJSTATE::Active))
		return PxControllerCollisionFlags();

	if (!m_bEnableGravity)
		return PxControllerCollisionFlags();

	Bind_FilterOptions(filters);
	_float fDeltaHeight = -3.f * m_fGravity * fDeltaTime * (m_fGravityAcc * 2.f + fDeltaTime);
	fDeltaHeight += 0.0001f;
	m_fGravityAcc += fDeltaTime;

	m_fGravityAcc = min(3.f, m_fGravityAcc);

	m_bGravityCalculation = true;

	return m_pController->move({ 0.f, fDeltaHeight, 0.f }, 0.f, fDeltaTime, filters);
}

void CPhysX_Controller::Reset_Gravity()
{
	if (!m_pOwner.lock()->IsState(OBJSTATE::Active))
		return;

	m_fGravityAcc = 0.f;
}

void CPhysX_Controller::Add_Force(_vector _vForce)
{
	PxVec3	vForce;
	vForce = { XMVectorGetX(_vForce), XMVectorGetY(_vForce), XMVectorGetZ(_vForce) };

	m_pController->getActor()->addForce(vForce);
}

void CPhysX_Controller::Clear_Force()
{	
	m_pController->getActor()->clearForce(PxForceMode::eACCELERATION);
	m_pController->getActor()->clearForce(PxForceMode::eFORCE);
	m_pController->getActor()->clearForce(PxForceMode::eIMPULSE);
	m_pController->getActor()->clearForce(PxForceMode::eVELOCITY_CHANGE);
	
	PutToSleep();
	WakeUp();
}

void CPhysX_Controller::PutToSleep()
{
	if (m_EnableColliderSimulation == true)
	{
		m_pController->getActor()->putToSleep();
		m_EnableColliderSimulation = false;
	}
}

void CPhysX_Controller::WakeUp()
{
	if (m_EnableColliderSimulation == false)
	{
		m_pController->getActor()->wakeUp();
		m_EnableColliderSimulation = true;
	}
}

void CPhysX_Controller::Set_EnableSimulation(const _bool In_EnableSimulation)
{
	m_EnableSimulation = In_EnableSimulation;
}


PxController* CPhysX_Controller::Get_Controller()
{
	return m_pController;
}

void CPhysX_Controller::Enable_Gravity(const _bool In_bGravity)
{
	m_bEnableGravity = In_bGravity;
	if (!m_bEnableGravity)
		m_bGravityCalculation = false;
}

_bool CPhysX_Controller::Is_Ground()
{
	return m_bGravityCalculation && m_bEnableGravity && (m_fGravityAcc <= 0.0f);
}

void CPhysX_Controller::Bind_FilterOptions(PxControllerFilters& Out_Filters)
{
	Out_Filters.mCCTFilterCallback = this;
	Out_Filters.mFilterCallback = this;
}

void CPhysX_Controller::Free()
{
	if(m_pController)
	m_pController->release();
}
