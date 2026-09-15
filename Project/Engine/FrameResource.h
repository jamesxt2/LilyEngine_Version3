#pragma once

#include "CConstantBuffer.h"

struct FrameResource
{
public:
	FrameResource(ID3D12Device* device);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	void CreateCB(UINT elementByteSize, UINT elementCount, CB_TYPE type);

	ComPtr<ID3D12CommandAllocator> m_CmdListAlloc;

	std::shared_ptr<CConstantBuffer> m_CBs[(UINT)CB_TYPE::END];

	UINT64 Fence = 0;

	std::shared_ptr<CConstantBuffer> GetConstantBuffer(CB_TYPE type)
	{
		return m_CBs[(UINT)type];
	}
};

