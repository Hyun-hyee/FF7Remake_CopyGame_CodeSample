/*
* PhysX_Controller 클래스입니다.
* PxController을 소유하며, 주로 캐릭터의 몸체에 사용됩니다.
* PxController는 다른 충돌체들을 밀어내고, 지형을 탈 수 있습니다.
* PxControllerFilterCallback,PxUserControllerHitReport,PxQueryFilterCallback 을 상속받아
* CCT 충돌 필터링, 충돌 이벤트 등을 커스텀했습니다.
*/

#pragma once
#include "Component.h"

BEGIN(Engine)
class CTransform;

class ENGINE_DLL CPhysX_Controller : public CComponent, public PxControllerFilterCallback, public PxUserControllerHitReport, public PxQueryFilterCallback
{
public:
	CPhysX_Controller(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	CPhysX_Controller(const CPhysX_Controller& rhs);
	virtual ~CPhysX_Controller() = default;

public:
	virtual HRESULT			Initialize_Prototype();
	virtual HRESULT			Initialize(void* pArg);

public:
	void					Create_Controller(const PHYSXCONTROLLER_DESC& pDesc);

public: // PxControllerFilterCallback (CCT-CCT 충돌 필터링)
	virtual bool			filter(const PxController& a, const PxController& b) override;

public: // Hit Report
	/*	캐릭터가 Shape (정적 또는 동적)과 충돌*/
	virtual void			onShapeHit(const PxControllerShapeHit& hit);

	/*캐릭터가 다른 캐릭터와 충돌*/
	virtual void			onControllerHit(const PxControllersHit& hit);

	/*캐릭터가 사용자 정의 장애물과 충돌*/
	virtual void			onObstacleHit(const PxControllerObstacleHit& hit);
	
	// PxQueryFilterCallback (Scene Query 통과한 후보 객체에 대해 추가적 필터링)
	virtual PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags) override;
	
	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor) override;

public:
	/*CCT 위치,회전,이동,중력 관련 함수*/
	virtual void						Synchronize_Transform(weak_ptr<CTransform> pTransform ,_fvector In_vLocalOffset = { 0.f, 0.f, 0.f },  _fvector In_vWorldOffset = { 0.f, 0.f, 0.f });
	virtual void						Synchronize_Controller(weak_ptr<CTransform> pTransform, _fvector In_vLocalOffset = { 0.f, 0.f, 0.f },  _fvector In_vWorldOffset = { 0.f, 0.f, 0.f });

	virtual PxControllerCollisionFlags	Synchronize_Controller(weak_ptr<CTransform> pTransform, PxF32 elapsedTime, PxControllerFilters& filters, _fvector In_vOffset = { 0.f, 0.f, 0.f });
	virtual PxControllerCollisionFlags	Set_Position(_fvector In_vPosition, PxF32 elapsedTime, PxControllerFilters& filters);
	virtual _bool						Set_Position(_fvector In_vPosition);
	virtual _vector						Get_Position();
	virtual PxControllerCollisionFlags	Move(_fvector disp, PxF32 minDist, PxF32 elapsedTime, PxControllerFilters& filters, const PxObstacleContext* obstacles = nullptr);

	virtual PxControllerCollisionFlags	MoveGravity(const _float fDeltaTime, PxControllerFilters& filters);
	void								Reset_Gravity();
	void								Enable_Gravity(const _bool In_bGravity);
	_bool								Is_Gravity() { return m_bEnableGravity; } // 중력 사용하는지 판단
	_bool								Is_Ground(); // 땅 위에 있는지 판단.

public:
	void								Add_Force(_vector _vForce);
	void								Clear_Force();

public:
	void								PutToSleep();
	void								WakeUp();

public:
	_uint								Get_ControllerID() const { return m_iControllerID; }
	_bool								Is_EnableSimulation() const { return m_EnableSimulation; } 
	void								Set_EnableSimulation(const _bool In_EnableSimulation);
	void								Set_EnableColliderSimulation(const _bool In_EnableSimulation) { m_EnableColliderSimulation = In_EnableSimulation; }
	PxController*						Get_Controller();

public:
	_uint								Get_ControllerID() { return m_iControllerID; }
	PxController*						Get_PxController() { return m_pController; }
	PHYSXCONTROLLER_DESC				Get_ControllerDesc() { return m_pControllerDesc; }
	void								Set_Gravity(_float fGravity) { m_fGravity = fGravity; }
	
	GETSET_EX2(_bool, m_bIsBlocking, IsBlocking, GET, SET)
	GETSET_EX2(_float, m_fHalfWidth, HalfWidth, GET, SET)

	GETSET_EX2(_float3, m_vPhysXControllerWorldOffset, PhysXControllerWorldOffset, SET, GET)
	GETSET_EX2(_float3, m_vPhysXControllerLocalOffset, PhysXControllerLocalOffset, SET, GET)

protected:
	void								Bind_FilterOptions(PxControllerFilters& Out_Filters);

public:
	FDelegate<const PxControllersHit&>		Callback_ControllerHit;
	
protected:
	PxFilterData							m_FilterData;
	_vector									m_vMaxVelocity;
	_float									m_fGravityAcc = 0.f;
	_float									m_fGravity = { 9.81f };

	_float3 m_vPhysXControllerWorldOffset = { 0.f,0.f,0.f };
	_float3 m_vPhysXControllerLocalOffset = { 0.f,0.f,0.f };

protected:
	PxController*							m_pController = nullptr;
	PHYSXCONTROLLER_DESC					m_pControllerDesc;
	
	_bool									m_EnableSimulation = true; //CCT-CCT
	_bool									m_EnableColliderSimulation = true; //CCT-COL
	_bool									m_bEnableGravity = true;
	_bool									m_bGravityCalculation = true; // 중력 값 계산을 했는지 판단

	_bool									m_bGround = false;
	_bool									m_bIsBlocking = false;	
	_float									m_fHalfWidth = 0.f;		

private:
	static _uint			m_iNewControllerID;
	_uint					m_iControllerID;

public:
	static shared_ptr<CPhysX_Controller> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CComponent> Clone(void* pArg) override;
	virtual void Free() override;
};

END