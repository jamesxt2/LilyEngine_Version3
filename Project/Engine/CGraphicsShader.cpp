#include "pch.h"
#include "CGraphicsShader.h"

#include "CDevice.h"
#include "CConstantBuffer.h"

CGraphicsShader::CGraphicsShader()
    : CShader(ASSET_TYPE::GRAPHICS_SHADER)
{
}

CGraphicsShader::~CGraphicsShader()
{
}

void CGraphicsShader::BuildVertexShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint)
{
    m_vsByteCodeMap[key] = CompileShader(filename, defines, entrypoint, "vs_5_0");
}

void CGraphicsShader::BuildGeometryShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint)
{
    m_gsByteCodeMap[key] = CompileShader(filename, defines, entrypoint, "gs_5_0");
}

void CGraphicsShader::BuildPixelShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint)
{
    m_psByteCodeMap[key] = CompileShader(filename, defines, entrypoint, "ps_5_0");
}

ComPtr<ID3DBlob> CGraphicsShader::GetVsByteCode(const std::wstring& key) const
{
    std::unordered_map<std::wstring, ComPtr<ID3DBlob>>::const_iterator iter = m_vsByteCodeMap.find(key);
    if (iter == m_vsByteCodeMap.end())
        return nullptr;
    return iter->second;
}

ComPtr<ID3DBlob> CGraphicsShader::GetGsByteCode(const std::wstring& key) const
{
    std::unordered_map<std::wstring, ComPtr<ID3DBlob>>::const_iterator iter = m_gsByteCodeMap.find(key);
    if (iter == m_gsByteCodeMap.end())
        return nullptr;
    return iter->second;
}

ComPtr<ID3DBlob> CGraphicsShader::GetPsByteCode(const std::wstring& key) const
{
    std::unordered_map<std::wstring, ComPtr<ID3DBlob>>::const_iterator iter = m_psByteCodeMap.find(key);
    if (iter == m_psByteCodeMap.end())
        return nullptr;
    return iter->second;
}



