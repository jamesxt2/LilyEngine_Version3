#pragma once
#include "CEntity.h"

class CStructuredBuffer : public CEntity
{
public:
	CStructuredBuffer();
	CStructuredBuffer(const CStructuredBuffer& other);
	~CStructuredBuffer();
	CLONE(CStructuredBuffer)

private:
	ComPtr<ID3D12Resource>			m_Main;
	ComPtr<ID3D12Resource>			m_Upload;
	ComPtr<ID3D12Resource>			m_Readback;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_CpuSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_CpuUav;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_GpuSrv;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_GpuUav;
};

