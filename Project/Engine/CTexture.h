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

	void CreateFromFile(const std::wstring& filename, bool isTextureArray = false);

	void Bind_Graphics_Table(UINT rootParamIndex = 0);
	void Bind_Graphics_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex = 0);
	void Bind_Compute_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);

private:

	ComPtr<ID3D12Resource>					m_Resource;
	ComPtr<ID3D12Resource>					m_UploadHeap;

	CDescriptorAllocator::Allocation		m_Alloc;
};

