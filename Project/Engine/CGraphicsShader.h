#pragma once
#include "CShader.h"

class CGraphicsShader : public CShader
{
public:
	CGraphicsShader();
	~CGraphicsShader();

	virtual void Bind() override;
	void BuildVertexShaderAndInputLayout(const std::wstring& filename, const std::string& entrypoint);
	void BuildPixelShader(const std::wstring& filename, const std::string& entrypoint);

	void BuildPSO();

private:


	ComPtr<ID3DBlob>							m_vsByteCode;
	ComPtr<ID3DBlob>							m_psByteCode;

	std::vector<D3D12_INPUT_ELEMENT_DESC>		m_InputLayout;

	ComPtr<ID3D12PipelineState>					m_PSO;
};

