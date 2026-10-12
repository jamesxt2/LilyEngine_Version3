#pragma once
#include "CEntity.h"

#include "CDescriptorAllocator.h"

enum class STRUCTURED_BUFFER_TYPE
{
	TABLE, ROOT
};

class CStructuredBuffer : public CEntity
{
public:
	CStructuredBuffer() = delete;
	CStructuredBuffer(const CStructuredBuffer& other) = delete;
	CStructuredBuffer(UINT elementByteSize, UINT elementCount, STRUCTURED_BUFFER_TYPE type = STRUCTURED_BUFFER_TYPE::TABLE);
	~CStructuredBuffer();
	CLONE_DISABLE(CStructuredBuffer)

	void CopyData(UINT elementIndex, const void* data);
	void CopyDataFull(const void* data);
	void UploadDataToGpu();
	void ReadbackData(void* outData);

	void Bind_Graphics_SRV_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
	void Bind_Graphics_SRV_Root(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
	void Bind_Compute_SRV_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
	void Bind_Compute_UAV_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
private:

	void BuildResources();
	void BuildDescriptors();

	STRUCTURED_BUFFER_TYPE m_Type;

	UINT m_ElementByteSize;
	UINT m_ElementCount;
	uint64 m_BufferSize;

	BYTE* m_MappedData;

	ComPtr<ID3D12Resource> m_GpuBuffer; // both SRV and UAV
	ComPtr<ID3D12Resource> m_InputUploadBuffer;
	ComPtr<ID3D12Resource> m_ReadbackBuffer;

	CDescriptorAllocator::Allocation m_SrvAlloc;
	CDescriptorAllocator::Allocation m_UavAlloc;

	D3D12_RESOURCE_STATES m_CurrentState = D3D12_RESOURCE_STATE_COMMON;

public:
	inline UINT GetElementCount() const { return m_ElementCount; }
};

