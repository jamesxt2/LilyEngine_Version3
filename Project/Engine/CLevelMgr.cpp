#include "pch.h"
#include "CLevelMgr.h"

#include "CLevel.h"

CLevelMgr::CLevelMgr()
	: m_CurLevel(nullptr)
{
}

CLevelMgr::~CLevelMgr()
{
	if (m_CurLevel != nullptr)
		delete m_CurLevel;
}

void CLevelMgr::Init()
{

}

void CLevelMgr::Tick()
{
	if (m_CurLevel != nullptr)
	{
		if (m_CurLevel->GetState() == LEVEL_STATE::PLAY)
			m_CurLevel->Tick();
		m_CurLevel->RegisterClear();
		m_CurLevel->FinalTick();
	}
}

void CLevelMgr::ChangeLevel(CLevel* nextLevel)
{
	if (m_CurLevel != nullptr)
	{
		delete m_CurLevel;
		m_CurLevel = nullptr;
	}
	m_CurLevel = nextLevel;
}

