#pragma once
#include "CAsset.h"

#include "CGraphicsShader.h"


class CMesh : public CAsset
{
public:
	CMesh();
	CMesh(const CMesh& other) = delete;
	~CMesh();
	CLONE_DISABLE(CMesh)

	virtual int Load(const std::wstring& filePath) override { return S_OK; }
	virtual int Save(const std::wstring& filePath) override { return S_OK; }

	void CreateVertexBuffer(Vertex* vtxData, UINT vtxCount);
	void CreateIndexBuffer(UINT* idxData, UINT idxCount);

	void Render();

private:

	void Bind();

	CGraphicsShader*						m_Shader;

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

public:
	inline void SetShader(CGraphicsShader* shader) { m_Shader = shader; }
};

