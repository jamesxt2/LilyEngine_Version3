#include "pch.h"
#include "CComputeShader.h"

CComputeShader::CComputeShader()
	: CShader(ASSET_TYPE::COMPUTE_SHADER)
{
}

CComputeShader::~CComputeShader()
{
}

void CComputeShader::BuildComputeShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint)
{
	m_csByteCode = CompileShader(filename, defines, entrypoint, "cs_5_0");
}
