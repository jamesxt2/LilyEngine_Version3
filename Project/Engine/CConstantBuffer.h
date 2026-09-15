#pragma once
#include "CEntity.h"

class CConstantBuffer : public CEntity
{
public:
	CConstantBuffer();
	CConstantBuffer(UINT elementByteSize, UINT elementCount, CB_TYPE type);
	CConstantBuffer(const CConstantBuffer& _other) = delete;
	~CConstantBuffer();
	CLONE_DISABLE(CConstantBuffer)

	void Create(UINT elementByteSize, UINT elementCount, CB_TYPE type);
	void Bind(int ObjCBIndex = 0, int slot = 0);
	void CopyData(int elementIndex, const void* data);

	static void Init(UINT cbvSlotNums);

	static void BuildCbvDescriptorHeap();
	static void BuildRootSignature(UINT slotCount);


private:
	CB_TYPE									m_Type;
	UINT									m_BufferSize;
	UINT									m_ElementByteSize;
	UINT									m_TotalSize;

	static ComPtr<ID3D12DescriptorHeap>		m_CbvHeap;
	ComPtr<ID3D12Resource>					m_UploadBuffer;
	BYTE*									m_MappedData;
	static ComPtr<ID3D12RootSignature>		m_RootSignature;

public:
	inline ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_RootSignature; }
};

