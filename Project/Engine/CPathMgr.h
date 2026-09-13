#pragma once
#include "singleton.h"

class CPathMgr : public CSingleton<CPathMgr>
{
	SINGLE(CPathMgr)

public:
	void Init();

private:
	wchar_t m_ContentPath[255];

public:
	inline const wchar_t* GetContentPath() { return m_ContentPath; }
};

