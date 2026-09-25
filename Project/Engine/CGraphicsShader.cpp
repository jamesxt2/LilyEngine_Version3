#include "pch.h"
#include "CGraphicsShader.h"

#include "CDevice.h"
#include "CConstantBuffer.h"

CGraphicsShader::CGraphicsShader()
    : CShader(ASSET_TYPE::GRAPHICS_SHADER), 
    m_vsByteCode(nullptr), m_psByteCode(nullptr)
{
}

CGraphicsShader::~CGraphicsShader()
{
}

void CGraphicsShader::Bind(const std::string& PSOGroupKey)
{
    
}

void CGraphicsShader::BuildVertexShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint)
{
    m_vsByteCode = CompileShader(filename, defines, entrypoint, "vs_5_0");
}

void CGraphicsShader::BuildGeometryShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint)
{
    m_gsByteCode = CompileShader(filename, defines, entrypoint, "gs_5_0");
}

void CGraphicsShader::BuildPixelShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint)
{
    m_psByteCode = CompileShader(filename, defines, entrypoint, "ps_5_0");
}



