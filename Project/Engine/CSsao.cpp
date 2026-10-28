#include "pch.h"
#include "CSsao.h"

#include "CDevice.h"
#include "CGameObject.h"
#include "CAssetMgr.h"

CSsao::CSsao()
{

}

CSsao::~CSsao()
{

}

void CSsao::Initialize(UINT width, UINT height)
{
    OnResize(width, height);

    BuildOffsetVectors();
    BuildRandomVectorTexture();
}

void CSsao::OnResize(UINT newWidth, UINT newHeight)
{
    if (m_RenderTargetWidth != newWidth || m_RenderTargetHeight != newHeight)
    {
        m_RenderTargetWidth = newWidth;
        m_RenderTargetHeight = newHeight;

        // We render to ambient map at half the resolution.
        m_Viewport.TopLeftX = 0.0f;
        m_Viewport.TopLeftY = 0.0f;
        m_Viewport.Width = m_RenderTargetWidth / 2.0f;
        m_Viewport.Height = m_RenderTargetHeight / 2.0f;
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;

        m_ScissorRect = { 0, 0, (int)m_RenderTargetWidth / 2, (int)m_RenderTargetHeight / 2 };

        BuildResources();
    }
}

void CSsao::BuildOffsetVectors()
{
	m_Offsets[0] = Vector4(1.f, 1.f, 1.f, 0.f);
	m_Offsets[1] = Vector4(-1.f, -1.f, -1.f, 0.f);

	m_Offsets[2] = Vector4(-1.f, 1.f, 1.f, 0.f);
	m_Offsets[3] = Vector4(1.f, -1.f, -1.f, 0.f);

	m_Offsets[4] = Vector4(1.f, 1.f, -1.f, 0.f);
	m_Offsets[5] = Vector4(-1.f, -1.f, 1.f, 0.f);

	m_Offsets[6] = Vector4(-1.f, 1.f, -1.f, 0.f);
	m_Offsets[7] = Vector4(1.f, -1.f, 1.f, 0.f);

	m_Offsets[8] = Vector4(-1.f, 0.f, 0.f, 0.f);
	m_Offsets[9] = Vector4(1.f, 0.f, 0.f, 0.f);

	m_Offsets[10] = Vector4(0.f, -1.f, 0.f, 0.f);
	m_Offsets[11] = Vector4(0.f, 1.f, 0.f, 0.f);

	m_Offsets[12] = Vector4(0.f, 0.f, -1.f, 0.f);
	m_Offsets[13] = Vector4(0.f, 0.f, 1.f, 0.f);

	for (int i = 0; i < 14; ++i)
	{
		float s = Utilities::RandF(0.25f, 1.f);
		m_Offsets[i].Normalize();
		m_Offsets[i] *= s;
	}
}

void CSsao::BuildResources()
{
	m_NormalMap = nullptr;
	m_AmbientMap0 = nullptr;
	m_AmbientMap1 = nullptr;

    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = m_RenderTargetWidth;
    texDesc.Height = m_RenderTargetHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = NormalMapFormat;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;


    float normalClearColor[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    CD3DX12_CLEAR_VALUE optClear(NormalMapFormat, normalClearColor);
    CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(DEVICE->CreateCommittedResource(
        &defaultHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        &optClear,
        IID_PPV_ARGS(&m_NormalMap)));

    // Ambient occlusion maps are at half resolution.
    texDesc.Width = (UINT)m_Viewport.Width;
    texDesc.Height = (UINT)m_Viewport.Height;
    texDesc.Format = AmbientMapFormat;

    float ambientClearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    optClear = CD3DX12_CLEAR_VALUE(AmbientMapFormat, ambientClearColor);

    ThrowIfFailed(DEVICE->CreateCommittedResource(
        &defaultHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        &optClear,
        IID_PPV_ARGS(&m_AmbientMap0)));

    ThrowIfFailed(DEVICE->CreateCommittedResource(
        &defaultHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        &optClear,
        IID_PPV_ARGS(&m_AmbientMap1)));
}

void CSsao::BuildRandomVectorTexture()
{
    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = 256;
    texDesc.Height = 256;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(DEVICE->CreateCommittedResource(
        &defaultHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_RandomVectorMap)));

    //
    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    //

    const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_RandomVectorMap.Get(), 0, num2DSubresources);

    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC uploadBufferDesc(CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize));
    ThrowIfFailed(DEVICE->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_RandomVectorMapUploadBuffer.GetAddressOf())));

    XMCOLOR* initData = new XMCOLOR[256 * 256];
    for (int i = 0; i < 256; ++i)
    {
        for (int j = 0; j < 256; ++j)
        {
            // Random vector in [0,1].  We will decompress in shader to [-1,1].
            Vector3 v(Utilities::RandF(), Utilities::RandF(), Utilities::RandF());

            initData[i * 256 + j] = XMCOLOR(v.x, v.y, v.z, 0.0f);
        }
    }

    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = 256 * sizeof(XMCOLOR);
    subResourceData.SlicePitch = subResourceData.RowPitch * 256;

    //
    // Schedule to copy the data to the default resource, and change states.
    // Note that mCurrSol is put in the GENERIC_READ state so it can be 
    // read by a shader.
    //

    CDevice::GetInst()->UploadResourceAsync([&](ID3D12GraphicsCommandList* cmdlist) {
        CD3DX12_RESOURCE_BARRIER GR2CDRvBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_RandomVectorMap.Get(),
            D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
        cmdlist->ResourceBarrier(1, &GR2CDRvBarrier);
        UpdateSubresources(cmdlist, m_RandomVectorMap.Get(), m_RandomVectorMapUploadBuffer.Get(),
            0, 0, num2DSubresources, &subResourceData);
        CD3DX12_RESOURCE_BARRIER CD2GRRvBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_RandomVectorMap.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
        cmdlist->ResourceBarrier(1, &CD2GRRvBarrier);
        });
    CDevice::GetInst()->WaitForAllUploads();

    delete[] initData;
}


