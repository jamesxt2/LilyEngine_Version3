#pragma once
#include "CRenderComponent.h"

class CMeshRender : public CRenderComponent
{
public:
	CMeshRender();
	~CMeshRender();
	CLONE(CMeshRender)

	virtual void FinalTick() override;
	virtual void Render() override;
};

