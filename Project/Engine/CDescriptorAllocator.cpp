#include "pch.h"
#include "CDescriptorAllocator.h"

#include "CDevice.h"

CDescriptorAllocator::CDescriptorAllocator()
{

}

CDescriptorAllocator::~CDescriptorAllocator()
{

}

void CDescriptorAllocator::InitHeapData(ComPtr<ID3D12DescriptorHeap> heap, UINT capacity)
{
	HeapData data = {};
	data.Heap = heap;
	data.Capacity = capacity;
	data.DescriptorSize = DEVICE->GetDescriptorHandleIncrementSize(heap->GetDesc().Type);
	data.FreeBlocks.clear();
	data.FreeBlocks.push_back({ 0, capacity });

	m_HeapDataMap[heap->GetDesc().Type] = std::move(data);
}

CDescriptorAllocator::Allocation CDescriptorAllocator::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count)
{
	assert(count > 0);
	assert(m_HeapDataMap[type].Heap != nullptr && "CDescriptorAllocator is not initialized!");

	Allocation alloc = {};

	UINT startIndex = 0;
	if (!FindFreeBlock(type, count, startIndex))
	{
		assert(false && "Descriptor heap overflow! Increase capacity!");
		return alloc;
	}

	alloc.startIndex = startIndex;
	alloc.count = count;
	alloc.cpuHandle = GetCPUDescriptorHandle(type, startIndex);
	if (m_HeapDataMap[type].Heap->GetDesc().Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		alloc.gpuHandle = GetGPUDescriptorHandle(type, startIndex);

	return alloc;
}

CDescriptorAllocator::Allocation CDescriptorAllocator::GetAllocation(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT startIdx, UINT count)
{
	assert(m_HeapDataMap[type].Heap != nullptr && "CDescriptorAllocator is not initialized!");
	Allocation alloc = {};
	alloc.startIndex = startIdx;
	alloc.count = count;
	alloc.cpuHandle = GetCPUDescriptorHandle(type, startIdx);
	if (m_HeapDataMap[type].Heap->GetDesc().Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		alloc.gpuHandle = GetGPUDescriptorHandle(type, startIdx);

	return alloc;
}

void CDescriptorAllocator::Free(D3D12_DESCRIPTOR_HEAP_TYPE type, const Allocation& alloc)
{
	if (!alloc.IsValid()) return;

	m_HeapDataMap[type].FreeBlocks.push_back({ alloc.startIndex, alloc.count });

	std::sort(m_HeapDataMap[type].FreeBlocks.begin(), m_HeapDataMap[type].FreeBlocks.end(),
		[](const FreeBlock& a, const FreeBlock& b) {
			return a.startIndex < b.startIndex;
		});

	MergeFreeBlocks(type);
}

bool CDescriptorAllocator::FindFreeBlock(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count, _Out_ UINT& outStartIndex)
{
	outStartIndex = 0;
	for (auto& block : m_HeapDataMap[type].FreeBlocks)
	{
		if (block.count >= count)
		{
			outStartIndex = block.startIndex;
			if (block.count == count)
			{
				m_HeapDataMap[type].FreeBlocks.erase(
					std::remove_if(m_HeapDataMap[type].FreeBlocks.begin(), m_HeapDataMap[type].FreeBlocks.end(),
						[&](const FreeBlock& b) {return b.startIndex == block.startIndex; }),
					m_HeapDataMap[type].FreeBlocks.end()
				);
			}
			else
			{
				block.startIndex += count;
				block.count -= count;
			}
			return true;
		}
	}
	return false;
}

void CDescriptorAllocator::MergeFreeBlocks(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	for (size_t i = 0; i < m_HeapDataMap[type].FreeBlocks.size(); )
	{
		FreeBlock& current = m_HeapDataMap[type].FreeBlocks[i];
		FreeBlock& next = m_HeapDataMap[type].FreeBlocks[i + 1];
		if (current.startIndex + current.count == next.startIndex)
		{
			current.count += next.count;
			m_HeapDataMap[type].FreeBlocks.erase(m_HeapDataMap[type].FreeBlocks.begin() + i + 1);
		}
		else
			++i;
	}
}
