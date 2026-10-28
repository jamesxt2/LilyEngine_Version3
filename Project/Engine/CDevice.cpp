#include "pch.h"
#include "CDevice.h"

#include "CConstantBuffer.h"
#include "CSsao.h"
#include "CAssetMgr.h"

CDevice::CDevice()
	: m_MainWnd(nullptr), m_RenderResolution{}, m_Format(DXGI_FORMAT_R8G8B8A8_UNORM),
	m_ScreenViewport(), m_ScissorRect{},
	m_CurrFrameResource(nullptr), m_CurrFrameResourceIndex(0)
{

}

CDevice::~CDevice()
{

}

void CDevice::Release()
{
	FlushCommandQueue();
	m_FrameResources.clear();
	m_DsvHeap.Reset();
	m_RtvHeap.Reset();
	m_CbvSrvUavDescriptorHeap.Reset();
	m_DepthStencilBuffer.Reset();
	for (int i = 0; i < m_SwapChainBufferCount; ++i)
		m_SwapChainBuffer[i].Reset();
	m_SwapChain.Reset();
	m_dxgiFactory.Reset();
	m_UploadContexts.clear();
	m_Fence.Reset();
	m_CommandQueue.Reset();
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}
	m_SceneColor.Reset();
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
	BuildDescriptorHeaps();

	BuildSceneColorResources();

	/*************************************/
	// SSAO
	/*************************************/
	CSsao::GetInst()->Initialize(m_RenderResolution.x, m_RenderResolution.y);


	OnResize(m_RenderResolution);

	
	return S_OK;
}

void CDevice::PostInit()
{
	/*************************************/
	// Frame Resources
	/*************************************/
	BuildFrameResources();
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

void CDevice::BuildDescriptorHeaps()
{
	const UINT maxCbvSrvUavDescriptorCount = 16384;
	const UINT maxRtvDescriptorCount = m_SwapChainBufferCount + 4;
	const UINT maxDsvDescriptorCount = 2;

	D3D12_DESCRIPTOR_HEAP_DESC CbvSrvUavHeapDesc = {};
	CbvSrvUavHeapDesc.NumDescriptors = maxCbvSrvUavDescriptorCount;
	CbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(DEVICE->CreateDescriptorHeap(
		&CbvSrvUavHeapDesc, IID_PPV_ARGS(&m_CbvSrvUavDescriptorHeap)));

	CDescriptorAllocator::GetInst()->InitHeapData(m_CbvSrvUavDescriptorHeap, maxCbvSrvUavDescriptorCount);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = maxRtvDescriptorCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

	CDescriptorAllocator::GetInst()->InitHeapData(m_RtvHeap, maxRtvDescriptorCount);
	CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_SwapChainBufferCount);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = maxDsvDescriptorCount;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DsvHeap)));

	CDescriptorAllocator::GetInst()->InitHeapData(m_DsvHeap, maxDsvDescriptorCount);
	CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	CSsao::GetInst()->BuildDescriptors(m_DepthStencilBuffer);

	m_SceneColorRtv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
	m_SceneColorSrv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
}

void CDevice::BuildSceneColorResources()
{
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = (UINT64)m_RenderResolution.x;
	texDesc.Height = (UINT)m_RenderResolution.y;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	texDesc.SampleDesc.Count = 1;                
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; 

	D3D12_CLEAR_VALUE optClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, m_ClearColor);

	CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&optClear,
		IID_PPV_ARGS(&m_SceneColor)));

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	DEVICE->CreateRenderTargetView(m_SceneColor.Get(), &rtvDesc, m_SceneColorRtv.cpuHandle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	DEVICE->CreateShaderResourceView(m_SceneColor.Get(), &srvDesc, m_SceneColorSrv.cpuHandle);
}

ID3D12Resource* CDevice::CurrentBackBuffer() const
{
	return m_SwapChainBuffer[m_CurrentBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE CDevice::CurrentBackBufferView() const
{
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
		m_FrameResources[i]->CreateCB(sizeof(TGlobal), 2, CB_TYPE::GLOBAL);
		m_FrameResources[i]->CreateSB(sizeof(TMaterial), g_MaxMaterialCount, SB_TYPE::MATERIAL, STRUCTURED_BUFFER_TYPE::TABLE);
		m_FrameResources[i]->CreateCB(sizeof(TSSAO), 1, CB_TYPE::SSAO);
		m_FrameResources[i]->CreateCB(sizeof(TSkinned), 1, CB_TYPE::SKINNED);
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
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
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
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
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
	// Scene Color
	/*************************************************/
	BuildSceneColorResources();

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

	/*************************************************/
	// Update SSAO
	/*************************************************/
	CSsao::GetInst()->OnResize(m_RenderResolution.x, m_RenderResolution.y);
	CSsao::GetInst()->RebuildDescriptors(m_DepthStencilBuffer);
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

void CDevice::SetViewportAndScissorRect()
{
	m_CurrFrameResource->m_CmdList->RSSetViewports(1, &m_ScreenViewport);
	m_CurrFrameResource->m_CmdList->RSSetScissorRects(1, &m_ScissorRect);
}

void CDevice::ClearTargetAndPrepareRender()
{
	SetViewportAndScissorRect();

	// Indicate a state transition on the resource usage.
	CD3DX12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(m_SceneColor.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &barrier);

	// Clear the back buffer and depth buffer.
	m_CurrFrameResource->m_CmdList->ClearRenderTargetView(m_SceneColorRtv.cpuHandle, m_ClearColor, 0, nullptr);
	m_CurrFrameResource->m_CmdList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE SceneRtv = m_SceneColorRtv.cpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE DSV = DepthStencilView();
	m_CurrFrameResource->m_CmdList->OMSetRenderTargets(1, &SceneRtv, true, &DSV);

}

void CDevice::ExecuteAndFinishDrawCall()
{
	CD3DX12_RESOURCE_BARRIER scRT2PSRBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_SceneColor.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &scRT2PSRBarrier);

	
	CD3DX12_RESOURCE_BARRIER bbP2RTBarrier(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &bbP2RTBarrier);

	m_CurrFrameResource->m_CmdList->SetGraphicsRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"fxaa").Get());
	m_CurrFrameResource->m_CmdList->SetPipelineState(CAssetMgr::GetInst()->GetPSO(PSO_TYPE::PSO_FXAA).Get());


	D3D12_CPU_DESCRIPTOR_HANDLE CurrBBV = CurrentBackBufferView();
	m_CurrFrameResource->m_CmdList->OMSetRenderTargets(1, &CurrBBV, false, nullptr);

	m_CurrFrameResource->m_CmdList->SetGraphicsRootDescriptorTable(0, m_SceneColorSrv.gpuHandle);
	float invScreenSize[2] = { 1.f / m_RenderResolution.x, 1.f / m_RenderResolution.y };
	m_CurrFrameResource->m_CmdList->SetGraphicsRoot32BitConstants(1, 2, invScreenSize, 0);

	m_CurrFrameResource->m_CmdList->IASetVertexBuffers(0, 0, nullptr);
	m_CurrFrameResource->m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CurrFrameResource->m_CmdList->DrawInstanced(3, 1, 0, 0);

	CD3DX12_RESOURCE_BARRIER bbRT2PBarrier(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_CurrFrameResource->m_CmdList->ResourceBarrier(1, &bbRT2PBarrier);

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

