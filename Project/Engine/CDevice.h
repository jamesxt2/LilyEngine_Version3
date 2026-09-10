#pragma once
#include "singleton.h"

class CDevice : public CSingleton<CDevice>
{
	SINGLE(CDevice)

public:
	int Init(HWND _MainWnd, POINT _RenderResolution);

private:

	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	HWND								m_MainWnd;
	POINT								m_RenderResolution;

	ComPtr<ID3D12Device>				m_d3dDevice;
	ComPtr<IDXGIFactory4>				m_dxgiFactory;

	ComPtr<ID3D12Fence>					m_Fence;
	UINT								m_RtvDescriptorSize;
	UINT								m_DsvDescriptorSize;
	UINT								m_CbvUavDescriptorSize;

	UINT								m_4xMsaaQuality;

	ComPtr<ID3D12CommandQueue>			m_CommandQueue;
	ComPtr<ID3D12CommandAllocator>		m_DirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList>	m_CommandList;

	int									m_CurrentBufferCount = 0;
	static const int					m_SwapChainBufferCount = 2;
	ComPtr<IDXGISwapChain>				m_SwapChain;

	ComPtr<ID3D12DescriptorHeap>		m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap>		m_DsvHeap;
};

