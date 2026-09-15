#pragma once
#include "CScript.h"

class CCameraMoveScript : public CScript
{
public:
	CCameraMoveScript();
	~CCameraMoveScript();
	CLONE(CCameraMoveScript)

	virtual void Tick() override;

private:
	void MoveByPerspective();
	void MoveByOrthographic();

	void MarkDirty();

private:
	float m_Speed;
};

