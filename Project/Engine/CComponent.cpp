#include "pch.h"
#include "CComponent.h"


CComponent::CComponent(COMPONENT_TYPE type)
	: m_Owner(nullptr), m_Type(type)
{
}

CComponent::CComponent(const CComponent& other)
	: CEntity(other), m_Owner(nullptr), m_Type(other.m_Type)
{
}

CComponent::~CComponent()
{
}
