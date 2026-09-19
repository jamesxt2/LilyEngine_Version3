#pragma once
#include "singleton.h"

class CConstantBuffer;

class CDevice : public CSingleton<CDevice>
{
	SINGLE(CDevice)

public:
	int Init(HWND _MainWnd, POINT _RenderResolution);
	void PostInit();

	void OnResize(POINT newRenderResolution);

	void Update();

	void ClearTargetAndPrepareRender(XMVECTORF32 color);
	void ExecuteAndFinishDrawCall();

	void Reset();
	void Close();

	void FlushCommandQueue();

	UINT											m_RtvDescriptorSize;
	UINT											m_DsvDescriptorSize;
	UINT											m_CbvUavDescriptorSize;

private:

	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	void BuildRootSignature(UINT slotCount);

	void BuildFrameResources();


	HWND											m_MainWnd;
	POINT											m_RenderResolution;

	ComPtr<ID3D12Device>							m_d3dDevice;
	ComPtr<IDXGIFactory4>							m_dxgiFactory;

	ComPtr<ID3D12Fence>								m_Fence;
	UINT64											m_CurrentFence;
	

	UINT											m_4xMsaaQuality;

	ComPtr<ID3D12CommandQueue>						m_CommandQueue;
	ComPtr<ID3D12CommandAllocator>					m_DirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList>				m_CommandList;

	int												m_CurrentBackBuffer = 0;
	static constexpr int							m_SwapChainBufferCount = 2;
	ComPtr<IDXGISwapChain>							m_SwapChain;

	ComPtr<ID3D12Resource>							m_SwapChainBuffer[m_SwapChainBufferCount];
	ComPtr<ID3D12Resource>							m_DepthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap>					m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap>					m_DsvHeap;

	ComPtr<ID3D12Resource>							m_MsaaRenderTarget;
	ComPtr<ID3D12DescriptorHeap>					m_MsaaRtvHeap;
	bool											m_EnableMSAA;

	D3D12_VIEWPORT									m_ScreenViewport;
	D3D12_RECT										m_ScissorRect;

	std::vector<std::unique_ptr<FrameResource>>		m_FrameResources;
	FrameResource*									m_CurrFrameResource;
	int												m_CurrFrameResourceIndex;

	ComPtr<ID3D12RootSignature>						m_RootSignature;

	const float m_ClearColor[4] = { 0.69f, 0.77f, 0.87f, 1.0f };

public:
	inline ComPtr<ID3D12Device> GetDevice() const { return m_d3dDevice; }
	inline ComPtr<ID3D12GraphicsCommandList> GetCmdList() const { return m_CommandList; }
	inline std::shared_ptr<CConstantBuffer> GetConstBuffer(CB_TYPE type)
	{
		return m_CurrFrameResource->GetConstantBuffer(type);
	}

	inline Vector2 GetRenderResolution() const { return Vector2((float)m_RenderResolution.x, (float)m_RenderResolution.y); }
	inline float GetAspectRatio() const { return (float)m_RenderResolution.x / (float)m_RenderResolution.y; }

	inline FrameResource* GetCurrFrameResource() const { return m_CurrFrameResource; }

	inline ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_RootSignature; }

	inline bool EnableMSAA() const { return m_EnableMSAA; }
	inline UINT Get4xMSAAQuality() const { return m_4xMsaaQuality; }
};

