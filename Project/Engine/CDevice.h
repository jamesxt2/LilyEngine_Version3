#pragma once
#include "singleton.h"

class CConstantBuffer;

class CDevice : public CSingleton<CDevice>
{
	SINGLE(CDevice)

public:
	int Init(HWND _MainWnd, POINT _RenderResolution);

	void OnResize(POINT newRenderResolution);
	MulticastDelegate<UINT, UINT> OnWindowResize;

	void Update();

	void ClearTargetAndPrepareRender(XMVECTORF32 color);
	void ExecuteAndFinishDrawCall();

	void FlushCommandQueue();

private:

	void CreateCommandObjectsAndFence();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	void BuildFrameResources();


	HWND											m_MainWnd;
	POINT											m_RenderResolution;
	DXGI_FORMAT										m_Format;

	ComPtr<ID3D12Device>							m_d3dDevice;
	ComPtr<IDXGIFactory4>							m_dxgiFactory;

	UINT											m_4xMsaaQuality;

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

	const float m_ClearColor[4] = { 0.69f, 0.77f, 0.87f, 1.0f };

public:
	inline ComPtr<ID3D12Device> GetDevice() const { return m_d3dDevice; }

	inline std::shared_ptr<CConstantBuffer> GetConstBuffer(CB_TYPE type)
	{
		return m_CurrFrameResource->GetConstantBuffer(type);
	}

	inline DXGI_FORMAT GetFormat() const { return m_Format; }

	inline Vector2 GetRenderResolution() const { return Vector2((float)m_RenderResolution.x, (float)m_RenderResolution.y); }
	inline float GetAspectRatio() const { return (float)m_RenderResolution.x / (float)m_RenderResolution.y; }

	inline FrameResource* GetCurrFrameResource() const { return m_CurrFrameResource; }

	inline bool EnableMSAA() const { return m_EnableMSAA; }
	inline UINT Get4xMSAAQuality() const { return m_4xMsaaQuality; }


	// Help function
	ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer, ID3D12GraphicsCommandList* cmdlist = nullptr);

/***********************Command Objects And Fence******************************/
public:

	ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return m_CommandQueue; }

	template<typename F>
	void UploadResourceAsync(F&& recordFunc)
	{
		UploadContext& ctx = AcquireUploadContext();

		ThrowIfFailed(ctx.alloc->Reset());
		ThrowIfFailed(ctx.list->Reset(ctx.alloc.Get(), nullptr));

		recordFunc(ctx.list.Get());

		ThrowIfFailed(ctx.list->Close());
		ID3D12CommandList* lists[] = { ctx.list.Get() };
		m_CommandQueue->ExecuteCommandLists(1, lists);

		ctx.fenceValue = ++m_FenceValue;
		m_CommandQueue->Signal(m_Fence.Get(), ctx.fenceValue);
		ctx.bInUse = true;

		m_PendingUploadFenceValues.push_back(ctx.fenceValue);
	}

	void WaitForAllUploads()
	{
		if (m_PendingUploadFenceValues.empty()) return;

		uint64 latestFence = m_PendingUploadFenceValues.back();
		if (m_Fence->GetCompletedValue() < latestFence)
		{
			ThrowIfFailed(m_Fence->SetEventOnCompletion(latestFence, m_FenceEvent));
			WaitForSingleObject(m_FenceEvent, INFINITE);
		}
		m_PendingUploadFenceValues.clear();
	}

private:

	struct UploadContext
	{
		ComPtr<ID3D12CommandAllocator> alloc;
		ComPtr<ID3D12GraphicsCommandList> list;
		uint64 fenceValue = 0;
		bool bInUse = false;
	};

	UploadContext& AcquireUploadContext()
	{
		for (auto& ctx : m_UploadContexts)
		{
			if (!ctx.bInUse) return ctx;

			if (m_Fence->GetCompletedValue() >= ctx.fenceValue)
			{
				ctx.bInUse = false;
				return ctx;
			}
		}

		UploadContext& oldest = *std::min_element(
			m_UploadContexts.begin(), m_UploadContexts.end(),
			[](const UploadContext& a, const UploadContext& b) {
				return a.fenceValue < b.fenceValue;
			});

		ThrowIfFailed(m_Fence->SetEventOnCompletion(oldest.fenceValue, m_FenceEvent));
		WaitForSingleObject(m_FenceEvent, INFINITE);
		oldest.bInUse = false;
		return oldest;
	}
	
	static constexpr UINT kUploadContextCount = 4;

	ComPtr<ID3D12CommandQueue>						m_CommandQueue;

	std::vector<UploadContext> m_UploadContexts;
	ComPtr<ID3D12Fence> m_Fence;
	uint64 m_FenceValue = 0;
	HANDLE m_FenceEvent = nullptr;
	std::vector<uint64> m_PendingUploadFenceValues;

/***********************Command Objects And Fence******************************/

};

