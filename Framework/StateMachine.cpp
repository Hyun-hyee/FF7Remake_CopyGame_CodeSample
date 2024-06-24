
#include "StateMachine.h"

IMPLEMENT_CLONE(CStateMachine, CComponent)
IMPLEMENT_CREATE(CStateMachine)

CStateMachine::CStateMachine(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	:CComponent(pDevice,pContext)
{
}

CStateMachine::CStateMachine(const CStateMachine& rhs)
	: CComponent(rhs)
{
}


HRESULT CStateMachine::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStateMachine::Initialize(void* pArg)
{
	return S_OK;
}

void CStateMachine::Priority_Tick(_cref_time fTimeDelta)
{
	if (m_pNextState != nullptr || m_pWaitState != nullptr)
	{
		if (m_pWaitState != nullptr)
		{
			if(m_pCurState->isValid_NextState(m_pWaitState))
			{
				m_pNextState = m_pWaitState;
				m_pWaitState = nullptr;
			}
		}

		if (m_pCurState != m_pNextState || m_bIsEnterSameState)
		{
			if (m_pCurState != nullptr)
				m_pCurState->Transition_State(m_pNextState);

			CState* pPrevState = m_pCurState;
			m_pCurState = m_pNextState;
			m_pCurState->Initialize_State(pPrevState);
			
			m_bIsEnterSameState = false;

		#ifdef _DEBUG
			cout << "State :: " << typeid(*m_pCurState).name() << "\n" << endl;
		#endif
		}

		m_pCurState->Priority_Tick(fTimeDelta);
	}
}

void CStateMachine::Tick(_cref_time fTimeDelta)
{
	if (m_pCurState != nullptr)
	{
		m_pCurState->Tick(fTimeDelta);
	}
}

void CStateMachine::Late_Tick(_cref_time fTimeDelta)
{
	if (m_pCurState != nullptr)
	{
		m_pCurState->Late_Tick(fTimeDelta);
	}
	
}


HRESULT CStateMachine::Add_RefStateMachine(const string& strName, weak_ptr<CStateMachine> pStateMachine)
{
	if (!m_pRefStateMachines.emplace(strName, pStateMachine).second)
		return E_FAIL;

	return S_OK;
}

weak_ptr<CStateMachine> CStateMachine::Get_RefStateMachine(const string& strName)
{ 
	auto iter = m_pRefStateMachines.find(strName);
	if (iter == m_pRefStateMachines.end())
		assert(false);

	return (*iter).second;
}

void CStateMachine::Free()
{
	m_pCurState = nullptr;
	m_pNextState = nullptr;

	for (auto& iter : m_mapState)
		Safe_Release(iter.second);

	m_mapState.clear();

	__super::Free();
}