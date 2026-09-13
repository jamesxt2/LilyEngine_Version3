#pragma once
#include "CAsset.h"

class CShader : public CAsset
{
public:
	CShader(ASSET_TYPE type);
	CShader(const CShader& other) = delete;
	~CShader();
	CLONE_DISABLE(CShader)

	virtual void Bind() = 0;

	virtual int Load(const std::wstring& filePath) override { return S_OK; }
	virtual int Save(const std::wstring& filePath) override { return S_OK; }

protected:
	ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target);

	ComPtr<ID3D12RootSignature> m_RootSignature;

public:
	inline void SetRootSignature(const ComPtr<ID3D12RootSignature> rootSig) { m_RootSignature = rootSig; }
};

