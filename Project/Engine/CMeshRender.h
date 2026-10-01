#pragma once
#include "CRenderComponent.h"

struct SubmeshGeometry;

class CMeshRender : public CRenderComponent
{
public:
	CMeshRender();
	CMeshRender(const CMeshRender& _other);
	~CMeshRender();
	CLONE(CMeshRender)

	virtual void FinalTick() override;
	virtual void Render() override;

private:
	SubmeshGeometry* m_SubMeshGeo;

public:
	inline void SetSubMeshGeo(SubmeshGeometry* geo) { m_SubMeshGeo = geo; }
};

