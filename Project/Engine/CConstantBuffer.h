#pragma once
#include "CEntity.h"

class CConstantBuffer : public CEntity
{
public:
	CConstantBuffer() = delete;
	CConstantBuffer(UINT elementByteSize, UINT elementCount, CB_TYPE type);
	CConstantBuffer(const CConstantBuffer& _other) = delete;
	~CConstantBuffer();
	CLONE_DISABLE(CConstantBuffer)

	void Bind_Graphics(UINT CBIndex, UINT rootParamIndex);
	void Bind_Compute(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
	void CopyData(int elementIndex, const void* data);

private:
	void BuildResources(UINT elementByteSize, UINT elementCount);

	CB_TYPE									m_Type;
	UINT									m_ElementByteSize;
	UINT									m_BufferByteSize;

	ComPtr<ID3D12Resource>					m_UploadBuffer;
	BYTE*									m_MappedData;

};

