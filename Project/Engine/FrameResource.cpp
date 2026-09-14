#include "pch.h"
#include "FrameResource.h"


FrameResource::FrameResource(ID3D12Device* device, UINT passBufferSize, CB_TYPE passBufferType, UINT objectBufferSize, CB_TYPE objectBufferType)
{
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())
	));
	PassCB = std::make_unique<CConstantBuffer>(passBufferSize, passBufferType);
	ObjectCB = std::make_unique<CConstantBuffer>(objectBufferSize, objectBufferType);
}

FrameResource::~FrameResource() {}