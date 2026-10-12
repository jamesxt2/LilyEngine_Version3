#include "pch.h"
#include "CTexture.h"

#include "CPathMgr.h"
#include "CDevice.h"
#include "CAssetMgr.h"
#include "CTexturePool.h"

CTexture::CTexture()
	: CAsset(ASSET_TYPE::TEXTURE),
	m_Resource(nullptr), m_UploadHeap(nullptr)
{
	
}

CTexture::~CTexture()
{
	m_UploadHeap.Reset();
	m_Resource.Reset();
}

void CTexture::CreateFromFile(const std::wstring& filename)
{
	std::wstring strPath = CPathMgr::GetInst()->GetContentPath();

	CDevice::GetInst()->UploadResourceAsync([&](ID3D12GraphicsCommandList* cmdlist) {
		ThrowIfFailed(CreateDDSTextureFromFile12(
			DEVICE.Get(), cmdlist,
			(strPath + filename).c_str(),
			m_Resource, m_UploadHeap));
		});

	m_Resource->SetName(L"Texture");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_Resource->GetDesc().Format;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	if (m_Resource->GetDesc().DepthOrArraySize > 1)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = -1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = m_Resource->GetDesc().DepthOrArraySize;
		m_PoolIndex = CTexturePool::GetInst()->AllocateArrayIndex();
		cpuHandle = CTexturePool::GetInst()->GetCpuHandle_Array(m_PoolIndex);
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = m_Resource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		m_PoolIndex = CTexturePool::GetInst()->Allocate2DIndex();
		cpuHandle = CTexturePool::GetInst()->GetCpuHandle_2D(m_PoolIndex);
	}
	DEVICE->CreateShaderResourceView(m_Resource.Get(), &srvDesc, cpuHandle);
}

//void CTexture::Bind_Graphics_Table(UINT rootParamIndex)
//{
//	CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(m_Alloc.gpuHandle);
//	CMDLIST->SetGraphicsRootDescriptorTable(rootParamIndex, texHandle);
//}
//
//void CTexture::Bind_Graphics_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex)
//{
//	CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(m_Alloc.gpuHandle);
//	cmdlist->SetGraphicsRootDescriptorTable(rootParamIndex, texHandle);
//}

//void CTexture::Bind_Compute_Table(ID3D12GraphicsCommandList* cmdlist, UINT rootParamIndex)
//{
//	cmdlist->SetComputeRootDescriptorTable(rootParamIndex, CTexturePool::GetInst()->GetGpuHandle_2D(m_PoolIndex));
//}

