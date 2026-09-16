#pragma once
#include "CAsset.h"

#include "CGraphicsShader.h"
#include "MeshData.h"

class CMesh : public CAsset
{
public:
	CMesh();
	CMesh(const CMesh& other) = delete;
	~CMesh();
	CLONE_DISABLE(CMesh)
	friend class CAssetMgr;

	virtual int Load(const std::wstring& filePath) override { return S_OK; }
	virtual int Save(const std::wstring& filePath) override { return S_OK; }

	void CreateVertexBuffer(Vertex* vtxData, UINT vtxCount);
	void CreateIndexBuffer32(UINT* idxData, UINT idxCount);
	void CreateIndexBuffer16(uint16* idxData, UINT idxCount);

	void Render();

private:

	void Bind();

	CGraphicsShader* m_Shader;

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

	std::unordered_map<std::string, SubmeshGeometry> m_DrawArgs;

public:
	inline void SetShader(CGraphicsShader* shader) { m_Shader = shader; }
	SubmeshGeometry* GetSubGeo(const std::string& key);
};

