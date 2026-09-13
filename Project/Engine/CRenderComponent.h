#pragma once
#include "CComponent.h"

#include "CMesh.h"

class CRenderComponent : public CComponent
{
public:
	CRenderComponent(COMPONENT_TYPE type);
	CRenderComponent(const CRenderComponent& other);
	virtual ~CRenderComponent();

	virtual void Render() = 0;

private:
	Ptr<CMesh>					m_Mesh;

public:
	inline void SetMesh(Ptr<CMesh> mesh) { m_Mesh = mesh; }
	inline Ptr<CMesh> GetMesh() const { return m_Mesh; }

};

