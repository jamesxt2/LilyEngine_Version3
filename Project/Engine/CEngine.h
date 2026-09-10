#pragma once
#include "singleton.h"

class CEngine : public CSingleton<CEngine>
{
	SINGLE(CEngine)

public:
	int Init(HWND _MainWnd, POINT _Resolution);

private:
	HWND		m_MainWnd;
	POINT		m_Resolution;
};

