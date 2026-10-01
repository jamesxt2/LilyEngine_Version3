#include "pch.h"
#include "CMesh.h"

CMesh::CMesh()
	: CAsset(ASSET_TYPE::MESH),
	m_VertexBufferCPU(nullptr), m_VertexBufferGPU(nullptr),
	m_VertexBufferUploader(nullptr), m_VertexCount(0),
	m_VertexByteStride(0), m_VertexBufferByteSize(0),
	m_IndexBufferCPU(nullptr), m_IndexBufferGPU(nullptr),
	m_IndexBufferUploader(nullptr), m_IndexCount(0),
	m_IndexFormat(DXGI_FORMAT_R16_UINT), m_IndexBufferByteSize(0)
{

}

CMesh::~CMesh()
{

}

void CMesh::CreateIndexBuffer32(UINT* idxData, UINT idxCount, ID3D12GraphicsCommandList* cmdlist)
{
	m_IndexCount = idxCount;
	m_IndexFormat = DXGI_FORMAT_R32_UINT;
	m_IndexBufferByteSize = idxCount * sizeof(UINT);
	ThrowIfFailed(D3DCreateBlob(m_IndexBufferByteSize, &m_IndexBufferCPU));
	CopyMemory(m_IndexBufferCPU->GetBufferPointer(), idxData, m_IndexBufferByteSize);
	m_IndexBufferGPU = CDevice::GetInst()->CreateDefaultBuffer(idxData, m_IndexBufferByteSize, m_IndexBufferUploader, cmdlist);
}

void CMesh::CreateIndexBuffer16(uint16* idxData, UINT idxCount, ID3D12GraphicsCommandList* cmdlist)
{
	m_IndexCount = idxCount;
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
	m_IndexBufferByteSize = idxCount * sizeof(uint16);
	ThrowIfFailed(D3DCreateBlob(m_IndexBufferByteSize, &m_IndexBufferCPU));
	CopyMemory(m_IndexBufferCPU->GetBufferPointer(), idxData, m_IndexBufferByteSize);
	m_IndexBufferGPU = CDevice::GetInst()->CreateDefaultBuffer(idxData, m_IndexBufferByteSize, m_IndexBufferUploader, cmdlist);
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

SubmeshGeometry* CMesh::GetSubGeo(const std::string& key)
{
	return &m_DrawArgs.find(key)->second;
}
