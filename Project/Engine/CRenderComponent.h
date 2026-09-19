#pragma once
#include "CComponent.h"

#include "CMesh.h"
#include "CMaterial.h"

class CRenderComponent : public CComponent
{
public:
	CRenderComponent(COMPONENT_TYPE type);
	CRenderComponent(const CRenderComponent& other);
	virtual ~CRenderComponent();

	virtual void Render() = 0;

private:
	Ptr<CMesh>					m_Mesh;
	Ptr<CMaterial>				m_Material;

public:
	inline void SetMesh(Ptr<CMesh> mesh) { m_Mesh = mesh; }
	inline Ptr<CMesh> GetMesh() const { return m_Mesh; }
	inline void SetMaterial(Ptr<CMaterial> material) { m_Material = material; }
	inline Ptr<CMaterial> GetMaterial() const { return m_Material; }
};

