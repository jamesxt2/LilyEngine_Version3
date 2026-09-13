#pragma once

class CEntity
{
public:
	CEntity();
	CEntity(const CEntity& _other);
	virtual ~CEntity();

	virtual CEntity* Clone() = 0;

private:
	static UINT g_nextID;

	std::wstring m_Name;
	UINT m_ID;

public:
	inline const std::wstring& GetName() const { return m_Name; }
	inline void SetName(const std::wstring& name) { m_Name = name; }
	inline UINT GetID() const { return m_ID; }
};

