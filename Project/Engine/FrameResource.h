#pragma once

#include "CConstantBuffer.h"
#include "CStructuredBuffer.h"

struct FrameResource
{
public:
	FrameResource(ID3D12Device* device);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	void CreateCB(UINT elementByteSize, UINT elementCount, CB_TYPE type);
	void CreateSB(UINT elementByteSize, UINT elementCount, SB_TYPE type, STRUCTURED_BUFFER_TYPE type_Buffer = STRUCTURED_BUFFER_TYPE::TABLE);
	//void CreateWavesVB(UINT waveVtxCount);

	ComPtr<ID3D12CommandAllocator> m_CmdAlloc;
	ComPtr<ID3D12GraphicsCommandList> m_CmdList;

	std::shared_ptr<CConstantBuffer> m_CBs[(UINT)CB_TYPE::END];
	std::shared_ptr<CStructuredBuffer> m_SBs[(UINT)SB_TYPE::END];
	//CStructuredBuffer* m_SBs[(UINT)SB_TYPE::END];
	//std::unique_ptr<UploadBuffer<sizeof(Vertex)>> m_WavesVB;

	UINT64 Fence = 0;

	inline std::shared_ptr<CConstantBuffer> GetConstantBuffer(CB_TYPE type)
	{
		return m_CBs[(UINT)type];
	}

	inline std::shared_ptr<CStructuredBuffer> GetStructuredBuffer(SB_TYPE type)
	{
		return m_SBs[(UINT)type];
	}

};

