#pragma once
#include "CShader.h"

class CGraphicsShader : public CShader
{
public:
	CGraphicsShader();
	~CGraphicsShader();

	virtual void Bind(const std::string& PSOGroupKey) override;
	void BuildVertexShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint);
	void BuildGeometryShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint);
	void BuildPixelShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint);

	std::vector<D3D12_INPUT_ELEMENT_DESC>							m_InputLayout;

private:
	ComPtr<ID3DBlob>												m_vsByteCode;
	ComPtr<ID3DBlob>												m_gsByteCode;
	ComPtr<ID3DBlob>												m_psByteCode;

public:
	inline ComPtr<ID3DBlob> GetVsByteCode() const { return m_vsByteCode; }
	inline ComPtr<ID3DBlob> GetGsByteCode() const { return m_gsByteCode; }
	inline ComPtr<ID3DBlob> GetPsByteCode() const { return m_psByteCode; }
	inline std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputLayout() { return m_InputLayout; }
};

