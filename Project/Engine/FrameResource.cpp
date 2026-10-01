#include "pch.h"
#include "FrameResource.h"

#include "CDevice.h"


FrameResource::FrameResource(ID3D12Device* device)
{
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_CmdAlloc.GetAddressOf())
	));

	ThrowIfFailed(device->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CmdAlloc.Get(), 
		nullptr, IID_PPV_ARGS(&m_CmdList)));

	ThrowIfFailed(m_CmdList->Close());
}

FrameResource::~FrameResource() {}

void FrameResource::CreateCB(UINT elementByteSize, UINT elementCount, CB_TYPE type)
{
	m_CBs[(UINT)type] = std::make_shared<CConstantBuffer>(elementByteSize, elementCount, type);
}

/*
void FrameResource::CreateWavesVB(UINT waveVtxCount)
{
	m_WavesVB = std::make_unique<UploadBuffer<(UINT)sizeof(Vertex)>>(DEVICE.Get(), waveVtxCount, false);
}
*/