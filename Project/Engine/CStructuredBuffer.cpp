#include "pch.h"
#include "CStructuredBuffer.h"

#include "CDevice.h"


CStructuredBuffer::CStructuredBuffer(UINT elementByteSize, UINT elementCount)
	: m_ElementByteSize(elementByteSize), m_ElementCount(elementCount), m_BufferSize(elementByteSize * elementCount)
{
	BuildResources();
	BuildDescriptors();
}

CStructuredBuffer::~CStructuredBuffer()
{
}

void CStructuredBuffer::BuildResources()
{
	assert(m_ElementByteSize % 16 == 0 && "StructuredBuffer element byte size must be multiple of 16!");

	CD3DX12_RESOURCE_DESC gpuDesc(CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
	CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(DEVICE->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE,
		&gpuDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&m_GpuBuffer)));

	CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC uploadDesc(CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize));
	ThrowIfFailed(DEVICE->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE,
		&uploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&m_InputUploadBuffer)));

	CD3DX12_HEAP_PROPERTIES readbackHeapProps(D3D12_HEAP_TYPE_READBACK);
	CD3DX12_RESOURCE_DESC readbackDesc(CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize));
	ThrowIfFailed(DEVICE->CreateCommittedResource(&readbackHeapProps, D3D12_HEAP_FLAG_NONE,
		&readbackDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
		IID_PPV_ARGS(&m_ReadbackBuffer)));
}

void CStructuredBuffer::BuildDescriptors()
{
	m_SrvAlloc = CDescriptorAllocator::GetInst()->Allocate(1);
	assert(m_SrvAlloc.IsValid());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_ElementCount;
	srvDesc.Buffer.StructureByteStride = m_ElementByteSize;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	DEVICE->CreateShaderResourceView(m_GpuBuffer.Get(), &srvDesc, m_SrvAlloc.cpuHandle);

	m_UavAlloc = CDescriptorAllocator::GetInst()->Allocate(1);
	assert(m_UavAlloc.IsValid());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_ElementCount;
	uavDesc.Buffer.StructureByteStride = m_ElementByteSize;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	DEVICE->CreateUnorderedAccessView(m_GpuBuffer.Get(), nullptr, &uavDesc, m_UavAlloc.cpuHandle);
}

void CStructuredBuffer::UploadData(const void* data)
{
	// CPU -> UploadBuffer
	void* mapped = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	m_InputUploadBuffer->Map(0, &readRange, &mapped);
	memcpy(mapped, data, m_BufferSize);
	m_InputUploadBuffer->Unmap(0, nullptr);

	CDevice::GetInst()->UploadResourceAsync([&](ID3D12GraphicsCommandList* cmdlist) {
		CD3DX12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(
			m_GpuBuffer.Get(), m_CurrentState, D3D12_RESOURCE_STATE_COPY_DEST));
		cmdlist->ResourceBarrier(1, &barrier);

		cmdlist->CopyBufferRegion(m_GpuBuffer.Get(), 0, m_InputUploadBuffer.Get(), 0, m_BufferSize);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_GpuBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cmdlist->ResourceBarrier(1, &barrier);
		m_CurrentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		});

}

void CStructuredBuffer::ReadbackData(void* outData)
{
	CDevice::GetInst()->UploadResourceAsync([&](ID3D12GraphicsCommandList* cmdlist) {
		CD3DX12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(
			m_GpuBuffer.Get(), m_CurrentState,
			D3D12_RESOURCE_STATE_COPY_SOURCE));
		cmdlist->ResourceBarrier(1, &barrier);

		cmdlist->CopyBufferRegion(m_ReadbackBuffer.Get(), 0, m_GpuBuffer.Get(), 0, m_BufferSize);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_GpuBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_CurrentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		});

	CDevice::GetInst()->WaitForAllUploads();

	void* pMapped = nullptr;
	CD3DX12_RANGE readRange(0, m_BufferSize);
	m_ReadbackBuffer->Map(0, &readRange, &pMapped);
	memcpy(outData, pMapped, m_BufferSize);
	m_ReadbackBuffer->Unmap(0, nullptr);
}

void CStructuredBuffer::Bind_Graphics_SRV_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex)
{
	if (m_CurrentState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
	{
		CD3DX12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(
			m_GpuBuffer.Get(), m_CurrentState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		cmdlist->ResourceBarrier(1, &barrier);
		m_CurrentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}
	cmdlist->SetGraphicsRootDescriptorTable(rootParamIndex, m_SrvAlloc.gpuHandle);
}

void CStructuredBuffer::Bind_Compute_SRV_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex)
{
	if (m_CurrentState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
	{
		CD3DX12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(
			m_GpuBuffer.Get(), m_CurrentState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		cmdlist->ResourceBarrier(1, &barrier);
		m_CurrentState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}
	cmdlist->SetComputeRootDescriptorTable(rootParamIndex, m_SrvAlloc.gpuHandle);
}

void CStructuredBuffer::Bind_Compute_UAV_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex)
{
	if (m_CurrentState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	{
		CD3DX12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(
			m_GpuBuffer.Get(), m_CurrentState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		cmdlist->ResourceBarrier(1, &barrier);
		m_CurrentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
	cmdlist->SetComputeRootDescriptorTable(rootParamIndex, m_UavAlloc.gpuHandle);
}
