#include "pch.h"
#include "CLevel.h"

#include "CGameObject.h"

CLevel::CLevel()
	: m_State(LEVEL_STATE::STOP)
{

}

CLevel::~CLevel()
{
	
}

void CLevel::Begin()
{
	
}

void CLevel::Tick()
{
	for (size_t i = 0; i < m_vecParent.size(); ++i)
	{
		m_vecParent[i]->Tick();
	}
}

void CLevel::FinalTick()
{
	std::vector<CGameObject*>::iterator iter = m_vecParent.begin();
	for (; iter != m_vecParent.end(); )
	{
		(*iter)->FinalTick();
		if ((*iter)->IsDead())
			iter = m_vecParent.erase(iter);
		else
			++iter;
	}
}

void CLevel::RegisterClear()
{
	m_vecObject.clear();
}

void CLevel::AddObject(CGameObject* object, bool bChildMove)
{
	if (!object->GetParent())
		m_vecParent.push_back(object);
}

CGameObject* CLevel::FindObjectByName(const std::wstring& name)
{
	for (size_t j = 0; j < m_vecObject.size(); ++j)
	{
		if (m_vecObject[j]->GetName() == name)
			return m_vecObject[j];
	}

	return nullptr;
}

void CLevel::ChangeState(LEVEL_STATE nextState)
{
	//if (nextState == LEVEL_STATE::STOP || nextState == LEVEL_STATE::PAUSE)
		//CRenderMgr::GetInst()->ChangeRenderMode(RENDER_MODE::EDITOR);

	if (m_State == nextState) return;
	assert(!(m_State == LEVEL_STATE::STOP && nextState == LEVEL_STATE::PAUSE));

	if (m_State == LEVEL_STATE::STOP && nextState == LEVEL_STATE::PLAY)
		Begin();

	m_State = nextState;
}
