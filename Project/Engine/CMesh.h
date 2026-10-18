#pragma once
#include "CAsset.h"

#include "MeshData.h"
#include "CDevice.h"

class CMesh : public CAsset
{
public:
	CMesh();
	CMesh(const CMesh& other) = delete;
	~CMesh();
	CLONE_DISABLE(CMesh)
	friend class CAssetMgr;

	template<typename T>
	void CreateVertexBuffer(T* vtxData, UINT vtxCount, ID3D12GraphicsCommandList* cmdlist);


	void CreateIndexBuffer32(UINT* idxData, UINT idxCount, ID3D12GraphicsCommandList* cmdlist);
	void CreateIndexBuffer16(uint16* idxData, UINT idxCount, ID3D12GraphicsCommandList* cmdlist);

	void Bind();

private:

	ComPtr<ID3D12Resource> BuildBufferResource(const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer, ID3D12GraphicsCommandList* cmdlist);

	// Vertex
	ComPtr<ID3DBlob>						m_VertexBufferCPU;
	ComPtr<ID3D12Resource>					m_VertexBufferGPU;
	ComPtr<ID3D12Resource>					m_VertexBufferUploader;
	UINT									m_VertexCount;
	UINT									m_VertexByteStride;
	UINT									m_VertexBufferByteSize;

	// Index
	ComPtr<ID3DBlob>						m_IndexBufferCPU;
	ComPtr<ID3D12Resource>					m_IndexBufferGPU;
	ComPtr<ID3D12Resource>					m_IndexBufferUploader;
	UINT									m_IndexCount;
	DXGI_FORMAT								m_IndexFormat;
	UINT									m_IndexBufferByteSize;

	std::unordered_map<std::wstring, Ptr<SubmeshGeometry>> m_DrawArgs;
	BoundingBox								m_MeshBounds;

public:
	Ptr<SubmeshGeometry> GetSubGeo(const std::wstring& key);

	inline UINT GetIndexCount() const { return m_IndexCount; }
	inline BoundingBox GetMeshBounds() const { return m_MeshBounds; }
};

template<typename T>
inline void CMesh::CreateVertexBuffer(T* vtxData, UINT vtxCount, ID3D12GraphicsCommandList* cmdlist)
{
	m_VertexByteStride = sizeof(T);
	m_VertexBufferByteSize = vtxCount * m_VertexByteStride;
	ThrowIfFailed(D3DCreateBlob(m_VertexBufferByteSize, &m_VertexBufferCPU));
	CopyMemory(m_VertexBufferCPU->GetBufferPointer(), vtxData, m_VertexBufferByteSize);
	m_VertexBufferGPU = BuildBufferResource(vtxData, m_VertexBufferByteSize, m_VertexBufferUploader, cmdlist);
}
