#include "pch.h"
#include "FrameResource.h"

#include "CDevice.h"


FrameResource::FrameResource(ID3D12Device* device)
	: m_WavesVB(nullptr)
{
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_CmdListAlloc.GetAddressOf())
	));
}

FrameResource::~FrameResource() {}

void FrameResource::CreateCB(UINT elementByteSize, UINT elementCount, CB_TYPE type)
{
	m_CBs[(UINT)type] = std::make_shared<CConstantBuffer>(elementByteSize, elementCount, type);
}

void FrameResource::CreateWavesVB(UINT waveVtxCount)
{
	m_WavesVB = std::make_unique<UploadBuffer<(UINT)sizeof(Vertex)>>(DEVICE.Get(), waveVtxCount, false);
}
