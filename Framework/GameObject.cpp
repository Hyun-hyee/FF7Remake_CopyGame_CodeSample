#include "..\Public\GameObject.h"
#include "GameInstance.h"
#include "PartObject.h"

CGameObject::CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
	, m_pGameInstance(CGameInstance::GetInstance())
	, m_bIsCloned(false)
{
	Safe_AddRef(m_pGameInstance);
}

CGameObject::CGameObject(const CGameObject & rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
	, m_pGameInstance(rhs.m_pGameInstance)
	, m_bIsCloned(true)
	, m_strObjectTag(rhs.m_strObjectTag)
	, m_strPrototypeTag(rhs.m_strPrototypeTag)
	, m_eObjectType(rhs.m_eObjectType)
{
	Safe_AddRef(m_pGameInstance);	  
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGameObject::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		Desc = {};

	//기본 ObjState 설정
	TurnOn_State(OBJSTATE::Active);
	TurnOn_State(OBJSTATE::Begin_Play);
	TurnOn_State(OBJSTATE::Render);
	TurnOn_State(OBJSTATE::Tick);

	if (nullptr != pArg)
		Desc = *(GAMEOBJECT_DESC*)pArg;

	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransformCom)
		RETURN_EFAIL;

	if (nullptr != Find_Component(g_pTransformTag))
		RETURN_EFAIL;

	m_Components.emplace(g_pTransformTag, m_pTransformCom);

	m_pTransformCom->Set_State(CTransform::STATE::STATE_POSITION, _float4(Desc.vInitialPosition.x, Desc.vInitialPosition.y, Desc.vInitialPosition.z, 1));

	m_pTransformCom->Set_Owner(shared_from_this());

	return S_OK;
}

void CGameObject::Begin_Play(_cref_time fTimeDelta)
{
	if (!m_PartObjects.empty())
	{
		for (auto& Pair : m_PartObjects)
		{
			if (nullptr != Pair.second)
				Pair.second->Begin_Play(fTimeDelta);
		}
	}

	TurnOff_State(OBJSTATE::Begin_Play);
}

void CGameObject::Priority_Tick(_cref_time fTimeDelta)
{
	m_PrevWorldMatrix = m_pTransformCom->Get_WorldFloat4x4();
	XMStoreFloat4x4(& m_PrevViewMatrix,m_pGameInstance->Get_TransformMatrix(CPipeLine::TS_VIEW));

	if (!m_PartObjects.empty())
	{
		for (auto& Pair : m_PartObjects)
		{
			if (nullptr != Pair.second)
				if(Pair.second->IsState(OBJSTATE::Tick))
					Pair.second->Priority_Tick(fTimeDelta);
		}
	}
}

void CGameObject::Tick(_cref_time fTimeDelta)
{
	if (!m_PartObjects.empty())
	{
		for (auto& Pair : m_PartObjects)
		{
			if (nullptr != Pair.second)
				if (Pair.second->IsState(OBJSTATE::Tick))
					Pair.second->Tick(fTimeDelta);
		}
	}
}

void CGameObject::Late_Tick(_cref_time fTimeDelta)
{
	if (!m_PartObjects.empty())
	{
		for (auto& Pair : m_PartObjects)
		{
			if (nullptr != Pair.second)
				if (Pair.second->IsState(OBJSTATE::Tick))
					Pair.second->Late_Tick(fTimeDelta);
		}
	}
}

void CGameObject::Before_Render(_cref_time fTimeDelta)
{
	if (!m_PartObjects.empty())
	{
		for (auto& Pair : m_PartObjects)
		{
			if (nullptr != Pair.second)
				if (Pair.second->IsState(OBJSTATE::Render))
						Pair.second->Before_Render(fTimeDelta);
		}
	}
}

void CGameObject::End_Play(_cref_time fTimeDelta)
{
	TurnOff_State(OBJSTATE::End_Play);
}

HRESULT CGameObject::Render()
{
	return S_OK;
}

_bool CGameObject::Intersect_MousePos(SPACETYPE eSpacetype, _float3* pMousePos, _float* pLengthOut)
{
	if (m_pVIBufferCom != nullptr)
		return m_pVIBufferCom->Intersect_MousePos(eSpacetype, pMousePos, m_pTransformCom->Get_WorldMatrix(), pLengthOut);
	else if (m_pModelCom != nullptr)
		return m_pModelCom->Intersect_MousePos(eSpacetype, pMousePos, m_pTransformCom->Get_WorldMatrix(), pLengthOut);
	else
		return false;
}

