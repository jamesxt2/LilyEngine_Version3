#include "pch.h"
#include "CMesh.h"

#include "CDevice.h"

CMesh::CMesh()
	: CAsset(ASSET_TYPE::MESH),
	m_VertexCount(0),
	m_VertexByteStride(0), m_VertexBufferByteSize(0),
	m_IndexCount(0),
	m_IndexFormat(DXGI_FORMAT_R16_UINT), m_IndexBufferByteSize(0)
{

}

CMesh::~CMesh()
{
	
}

ComPtr<ID3D12Resource> CMesh::BuildBufferResource(const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer, ID3D12GraphicsCommandList* cmdlist)
{
	ComPtr<ID3D12Resource> defaultBuffer;

	// Create actual default buffer resource
	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc(CD3DX12_RESOURCE_DESC::Buffer(byteSize));
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())
	));

	// To copy the data from CPU memory to default buffer, we need to create an intermediate upload buffer
	heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())
	));

	// describe the data we want to copy to the default buffer
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = byteSize;

	// copy

	assert(cmdlist);

	CD3DX12_RESOURCE_BARRIER barrier1(CD3DX12_RESOURCE_BARRIER::Transition(
		defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	));
	cmdlist->ResourceBarrier(1, &barrier1);
	UpdateSubresources<1>(
		cmdlist, defaultBuffer.Get(), uploadBuffer.Get(),
		0, 0, 1, &subResourceData
	);
	CD3DX12_RESOURCE_BARRIER barrier2(CD3DX12_RESOURCE_BARRIER::Transition(
		defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ
	));
	cmdlist->ResourceBarrier(1, &barrier2);

	return defaultBuffer;
}

void CMesh::CreateIndexBuffer32(UINT* idxData, UINT idxCount, ID3D12GraphicsCommandList* cmdlist)
{
	m_IndexCount = idxCount;
	m_IndexFormat = DXGI_FORMAT_R32_UINT;
	m_IndexBufferByteSize = idxCount * sizeof(UINT);
	ThrowIfFailed(D3DCreateBlob(m_IndexBufferByteSize, &m_IndexBufferCPU));
	CopyMemory(m_IndexBufferCPU->GetBufferPointer(), idxData, m_IndexBufferByteSize);
	m_IndexBufferGPU = BuildBufferResource(idxData, m_IndexBufferByteSize, m_IndexBufferUploader, cmdlist);
}

void CMesh::CreateIndexBuffer16(uint16* idxData, UINT idxCount, ID3D12GraphicsCommandList* cmdlist)
{
	m_IndexCount = idxCount;
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
	m_IndexBufferByteSize = idxCount * sizeof(uint16);
	ThrowIfFailed(D3DCreateBlob(m_IndexBufferByteSize, &m_IndexBufferCPU));
	CopyMemory(m_IndexBufferCPU->GetBufferPointer(), idxData, m_IndexBufferByteSize);
	m_IndexBufferGPU = BuildBufferResource(idxData, m_IndexBufferByteSize, m_IndexBufferUploader, cmdlist);
}

void CMesh::Bind()
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = m_VertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = m_VertexByteStride;
	vbv.SizeInBytes = m_VertexBufferByteSize;
	CMDLIST->IASetVertexBuffers(0, 1, &vbv);

	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = m_IndexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = m_IndexFormat;
	ibv.SizeInBytes = m_IndexBufferByteSize;
	CMDLIST->IASetIndexBuffer(&ibv);
}

Ptr<SubmeshGeometry> CMesh::GetSubGeo(const std::wstring& key)
{
	return m_DrawArgs.find(key)->second;
}