void CSsao::BuildDescriptors(ComPtr<ID3D12Resource> depthStencilBuffer)
{
    CDescriptorAllocator::Allocation NormalAndDepthSrv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);
    m_NormalMapSrv = CDescriptorAllocator::GetInst()->GetAllocation(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NormalAndDepthSrv.startIndex, 1);
    m_NormalMapRtv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
    m_DepthMapSrv = CDescriptorAllocator::GetInst()->GetAllocation(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NormalAndDepthSrv.startIndex + 1, 1);
    m_RandomVectorMapSrv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
    m_AmbientMap0Srv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
    m_AmbientMap0Rtv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
    m_AmbientMap1Srv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
    m_AmbientMap1Rtv = CDescriptorAllocator::GetInst()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);

    RebuildDescriptors(depthStencilBuffer);
}

void CSsao::RebuildDescriptors(ComPtr<ID3D12Resource> depthStencilBuffer)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = NormalMapFormat;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    DEVICE->CreateShaderResourceView(m_NormalMap.Get(), &srvDesc, m_NormalMapSrv.cpuHandle);

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DEVICE->CreateShaderResourceView(m_RandomVectorMap.Get(), &srvDesc, m_RandomVectorMapSrv.cpuHandle);

    srvDesc.Format = AmbientMapFormat;
    DEVICE->CreateShaderResourceView(m_AmbientMap0.Get(), &srvDesc, m_AmbientMap0Srv.cpuHandle);
    DEVICE->CreateShaderResourceView(m_AmbientMap1.Get(), &srvDesc, m_AmbientMap1Srv.cpuHandle);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;;
    rtvDesc.Format = NormalMapFormat;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    DEVICE->CreateRenderTargetView(m_NormalMap.Get(), &rtvDesc, m_NormalMapRtv.cpuHandle);

    rtvDesc.Format = AmbientMapFormat;
    DEVICE->CreateRenderTargetView(m_AmbientMap0.Get(), &rtvDesc, m_AmbientMap0Rtv.cpuHandle);
    DEVICE->CreateRenderTargetView(m_AmbientMap1.Get(), &rtvDesc, m_AmbientMap1Rtv.cpuHandle);

    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    DEVICE->CreateShaderResourceView(depthStencilBuffer.Get(), &srvDesc, m_DepthMapSrv.cpuHandle);
}

std::vector<float> CSsao::CalcGaussWeights(float sigma)
{
    float twoSigma2 = 2.0f * sigma * sigma;

    // Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
    // For example, for sigma = 3, the width of the bell curve is 
    int blurRadius = (int)ceil(2.0f * sigma);

    assert(blurRadius <= MaxBlurRadius);

    std::vector<float> weights;
    weights.resize(2 * blurRadius + 1);

    float weightSum = 0.0f;

    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        float x = (float)i;

        weights[i + blurRadius] = expf(-x * x / twoSigma2);

        weightSum += weights[i + blurRadius];
    }

    // Divide by the sum so all the weights add up to 1.0.
    for (int i = 0; i < weights.size(); ++i)
    {
        weights[i] /= weightSum;
    }

    return weights;
}

void CSsao::DrawNormalsAndDepth(const std::vector<CGameObject*>& defaultObjs, const std::vector<CGameObject*>& skinnedObjs)
{
    CDevice::GetInst()->SetViewportAndScissorRect();

    // Change to RENDER_TARGET.
    CD3DX12_RESOURCE_BARRIER NmGR2RTBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_NormalMap.Get(),
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    CMDLIST->ResourceBarrier(1, &NmGR2RTBarrier);

    D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle = CDevice::GetInst()->DepthStencilView();

    // Clear the screen normal map and depth buffer.
    float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    CMDLIST->ClearRenderTargetView(m_NormalMapRtv.cpuHandle, clearValue, 0, nullptr);
    CMDLIST->ClearDepthStencilView(dsvCpuHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    CMDLIST->OMSetRenderTargets(1, &m_NormalMapRtv.cpuHandle, true, &dsvCpuHandle);

    // Bind the constant buffer for this pass.
    CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->Bind_Graphics(0, 0);

    CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(PSO_TYPE::PSO_DRAW_NORMALS).Get());

    for (const auto& obj : defaultObjs)
        obj->Render();

    CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(PSO_TYPE::PSO_DRAW_NORMALS_SKINNED).Get());

    for (const auto& obj : skinnedObjs)
        obj->Render();

    // Change back to GENERIC_READ so we can read the texture in a shader.
    CD3DX12_RESOURCE_BARRIER NmRT2GRBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_NormalMap.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
    CMDLIST->ResourceBarrier(1, &NmRT2GRBarrier);
}

