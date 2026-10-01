#include "pch.h"
#include "CDescriptorAllocator.h"

CDescriptorAllocator::CDescriptorAllocator()
{

}

CDescriptorAllocator::~CDescriptorAllocator()
{

}

void CDescriptorAllocator::Initialize(ComPtr<ID3D12Device> device, ComPtr<ID3D12DescriptorHeap> heap, UINT capacity)
{
	m_Device = device;
	m_Heap = heap;
	m_Capacity = capacity;
	m_UsedCount = 0;
	m_DescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_FreeBlocks.clear();
	m_FreeBlocks.push_back({ 0, capacity });
}

CDescriptorAllocator::Allocation CDescriptorAllocator::Allocate(UINT count)
{
	assert(count > 0);
	assert(m_Heap != nullptr && "CDescriptorAllocator is not initialized!");

	Allocation alloc = {};

	UINT startIndex = 0;
	if (!FindFreeBlock(count, startIndex))
	{
		assert(false && "Descriptor heap overflow! Increase capacity!");
		return alloc;
	}

	alloc.startIndex = startIndex;
	alloc.count = count;
	alloc.cpuHandle = GetCPUDescriptorHandle(startIndex);
	alloc.gpuHandle = GetGPUDescriptorHandle(startIndex);

	m_UsedCount += count;
	return alloc;
}

void CDescriptorAllocator::Free(const Allocation& alloc)
{
	if (!alloc.IsValid()) return;

	m_FreeBlocks.push_back({ alloc.startIndex, alloc.count });
	m_UsedCount -= alloc.count;

	std::sort(m_FreeBlocks.begin(), m_FreeBlocks.end(),
		[](const FreeBlock& a, const FreeBlock& b) {
			return a.startIndex < b.startIndex;
		});

	MergeFreeBlocks();
}

bool CDescriptorAllocator::FindFreeBlock(UINT count, _Out_ UINT& outStartIndex)
{
	outStartIndex = 0;
	for (auto& block : m_FreeBlocks)
	{
		if (block.count >= count)
		{
			outStartIndex = block.startIndex;
			if (block.count == count)
			{
				m_FreeBlocks.erase(
					std::remove_if(m_FreeBlocks.begin(), m_FreeBlocks.end(),
						[&](const FreeBlock& b) {return b.startIndex == block.startIndex; }),
					m_FreeBlocks.end()
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

void CDescriptorAllocator::MergeFreeBlocks()
{
	for (size_t i = 0; i < m_FreeBlocks.size(); )
	{
		FreeBlock& current = m_FreeBlocks[i];
		FreeBlock& next = m_FreeBlocks[i + 1];
		if (current.startIndex + current.count == next.startIndex)
		{
			current.count += next.count;
			m_FreeBlocks.erase(m_FreeBlocks.begin() + i + 1);
		}
		else
			++i;
	}
}
