#pragma once

#include "CConstantBuffer.h"

struct FrameResource
{
public:
	FrameResource(ID3D12Device* device, UINT passBufferSize, CB_TYPE passBufferType, UINT objectBufferSize, CB_TYPE objectBufferType);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	std::unique_ptr<CConstantBuffer> PassCB = nullptr;
	std::unique_ptr<CConstantBuffer> ObjectCB = nullptr;

	UINT64 Fence = 0;
};

