#include "pch.h"
#include "CConstantBuffer.h"

#include "CDevice.h"

ComPtr<ID3D12DescriptorHeap> CConstantBuffer::m_CbvHeap = nullptr;
ComPtr<ID3D12RootSignature> CConstantBuffer::m_RootSignature = nullptr;

CConstantBuffer::CConstantBuffer()
	: m_Type(CB_TYPE::END), m_BufferSize(0), m_ElementByteSize(0),
	m_TotalSize(0),
	m_UploadBuffer(nullptr), m_MappedData(nullptr)
{

}

CConstantBuffer::CConstantBuffer(UINT elementByteSize, UINT elementCount, CB_TYPE type)
	: m_Type(CB_TYPE::END), m_BufferSize(0), m_ElementByteSize(0),
	m_TotalSize(0),
	 m_UploadBuffer(nullptr), m_MappedData(nullptr)
{
	Create(elementByteSize, elementCount, type);
}

CConstantBuffer::~CConstantBuffer()
{
	if (m_UploadBuffer != nullptr)
		m_UploadBuffer->Unmap(0, nullptr);
	m_MappedData = nullptr;
}

void CConstantBuffer::BuildCbvDescriptorHeap()
{
	UINT NumDescriptors = g_MaxObjectCount * g_NumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = NumDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(DEVICE->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_CbvHeap)));
}

void CConstantBuffer::Create(UINT elementByteSize, UINT elementCount, CB_TYPE type)
{
	m_BufferSize = (UINT)elementByteSize;
	m_Type = type;
	m_ElementByteSize = CalcConstantBufferByteSize((UINT)elementByteSize);
	m_TotalSize = m_ElementByteSize * elementCount;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC rDesc(CD3DX12_RESOURCE_DESC::Buffer(m_TotalSize));
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&rDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UploadBuffer)
	));

	ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));

	for (int frameIndex = 0; frameIndex < g_NumFrameResources; ++frameIndex)
	{
		for (UINT i = 0; i < elementCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_UploadBuffer->GetGPUVirtualAddress();
			cbAddress += i * m_ElementByteSize;

			int heapIndex = frameIndex * elementCount + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, CDevice::GetInst()->m_CbvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = m_ElementByteSize;

			DEVICE->CreateConstantBufferView(&cbvDesc, handle);
		}
	}
}

// Set which slot to bind
void CConstantBuffer::BuildRootSignature(UINT slotCount)
{
	std::vector<CD3DX12_DESCRIPTOR_RANGE> cbvTables;

	for (UINT i = 0; i < slotCount; ++i)
	{
		CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i);
		cbvTables.push_back(cbvTable);
	}

	// Root parameter can be a table, root descriptor or root constants.
	std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters;

	// Create root CBVs.
	for (UINT i = 0; i < slotCount; ++i)
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter;
		slotRootParameter.InitAsDescriptorTable(1, &cbvTables[i]);
		slotRootParameters.push_back(slotRootParameter);
	}

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameters.data(), 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(DEVICE->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

void CConstantBuffer::Bind(int ObjCBIndex, int slot)
{
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	CMDLIST->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	CMDLIST->SetGraphicsRootSignature(m_RootSignature.Get());

	auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		m_CbvHeap->GetGPUDescriptorHandleForHeapStart(), ObjCBIndex, CDevice::GetInst()->m_CbvUavDescriptorSize
	);
	CMDLIST->SetGraphicsRootDescriptorTable(slot, handle);
}

void CConstantBuffer::CopyData(int elementIndex, const void* data)
{
	memcpy(&m_MappedData[elementIndex * m_ElementByteSize], data, m_BufferSize);
}

void CConstantBuffer::Init(UINT cbvSlotNums)
{
	BuildCbvDescriptorHeap();
	BuildRootSignature(cbvSlotNums);
}
