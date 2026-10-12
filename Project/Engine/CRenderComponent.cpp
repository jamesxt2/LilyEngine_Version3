#include "pch.h"
#include "CRenderComponent.h"

CRenderComponent::CRenderComponent(COMPONENT_TYPE type)
	: CComponent(type), m_PSOType(OBJ_PSO_TYPE::PSO_NONE)
{
}

CRenderComponent::CRenderComponent(const CRenderComponent& other)
	: CComponent(other),
	m_Mesh(other.m_Mesh),
	m_Material(other.m_Material),
	m_PSOType(other.m_PSOType)
{
	
}

CRenderComponent::~CRenderComponent()
{
}

int CRenderComponent::GetMaterialIndex() const
{
	if (m_Material == nullptr)
		return -1;
	return m_Material->GetMtrlCBIndex();
}
