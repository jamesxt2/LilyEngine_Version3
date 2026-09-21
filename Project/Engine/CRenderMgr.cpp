#include "pch.h"
#include "CRenderMgr.h"

#include "CDevice.h"
#include "CLevelMgr.h"
#include "CCamera.h"

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

	// Output Merge Set Render Targets
	//Ptr<CTexture> pRTTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"RenderTargetTex");
	//Ptr<CTexture> pDSTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"DepthStencilTex");
	//CONTEXT->OMSetRenderTargets(1, pRTTex->GetRTV().GetAddressOf(), pDSTex->GetDSV().Get());

	// Target Clear
	//float ClearColor[4] = { 0.3f, 0.3f, 0.3f, 1.f };
	CDevice::GetInst()->ClearTargetAndPrepareRender(Colors::LightSteelBlue);

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

void CRenderMgr::CopyRenderTarget()
{
	//Ptr<CTexture> pRenderTargetTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"RenderTargetTex");
	//CONTEXT->CopyResource(m_RenderTargetCopyTex->GetTex2D().Get(), pRenderTargetTex->GetTex2D().Get());
}
