#include "pch.h"
#include "CTexture.h"

#include "CPathMgr.h"
#include "CDevice.h"

ComPtr<ID3D12DescriptorHeap> CTexture::m_SrvDescriptorHeap = nullptr;

CTexture::CTexture()
	: CAsset(ASSET_TYPE::TEXTURE),
	m_Resource(nullptr), m_UploadHeap(nullptr),
	m_DescriptorHeapOffsetSize(0)
{
	if (m_SrvDescriptorHeap == nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 6;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(DEVICE->CreateDescriptorHeap(
			&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));
	}
	assert(m_SrvDescriptorHeap);
}

CTexture::~CTexture()
{
}

void CTexture::CreateFromFile(const std::wstring& filename, INT descriptorOffset)
{
	m_DescriptorHeapOffsetSize = descriptorOffset;

	std::wstring strPath = CPathMgr::GetInst()->GetContentPath();

	ThrowIfFailed(CreateDDSTextureFromFile12(
		DEVICE.Get(), CMDLIST.Get(),
		(strPath + filename).c_str(),
		m_Resource, m_UploadHeap
	));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(
		m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(descriptorOffset, CDevice::GetInst()->m_CbvSrvUavDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_Resource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = m_Resource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
	DEVICE->CreateShaderResourceView(m_Resource.Get(), &srvDesc, hDescriptor);
}

void CTexture::Bind()
{
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvDescriptorHeap.Get() };
	CMDLIST->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(
		m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	texHandle.Offset(m_DescriptorHeapOffsetSize, CDevice::GetInst()->m_CbvSrvUavDescriptorSize);
	CMDLIST->SetGraphicsRootDescriptorTable(0, texHandle);
}