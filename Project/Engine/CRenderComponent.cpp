#include "pch.h"
#include "CRenderComponent.h"

CRenderComponent::CRenderComponent(COMPONENT_TYPE type)
	: CComponent(type)
{
}

CRenderComponent::CRenderComponent(const CRenderComponent& other)
	: CComponent(other),
	m_Mesh(other.m_Mesh)
{
	
}

CRenderComponent::~CRenderComponent()
{
}