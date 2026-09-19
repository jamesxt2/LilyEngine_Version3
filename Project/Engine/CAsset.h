#pragma once
#include "CEntity.h"

#include "ptr.h"

class CAsset : public CEntity
{
public:
	CAsset(ASSET_TYPE type);
	CAsset(const CAsset& _other);
	virtual ~CAsset();
	friend class CAssetMgr;
	template<typename T>
	friend class Ptr;

	virtual CEntity* Clone() = 0;

	friend class Ptr<CAsset>;

private:
	std::wstring			m_Key;
	std::wstring			m_RelativePath;
	const ASSET_TYPE		m_Type;
	int						m_RefCount;

	inline void AddRef() { ++m_RefCount; }
	void Release()
	{
		--m_RefCount;
		if (m_RefCount <= 0)
			delete this;
	}

public:
	inline const std::wstring& GetKey() const { return m_Key; }
	inline const std::wstring& GetRelativePath() const { return m_RelativePath; }
	inline ASSET_TYPE GetAssetType() const { return m_Type; }
};

