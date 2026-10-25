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

	void InitHeapData(ComPtr<ID3D12DescriptorHeap> heap, UINT capacity);

	Allocation Allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count);
	Allocation GetAllocation(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT startIdx, UINT count);

	void Free(D3D12_DESCRIPTOR_HEAP_TYPE type, const Allocation& alloc);

private:

	struct FreeBlock
	{
		UINT startIndex;
		UINT count;
	};

	bool FindFreeBlock(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count, _Out_ UINT& outStartIndex);
	void MergeFreeBlocks(D3D12_DESCRIPTOR_HEAP_TYPE type);

	struct HeapData
	{
		ComPtr<ID3D12DescriptorHeap>					Heap;
		UINT											Capacity{ 0 };
		UINT											DescriptorSize{ 0 };
		std::vector<FreeBlock>							FreeBlocks;
	};
	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, HeapData> m_HeapDataMap;
	

public:
	inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index)
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_HeapDataMap[type].Heap->GetCPUDescriptorHandleForHeapStart(),
			index, m_HeapDataMap[type].DescriptorSize
		);
	}
	inline CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index)
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_HeapDataMap[type].Heap->GetGPUDescriptorHandleForHeapStart(),
			index, m_HeapDataMap[type].DescriptorSize
		);
	}

};

