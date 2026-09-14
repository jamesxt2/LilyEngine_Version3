#pragma once
#include "singleton.h"

enum class KEY
{
	A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	_0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
	NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
	LEFT, RIGHT, UP, DOWN,
	LBUTTON, RBUTTON,
	ENTER, ESC, SPACE, LSHIFT, CTRL, ALT,

	KEY_END
};

enum class KEY_STATE
{
	TAP, PRESSED, RELEASED, NONE
};

struct TKeyInfo
{
	KEY_STATE	State;
	bool		PrevPressed;

	TKeyInfo()
		: State(KEY_STATE::NONE), PrevPressed(false)
	{
	}
};

class CKeyMgr : public CSingleton<CKeyMgr>
{
	SINGLE(CKeyMgr)

public:
	void Init();
	void Tick();

private:
	std::vector<TKeyInfo>	m_VecKey;
	Vector2					m_CurMousePos;
	Vector2					m_PrevMousePos;
	Vector2					m_MouseMoveDir;
public:
	inline KEY_STATE GetKeyState(KEY _Key) { return m_VecKey[(UINT)_Key].State; }

	inline Vector2 GetCurMousePos() const { return m_CurMousePos; }
	inline Vector2 GetPrevMousePos() const { return m_PrevMousePos; }
	inline Vector2 GetMouseMoveDir() const { return m_MouseMoveDir; }
};

