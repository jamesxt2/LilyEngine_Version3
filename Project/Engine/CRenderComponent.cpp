#include "pch.h"
#include "CRenderComponent.h"

CRenderComponent::CRenderComponent(COMPONENT_TYPE type)
	: CComponent(type), m_PSOType(OBJ_PSO_TYPE::PSO_NONE)
{
}

CRenderComponent::CRenderComponent(const CRenderComponent& other)
	: CComponent(other),
	m_Mesh(other.m_Mesh), m_PSOType(other.m_PSOType)
{
	
}

CRenderComponent::~CRenderComponent()
{
}