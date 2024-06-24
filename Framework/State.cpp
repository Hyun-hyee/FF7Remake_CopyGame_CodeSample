#include "State.h"
#include "StateMachine.h"
#include "GameObject.h"
#include "GameInstance.h"
#include "PartObject.h"

CState::CState(shared_ptr<CGameObject> pActor, shared_ptr<CStateMachine> pStatemachine)
{
	m_pActor = pActor;
	m_pActor_TransformCom = pActor->Get_TransformCom();
	m_pActor_ModelCom = pActor->Get_ModelCom();
	m_pStateMachineCom = pStatemachine;
	m_pGameInstance = CGameInstance::GetInstance();
}

HRESULT CState::Initialize_State(CState* pPreviousState)
{
	return S_OK;
}

void CState::Priority_Tick(_cref_time fTimeDelta)
{
}

void CState::Tick(_cref_time fTimeDelta)
{
}

void CState::Late_Tick(_cref_time fTimeDelta)
{
}

void CState::Transition_State(CState* pNextState)
{
}

bool CState::isValid_NextState(CState* state)
{
	return true;
}


void CState::Free()
{
	__super::Free();
}

