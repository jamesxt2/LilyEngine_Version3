#pragma once
#include "CAsset.h"

#include "CDescriptorAllocator.h"

class CTexture : public CAsset
{
public:
	CTexture();
	CTexture(const CTexture& other) = delete;
	~CTexture();
	friend class CAssetMgr;
	CLONE_DISABLE(CTexture)

	void CreateFromFile(const std::wstring& filename, bool isTextureCube = false);

private:

	ComPtr<ID3D12Resource>					m_Resource;
	ComPtr<ID3D12Resource>					m_UploadHeap;

	UINT									m_PoolIndex = 0;

public:
	inline UINT GetPoolIndex() const { return m_PoolIndex; }
};

