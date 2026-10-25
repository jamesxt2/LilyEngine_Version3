#include "pch.h"
#include "CShadowMap.h"

#include "CDevice.h"
#include "CGameObject.h"
#include "CAssetMgr.h"
#include "CTexturePool.h"

CShadowMap::CShadowMap()
{
}

CShadowMap::~CShadowMap()
{

}

void CShadowMap::Initialize(UINT width, UINT height)
{
	m_Width = width;
	m_Height = height;
	m_Viewport = { 0.f, 0.f ,(float)width, (float)height, 0.f, 1.f };
	m_ScissorRect = { 0, 0, (int)width, (int)height };
	BuildResource();

	m_AllocSrv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	m_NullSrv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	m_AllocDsv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
	BuildDescriptors();
}

void CShadowMap::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_Width;
	texDesc.Height = m_Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = m_Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.f;
	optClear.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(DEVICE->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
		&texDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &optClear,
		IID_PPV_ARGS(&m_ShadowMap)));
}

void CShadowMap::BuildDescriptors()
{
	// Create SRV to resource so we can sample the shadow map in a shader program.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	DEVICE->CreateShaderResourceView(m_ShadowMap.Get(), &srvDesc, m_AllocSrv.cpuHandle);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	DEVICE->CreateDepthStencilView(m_ShadowMap.Get(), &dsvDesc, m_AllocDsv.cpuHandle);
}

void CShadowMap::DrawSceneToShadowMap(const std::vector<CGameObject*>& objs)
{
	CMDLIST->SetGraphicsRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"Default").Get());

	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->Bind_Graphics(1, 0);
	CDevice::GetInst()->GetCurrFrameResource()->GetStructuredBuffer(SB_TYPE::MATERIAL)->Bind_Graphics_SRV_Table(CMDLIST.Get(), 2);
	CTexturePool::GetInst()->Bind_Graphics_Table_2D(CMDLIST.Get(), 3);
	CTexturePool::GetInst()->Bind_Graphics_Table_Array(CMDLIST.Get(), 4);
	CTexturePool::GetInst()->Bind_Graphics_Table_Cube(CMDLIST.Get(), 6);


	// Bind null SRV for shadow map pass.
	CMDLIST->SetGraphicsRootDescriptorTable(7, m_NullSrv.gpuHandle);

	CMDLIST->RSSetViewports(1, &m_Viewport);
	CMDLIST->RSSetScissorRects(1, &m_ScissorRect);

	// Change to DEPTH_WRITE.
	CD3DX12_RESOURCE_BARRIER SmGR2DWbarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	CMDLIST->ResourceBarrier(1, &SmGR2DWbarrier);

	UINT passCBByteSize = Utilities::CalcConstantBufferByteSize(sizeof(TGlobal));

	// Clear the back buffer and depth buffer.
	CMDLIST->ClearDepthStencilView(m_AllocDsv.cpuHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Set null render target because we are only going to draw to
	// depth buffer.  Setting a null render target will disable color writes.
	// Note the active PSO also must specify a render target count of 0.
	CMDLIST->OMSetRenderTargets(0, nullptr, false, &m_AllocDsv.cpuHandle);

	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(PSO_TYPE::PSO_SHADOW_MAP).Get());
	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (const auto& obj : objs)
		obj->Render();

	// Change back to GENERIC_READ so we can read the texture in a shader.
	CD3DX12_RESOURCE_BARRIER SmDW2GRbarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap.Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	CMDLIST->ResourceBarrier(1, &SmDW2GRbarrier);
}