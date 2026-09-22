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

void CGraphicsShader::BuildVertexShaderAndInputLayout(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint)
{
    m_vsByteCode = CompileShader(filename, defines, entrypoint, "vs_5_0");

    m_InputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void CGraphicsShader::BuildPixelShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint)
{
    m_psByteCode = CompileShader(filename, defines, entrypoint, "ps_5_0");
}



