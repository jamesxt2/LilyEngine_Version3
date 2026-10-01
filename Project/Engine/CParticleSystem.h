#pragma once
#include "CRenderComponent.h"

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

	UINT m_MaxParticle;
};

