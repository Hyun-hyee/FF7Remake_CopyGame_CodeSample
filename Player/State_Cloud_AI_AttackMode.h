/*
* 플레이어 AI State중 하나인 State_Cloud_AI_AttackMode 클래스입니다.
* 게임에서 전투 상황에는 두 플레이어 모두 조작 가능하기 때문에 AI_AttackMode는 Cloud,Aerith 모두 존재합니다.
*
* 플레이어 AI는 각 AI State 클래스를 메인으로 합니다.
* 즉, AI 모드일 때는 AI 상태의 Behavior Tree에서 조건에 맞는 행동 또는  State를 호출합니다,
*  State 호출시에는 호출된 State가 끝나면 다시 AI State로 돌아옵니다.
*
* AttackMode는 1차적으로 조작중인 플레이어와의 거리를 우선으로 체크합니다.
* 이후 일정거리 내에 몬스터가 있을 경우 가까운 몬스터를 타겟팅합니다. 만약 LockOn한 몬스터가 있다면 해당 몬스터를 타겟팅합니다.
* 타겟의 행동 상태에 따라 달라지는 ActionPower값에 따라 5단계로 행동합니다.
* ActionPower값이 작을수록 타겟이 무방비한 상태로, 강도 높은 공격 또는 스킬을 사용합니다.
* ActionPower값이 클수록 타겟이 강력한 공격을 사용중이거나 무적상태에 가까우므로 회피하며 공격하거나, 타겟을 변경합니다.
*/


#pragma once
#include "State_Cloud.h"
#include "Utility/LogicDeviceBasic.h"

class CState_Cloud_AI_AttackMode : public CState_Cloud
{
	INFO_CLASS(CState_Cloud_AI_AttackMode, CState)

public:
	explicit CState_Cloud_AI_AttackMode(shared_ptr<class CGameObject> pActor, shared_ptr<class CStateMachine> pStatemachine);
	virtual ~CState_Cloud_AI_AttackMode() = default;

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

