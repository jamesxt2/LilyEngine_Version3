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

	inline void Clear() { m_vecObject.clear(); }

	CGameObject* FindObjectByName(const std::wstring& name);

private:
	LEVEL_STATE m_State;

	std::vector<CGameObject*> m_vecParent;
	std::vector<CGameObject*> m_vecObject;

public:
	inline LEVEL_STATE GetState() const { return m_State; }
	void ChangeState(LEVEL_STATE nextState);

	inline const std::vector<CGameObject*>& GetParentObjects() const { return m_vecParent; }
	inline const std::vector<CGameObject*>& GetObjects() const { return m_vecObject; }
};

