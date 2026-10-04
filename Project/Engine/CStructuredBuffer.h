#pragma once
#include "CEntity.h"

#include "CDescriptorAllocator.h"

class CStructuredBuffer : public CEntity
{
public:
	CStructuredBuffer() = delete;
	CStructuredBuffer(const CStructuredBuffer& other) = delete;
	CStructuredBuffer(UINT elementByteSize, UINT elementCount);
	~CStructuredBuffer();
	CLONE_DISABLE(CStructuredBuffer)

	void UploadData(const void* data);
	void ReadbackData(void* outData);

	void Bind_Graphics_SRV_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
	void Bind_Compute_SRV_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
	void Bind_Compute_UAV_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
private:

	void BuildResources();
	void BuildDescriptors();

	UINT m_ElementByteSize;
	UINT m_ElementCount;
	uint64 m_BufferSize;

	ComPtr<ID3D12Resource> m_GpuBuffer; // both SRV and UAV
	ComPtr<ID3D12Resource> m_InputUploadBuffer;
	ComPtr<ID3D12Resource> m_ReadbackBuffer;

	CDescriptorAllocator::Allocation m_SrvAlloc;
	CDescriptorAllocator::Allocation m_UavAlloc;

	D3D12_RESOURCE_STATES m_CurrentState = D3D12_RESOURCE_STATE_COMMON;

public:
	inline UINT GetElementCount() const { return m_ElementCount; }
};

