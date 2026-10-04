#pragma once
#include "CRenderComponent.h"

class CStructuredBuffer;

class CParticleSystem : public CRenderComponent
{
public:
	CParticleSystem();
	CParticleSystem(const CParticleSystem& _other);
	~CParticleSystem();
	CLONE(CParticleSystem)

	virtual void FinalTick() override;
	virtual void Render() override;

private:

	void CalculateSpawnCount();

	UINT m_MaxParticleCount;

	CStructuredBuffer* m_ParticleBuffer;

	float m_Time;

	CStructuredBuffer* m_SpawnCountBuffer;
	CStructuredBuffer* m_ModuleBuffer;
	TParticleModule m_Module;
};

