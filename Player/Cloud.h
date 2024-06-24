/*
* 게임 내 메인 플레이어 캐릭터인 Cloud 클래스입니다.
* 캐릭터의 무기는 PartObject로 소유하고 관리합니다.
* 기본적인 캐릭터 몸체에 대한 충돌 이벤트(데미지 등)를 제어합니다. 
* 무기 등의 PartObject의 충돌 이벤트는 각각의 클래스에서 별도로 제어합니다.
* 행동은 Component인 StateMachine에서 제어합니다.
*/

#pragma once
#include "Client_Defines.h"
#include "Player.h"

BEGIN(Client)

class CCloud : public CPlayer
{
	INFO_CLASS(CCloud, CPlayer)

private:
	CCloud(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	CCloud(const CCloud& rhs);
	virtual ~CCloud() = default;


public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Begin_Play(_cref_time fTimeDelta) override;
	virtual void Priority_Tick(_cref_time fTimeDelta) override;
	virtual void Tick(_cref_time fTimeDelta) override;
	virtual void Late_Tick(_cref_time fTimeDelta) override;
	virtual void Before_Render(_cref_time fTimeDelta) override;
	virtual void End_Play(_cref_time fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_PartObjects();

public:
	virtual void PhysX_OnCollision_Enter(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo) override;
	virtual void PhysX_OnCollision_Stay(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo) override;
	virtual void PhysX_OnCollision_Exit(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo) override;

public:
	virtual void PhysX_Raycast_Stay(weak_ptr<CGameObject> pOther, _uint iOtherColLayer, _float4 vOrigin, _float4 vDirection, _float4 vColPosition) override;

public:
	virtual void Status_Damaged(_uint iStatusDamaged, _uint iHitPower, _uint iAddHitPower) override;
	virtual void Set_State_AIMode(_bool bAIMode) override;

public:
	static shared_ptr<CCloud> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CGameObject> Clone(void* pArg) override;
	virtual void Free() override;

};

END