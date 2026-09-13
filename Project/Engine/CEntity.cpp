#include "pch.h"
#include "CEntity.h"

UINT CEntity::g_nextID = 0;

CEntity::CEntity()
	: m_ID(g_nextID++)
{
}

CEntity::CEntity(const CEntity& _other)
	: m_Name(_other.m_Name), m_ID(g_nextID++)
{
}

CEntity::~CEntity()
{
}
