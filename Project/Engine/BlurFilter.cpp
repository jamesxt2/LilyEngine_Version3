#include "pch.h"
#include "BlurFilter.h"

#include "CDevice.h"
#include "CAssetMgr.h"

BlurFilter::BlurFilter()
{
	m_Width = (UINT)CDevice::GetInst()->GetRenderResolution().x;
	m_Height = (UINT)CDevice::GetInst()->GetRenderResolution().y;
	m_Format = CDevice::GetInst()->GetFormat();

	BuildResource();

	CDevice::GetInst()->OnWindowResize.AddDynamic(this, &BlurFilter::OnResize);
}

void BlurFilter::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor, UINT descriptorSize)
{
	// Save references to the descriptors. 
	m_Blur0CpuSrv = hCpuDescriptor;
	m_Blur0CpuUav = hCpuDescriptor.Offset(1, descriptorSize);
	m_Blur1CpuSrv = hCpuDescriptor.Offset(1, descriptorSize);
	m_Blur1CpuUav = hCpuDescriptor.Offset(1, descriptorSize);

	m_Blur0GpuSrv = hGpuDescriptor;
	m_Blur0GpuUav = hGpuDescriptor.Offset(1, descriptorSize);
	m_Blur1GpuSrv = hGpuDescriptor.Offset(1, descriptorSize);
	m_Blur1GpuUav = hGpuDescriptor.Offset(1, descriptorSize);

	BuildDescriptors();
}

void BlurFilter::OnResize(UINT newWidth, UINT newHeight)
{
	if ((m_Width != newWidth) || (m_Height != newHeight))
	{
		m_Width = newWidth;
		m_Height = newHeight;

		BuildResource();

		// New resource, so we need new descriptors to that resource.
		BuildDescriptors();
	}
}

void BlurFilter::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	uavDesc.Format = m_Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	DEVICE->CreateShaderResourceView(m_BlurMap0.Get(), &srvDesc, m_Blur0CpuSrv);
	DEVICE->CreateUnorderedAccessView(m_BlurMap0.Get(), nullptr, &uavDesc, m_Blur0CpuUav);

	DEVICE->CreateShaderResourceView(m_BlurMap1.Get(), &srvDesc, m_Blur1CpuSrv);
	DEVICE->CreateUnorderedAccessView(m_BlurMap1.Get(), nullptr, &uavDesc, m_Blur1CpuUav);
}

void BlurFilter::BuildResource()
{
	// Note, compressed formats cannot be used for UAV.  We get error like:
	// ERROR: ID3D11Device::CreateTexture2D: The format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView.  Therefore this format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

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
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_BlurMap0)));

	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_BlurMap1)));
}

void BlurFilter::Execute(ID3D12Resource* input, int blurCount)
{
	auto weights = CalcGaussWeights(2.5f);
	int blurRadius = (int)weights.size() / 2;

	CMDLIST->SetComputeRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"PostProcess").Get());

	Vector2 resolution = CDevice::GetInst()->GetRenderResolution();
	CMDLIST->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	CMDLIST->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);
	CMDLIST->SetComputeRoot32BitConstants(0, 1, &resolution.x, (UINT)weights.size() + 1);
	CMDLIST->SetComputeRoot32BitConstants(0, 1, &resolution.y, (UINT)weights.size() + 2);

	CD3DX12_RESOURCE_BARRIER RT2CSBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
		input, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
	CMDLIST->ResourceBarrier(1, &RT2CSBarrier);

	CD3DX12_RESOURCE_BARRIER C2CDBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
		m_BlurMap0.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	CMDLIST->ResourceBarrier(1, &C2CDBarrier);

	CMDLIST->CopyResource(m_BlurMap0.Get(), input);

	CD3DX12_RESOURCE_BARRIER CD2GRBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
		m_BlurMap0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	CMDLIST->ResourceBarrier(1, &CD2GRBarrier);

	CD3DX12_RESOURCE_BARRIER C2UABarrier(CD3DX12_RESOURCE_BARRIER::Transition(
		m_BlurMap1.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	CMDLIST->ResourceBarrier(1, &C2UABarrier);

	for (int i = 0; i < blurCount; ++i)
	{
		// Horizontal
		CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_HORIZONTAL_BLUR).Get());
		CMDLIST->SetComputeRootDescriptorTable(1, m_Blur0GpuSrv);
		CMDLIST->SetComputeRootDescriptorTable(2, m_Blur1GpuUav);

		UINT numGroupsX = (UINT)ceilf(m_Width / 256.f);
		CMDLIST->Dispatch(numGroupsX, m_Height, 1);

		CD3DX12_RESOURCE_BARRIER GR2UABarrier0(CD3DX12_RESOURCE_BARRIER::Transition(
			m_BlurMap0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		CMDLIST->ResourceBarrier(1, &GR2UABarrier0);

		CD3DX12_RESOURCE_BARRIER UA2GRBarrier1(CD3DX12_RESOURCE_BARRIER::Transition(
			m_BlurMap1.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
		CMDLIST->ResourceBarrier(1, &UA2GRBarrier1);

		// Vertical
		CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_VERTICAL_BLUR).Get());
		CMDLIST->SetComputeRootDescriptorTable(1, m_Blur1GpuSrv);
		CMDLIST->SetComputeRootDescriptorTable(2, m_Blur0GpuUav);

		UINT numGroupsY = (UINT)ceilf(m_Height / 256.f);
		CMDLIST->Dispatch(m_Width, numGroupsY, 1);

		CD3DX12_RESOURCE_BARRIER UA2GRBarrier0(CD3DX12_RESOURCE_BARRIER::Transition(
			m_BlurMap0.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
		CMDLIST->ResourceBarrier(1, &UA2GRBarrier0);

		CD3DX12_RESOURCE_BARRIER GR2UABarrier1(CD3DX12_RESOURCE_BARRIER::Transition(
			m_BlurMap1.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		CMDLIST->ResourceBarrier(1, &GR2UABarrier1);
	}

	CD3DX12_RESOURCE_BARRIER CS2CDBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
		input, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	CMDLIST->ResourceBarrier(1, &CS2CDBarrier);

	CD3DX12_RESOURCE_BARRIER GR2CSBarrier0(CD3DX12_RESOURCE_BARRIER::Transition(
		m_BlurMap0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE));
	CMDLIST->ResourceBarrier(1, &GR2CSBarrier0);

	CMDLIST->CopyResource(input, m_BlurMap0.Get());

	CD3DX12_RESOURCE_BARRIER CD2RTBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
		input, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CMDLIST->ResourceBarrier(1, &CD2RTBarrier);

	CD3DX12_RESOURCE_BARRIER CS2CBarrier0(CD3DX12_RESOURCE_BARRIER::Transition(
		m_BlurMap0.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
	CMDLIST->ResourceBarrier(1, &CS2CBarrier0);
	
	CD3DX12_RESOURCE_BARRIER UA2CBarrier1(CD3DX12_RESOURCE_BARRIER::Transition(
		m_BlurMap1.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON));
	CMDLIST->ResourceBarrier(1, &UA2CBarrier1);
}

std::vector<float> BlurFilter::CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.f * sigma * sigma;

	int blurRadius = (int)ceil(2.f * sigma);
	assert(blurRadius <= MaxBlurRadius);

	std::vector<float> weights(2 * blurRadius + 1);
	float weightSum = 0.f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;
		weights[i + blurRadius] = expf(-x * x / twoSigma2);
		weightSum += weights[i + blurRadius];
	}
	for (size_t i = 0; i < weights.size(); ++i)
		weights[i] /= weightSum;

	return weights;
}