#pragma once
#include "CEntity.h"

class CComponent;
class CTransform;
class CCamera;
class CMeshRender;
class CRenderComponent;
class CScript;

class CGameObject : public CEntity
{
public:
	CGameObject();
	CGameObject(const CGameObject& _other);
	~CGameObject();
	CLONE(CGameObject)

	void Begin();
	void Tick();
	virtual void FinalTick();
	void Render();

	void AddComponent(CComponent* component);

	void AddChild(CGameObject* obj);

	void Destroy();

private:
	CComponent*					m_arrComp[(UINT)COMPONENT_TYPE::END];
	CRenderComponent*			m_RenderComp;
	std::vector<CScript*>		m_vecScript;

	CGameObject*				 m_Parent;
	std::vector<CGameObject*>	m_vecChild;

	bool						m_Dead;

public:
	inline CComponent* GetComponent(COMPONENT_TYPE type) const { return m_arrComp[(UINT)type]; }
	inline CTransform* GetTransformComp() const { return (CTransform*)m_arrComp[(UINT)COMPONENT_TYPE::TRANSFORM]; }
	inline CCamera* GetCameraComp() const { return (CCamera*)m_arrComp[(UINT)COMPONENT_TYPE::CAMERA]; }
	inline CRenderComponent* GetRenderComp() const { return m_RenderComp; }
	inline CMeshRender* GetMeshRenderComp() const { return (CMeshRender*)m_arrComp[(UINT)COMPONENT_TYPE::MESHRENDER]; }

	inline const std::vector<CGameObject*>& GetChild() const { return m_vecChild; }
	inline CGameObject* GetParent() const { return m_Parent; }

	inline bool IsDead() const { return m_Dead; }
};

