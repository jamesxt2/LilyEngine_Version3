#include "pch.h"
#include "CDevice.h"

#include "CConstantBuffer.h"

CDevice::CDevice()
	: m_MainWnd(nullptr), m_RenderResolution{}, m_Format(DXGI_FORMAT_R8G8B8A8_UNORM),
	m_4xMsaaQuality(0), m_EnableMSAA(true),
	m_ScreenViewport(), m_ScissorRect{},
	m_CurrFrameResource(nullptr), m_CurrFrameResourceIndex(0)
{

}

CDevice::~CDevice()
{
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}
}

int CDevice::Init(HWND _MainWnd, POINT _RenderResolution)
{
	m_MainWnd = _MainWnd;
	m_RenderResolution = _RenderResolution;

	/*************************************/
	// Create Device
	/*************************************/

#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

	HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice));
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice)));
	}

	/*************************************/
	// Detect 4X MSAA
	/*************************************/
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS m_sQualityLevels;
	m_sQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_sQualityLevels.SampleCount = 4;
	m_sQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	m_sQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &m_sQualityLevels, sizeof(m_sQualityLevels)));

	m_4xMsaaQuality = m_sQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	/*************************************/
	// Create Command Objects And Fence
	/*************************************/
	CreateCommandObjectsAndFence();

	/*************************************/
	// Create Swap Chain
	/*************************************/
	CreateSwapChain();

	/*************************************/
	// Create RTV and DSV descriptor heaps
	/*************************************/
	CreateRtvAndDsvDescriptorHeaps();

	OnResize(m_RenderResolution);

	/*************************************/
	// Frame Resources
	/*************************************/
	BuildFrameResources();

	return S_OK;
}

void CDevice::CreateCommandObjectsAndFence()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

	m_UploadContexts.resize(kUploadContextCount);
	for (auto& ctx : m_UploadContexts)
	{
		ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&ctx.alloc)));
		ThrowIfFailed(m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ctx.alloc.Get(), nullptr, IID_PPV_ARGS(&ctx.list)));

		ThrowIfFailed(ctx.list->Close());
		ctx.fenceValue = 0;
		ctx.bInUse = false;
	}

	ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
	m_FenceEvent = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
	m_FenceValue = 0;
}

void CDevice::CreateSwapChain()
{
	m_SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC scDesc = {};
	scDesc.BufferDesc.Width = m_RenderResolution.x;
	scDesc.BufferDesc.Height = m_RenderResolution.y;
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = m_SwapChainBufferCount;
	scDesc.OutputWindow = m_MainWnd;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	// Swap chain uses queue to perform flush
	ThrowIfFailed(m_dxgiFactory->CreateSwapChain(m_CommandQueue.Get(), &scDesc, m_SwapChain.GetAddressOf()));
}

void CDevice::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = m_SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

	if (m_EnableMSAA)
	{
		D3D12_DESCRIPTOR_HEAP_DESC msaRrtvHeapDesc;
		msaRrtvHeapDesc.NumDescriptors = 1;
		msaRrtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		msaRrtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		msaRrtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&msaRrtvHeapDesc, IID_PPV_ARGS(&m_MsaaRtvHeap)));
	}

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DsvHeap)));
}

