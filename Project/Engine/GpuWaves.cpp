#include "pch.h"
#include "GpuWaves.h"

#include "CDevice.h"
#include "CTimeMgr.h"
#include "CAssetMgr.h"
#include "CDescriptorAllocator.h"

GpuWaves::GpuWaves(int m, int n, float dx, float dt, float speed, float damping)
	: m_NumRows(m), m_NumCols(n), m_VertexCount(m * n), 
	m_TriangleCount((m - 1)* (n - 1) * 2), m_TimeStep(dt),
	m_SpatialStep(dx), m_IsResourceUploaded(false)
{
	assert((m * n) % 256 == 0);

	float d = damping * dt + 2.0f;
	float e = (speed * speed) * (dt * dt) / (dx * dx);
	m_K[0] = (damping * dt - 2.0f) / d;
	m_K[1] = (4.0f - 8.0f * e) / d;
	m_K[2] = (2.0f * e) / d;

	BuildResources();
	BuildDescriptors();
}

void GpuWaves::BuildResources()
{
	// All the textures for the wave simulation will be bound as a shader resource and
	// unordered access view at some point since we ping-pong the buffers.

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_NumCols;
	texDesc.Height = m_NumRows;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_PrevSol)));

	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_CurrSol)));

	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_NextSol)));

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 

	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_CurrSol.Get(), 0, num2DSubresources);

	CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC uploadDesc(CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize));
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_PrevUploadBuffer.GetAddressOf())));

	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_CurrUploadBuffer.GetAddressOf())));
}

void GpuWaves::UploadResources(ID3D12GraphicsCommandList* cmdlist)
{
	const UINT num2DSubresources = 1;
	//const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;

	// Describe the data we want to copy into the default buffer.
	std::vector<float> initData(m_NumRows * m_NumCols, 0.f);

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData.data();
	subResourceData.RowPitch = m_NumCols * sizeof(float);
	subResourceData.SlicePitch = subResourceData.RowPitch * m_NumRows;

	// Schedule to copy the data to the default resource, and change states.
	// Note that mCurrSol is put in the GENERIC_READ state so it can be 
	// read by a shader.
	CD3DX12_RESOURCE_BARRIER C2CDPrevBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_PrevSol.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	cmdlist->ResourceBarrier(1, &C2CDPrevBarrier);
	UpdateSubresources(cmdlist, m_PrevSol.Get(), m_PrevUploadBuffer.Get(), 0, 0, num2DSubresources, &subResourceData);
	CD3DX12_RESOURCE_BARRIER CD2UAPrevBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_PrevSol.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	cmdlist->ResourceBarrier(1, &CD2UAPrevBarrier);

	CD3DX12_RESOURCE_BARRIER C2CDCurrBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_CurrSol.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	cmdlist->ResourceBarrier(1, &C2CDCurrBarrier);
	UpdateSubresources(cmdlist, m_CurrSol.Get(), m_CurrUploadBuffer.Get(), 0, 0, num2DSubresources, &subResourceData);
	CD3DX12_RESOURCE_BARRIER CD2GRCurrBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_CurrSol.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	cmdlist->ResourceBarrier(1, &CD2GRCurrBarrier);

	CD3DX12_RESOURCE_BARRIER C2UANextBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_NextSol.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	cmdlist->ResourceBarrier(1, &C2UANextBarrier);
}

