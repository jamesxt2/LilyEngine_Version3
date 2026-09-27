#include "pch.h"
#include "CTexture.h"

#include "CPathMgr.h"
#include "CDevice.h"
#include "CAssetMgr.h"

CTexture::CTexture()
	: CAsset(ASSET_TYPE::TEXTURE),
	m_Resource(nullptr), m_UploadHeap(nullptr),
	m_DescriptorHeapOffsetSize(0)
{
	
}

CTexture::~CTexture()
{
}

void CTexture::CreateFromFile(const std::wstring& filename, INT descriptorOffset, bool isTextureArray)
{
	m_DescriptorHeapOffsetSize = descriptorOffset;

	std::wstring strPath = CPathMgr::GetInst()->GetContentPath();

	ThrowIfFailed(CreateDDSTextureFromFile12(
		DEVICE.Get(), CMDLIST.Get(),
		(strPath + filename).c_str(),
		m_Resource, m_UploadHeap
	));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(
		CAssetMgr::GetInst()->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(descriptorOffset, CDevice::GetInst()->m_CbvSrvUavDescriptorSize);

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
	CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(
		CAssetMgr::GetInst()->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
	texHandle.Offset(m_DescriptorHeapOffsetSize, CDevice::GetInst()->m_CbvSrvUavDescriptorSize);
	CMDLIST->SetGraphicsRootDescriptorTable(0, texHandle);
}