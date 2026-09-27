#pragma once
#include "CShader.h"

class CComputeShader : public CShader
{
public:
	CComputeShader();
	~CComputeShader();

	void BuildComputeShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint);

private:
	ComPtr<ID3DBlob>												m_csByteCode;

public:
	inline ComPtr<ID3DBlob> GetCsByteCode() const { return m_csByteCode; }
};

