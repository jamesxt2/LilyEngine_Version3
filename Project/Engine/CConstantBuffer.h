#pragma once
#include "CEntity.h"

class CConstantBuffer : public CEntity
{
public:
	CConstantBuffer();
	CConstantBuffer(size_t bufferSize, CB_TYPE type);
	CConstantBuffer(const CConstantBuffer& _other) = delete;
	~CConstantBuffer();
	CLONE_DISABLE(CConstantBuffer)

	void Create(size_t bufferSize, CB_TYPE type);
	void Bind();
	void CopyData(const void* data);

private:

	void BuildCbvDescriptorHeap();
	void BuildRootSignature();

	CB_TYPE									m_Type;
	UINT									m_BufferSize;
	UINT									m_ElementByteSize;
	ComPtr<ID3D12DescriptorHeap>			m_CbvHeap;
	ComPtr<ID3D12Resource>					m_UploadBuffer;
	void*									m_MappedData;
	ComPtr<ID3D12RootSignature>				m_RootSignature;

public:
	inline ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_RootSignature; }
};

