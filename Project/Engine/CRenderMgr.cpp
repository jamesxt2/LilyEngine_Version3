#include "pch.h"
#include "CRenderMgr.h"

#include "CDevice.h"
#include "CLevel.h"
#include "CLevelMgr.h"
#include "CCamera.h"
#include "CSsao.h"
#include "CAssetMgr.h"

CRenderMgr::CRenderMgr()
	: m_EditorCam(nullptr)
{
	Render_Func = &CRenderMgr::Render_Play;
}

CRenderMgr::~CRenderMgr()
{

}

void CRenderMgr::Init()
{
	Vector2 vRenderResolution = CDevice::GetInst()->GetRenderResolution();
}

void CRenderMgr::Tick()
{

}

void CRenderMgr::Render()
{
	if (!CLevelMgr::GetInst()->GetCurrentLevel()) return;

	ID3D12DescriptorHeap* descriptorHeaps[] = { CDevice::GetInst()->GetCbvSrvUavDescriptorHeap().Get() };
	CMDLIST->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	CLevelMgr::GetInst()->GetCurrentLevel()->BuildLevelShadowMap();
	CLevelMgr::GetInst()->GetCurrentLevel()->BuildNormalsAndDepth();
	CSsao::GetInst()->ComputeSsao(3);

	CMDLIST->SetGraphicsRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"Default").Get());
	CDevice::GetInst()->ClearTargetAndPrepareRender();

	(this->*Render_Func)();

	OnObjRenderFinish.Broadcast();

	CDevice::GetInst()->ExecuteAndFinishDrawCall();
}

void CRenderMgr::Render_Play()
{
	for (size_t i = 0; i < m_LevelCameraMap[m_CurrLevelName].size(); ++i)
	{
		if (m_LevelCameraMap[m_CurrLevelName][i] == nullptr)
			continue;
		m_LevelCameraMap[m_CurrLevelName][i]->Render();
	}
}

void CRenderMgr::RegisterLevelCamera(const std::wstring& levelname, CCamera* camera, int priority)
{
	std::vector<CCamera*>& cameras = m_LevelCameraMap[levelname];

	if (priority >= cameras.size())
		cameras.resize(priority + 1);

	assert(!(cameras[priority] && cameras[priority] != camera));

	cameras[priority] = camera;
}

void CRenderMgr::SetCurrentLevel(const std::wstring& levalname)
{
	m_CurrLevelName = levalname;

	for (size_t i = 0; i < m_LevelCameraMap[m_CurrLevelName].size(); ++i)
	{
		if (m_LevelCameraMap[m_CurrLevelName][i] == nullptr)
			continue;
		m_LevelCameraMap[m_CurrLevelName][i]->SetDirty();
	}
}

