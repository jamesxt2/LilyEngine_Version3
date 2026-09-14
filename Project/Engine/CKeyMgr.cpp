#include "pch.h"
#include "CKeyMgr.h"

#include "CEngine.h"

UINT g_KeyValue[(UINT)KEY::KEY_END] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9,
	VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
	VK_LBUTTON, VK_RBUTTON,
	VK_RETURN, VK_ESCAPE, VK_SPACE, VK_LSHIFT, VK_CONTROL, VK_MENU
};


CKeyMgr::CKeyMgr()
{

}

CKeyMgr::~CKeyMgr()
{

}

void CKeyMgr::Init()
{
	for (int i = 0; i < (UINT)KEY::KEY_END; ++i)
	{
		TKeyInfo info;
		m_VecKey.push_back(info);
	}
}

void CKeyMgr::Tick()
{
	// Key
	for (size_t i = 0; i < m_VecKey.size(); ++i)
	{
		if (GetAsyncKeyState(g_KeyValue[i]) & 0x8000)
		{
			if (!m_VecKey[i].PrevPressed)
				m_VecKey[i].State = KEY_STATE::TAP;
			else
				m_VecKey[i].State = KEY_STATE::PRESSED;

			m_VecKey[i].PrevPressed = true;
		}
		else
		{
			if (m_VecKey[i].PrevPressed)
				m_VecKey[i].State = KEY_STATE::RELEASED;
			else
				m_VecKey[i].State = KEY_STATE::NONE;

			m_VecKey[i].PrevPressed = false;
		}
	}

	// Mouse
	m_PrevMousePos = m_CurMousePos;

	POINT MousePos{};
	GetCursorPos(&MousePos);
	ScreenToClient(CEngine::GetInst()->GetMainWnd(), &MousePos);
	m_CurMousePos = Vector2((float)MousePos.x, (float)MousePos.y);

	m_MouseMoveDir = m_CurMousePos - m_PrevMousePos;
	m_MouseMoveDir.y *= -1;
	m_MouseMoveDir.Normalize();
}