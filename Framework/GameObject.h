/*
* GameObject 클래스입니다.
* 게임 내 배치되는 오브젝트들(Ex-Player,MapObject,Effect 등)의 기반이 되는 클래스입니다. 
*/

#pragma once

#include "Base.h"

BEGIN(Engine)
class CPhysX_Collider;

class ENGINE_DLL CGameObject abstract : public CBase, public enable_shared_from_this<CGameObject>
{
public:
	typedef struct tagGameObjectDesc
	{
		tagGameObjectDesc() {}
		tagGameObjectDesc(_float fSpeed, _float fRotation, _float3 vPosition = {0.f,0.f,0.f})
			: fSpeedPerSec(fSpeed), fRotationPerSec(fRotation), vInitialPosition(vPosition) {}
		_float	fSpeedPerSec = 8.f;
		_float	fRotationPerSec = 20.f;
		_float3	vInitialPosition = { 0.f,0.f,0.f };
		_bool	bIsCloneInPool = { false };
	}GAMEOBJECT_DESC;

	INFO_CLASS(CGameObject, CBase) //DERIVED,BASE

protected:
	CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	CGameObject(const CGameObject& rhs);
	virtual ~CGameObject() = default;

public:
	virtual HRESULT Initialize_Prototype(); 			//프로토타입 객체 생성시 초기화 함수
	virtual HRESULT Initialize(void* pArg);				//실 사용 객체 생성시 초기화 함수
	virtual void Begin_Play(_cref_time fTimeDelta);		//객체 생성 후 첫 Priority Tick 함수 직전에 호출되는 함수
	virtual void Priority_Tick(_cref_time fTimeDelta);
	virtual void Tick(_cref_time fTimeDelta);
	virtual void Late_Tick(_cref_time fTimeDelta);
	virtual void Before_Render(_cref_time fTimeDelta);	//렌더 직전에 호출되는 함수
	virtual void End_Play(_cref_time fTimeDelta);		//객체 소멸시 마지막 Late_Tick 이후 호출되는 함수

	virtual HRESULT Render();
	virtual HRESULT Render_Shadow() { return S_OK; }

protected:
	ComPtr<ID3D11Device>				m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };

protected:
	class CGameInstance*		m_pGameInstance = { nullptr };

protected:
	/*주요 Component*/
	shared_ptr<class CTransform> m_pTransformCom = { nullptr };
	shared_ptr<class CVIBuffer> m_pVIBufferCom = { nullptr };
	shared_ptr<class CShader> m_pShaderCom = { nullptr };
	shared_ptr<class CCollider> m_pColliderCom = { nullptr };
	shared_ptr<class CCommonModelComp> m_pModelCom = { nullptr };
	shared_ptr<class CMaterialComponent> m_pMaterialCom = { nullptr };
	shared_ptr<class CPhysX_Collider> m_pPhysXColliderCom = { nullptr };
	shared_ptr<class CPhysX_Controller> m_pPhysXControllerCom = { nullptr };
	shared_ptr<class CStateMachine> m_pStateMachineCom = { nullptr };		// 메인 상태
	_unmap<string, shared_ptr<class CStateMachine>> m_pAdditionStateCom;	// 추가 스테이트용

	map<const wstring, shared_ptr<class CComponent>>		m_Components;
	map<const wstring, shared_ptr<class CGameObject>>		m_PartObjects;

	_float4x4 m_PrevWorldMatrix;
	_float4x4 m_PrevViewMatrix;

protected:
	/*GameObject 정보*/
	_bool						m_bIsCloned = { false };		//프로토타입 복사 여부
	_bool						m_bIsPoolObject = { false };	//ObjectPool에서 사용 여부
	
	wstring						m_strPrototypeTag = {};			
	wstring						m_strObjectTag = {};			
	OBJTYPE						m_eObjectType = { OBJTYPE_END };

	_uint						m_eObjState = { 0U };			//다중상태 사용(Active,Alive,Will_Removed,Render,Tick,Begin_Play, 등)

	weak_ptr<CGameObject>			  m_pOwner;

	_float3 m_vPhysXColliderWorldOffset = { 0.f,0.f,0.f };
	_float3 m_vPhysXColliderLocalOffset = { 0.f,0.f,0.f };

	_float3 m_vPhysXControllerWorldOffset = { 0.f,0.f,0.f };
	_float3 m_vPhysXControllerLocalOffset = { 0.f,0.f,0.f };

public:
	/*ObjState 컨트롤 함수*/
	_bool	IsState(const OBJSTATE value) const { return (m_eObjState & ECast(value)); }
	void	TurnOn_State(const OBJSTATE value);
	void	TurnOff_State(const OBJSTATE value);

	void	Toggle_State(const OBJSTATE value)
	{
		m_eObjState ^= ECast(value);
		for (auto& iter : m_PartObjects)
			iter.second->Toggle_State(value);
	}
	void Set_Dead()
	{
		TurnOn_State(OBJSTATE::WillRemoved);
		TurnOn_State(OBJSTATE::End_Play);
	}

