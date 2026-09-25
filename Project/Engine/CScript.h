#pragma once
#include "CComponent.h"

class CScript : public CComponent
{
public:
	CScript();
	CScript(const CScript& _other);
	~CScript();

	virtual void Tick() = 0;
	virtual void FinalTick() final {}
};

