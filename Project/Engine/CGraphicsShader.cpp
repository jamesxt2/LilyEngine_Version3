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

void CGraphicsShader::BuildVertexShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
    m_vsByteCodeMap[key] = CompileShader(filename, defines, entrypoint, target);
}

void CGraphicsShader::BuildGeometryShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
    m_gsByteCodeMap[key] = CompileShader(filename, defines, entrypoint, target);
}

void CGraphicsShader::BuildHullShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
    m_hsByteCodeMap[key] = CompileShader(filename, defines, entrypoint, target);
}

void CGraphicsShader::BuildDomainShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
    m_dsByteCodeMap[key] = CompileShader(filename, defines, entrypoint, target);
}

void CGraphicsShader::BuildPixelShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
    m_psByteCodeMap[key] = CompileShader(filename, defines, entrypoint, target);
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

ComPtr<ID3DBlob> CGraphicsShader::GetHsByteCode(const std::wstring& key) const
{
    std::unordered_map<std::wstring, ComPtr<ID3DBlob>>::const_iterator iter = m_hsByteCodeMap.find(key);
    if (iter == m_hsByteCodeMap.end())
        return nullptr;
    return iter->second;
}

ComPtr<ID3DBlob> CGraphicsShader::GetDsByteCode(const std::wstring& key) const
{
    std::unordered_map<std::wstring, ComPtr<ID3DBlob>>::const_iterator iter = m_dsByteCodeMap.find(key);
    if (iter == m_dsByteCodeMap.end())
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



