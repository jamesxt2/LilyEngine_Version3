#pragma once
#include "CEntity.h"

class CConstantBuffer : public CEntity
{
public:
	CConstantBuffer();
	CConstantBuffer(const CConstantBuffer& _other) = delete;
	~CConstantBuffer();
	CLONE_DISABLE(CConstantBuffer)

	void Create(size_t bufferSize, CB_TYPE type);
	void Bind();
	void CopyData(const void* data);

private:

	void BuildCbvDescriptorHeap();
	void BuildRootSignature();

	inline static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (byteSize + 255) & ~255;
	}

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