ID3D12Resource* CDevice::CurrentBackBuffer() const
{
	return m_SwapChainBuffer[m_CurrentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE CDevice::CurrentBackBufferView() const
{
	if (m_EnableMSAA)
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_MsaaRtvHeap->GetCPUDescriptorHandleForHeapStart());
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrentBackBuffer,
		m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
}

D3D12_CPU_DESCRIPTOR_HANDLE CDevice::DepthStencilView() const
{
	return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void CDevice::FlushCommandQueue()
{
	++m_FenceValue;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue));

	if (m_Fence->GetCompletedValue() < m_FenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValue, eventHandle));

		assert(eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void CDevice::BuildFrameResources()
{
	for (int i = 0; i < g_NumFrameResources; ++i)
	{
		m_FrameResources.push_back(std::make_unique<FrameResource>(m_d3dDevice.Get()));
		m_FrameResources[i]->CreateCB(sizeof(TObject), g_MaxObjectCount, CB_TYPE::OBJECT);
		m_FrameResources[i]->CreateCB(sizeof(TMaterial), g_MaxMaterialCount, CB_TYPE::MATERIAL);
		m_FrameResources[i]->CreateCB(sizeof(TGlobal), 2, CB_TYPE::GLOBAL);
	}
	m_CurrFrameResource = m_FrameResources[0].get();
}

void CDevice::OnResize(POINT newRenderResolution)
{
	if (!m_d3dDevice) return;
	assert(m_SwapChain);

	m_RenderResolution = newRenderResolution;

	/*************************************************/
	// Reset all command objects and swap chain
	/*************************************************/

	FlushCommandQueue();

	//ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	for (UINT i = 0; i < m_SwapChainBufferCount; ++i)
		m_SwapChainBuffer[i].Reset();
	m_DepthStencilBuffer.Reset();

	ThrowIfFailed(m_SwapChain->ResizeBuffers(m_SwapChainBufferCount, 
		m_RenderResolution.x, m_RenderResolution.y, 
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_CurrentBackBuffer = 0;

	/*************************************************/
	// Create RTV for every buffer in the swap chain
	/*************************************************/
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < m_SwapChainBufferCount; ++i)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
		m_d3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	}

	/*************************************************/
	// Create MSAA Render Target
	/*************************************************/
	if (m_EnableMSAA)
	{
		m_MsaaRenderTarget.Reset();

		D3D12_RESOURCE_DESC msaaDesc = {};
		msaaDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		msaaDesc.Alignment = 0;
		msaaDesc.Width = m_RenderResolution.x;
		msaaDesc.Height = m_RenderResolution.y;
		msaaDesc.DepthOrArraySize = 1;
		msaaDesc.MipLevels = 1;
		msaaDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaDesc.SampleDesc.Count = m_EnableMSAA ? 4 : 1;
		msaaDesc.SampleDesc.Quality = m_EnableMSAA ? (m_4xMsaaQuality - 1) : 0;
		msaaDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		msaaDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE msaaClear = {};
		msaaClear.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaClear.Color[0] = m_ClearColor[0];
		msaaClear.Color[1] = m_ClearColor[1];
		msaaClear.Color[2] = m_ClearColor[2];
		msaaClear.Color[3] = m_ClearColor[3];

		CD3DX12_HEAP_PROPERTIES msaaHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
			&msaaHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&msaaDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&msaaClear,
			IID_PPV_ARGS(&m_MsaaRenderTarget)
		));

		D3D12_RENDER_TARGET_VIEW_DESC msaaRtvDesc = {};
		msaaRtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaRtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
		//msaaRtvDesc.Texture2DMS.UnusedField_NothingToDefine = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE msaaRtvHandle(m_MsaaRtvHeap->GetCPUDescriptorHandleForHeapStart());
		m_d3dDevice->CreateRenderTargetView(m_MsaaRenderTarget.Get(), &msaaRtvDesc, msaaRtvHandle);
	}

	/*************************************************/
	// Create Depth/Stencil buffer and view
	/*************************************************/
	D3D12_RESOURCE_DESC dsDesc = {};
	dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsDesc.Alignment = 0;
	dsDesc.Width = m_RenderResolution.x;
	dsDesc.Height = m_RenderResolution.y;
	dsDesc.DepthOrArraySize = 1;
	dsDesc.MipLevels = 1;
	dsDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	dsDesc.SampleDesc.Count = m_EnableMSAA ? 4 : 1;
	dsDesc.SampleDesc.Quality = m_EnableMSAA ? (m_4xMsaaQuality - 1) : 0;
	dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optColor;
	optColor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optColor.DepthStencil.Depth = 1.f;
	optColor.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&dsDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optColor,
		IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())
	));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = m_EnableMSAA ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	m_d3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// trnsition the resource from its initial state to be used as depth buffer
	UploadResourceAsync([&](ID3D12GraphicsCommandList* cmdList) {
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		);
		cmdList->ResourceBarrier(1, &barrier);
		});

	WaitForAllUploads();

	/*************************************************/
	// Update the viewport transform
	/*************************************************/
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_RenderResolution.x);
	m_ScreenViewport.Height = static_cast<float>(m_RenderResolution.y);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, m_RenderResolution.x, m_RenderResolution.y };

	OnWindowResize.Broadcast(m_RenderResolution.x, m_RenderResolution.y);
}

