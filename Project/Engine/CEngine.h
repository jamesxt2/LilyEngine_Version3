#pragma once
#include "singleton.h"

class CEngine : public CSingleton<CEngine>
{
	SINGLE(CEngine)

public:
	int Init(HWND _MainWnd, POINT _Resolution);

	void Run();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

	void Draw();

	HWND		m_MainWnd;
	POINT		m_Resolution;

	bool		m_Paused{ false };
	bool		m_Minimized{ false };
	bool		m_Maximized{ false };
	bool		m_Resizing{ false };

public:
	inline HWND GetMainWnd() const { return m_MainWnd; }
};

