#pragma once
#include "CShader.h"

class CGraphicsShader : public CShader
{
public:
	CGraphicsShader();
	~CGraphicsShader();

	void BuildVertexShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint);
	void BuildGeometryShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint);
	void BuildPixelShader(const std::wstring& key, const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint);

	std::vector<D3D12_INPUT_ELEMENT_DESC>						m_InputLayout;

private:
	std::unordered_map<std::wstring, ComPtr<ID3DBlob>>			m_vsByteCodeMap;
	std::unordered_map<std::wstring, ComPtr<ID3DBlob>>			m_gsByteCodeMap;
	std::unordered_map<std::wstring, ComPtr<ID3DBlob>>			m_psByteCodeMap;

public:
	ComPtr<ID3DBlob> GetVsByteCode(const std::wstring& key) const;
	ComPtr<ID3DBlob> GetGsByteCode(const std::wstring& key) const;
	ComPtr<ID3DBlob> GetPsByteCode(const std::wstring& key) const;
	inline std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputLayout() { return m_InputLayout; }
};

