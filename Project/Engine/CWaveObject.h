#pragma once

#include "CGameObject.h"
#include "GpuWaves.h"

class CWaveObject : public CGameObject, public GpuWaves
{
public:
	CWaveObject(int m, int n, float dx, float dt, float speed, float damping);
	CWaveObject(const CWaveObject& _other) = delete;
	~CWaveObject();
	CLONE_DISABLE(CWaveObject)

	virtual void Begin() override;
	virtual void Tick() override;
	virtual void Render(UINT objCBRootParamIndex = 1) override;

};

