#include "pch.h"
#include "CConstantBuffer.h"

#include "CDevice.h"

CConstantBuffer::CConstantBuffer(UINT elementByteSize, UINT elementCount, CB_TYPE type)
	: m_Type(type), m_ElementByteSize(0), m_BufferByteSize(0),
	 m_UploadBuffer(nullptr), m_MappedData(nullptr)
{
	BuildResources(elementByteSize, elementCount);
}

CConstantBuffer::~CConstantBuffer()
{
	if (m_UploadBuffer != nullptr)
		m_UploadBuffer->Unmap(0, nullptr);
	m_MappedData = nullptr;
}

void CConstantBuffer::BuildResources(UINT elementByteSize, UINT elementCount)
{
	m_ElementByteSize = (UINT)elementByteSize;
	m_BufferByteSize = Utilities::CalcConstantBufferByteSize((UINT)elementByteSize);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC rDesc(CD3DX12_RESOURCE_DESC::Buffer(m_BufferByteSize * elementCount));
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

void CConstantBuffer::Bind_Graphics(UINT CBIndex, UINT rootParamIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS CBAddress = m_UploadBuffer->GetGPUVirtualAddress() + m_BufferByteSize * CBIndex;
	CMDLIST->SetGraphicsRootConstantBufferView(rootParamIndex, CBAddress);
}

void CConstantBuffer::Bind_Compute(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex)
{
	cmdlist->SetComputeRootConstantBufferView(rootParamIndex, m_UploadBuffer->GetGPUVirtualAddress());
}

void CConstantBuffer::CopyData(int elementIndex, const void* data)
{
	memcpy(&m_MappedData[elementIndex * m_BufferByteSize], data, m_ElementByteSize);
}
