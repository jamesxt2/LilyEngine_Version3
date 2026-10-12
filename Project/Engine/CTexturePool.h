#pragma once

#include "singleton.h"

#include "CDescriptorAllocator.h"

class CTexturePool : public CSingleton<CTexturePool>
{
	SINGLE(CTexturePool)

public:
	static constexpr UINT MAX_TEXTURES_2D = 4000;
	static constexpr UINT MAX_TEXTURES_ARRAY = 96;
	static constexpr UINT MAX_TEXTURES_TOTAL = 4096;

	void Init();
	UINT Allocate2DIndex();
	UINT AllocateArrayIndex();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle_2D(UINT index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle_2D(UINT index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle_Array(UINT index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle_Array(UINT index);

	void Bind_Graphics_Table_2D(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
	void Bind_Graphics_Table_Array(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);
	void Bind_Compute_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex);

private:
	CDescriptorAllocator::Allocation m_Alloc;
	UINT m_DescriptorSize = 0;
	UINT m_Next2DIndex = 0;
	UINT m_NextArrayIndex = 0;
};

