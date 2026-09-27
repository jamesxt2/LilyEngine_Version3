#pragma once
#include "singleton.h"

class CLevel;

class CLevelMgr : public CSingleton<CLevelMgr>
{
	SINGLE(CLevelMgr)

public:
	void Init();
	void Tick();
	friend class CTaskMgr;

	void ChangeLevel(const std::wstring& name);

	MulticastDelegate<> OnLevelChange;

private:

	CLevel* m_CurLevel;
	std::unordered_map<std::wstring, CLevel*> m_LevelMap;

public:
	inline CLevel* GetCurrentLevel() const { return m_CurLevel; }

	inline void AddLevel(const std::wstring& name, CLevel* level)
	{
		m_LevelMap.emplace(name, level);
	}
	inline void RemoveLevel(const std::wstring& name)
	{
		m_LevelMap.erase(name);
	}
};