void CSsao::ComputeSsao(int blurCount)
{
    CMDLIST->SetGraphicsRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"SSAO").Get());

    CMDLIST->RSSetViewports(1, &m_Viewport);
    CMDLIST->RSSetScissorRects(1, &m_ScissorRect);

    // We compute the initial SSAO to AmbientMap0.

    // Change to RENDER_TARGET.
    CD3DX12_RESOURCE_BARRIER Am0GR2RTBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_AmbientMap0.Get(),
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    CMDLIST->ResourceBarrier(1, &Am0GR2RTBarrier);

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    CMDLIST->ClearRenderTargetView(m_AmbientMap0Rtv.cpuHandle, clearValue, 0, nullptr);

    // Specify the buffers we are going to render to.
    CMDLIST->OMSetRenderTargets(1, &m_AmbientMap0Rtv.cpuHandle, true, nullptr);

    // Bind the constant buffer for this pass.
    CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::SSAO)->Bind_Graphics(0, 0);
    CMDLIST->SetGraphicsRoot32BitConstant(1, 0, 0);

    // Bind the normal and depth maps.
    CMDLIST->SetGraphicsRootDescriptorTable(2, m_NormalMapSrv.gpuHandle);
    CMDLIST->SetGraphicsRootDescriptorTable(4, m_DepthMapSrv.gpuHandle);

    // Bind the random vector map.
    CMDLIST->SetGraphicsRootDescriptorTable(3, m_RandomVectorMapSrv.gpuHandle);

    CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(PSO_TYPE::PSO_SSAO).Get());

    // Draw fullscreen quad.
    CMDLIST->IASetVertexBuffers(0, 0, nullptr);
    CMDLIST->IASetIndexBuffer(nullptr);
    CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    CMDLIST->DrawInstanced(6, 1, 0, 0);

    // Change back to GENERIC_READ so we can read the texture in a shader.
    CD3DX12_RESOURCE_BARRIER Am0RT2GRBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_AmbientMap0.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
    CMDLIST->ResourceBarrier(1, &Am0RT2GRBarrier);

    BlurAmbientMap(blurCount);
}

void CSsao::BlurAmbientMap(int blurCount)
{
    CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(PSO_TYPE::PSO_SSAO_BLUR).Get());

    CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::SSAO)->Bind_Graphics(0, 0);

    for (int i = 0; i < blurCount; ++i)
    {
        BlurAmbientMap(true);
        BlurAmbientMap(false);
    }
}

void CSsao::BlurAmbientMap(bool horzBlur)
{
    ID3D12Resource* output = nullptr;
    CD3DX12_GPU_DESCRIPTOR_HANDLE inputSrv;
    CD3DX12_CPU_DESCRIPTOR_HANDLE outputRtv;

    // Ping-pong the two ambient map textures as we apply
    // horizontal and vertical blur passes.
    if (horzBlur == true)
    {
        output = m_AmbientMap1.Get();
        inputSrv = m_AmbientMap0Srv.gpuHandle;
        outputRtv = m_AmbientMap1Rtv.cpuHandle;
        CMDLIST->SetGraphicsRoot32BitConstant(1, 1, 0);
    }
    else
    {
        output = m_AmbientMap0.Get();
        inputSrv = m_AmbientMap1Srv.gpuHandle;
        outputRtv = m_AmbientMap0Rtv.cpuHandle;
        CMDLIST->SetGraphicsRoot32BitConstant(1, 0, 0);
    }

    CD3DX12_RESOURCE_BARRIER outGR2RTBarrier(CD3DX12_RESOURCE_BARRIER::Transition(output,
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
    CMDLIST->ResourceBarrier(1, &outGR2RTBarrier);

    float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    CMDLIST->ClearRenderTargetView(outputRtv, clearValue, 0, nullptr);

    CMDLIST->OMSetRenderTargets(1, &outputRtv, true, nullptr);

    // Normal/depth map still bound.


    // Bind the normal and depth maps.
    CMDLIST->SetGraphicsRootDescriptorTable(2, m_NormalMapSrv.gpuHandle);

    // Bind the input ambient map to second texture table.
    CMDLIST->SetGraphicsRootDescriptorTable(3, inputSrv);

    // Draw fullscreen quad.
    CMDLIST->IASetVertexBuffers(0, 0, nullptr);
    CMDLIST->IASetIndexBuffer(nullptr);
    CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    CMDLIST->DrawInstanced(6, 1, 0, 0);

    CD3DX12_RESOURCE_BARRIER outRT2GRBarrier(CD3DX12_RESOURCE_BARRIER::Transition(output,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
    CMDLIST->ResourceBarrier(1, &outRT2GRBarrier);
}
