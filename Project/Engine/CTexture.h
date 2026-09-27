#pragma once
#include "CAsset.h"

class CTexture : public CAsset
{
public:
	CTexture();
	CTexture(const CTexture& other) = delete;
	~CTexture();
	friend class CAssetMgr;
	CLONE_DISABLE(CTexture)

	void CreateFromFile(const std::wstring& filename, INT descriptorOffset, bool isTextureArray = false);

	void Bind();

private:

	ComPtr<ID3D12Resource>					m_Resource;
	ComPtr<ID3D12Resource>					m_UploadHeap;

	INT										m_DescriptorHeapOffsetSize;
};

