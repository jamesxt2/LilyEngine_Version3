#include "pch.h"
#include "CTexturePool.h"

#include "CDevice.h"


CTexturePool::CTexturePool()
{

}

CTexturePool::~CTexturePool()
{

}

void CTexturePool::Init()
{
	m_Alloc = CDescriptorAllocator::GetInst()->Allocate(MAX_TEXTURES_TOTAL);
	m_DescriptorSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_Next2DIndex = 0;
	m_NextArrayIndex = 0;
}

UINT CTexturePool::Allocate2DIndex()
{
	UINT index = m_Next2DIndex++;
	assert(index < MAX_TEXTURES_2D && "TexturePool overflow!");
	return index;
}

UINT CTexturePool::AllocateArrayIndex()
{
	UINT index = m_NextArrayIndex++;
	assert(index < MAX_TEXTURES_ARRAY && "TexturePool overflow!");
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE CTexturePool::GetCpuHandle_2D(UINT index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Alloc.cpuHandle, index, m_DescriptorSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE CTexturePool::GetGpuHandle_2D(UINT index)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_Alloc.gpuHandle, index, m_DescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE CTexturePool::GetCpuHandle_Array(UINT index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Alloc.cpuHandle, MAX_TEXTURES_2D + index, m_DescriptorSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE CTexturePool::GetGpuHandle_Array(UINT index)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_Alloc.gpuHandle, MAX_TEXTURES_2D + index, m_DescriptorSize);
}

void CTexturePool::Bind_Graphics_Table_2D(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex)
{
	cmdlist->SetGraphicsRootDescriptorTable(rootParamIndex, m_Alloc.gpuHandle);
}

void CTexturePool::Bind_Graphics_Table_Array(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex)
{
	cmdlist->SetGraphicsRootDescriptorTable(rootParamIndex, CD3DX12_GPU_DESCRIPTOR_HANDLE(m_Alloc.gpuHandle, MAX_TEXTURES_2D, m_DescriptorSize));
}

void CTexturePool::Bind_Compute_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex)
{
	cmdlist->SetComputeRootDescriptorTable(rootParamIndex, m_Alloc.gpuHandle);
}