void GpuWaves::BuildDescriptors()
{
	auto alloc = CDescriptorAllocator::GetInst()->Allocate(6);
	assert(alloc.IsValid());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	UINT descriptorSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DEVICE->CreateShaderResourceView(m_PrevSol.Get(), &srvDesc, alloc.cpuHandle);
	DEVICE->CreateShaderResourceView(m_CurrSol.Get(), &srvDesc, alloc.cpuHandle.Offset(1, descriptorSize));
	DEVICE->CreateShaderResourceView(m_NextSol.Get(), &srvDesc, alloc.cpuHandle.Offset(1, descriptorSize));

	DEVICE->CreateUnorderedAccessView(m_PrevSol.Get(), nullptr, &uavDesc, alloc.cpuHandle.Offset(1, descriptorSize));
	DEVICE->CreateUnorderedAccessView(m_CurrSol.Get(), nullptr, &uavDesc, alloc.cpuHandle.Offset(1, descriptorSize));
	DEVICE->CreateUnorderedAccessView(m_NextSol.Get(), nullptr, &uavDesc, alloc.cpuHandle.Offset(1, descriptorSize));

	// Save references to the GPU descriptors. 
	m_PrevSolSrv = alloc.gpuHandle;
	m_CurrSolSrv = alloc.gpuHandle.Offset(1, descriptorSize);
	m_NextSolSrv = alloc.gpuHandle.Offset(1, descriptorSize);
	m_PrevSolUav = alloc.gpuHandle.Offset(1, descriptorSize);
	m_CurrSolUav = alloc.gpuHandle.Offset(1, descriptorSize);
	m_NextSolUav = alloc.gpuHandle.Offset(1, descriptorSize);
}

void GpuWaves::Update(ID3D12GraphicsCommandList* cmdlist)
{
	static float t = 0.f;
	t += CTimeMgr::GetInst()->DeltaTime();

	cmdlist->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_WAVE_UPDATE).Get());
	cmdlist->SetComputeRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"Waves").Get());

	if (t > m_TimeStep)
	{
		cmdlist->SetComputeRoot32BitConstants(0, 3, m_K, 0);
		cmdlist->SetComputeRootDescriptorTable(1, m_PrevSolUav);
		cmdlist->SetComputeRootDescriptorTable(2, m_CurrSolUav);
		cmdlist->SetComputeRootDescriptorTable(3, m_NextSolUav);

		UINT numGroupsX = m_NumCols / 16;
		UINT numGroupsY = m_NumRows / 16;
		cmdlist->Dispatch(numGroupsX, numGroupsY, 1);

		// ping-pong buffers
		auto resTemp = m_PrevSol;
		m_PrevSol = m_CurrSol;
		m_CurrSol = m_NextSol;
		m_NextSol = resTemp;

		auto srvTemp = m_PrevSolSrv;
		m_PrevSolSrv = m_CurrSolSrv;
		m_CurrSolSrv = m_NextSolSrv;
		m_NextSolSrv = srvTemp;

		auto uavTemp = m_PrevSolUav;
		m_PrevSolUav = m_CurrSolUav;
		m_CurrSolUav = m_NextSolUav;
		m_NextSolUav = uavTemp;

		t -= m_TimeStep;

	}
	// The current solution needs to be able to be read by the vertex shader, so change its state to GENERIC_READ.
	//CD3DX12_RESOURCE_BARRIER UA2GRCurrBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_CurrSol.Get(),
	//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
	//CMDLIST->ResourceBarrier(1, &UA2GRCurrBarrier);
}

void GpuWaves::Disturb(ID3D12GraphicsCommandList* cmdlist, UINT i, UINT j, float magnitude)
{
	cmdlist->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_WAVE_DISTURB).Get());
	cmdlist->SetComputeRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"Waves").Get());

	UINT disturbIndex[2] = { j, i };
	cmdlist->SetComputeRoot32BitConstants(0, 1, &magnitude, 3);
	cmdlist->SetComputeRoot32BitConstants(0, 2, disturbIndex, 4);
	cmdlist->SetComputeRootDescriptorTable(3, m_CurrSolUav);

	// The current solution is in the GENERIC_READ state so it can be read by the vertex shader.
	// Change it to UNORDERED_ACCESS for the compute shader.  Note that a UAV can still be
	// read in a compute shader.
	//CD3DX12_RESOURCE_BARRIER GR2UACurrBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_CurrSol.Get(),
	//	D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	//CMDLIST->ResourceBarrier(1, &GR2UACurrBarrier);

	// One thread group kicks off one thread, which displaces the height of one
	// vertex and its neighbors.
	cmdlist->Dispatch(1, 1, 1);
}
