#include "DynamicObject.h"

#include "PhysX_Collider.h"
#include "PhysX_Controller.h"

IMPLEMENT_CREATE(CDynamicObject)
IMPLEMENT_CLONE(CDynamicObject, CGameObject)

CDynamicObject::CDynamicObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(pDevice, pContext)
{
}

CDynamicObject::CDynamicObject(const CDynamicObject& rhs)
	: CGameObject(rhs)
{
}

HRESULT CDynamicObject::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		RETURN_EFAIL;

	return S_OK;
}

HRESULT CDynamicObject::Initialize(void* pArg)
{
	TurnOn_State(OBJSTATE::DynamicObject);

	if (FAILED(__super::Initialize(pArg)))
		RETURN_EFAIL;

	if (FAILED(Ready_Components(pArg)))
		RETURN_EFAIL;

	return S_OK;
}

void CDynamicObject::Begin_Play(_cref_time fTimeDelta)
{
	__super::Begin_Play(fTimeDelta);
}

void CDynamicObject::Priority_Tick(_cref_time fTimeDelta)
{
	__super::Priority_Tick(fTimeDelta);

	//PhysX 결과를 Transform에 적용
	//Controller있으면 Controller의 PhysX 결과값 적용, 없을때만 Collider의 PhysX 결과값 적용
	//Collider는 RigidDynamic + NON-Kinematic 전제 하에 적용되도록 한다. (Kinematic은 PhysX 운동량 등에 영향 X)
	if (m_pPhysXControllerCom)
		m_pPhysXControllerCom->Synchronize_Transform(m_pTransformCom, m_vPhysXControllerLocalOffset, m_vPhysXControllerWorldOffset);
	else if (m_pPhysXColliderCom)
		m_pPhysXColliderCom->Synchronize_Transform(m_pTransformCom,m_vPhysXColliderLocalOffset,m_vPhysXColliderWorldOffset);
}

void CDynamicObject::Tick(_cref_time fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CDynamicObject::Late_Tick(_cref_time fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

void CDynamicObject::Before_Render(_cref_time fTimeDelta)
{
	__super::Before_Render(fTimeDelta);
	
	PxControllerFilters filters;

	//중력 적용 (중력 끄는건 Controller에서 m_bEnableGravity 비활성화)
	if (m_pPhysXControllerCom)
	{
		PxControllerCollisionFlags m_LastCollisionFlags = m_pPhysXControllerCom->MoveGravity(fTimeDelta, filters);
		if (m_LastCollisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN)	
			m_pPhysXControllerCom->Reset_Gravity();
	}	

	//Transform결과를 PhysX에 적용 (Controller는 Transform으로 움직일때 바로 적용됨,NonKinematic은 Transform으로 조작하지 않으므로 PhysX 결과를 Transform에 적용하는 작업만 한다.)
	if (m_pPhysXColliderCom && m_pPhysXColliderCom->Get_ColliderDesc().bKinematic)
		m_pPhysXColliderCom->Synchronize_Collider(m_pTransformCom, m_vPhysXColliderLocalOffset, m_vPhysXColliderWorldOffset);
}

void CDynamicObject::End_Play(_cref_time fTimeDelta)
{
	__super::End_Play(fTimeDelta);
}

HRESULT CDynamicObject::Render()
{
	return S_OK;
}

HRESULT CDynamicObject::Ready_Components(void* pArg)
{
	return S_OK;
}

HRESULT CDynamicObject::Bind_ShaderResources()
{
	return S_OK;
}

void CDynamicObject::OnCollision_Enter(CCollider* pThisCol, CCollider* pOtherCol)
{
}

void CDynamicObject::OnCollision_Stay(CCollider* pThisCol, CCollider* pOtherCol)
{
}

void CDynamicObject::OnCollision_Exit(CCollider* pThisCol, CCollider* pOtherCol)
{
}

void CDynamicObject::PhysX_OnCollision_Enter(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
}

void CDynamicObject::PhysX_OnCollision_Stay(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
}

void CDynamicObject::PhysX_OnCollision_Exit(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
}

void CDynamicObject::Free()
{
	__super::Free();
}
