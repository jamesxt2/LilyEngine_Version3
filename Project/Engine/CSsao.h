#pragma once
#include "singleton.h"

#include "CDescriptorAllocator.h"

class CGameObject;

class CSsao : public CSingleton<CSsao>
{
	SINGLE(CSsao)

public:

	static const DXGI_FORMAT AmbientMapFormat = DXGI_FORMAT_R16_UNORM;
	static const DXGI_FORMAT NormalMapFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	static const int MaxBlurRadius = 5;

	/// <summary>
	/// Should be called when device and all managers are intializing
	/// </summary>
	void Initialize(UINT width, UINT height);

	///<summary>
	/// Call when the backbuffer is resized.  
	///</summary>
	void OnResize(UINT newWidth, UINT newHeight);

	void BuildDescriptors(ComPtr<ID3D12Resource> depthStencilBuffer);
	void RebuildDescriptors(ComPtr<ID3D12Resource> depthStencilBuffer);


	std::vector<float> CalcGaussWeights(float sigma);

	void DrawNormalsAndDepth(const std::vector<CGameObject*>& defaultObjs, const std::vector<CGameObject*>& skinnedObjs);

	void ComputeSsao(int blurCount);

private:
	
	void BuildOffsetVectors();
	void BuildResources();
	void BuildRandomVectorTexture();
	void BlurAmbientMap(int blurCount);
	void BlurAmbientMap(bool horzBlur);

	ComPtr<ID3D12Resource> m_RandomVectorMap;
	ComPtr<ID3D12Resource> m_RandomVectorMapUploadBuffer;

	ComPtr<ID3D12Resource> m_NormalMap;
	ComPtr<ID3D12Resource> m_AmbientMap0;
	ComPtr<ID3D12Resource> m_AmbientMap1;

	CDescriptorAllocator::Allocation m_NormalMapSrv;
	CDescriptorAllocator::Allocation m_NormalMapRtv;
	CDescriptorAllocator::Allocation m_DepthMapSrv;
	CDescriptorAllocator::Allocation m_RandomVectorMapSrv;
	CDescriptorAllocator::Allocation m_AmbientMap0Srv;
	CDescriptorAllocator::Allocation m_AmbientMap0Rtv;
	CDescriptorAllocator::Allocation m_AmbientMap1Srv;
	CDescriptorAllocator::Allocation m_AmbientMap1Rtv;

	UINT m_RenderTargetWidth = 0;
	UINT m_RenderTargetHeight = 0;

	Vector4 m_Offsets[14];

	D3D12_VIEWPORT m_Viewport = {};
	D3D12_RECT m_ScissorRect = {};

public:
	void GetOffsetVectors(Vector4 offsets[14])
	{
		std::copy(&m_Offsets[0], &m_Offsets[14], &offsets[0]);
	}

	inline UINT GetSsaoMapWidth() const { return m_RenderTargetWidth / 2; }

	inline UINT GetSsaoMapHeight() const { return m_RenderTargetHeight / 2; }

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetAmbientMap0SrvGpuHandle() { return m_AmbientMap0Srv.gpuHandle; }
};

