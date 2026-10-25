#pragma once
#include "singleton.h"

#include "CDescriptorAllocator.h"

class CGameObject;

class CShadowMap : public CSingleton<CShadowMap>
{
	SINGLE(CShadowMap)
public:
	
	void Initialize(UINT width, UINT height);
	
	void DrawSceneToShadowMap(const std::vector<CGameObject*>& objs);
private:
	void BuildResource();
	void BuildDescriptors();

	D3D12_VIEWPORT m_Viewport = {};
	D3D12_RECT m_ScissorRect = {};
	UINT m_Width = 0;
	UINT m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R24G8_TYPELESS;

	CDescriptorAllocator::Allocation m_AllocSrv;
	CDescriptorAllocator::Allocation m_AllocDsv;
	CDescriptorAllocator::Allocation m_NullSrv;

	ComPtr<ID3D12Resource> m_ShadowMap = nullptr;

public:
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() { return m_AllocSrv.gpuHandle; }
};

