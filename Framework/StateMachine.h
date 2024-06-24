#pragma once
/*
* FSM(유한 상태 머신) 클래스 입니다.
*/


#include "Component.h"
#include "State.h"

BEGIN(Engine)

class ENGINE_DLL CStateMachine final : public CComponent
{
	INFO_CLASS(CStateMachine,CComponent)

private:
	explicit CStateMachine(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	explicit CStateMachine(const CStateMachine& rhs);
	virtual ~CStateMachine() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Tick(_cref_time fTimeDelta);
	virtual void Tick(_cref_time fTimeDelta);
	virtual void Late_Tick(_cref_time fTimeDelta);

private:
	unordered_map<string, CState*> m_mapState;

	string						   m_sCurStateName;
	CState*						   m_pCurState		= { nullptr };
	CState*						   m_pNextState		= { nullptr };
	CState*						   m_pWaitState		= { nullptr };	
	_bool						   m_bIsEnterSameState = { false };

	unordered_map<string, weak_ptr<CStateMachine>>	m_pRefStateMachines; // 참조용 상태머신 등록 (다중 상태머신 사용시)

public:
	template<class T1>
	T1* Find_State();

	template<class T1>
	HRESULT			Add_State(); //상태 등록

	template<class T1>
	bool			isValid_EnterState();

	template<class T1>
	bool			Enter_State(); //m_pCurState->isValid_NextState() 체크 후 상태 변경

	template<class T1>
	bool			Wait_State(); //m_pCurState->isValid_NextState()가 true 반환할때까지 계속 체크한 후 상태 변경

	template<class T1>
	bool			Set_State();  //m_pCurState->isValid_NextState()체크 안하고 바로 상태 변경

	template<class T1>
	bool			Is_State();

	string			Get_CurStateName() { return m_sCurStateName; }
	CState*			Get_CurState() { return m_pCurState; }
	CState*			Get_NextState() { return m_pNextState; }

	void			Set_CurState(CState* pState) { m_pCurState = pState; }
	void			Set_NextState(CState* pState) { m_pNextState = pState; }

	HRESULT			Add_RefStateMachine(const string& strName, weak_ptr<CStateMachine> pStateMachine);
	weak_ptr<CStateMachine>	Get_RefStateMachine(const string& strStateMachineName);
	

public:
	static shared_ptr<CStateMachine> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CComponent> Clone(void* pArg) override;
	virtual void Free() override;

};



template<class T1>
T1* CStateMachine::Find_State()
{
	const std::string name = typeid(T1).name();
	if (m_mapState.count(name) != 0)
	{
		m_sCurStateName = name;
		return static_cast<T1*>(m_mapState[name]);
	}
	return nullptr;
}


template<class T1>
HRESULT CStateMachine::Add_State()
{
	if (typeid(CState) != typeid(T1::BASE))
		return false;

	if (Find_State<T1>() != nullptr)
		RETURN_EFAIL;

	string stateName = typeid(T1).name();
	CState* pState = new T1(m_pOwner.lock(), static_pointer_cast<CStateMachine>(shared_from_this()));
	m_mapState.insert({ stateName, pState });
	return S_OK;
}


template<class T1>
bool CStateMachine::Enter_State()
{
	if (typeid(CState) != typeid(T1::BASE))
		return false;

	CState* pState = Find_State<T1>();
	if (pState != nullptr)
	{
		if (m_pNextState == nullptr || m_pCurState == nullptr)
		{
			m_pNextState = pState;
			return true;
		}
		else
		{
			if (m_pCurState->isValid_NextState(pState))
			{
				m_pNextState = pState;
				if (typeid(*m_pCurState) == typeid(*m_pNextState))
					m_bIsEnterSameState = true;
				return true;
			}
			else
				return false;
		}
	}
	else 
		return false;

	return true;
}

template<class T1>
inline bool CStateMachine::Wait_State()
{
	if (typeid(CState) != typeid(T1::BASE))
		return false;

	CState* pState = Find_State<T1>();
	if (pState != nullptr)
	{
		if (m_pNextState == nullptr)
		{
			m_pNextState = pState;
			m_pWaitState = nullptr;
			return true;
		}
		else
		{
			if (!m_pCurState || m_pCurState->isValid_NextState(pState))
			{
				m_pNextState = pState;
				m_pWaitState = nullptr;
				if (m_pCurState && (typeid(*m_pCurState) == typeid(*m_pNextState)))
					m_bIsEnterSameState = true;
				return true;
			}
			else
			{
				m_pWaitState = pState;
				return false;
			}
		}
	}
	else
		return false;

	return true;
}


template<class T1>
bool CStateMachine::Set_State()
{
	if (typeid(CState) != typeid(T1::BASE))
		return false;

	CState* pState = Find_State<T1>();
	if (pState != nullptr)
	{
		m_pNextState = pState;
		return true;
	}
	else
		return false;
	return true;
}

template<class T1>
inline bool CStateMachine::Is_State()
{
	if (typeid(CState) != typeid(T1::BASE))
		return false;

	if (m_pCurState == nullptr 
		|| dynamic_cast<T1*>(m_pCurState) == nullptr)
		return false;

	return true;
}


END

//#endif 

