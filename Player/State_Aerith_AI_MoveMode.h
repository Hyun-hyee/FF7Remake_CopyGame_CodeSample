/*
* 플레이어 AI State중 하나인 State_Aerith_AI_MoveMode 클래스입니다.
* 게임에서 전투 상황이 아닌 경우 메인 플레이어인 Cloud만 직접 조작이 가능하기 때문에, AI_MoveMode 상태는 Aerith만 존재합니다.
* 
* 플레이어 AI는 각 AI State 클래스를 메인으로 합니다.
* 즉, AI 모드일 때는 AI 상태의 Behavior Tree에서 조건에 맞는 행동 또는  State를 호출합니다,
*  State 호출시에는 호출된 State가 끝나면 다시 AI State로 돌아옵니다.
* 
* MoveMode는 메인 플레이어와 일정한 거리를 유지하도록 구현했습니다.
* 이동중에 충돌, 멀어짐 등의 상황을 방지하기 위해 텔레포트, 뒤로 이동 기능 등을 추가했습니다.
*/

#pragma once
#include "Aerith/State/State_Aerith.h"
#include "Utility/LogicDeviceBasic.h"

class CState_Aerith_AI_MoveMode : public CState_Aerith
{
	INFO_CLASS(CState_Aerith_AI_MoveMode, CState)

public:
	explicit CState_Aerith_AI_MoveMode(shared_ptr<CGameObject> pActor, shared_ptr<class CStateMachine> pStatemachine);
	virtual ~CState_Aerith_AI_MoveMode() = default;

public:
	virtual HRESULT			Initialize_State(CState* pPreviousState)	override;
	virtual void			Priority_Tick(_cref_time fTimeDelta)			override;
	virtual void			Tick(_cref_time fTimeDelta)					    override;
	virtual void			Late_Tick(_cref_time fTimeDelta)			    override;
	virtual void			Transition_State(CState* pNextState)		override;
	virtual bool			isValid_NextState(CState* state)			override;

private:
	_uint					m_iDamageCount = { 0 };
	_bool					m_bIsExcute = { false };
	FTimeChecker			m_fTimeMax = { 1.f };

private:
	void					Build_BehaviorTree();
protected:
	virtual void		Free();
};

