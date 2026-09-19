#pragma once
#include "CShader.h"

class CGraphicsShader : public CShader
{
public:
	CGraphicsShader();
	~CGraphicsShader();

	virtual void Bind(const std::string& PSOGroupKey) override;
	void BuildVertexShaderAndInputLayout(const std::wstring& filename, const std::string& entrypoint);
	void BuildPixelShader(const std::wstring& filename, const std::string& entrypoint);

private:
	ComPtr<ID3DBlob>												m_vsByteCode;
	ComPtr<ID3DBlob>												m_psByteCode;

	std::vector<D3D12_INPUT_ELEMENT_DESC>							m_InputLayout;

public:
	inline ComPtr<ID3DBlob> GetVsByteCode() const { return m_vsByteCode; }
	inline ComPtr<ID3DBlob> GetPsByteCode() const { return m_psByteCode; }
	inline std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputLayout() { return m_InputLayout; }
};

