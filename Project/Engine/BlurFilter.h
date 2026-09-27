#pragma once

class BlurFilter
{
public:
	BlurFilter();

	BlurFilter(const BlurFilter& rhs) = delete;
	BlurFilter& operator=(const BlurFilter& rhs) = delete;
	~BlurFilter() = default;

	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
		UINT descriptorSize);

	void OnResize(UINT newWidth, UINT newHeight);

	void Execute(ID3D12Resource* input, int blurCount);

private:

	std::vector<float> CalcGaussWeights(float sigma);

	void BuildDescriptors();
	void BuildResource();

	const int MaxBlurRadius = 5;

	UINT m_Width = 0;
	UINT m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur0CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur0CpuUav;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur1CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur1CpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur0GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur0GpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur1GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur1GpuUav;

	// Two for ping-ponging the textures.
	ComPtr<ID3D12Resource> m_BlurMap0 = nullptr;
	ComPtr<ID3D12Resource> m_BlurMap1 = nullptr;
};

