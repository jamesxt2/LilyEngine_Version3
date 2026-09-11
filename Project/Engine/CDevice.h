#pragma once
#include "singleton.h"

class CDevice : public CSingleton<CDevice>
{
	SINGLE(CDevice)

public:
	int Init(HWND _MainWnd, POINT _RenderResolution);

	void OnResize(POINT newRenderResolution);

	void Draw();

private:

	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	void FlushCommandQueue();


	HWND								m_MainWnd;
	POINT								m_RenderResolution;

	ComPtr<ID3D12Device>				m_d3dDevice;
	ComPtr<IDXGIFactory4>				m_dxgiFactory;

	ComPtr<ID3D12Fence>					m_Fence;
	UINT64								m_CurrentFence;
	UINT								m_RtvDescriptorSize;
	UINT								m_DsvDescriptorSize;
	UINT								m_CbvUavDescriptorSize;

	UINT								m_4xMsaaQuality;

	ComPtr<ID3D12CommandQueue>			m_CommandQueue;
	ComPtr<ID3D12CommandAllocator>		m_DirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList>	m_CommandList;

	int									m_CurrentBackBuffer = 0;
	static constexpr int				m_SwapChainBufferCount = 2;
	ComPtr<IDXGISwapChain>				m_SwapChain;

	ComPtr<ID3D12Resource>				m_SwapChainBuffer[m_SwapChainBufferCount];
	ComPtr<ID3D12Resource>				m_DepthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap>		m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap>		m_DsvHeap;

	D3D12_VIEWPORT						m_ScreenViewport;
	D3D12_RECT							m_ScissorRect;
};

