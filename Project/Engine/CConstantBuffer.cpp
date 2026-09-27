#include "pch.h"
#include "CConstantBuffer.h"

#include "CDevice.h"

ComPtr<ID3D12DescriptorHeap> CConstantBuffer::m_CbvHeap = nullptr;

CConstantBuffer::CConstantBuffer()
	: m_Type(CB_TYPE::END), m_BufferSize(0), m_ElementByteSize(0),
	m_TotalSize(0),
	m_UploadBuffer(nullptr), m_MappedData(nullptr)
{

}

CConstantBuffer::CConstantBuffer(UINT elementByteSize, UINT elementCount, CB_TYPE type)
	: m_Type(CB_TYPE::END), m_BufferSize(0), m_ElementByteSize(0),
	m_TotalSize(0),
	 m_UploadBuffer(nullptr), m_MappedData(nullptr)
{
	Create(elementByteSize, elementCount, type);
}

CConstantBuffer::~CConstantBuffer()
{
	if (m_UploadBuffer != nullptr)
		m_UploadBuffer->Unmap(0, nullptr);
	m_MappedData = nullptr;
}

void CConstantBuffer::BuildCbvDescriptorHeap()
{
	UINT NumDescriptors = g_MaxObjectCount * g_NumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = NumDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(DEVICE->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_CbvHeap)));
}

void CConstantBuffer::Create(UINT elementByteSize, UINT elementCount, CB_TYPE type)
{
	m_BufferSize = (UINT)elementByteSize;
	m_Type = type;
	m_ElementByteSize = CalcConstantBufferByteSize((UINT)elementByteSize);
	m_TotalSize = m_ElementByteSize * elementCount;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC rDesc(CD3DX12_RESOURCE_DESC::Buffer(m_TotalSize));
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&rDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UploadBuffer)
	));

	ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
}

void CConstantBuffer::Bind(int CBIndex, int slot)
{
	D3D12_GPU_VIRTUAL_ADDRESS CBAddress = m_UploadBuffer->GetGPUVirtualAddress() + m_ElementByteSize * CBIndex;
	CMDLIST->SetGraphicsRootConstantBufferView(slot, CBAddress);
}

void CConstantBuffer::CopyData(int elementIndex, const void* data)
{
	memcpy(&m_MappedData[elementIndex * m_ElementByteSize], data, m_BufferSize);
}
