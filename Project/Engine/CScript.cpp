#include "pch.h"
#include "CScript.h"


CScript::CScript()
	: CComponent(COMPONENT_TYPE::SCRIPT)
{
}

CScript::CScript(const CScript& _other)
	: CComponent(_other)
{
}

CScript::~CScript()
{
}
