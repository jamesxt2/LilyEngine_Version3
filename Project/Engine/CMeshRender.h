#pragma once
#include "CRenderComponent.h"

class CMeshRender : public CRenderComponent
{
public:
	CMeshRender();
	CMeshRender(const CMeshRender& _other);
	~CMeshRender();
	CLONE(CMeshRender)

	virtual void FinalTick() override;
	virtual void Render() override;
};

