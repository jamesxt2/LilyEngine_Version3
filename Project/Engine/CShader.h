#pragma once
#include "CAsset.h"

class CShader : public CAsset
{
public:
	CShader(ASSET_TYPE type);
	CShader(const CShader& other) = delete;
	~CShader();
	CLONE_DISABLE(CShader)

	virtual void Bind(const std::string& PSOGroupKey) = 0;

protected:
	ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target);
};

