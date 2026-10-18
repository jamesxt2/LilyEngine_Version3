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
	inline ASSET_TYPE GetAssetType() const { return m_Type; }
	inline int GetRefCount() const { return m_RefCount; }
};

struct TAsset
{
public:
	TAsset(ASSET_TYPE type);
	TAsset(const TAsset& _other);
	virtual ~TAsset();
	friend class CAssetMgr;
	template<typename T>
	friend class Ptr;

	friend class Ptr<TAsset>;

private:
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
	inline ASSET_TYPE GetAssetType() const { return m_Type; }
	inline int GetRefCount() const { return m_RefCount; }
};

