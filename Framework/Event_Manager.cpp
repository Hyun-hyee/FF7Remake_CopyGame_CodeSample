
#include "Event_Manager.h"

CEvent_Manager::CEvent_Manager()
{
}

HRESULT CEvent_Manager::Initialize(_uint iNumLevels)
{
	m_iNumLevels = iNumLevels;
	m_mapLevelEvent = new LEVELEVENTS[iNumLevels];
	return S_OK;
}

void CEvent_Manager::Tick(_cref_time fTimeDelta)
{
	//Tick 이벤트 실행,삭제
	if (!m_mapTickEvent.empty())
		int a = 0;
	for (auto iter = m_mapTickEvent.begin(); iter != m_mapTickEvent.end();)
	{
		if (!iter->first.expired())
		{
			if (!iter->second(fTimeDelta))
			{
				iter = m_mapTickEvent.erase(iter);
			}
			else
			{
				++iter;
			}
		}
		else
		{
			iter = m_mapTickEvent.erase(iter);
		}
	}
}

void CEvent_Manager::Add_LevelEvent(_uint iLevelIndex, string sEvent, function<_bool(weak_ptr<CBase>, shared_ptr<CGameObject>)> Func)
{
	if (m_mapLevelEvent[iLevelIndex].count(sEvent) == 0)
	{
		vector<function<_bool(weak_ptr<CBase>, shared_ptr<CGameObject>)>> vecTemp;
		vecTemp.push_back(Func);
		m_mapLevelEvent[iLevelIndex].insert({ sEvent,vecTemp });
	}
	else
	{
		m_mapLevelEvent[iLevelIndex][sEvent].push_back(Func);
	}
}

void CEvent_Manager::Erase_LevelEvent(_uint iLevelIndex, string sEvent)
{
	if (m_mapLevelEvent[iLevelIndex].count(sEvent) != 0)
	{
		m_mapLevelEvent[iLevelIndex][sEvent].clear();
	}
}

HRESULT CEvent_Manager::Excute_LevelEvent(_uint iLevelIndex, string sEvent, weak_ptr<CBase> _pCaller, shared_ptr<CGameObject> pOther)
{
	if (m_mapLevelEvent[iLevelIndex].count(sEvent) != 0)
	{
		vector<function<_bool(weak_ptr<CBase> _pCaller, shared_ptr<CGameObject> pOther)>> vecFunc = m_mapLevelEvent[iLevelIndex][sEvent];

		for (auto iter = vecFunc.begin(); iter != vecFunc.end();)
		{
			_bool bResult = (*iter)(_pCaller, pOther);
			if (!bResult)
				iter = vecFunc.erase(iter);
			else
				iter++;
		}
		//cout << "Event Excute" << endl;
		return S_OK;
	}
	else
	{
		//cout << "Event Not Found" << endl;
		return S_OK;
	}
}

void CEvent_Manager::Add_TickEvent(weak_ptr<CBase> _pSubscriber, function<_bool(_cref_time)> Func)
{
	m_mapTickEvent.push_back(make_pair(_pSubscriber, Func));
}

void CEvent_Manager::Erase_TickEvent(weak_ptr<CBase> _pSubscriber)
{
	for (auto iter = m_mapTickEvent.begin(); iter != m_mapTickEvent.end();)
	{
		if ((*iter).first.lock() == _pSubscriber.lock())
			iter = m_mapTickEvent.erase(iter);
		else
			iter++;
	}
}

void CEvent_Manager::Clear_TickEvents()
{
	m_mapTickEvent.clear();
}

void CEvent_Manager::Clear(_uint iLevelIndex)
{
	if (iLevelIndex >= m_iNumLevels)
		return;

	for (auto iter : m_mapLevelEvent[iLevelIndex])
		iter.second.clear();

	m_mapLevelEvent[iLevelIndex].clear();
}

CEvent_Manager* CEvent_Manager::Create(_uint iNumLevels)
{
	CEvent_Manager* pInstance = new CEvent_Manager();

	if (FAILED(pInstance->Initialize(iNumLevels)))
	{
		MSG_BOX("Failed to Created : CEvent_Manager");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEvent_Manager::Free()
{
	Clear_TickEvents();
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto iter : m_mapLevelEvent[i])
			iter.second.clear();

		m_mapLevelEvent[i].clear();
	}
	Safe_Delete_Array(m_mapLevelEvent);
}
