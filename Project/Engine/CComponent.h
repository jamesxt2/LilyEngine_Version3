#pragma once
#include "CEntity.h"

class CComponent : public CEntity
{
public:
	CComponent(COMPONENT_TYPE type);
	CComponent(const CComponent& other);
	virtual ~CComponent();
	friend class CGameObject;

	virtual CComponent* Clone() = 0;

	virtual void Begin() {};
	virtual void Tick() {};
	virtual void FinalTick() = 0;

private:
	CGameObject*				m_Owner;
	const COMPONENT_TYPE		m_Type;

public:
	inline COMPONENT_TYPE GetComponentType() { return m_Type; }
	CGameObject* GetOwner() { return m_Owner; }
};