void CGameObject::OnCollision_Enter(CCollider* pThisCol, CCollider* pOtherCol)
{
}

void CGameObject::OnCollision_Stay(CCollider* pThisCol, CCollider* pOtherCol)
{
}

void CGameObject::OnCollision_Exit(CCollider* pThisCol, CCollider* pOtherCol)
{
}

void CGameObject::PhysX_OnCollision_Enter(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
}

void CGameObject::PhysX_OnCollision_Stay(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
}

void CGameObject::PhysX_OnCollision_Exit(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
}

void CGameObject::onControllerHit(class CPhysX_Controller* pOtherController, const PxControllersHit& hit)
{
}

void CGameObject::PhysX_Raycast_Stay(weak_ptr<CGameObject> pOther,_uint iOtherColLayer, _float4 vOrigin, _float4 vDirection, _float4 vColPosition)
{
}

void CGameObject::Write_Json(json& Out_Json)
{
	Out_Json["PrototypeTag"] = WstrToStr(m_strPrototypeTag);
	Out_Json["ObjectTag"] = WstrToStr(m_strObjectTag);

	for (auto iter : m_Components)
		iter.second->Write_Json(Out_Json["Component"]);
}

void CGameObject::Load_Json(const json& In_Json)
{
	m_strObjectTag = StrToWstr(In_Json["ObjectTag"]);

	for (auto iter : m_Components)
		iter.second->Load_Json(In_Json["Component"]);
}


shared_ptr<CComponent> CGameObject::Find_Component(const wstring& strComTag, const wstring& strPartTag)
{
	map<const wstring, shared_ptr<CComponent>>*		pComponents = nullptr;

	if (strPartTag != TEXT(""))
	{
		auto	iter = m_PartObjects.find(strPartTag);

		if (iter == m_PartObjects.end())
			return nullptr;

		pComponents = iter->second->Get_Componentss();
	}
	else
		pComponents = &m_Components;

	auto	iter = (*pComponents).find(strComTag);

	if (iter == (*pComponents).end())
		return nullptr;

	return iter->second;
}

shared_ptr<CPartObject> CGameObject::Find_PartObject(const wstring& strPartTag)
{
	auto	iter = m_PartObjects.find(strPartTag);

	if (iter == m_PartObjects.end())
		return nullptr;

	return dynamic_pointer_cast<CPartObject>(iter->second);
}

void	CGameObject::TurnOn_State(const OBJSTATE value)
{
	m_eObjState |= ECast(value);
	
	if (value == OBJSTATE::Active)
	{
		if (!dynamic_cast<CPartObject*>(this))
		{
			if(m_pPhysXColliderCom)
				m_pPhysXColliderCom->WakeUp();

			if(m_pPhysXControllerCom)	
				m_pPhysXControllerCom->WakeUp();
		}
	}

	for (auto& iter : m_PartObjects)
		iter.second->TurnOn_State(value);
}

void	CGameObject::TurnOff_State(const OBJSTATE value)
{
	m_eObjState &= ~ECast(value);

	if (value == OBJSTATE::Active)
	{
		if (!dynamic_cast<CPartObject*>(this))
		{
			if (m_pPhysXColliderCom)
				m_pPhysXColliderCom->PutToSleep();

			if (m_pPhysXControllerCom)
				m_pPhysXControllerCom->PutToSleep();
		}		
	}

	for (auto& iter : m_PartObjects)
		iter.second->TurnOff_State(value);
}

void CGameObject::Free()
{
	__super::Free();

	m_pTransformCom = nullptr;
	m_pVIBufferCom = nullptr;
	m_pColliderCom = nullptr;
	m_pModelCom = nullptr;
	m_pMaterialCom = nullptr;
	m_pPhysXColliderCom = nullptr;
	m_pPhysXControllerCom = nullptr;
	m_pStateMachineCom = nullptr;

	m_Components.clear();

	m_PartObjects.clear();

	Safe_Release(m_pGameInstance);
	 
	IsState(OBJSTATE::Active);
}
