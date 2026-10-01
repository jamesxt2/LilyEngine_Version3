#include "pch.h"
#include "CParticleSystem.h"

#include "CDevice.h"

CParticleSystem::CParticleSystem()
	: CRenderComponent(COMPONENT_TYPE::PARTICLESYSTEM),
	m_MaxParticle(10)
{
}

CParticleSystem::CParticleSystem(const CParticleSystem& _other)
	: CRenderComponent(_other),
	m_MaxParticle(_other.m_MaxParticle)
{
}

CParticleSystem::~CParticleSystem()
{
}

void CParticleSystem::FinalTick()
{
}

void CParticleSystem::Render()
{
	if (GetMesh() == nullptr || GetMaterial() == nullptr) return;

	CDevice::GetInst()->GetConstBuffer(CB_TYPE::MATERIAL)->Bind(GetMaterial()->GetMtrlCBIndex(), 2);
	GetMaterial()->Bind();

	GetMesh()->Bind();

	CMDLIST->DrawIndexedInstanced(GetMesh()->GetIndexCount(), m_MaxParticle, 0, 0, 0);
}
