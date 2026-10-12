#pragma once
#include "CRenderComponent.h"

class CStructuredBuffer;

class CParticleSystem : public CRenderComponent
{
public:
	CParticleSystem(UINT maxParticleCount, UINT noiseTexIndex);
	CParticleSystem(const CParticleSystem& _other);
	~CParticleSystem();
	CLONE(CParticleSystem)

	virtual void FinalTick() override;
	virtual void Render() override;

private:

	struct TParticleData
	{
		UINT MaxParticleCount = 0;
		UINT NoiseTexIndex = 0;
	};

	void CalculateSpawnCount();

	TParticleData m_ParticleData;

	CStructuredBuffer* m_ParticleBuffer;

	float m_Time;

	CStructuredBuffer* m_SpawnCountBuffer;
	CStructuredBuffer* m_ModuleBuffer;
	TParticleModule m_Module;
};

