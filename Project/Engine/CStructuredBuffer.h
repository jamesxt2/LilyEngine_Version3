#pragma once
#include "CEntity.h"

class CStructuredBuffer : public CEntity
{
public:
	CStructuredBuffer() = delete;
	CStructuredBuffer(const CStructuredBuffer& other) = delete;
	CStructuredBuffer(UINT elementByteSize, UINT elementCount);
	~CStructuredBuffer();
	CLONE_DISABLE(CStructuredBuffer)

private:
	UINT m_ElementByteSize;

	ComPtr<ID3D12Resource> m_InputBuffer; // System memory -> GPU
	ComPtr<ID3D12Resource> m_InputUploadBuffer;
};

