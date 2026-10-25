#pragma once
#include "CEntity.h"

class CGameObject;

class CLevel : public CEntity
{
public:
	CLevel();
	~CLevel();
	CLevel(const CLevel& other) = delete;
	CLONE_DISABLE(CLevel)

	void Begin();
	void Tick();
	void FinalTick();
	void RegisterClear();

	void AddObject(CGameObject* object, bool bChildMove = false);
	inline void RegisterObject(CGameObject* object) { m_vecObject.push_back(object); }

	CGameObject* FindObjectByName(const std::wstring& name);

	void SortObjects();

	void BuildLevelShadowMap();
	void BuildNormalsAndDepth();

private:

	void UpdateShadowTransform();

	LEVEL_STATE m_State;

	std::vector<CGameObject*> m_vecParent;
	std::vector<CGameObject*> m_vecObject;

	bool m_bIsObjSorted = false;
	std::unordered_map<PSO_TYPE, std::vector<CGameObject*>> m_SortedObjs;

	float m_SunTheta = 1.25f * XM_PI;
	float m_SunPhi = XM_PIDIV4;

	/*******************************Shadow Map***********************************/
	BoundingSphere m_SceneBounds;

	Vector3 m_LightPosW;
	float m_LightNearZ = 0.f;
	float m_LightFarZ = 0.f;

	float m_LightRotationAngle = 0.f;

	Matrix m_LightView;
	Matrix m_LightProj;
	Matrix m_ShadowTransform;

public:
	inline LEVEL_STATE GetState() const { return m_State; }
	void ChangeState(LEVEL_STATE nextState);

	inline const std::vector<CGameObject*>& GetParentObjects() const { return m_vecParent; }
	inline const std::vector<CGameObject*>& GetObjects() const { return m_vecObject; }

	inline std::unordered_map<PSO_TYPE, std::vector<CGameObject*>>& GetSortedObjects() { return m_SortedObjs; }
};

