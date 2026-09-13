#include "pch.h"
#include "CConstantBuffer.h"

#include "CDevice.h"

CConstantBuffer::CConstantBuffer()
	: m_Type(CB_TYPE::END), m_BufferSize(0), m_ElementByteSize(0),
	m_CbvHeap(nullptr), m_UploadBuffer(nullptr), m_MappedData(nullptr),
	m_RootSignature(nullptr)
{

}

CConstantBuffer::~CConstantBuffer()
{
	if (m_UploadBuffer != nullptr)
		m_UploadBuffer->Unmap(0, nullptr);
	m_MappedData = nullptr;
}

void CConstantBuffer::BuildCbvDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(DEVICE->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_CbvHeap)));
}

void CConstantBuffer::Create(size_t bufferSize, CB_TYPE type)
{
	BuildCbvDescriptorHeap();

	m_BufferSize = (UINT)bufferSize;
	m_Type = type;
	m_ElementByteSize = CalcConstantBufferByteSize((UINT)bufferSize);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC rDesc(CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize));
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&rDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UploadBuffer)
	));

	ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, &m_MappedData));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_UploadBuffer->GetGPUVirtualAddress();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = m_ElementByteSize;

	DEVICE->CreateConstantBufferView(&cbvDesc, m_CbvHeap->GetCPUDescriptorHandleForHeapStart());

	BuildRootSignature();
}

// Set which slot to bind
void CConstantBuffer::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// define descriptor range, 1 means one descriptor, 0 means bind to b0
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	// it means the shader can access the descriptor table containing b0's CBV
	// through root parameter 0
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	// Turn root signature into binary blob cause CreateRootSignature needs
	// binary data
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	ThrowIfFailed(DEVICE->CreateRootSignature(
		0, serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)
	));
}

void CConstantBuffer::Bind()
{
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	CMDLIST->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	CMDLIST->SetGraphicsRootSignature(m_RootSignature.Get());

	CMDLIST->SetGraphicsRootDescriptorTable(0, m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
}

void CConstantBuffer::CopyData(const void* data)
{
	memcpy(m_MappedData, data, m_BufferSize);
}
