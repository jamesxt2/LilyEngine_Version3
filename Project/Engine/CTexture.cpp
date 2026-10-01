#include "pch.h"
#include "CTexture.h"

#include "CPathMgr.h"
#include "CDevice.h"
#include "CAssetMgr.h"


CTexture::CTexture()
	: CAsset(ASSET_TYPE::TEXTURE),
	m_Resource(nullptr), m_UploadHeap(nullptr),
	m_Alloc{}
{
	
}

CTexture::~CTexture()
{

}

void CTexture::CreateFromFile(const std::wstring& filename, bool isTextureArray)
{
	std::wstring strPath = CPathMgr::GetInst()->GetContentPath();

	CDevice::GetInst()->UploadResourceAsync([&](ID3D12GraphicsCommandList* cmdlist) {
		ThrowIfFailed(CreateDDSTextureFromFile12(
			DEVICE.Get(), cmdlist,
			(strPath + filename).c_str(),
			m_Resource, m_UploadHeap));
		});

	m_Alloc = CDescriptorAllocator::GetInst()->Allocate(1);
	assert(m_Alloc.IsValid());

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_Alloc.cpuHandle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_Resource->GetDesc().Format;
	if (isTextureArray)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = -1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = m_Resource->GetDesc().DepthOrArraySize;
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = m_Resource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
	}
	DEVICE->CreateShaderResourceView(m_Resource.Get(), &srvDesc, hDescriptor);
}

void CTexture::Bind()
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(m_Alloc.gpuHandle);
	CMDLIST->SetGraphicsRootDescriptorTable(0, texHandle);
}