void CDevice::Update()
{
	m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % g_NumFrameResources;
	m_CurrFrameResource = m_FrameResources[m_CurrFrameResourceIndex].get();

	if (m_CurrFrameResource->Fence != 0 && m_Fence->GetCompletedValue() < m_CurrFrameResource->Fence)
	{
		ThrowIfFailed(m_Fence->SetEventOnCompletion(
			m_CurrFrameResource->Fence, m_FenceEvent
		));
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	WaitForAllUploads();

	ThrowIfFailed(m_CurrFrameResource->m_CmdAlloc->Reset());
	ThrowIfFailed(m_CurrFrameResource->m_CmdList->Reset(m_CurrFrameResource->m_CmdAlloc.Get(), nullptr));
}

void CDevice::ClearTargetAndPrepareRender(XMVECTORF32 color)
{
	// Reuse the memory associated with command recording.
   // We can only reset when the associated command lists have finished execution on the GPU.
	//ThrowIfFailed(m_CurrFrameResource->m_CmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	//ThrowIfFailed(m_CommandList->Reset(m_CurrFrameResource->m_CmdListAlloc.Get(), nullptr));

	m_CurrFrameResource->m_CmdList->RSSetViewports(1, &m_ScreenViewport);
	m_CurrFrameResource->m_CmdList->RSSetScissorRects(1, &m_ScissorRect);

	// Indicate a state transition on the resource usage.
	CD3DX12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &barrier);

	// Clear the back buffer and depth buffer.
	m_CurrFrameResource->m_CmdList->ClearRenderTargetView(CurrentBackBufferView(), m_ClearColor, 0, nullptr);
	m_CurrFrameResource->m_CmdList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE CurrBBV = CurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DSV = DepthStencilView();
	m_CurrFrameResource->m_CmdList->OMSetRenderTargets(1, &CurrBBV, true, &DSV);

}

void CDevice::ExecuteAndFinishDrawCall()
{
	if (m_EnableMSAA)
	{
		// Back Buffer: RenderTarget -> ResolveDest
		CD3DX12_RESOURCE_BARRIER toResolveDestBarrier(
			CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_DEST));
		m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &toResolveDestBarrier);

		// MSAA texture: RenderTarget -> ResolveSource
		CD3DX12_RESOURCE_BARRIER toResolveSrcBarrier(
			CD3DX12_RESOURCE_BARRIER::Transition(m_MsaaRenderTarget.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));
		m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &toResolveSrcBarrier);

		// MSAA -> Back Buffer
		m_CurrFrameResource->m_CmdList->ResolveSubresource(
			CurrentBackBuffer(), 0, m_MsaaRenderTarget.Get(),
			0, DXGI_FORMAT_R8G8B8A8_UNORM
		);

		// MSAA texture: ResolveSource -> RenderTarget
		CD3DX12_RESOURCE_BARRIER toRenderTargetBarrier(
			CD3DX12_RESOURCE_BARRIER::Transition(m_MsaaRenderTarget.Get(),
				D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &toRenderTargetBarrier);

		// Back Buffer: ResolveDest -> Render Target
		CD3DX12_RESOURCE_BARRIER toRTBarrier(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &toRTBarrier);
	}

	//CAssetMgr::GetInst()->ExecuteBlur(CurrentBackBuffer(), 4);

	// Back Buffer: RenderTarget -> Present
	CD3DX12_RESOURCE_BARRIER toPresentBarrier(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &toPresentBarrier);

	// Done recording commands.
	ThrowIfFailed(m_CurrFrameResource->m_CmdList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_CurrFrameResource->m_CmdList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrentBackBuffer = (m_CurrentBackBuffer + 1) % m_SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	//FlushCommandQueue();

	// Advance the fence value to mark commands up to this fence point.
	m_CurrFrameResource->Fence = ++m_FenceValue;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue);
}


ComPtr<ID3D12Resource> CDevice::CreateDefaultBuffer(const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer, ID3D12GraphicsCommandList* cmdlist)
{
	ComPtr<ID3D12Resource> defaultBuffer;

	// Create actual default buffer resource
	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc(CD3DX12_RESOURCE_DESC::Buffer(byteSize));
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())
	));

	// To copy the data from CPU memory to default buffer, we need to create an intermediate upload buffer
	heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())
	));

	// describe the data we want to copy to the default buffer
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = byteSize;

	// copy

	assert(cmdlist);

	CD3DX12_RESOURCE_BARRIER barrier1(CD3DX12_RESOURCE_BARRIER::Transition(
		defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	));
	cmdlist->ResourceBarrier(1, &barrier1);
	UpdateSubresources<1>(
		cmdlist, defaultBuffer.Get(), uploadBuffer.Get(),
		0, 0, 1, &subResourceData
	);
	CD3DX12_RESOURCE_BARRIER barrier2(CD3DX12_RESOURCE_BARRIER::Transition(
		defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ
	));
	cmdlist->ResourceBarrier(1, &barrier2);

	return defaultBuffer;
}