public:
	/*Component,PartObject 관련 함수*/
	virtual shared_ptr<class CComponent> Find_Component(const wstring& strComTag, const wstring& strPartTag = TEXT(""));
	shared_ptr<class CPartObject> Find_PartObject(const wstring& strPartTag);

	template<class T>
	HRESULT Add_PartObject(const wstring& strPrototypeTag, const wstring& strPartTag, void* pArg = nullptr, shared_ptr<T>* ppOut = nullptr);

	typedef 	map<const wstring, shared_ptr<class CGameObject>> PARTOBJECTS;
	PARTOBJECTS* Get_PartObjects() { return &m_PartObjects; }

	typedef 	map<const wstring, shared_ptr<class CComponent>> COMPONENTS;
	COMPONENTS* Get_Componentss() { return &m_Components; }

protected:
	template<class T>
	HRESULT Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag,
		const wstring& strComTag, _Inout_ shared_ptr<T>* ppOut, void* pArg = nullptr);

public:
	/*툴용 마우스 피킹 함수*/
	virtual _bool	Intersect_MousePos(SPACETYPE eSpacetype, _float3* pMousePos, _float* pLengthOut = nullptr);

public:
	GETSET_EX2(wstring, m_strPrototypeTag, PrototypeTag, SET_C_REF, GET)
	GETSET_EX2(wstring, m_strObjectTag, ObjectTag, SET_C_REF, GET)
	GETSET_EX2(OBJTYPE, m_eObjectType, ObjectType, SET_C_REF, GET)
	GETSET_EX2(weak_ptr<CGameObject>, m_pOwner, Owner, GET, SET)
	GETSET_EX2(_bool, m_bIsPoolObject, IsPoolObject, SET, GET)

	GETSET_EX2(_float3, m_vPhysXColliderWorldOffset, PhysXColliderWorldOffset, SET, GET)
	GETSET_EX2(_float3, m_vPhysXColliderLocalOffset, PhysXColliderLocalOffset, SET, GET)	
	GETSET_EX2(_float3, m_vPhysXControllerWorldOffset, PhysXControllerWorldOffset, SET, GET)
	GETSET_EX2(_float3, m_vPhysXControllerLocalOffset, PhysXControllerLocalOffset, SET, GET)

	weak_ptr<class CTransform> Get_TransformCom() { return m_pTransformCom; }
	weak_ptr<class CVIBuffer> Get_VIBufferCom() { return m_pVIBufferCom; }
	weak_ptr<class CCollider> Get_ColliderCom() { return m_pColliderCom; }
	weak_ptr<class CCommonModelComp> Get_ModelCom() { return m_pModelCom; }
	weak_ptr<class CMaterialComponent> Get_MaterialCom() { return m_pMaterialCom; }
	weak_ptr<class CPhysX_Collider> Get_PhysXColliderCom() { return m_pPhysXColliderCom; }
	weak_ptr<class CPhysX_Controller> Get_PhysXControllerCom() { return m_pPhysXControllerCom; }
	weak_ptr<class CStateMachine> Get_StateMachineCom() { return m_pStateMachineCom; }
	weak_ptr<class CStateMachine> Get_AdditionStateCom(const string& strStateName)
	{
		return m_pAdditionStateCom[strStateName];
	}

public:
	/*충돌 이벤트 함수*/
	virtual void PhysX_OnCollision_Enter(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo);
	virtual void PhysX_OnCollision_Stay(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo);
	virtual void PhysX_OnCollision_Exit(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo);
	virtual void PhysX_Raycast_Stay(weak_ptr<CGameObject> pOther, _uint iOtherColLayer, _float4 vOrigin, _float4 vDirection, _float4 vColPosition);

	virtual void onControllerHit(class CPhysX_Controller* pOtherController, const PxControllersHit& hit);

public:
	/*Json 저장,로드 함수*/
	virtual void Write_Json(json& Out_Json);
	virtual void Load_Json(const json& In_Json);

public:
	virtual shared_ptr<CGameObject> Clone(void* pArg) = 0;
	virtual void Free() override;
};


template<class T>
HRESULT CGameObject::Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag, const wstring& strComTag, shared_ptr<T>* ppOut, void* pArg)
{
	if (nullptr != Find_Component(strComTag))
		RETURN_EFAIL;

	shared_ptr<CComponent> pComponent = m_pGameInstance->Clone_Component(iLevelIndex, strPrototypeTag, shared_from_this(), pArg);
	if (nullptr == pComponent)
		RETURN_EFAIL;

	*ppOut = static_pointer_cast<T>(pComponent);

	m_Components.emplace(strComTag, pComponent);

	return S_OK;
}

template<class T>
HRESULT CGameObject::Add_PartObject(const wstring& strPrototypeTag, const wstring& strPartTag, void* pArg, shared_ptr<T>* ppOut)
{
	if (nullptr != Find_PartObject(strPrototypeTag))
		RETURN_EFAIL;

	shared_ptr<CPartObject> pPartObject = dynamic_pointer_cast<CPartObject>(m_pGameInstance->CloneObject(strPrototypeTag, pArg));
	if (nullptr == pPartObject)
		RETURN_EFAIL;

	if (ppOut != nullptr)
	{
		*ppOut = static_pointer_cast<T>(pPartObject);
	}

	if (pPartObject != nullptr)
	{
		pPartObject->Set_Owner(shared_from_this());
		pPartObject->Set_ObjectType(m_eObjectType);
	}
	m_PartObjects.emplace(strPartTag, pPartObject);

	return S_OK;
}



END