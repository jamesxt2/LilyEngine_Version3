#pragma once
#include "singleton.h"

class CDescriptorAllocator : public CSingleton<CDescriptorAllocator>
{
	SINGLE(CDescriptorAllocator)
		
public:

	struct Allocation
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = {};
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
		UINT startIndex = 0;
		UINT count = 0;

		bool IsValid() const { return count > 0; }
	};

	void Initialize(ComPtr<ID3D12Device> device, ComPtr<ID3D12DescriptorHeap> heap, UINT capacity);

	Allocation Allocate(UINT count);

	void Free(const Allocation& alloc);

private:

	struct FreeBlock
	{
		UINT startIndex;
		UINT count;
	};

	bool FindFreeBlock(UINT count, _Out_ UINT& outStartIndex);
	void MergeFreeBlocks();

	ComPtr<ID3D12Device>							m_Device;
	ComPtr<ID3D12DescriptorHeap>					m_Heap;
	UINT											m_Capacity{ 0 };
	UINT											m_UsedCount{ 0 };
	UINT											m_DescriptorSize{ 0 };
	std::vector<FreeBlock>							m_FreeBlocks;

public:
	inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(UINT index) const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_Heap->GetCPUDescriptorHandleForHeapStart(),
			index, m_DescriptorSize
		);
	}
	inline CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(UINT index) const
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_Heap->GetGPUDescriptorHandleForHeapStart(),
			index, m_DescriptorSize
		);
	}

};

