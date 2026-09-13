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

	DataBind();

	// Target Clear
	//float ClearColor[4] = { 0.3f, 0.3f, 0.3f, 1.f };
	CDevice::GetInst()->ClearTargetAndPrepareRender(Colors::LightSteelBlue);

	(this->*Render_Func)();

	CDevice::GetInst()->ExecuteAndFinishDrawCall();

	DataClear();
}

void CRenderMgr::Render_Play()
{
	for (size_t i = 0; i < m_vecCam.size(); ++i)
	{
		if (m_vecCam[i] == nullptr)
			continue;
		m_vecCam[i]->Render();
	}
}

void CRenderMgr::RegisterCamera(CCamera* camera, int priority)
{
	if (priority >= m_vecCam.size())
		m_vecCam.resize(priority + 1);

	assert(!(m_vecCam[priority] && m_vecCam[priority] != camera));

	m_vecCam[priority] = camera;
}

void CRenderMgr::CopyRenderTarget()
{
	//Ptr<CTexture> pRenderTargetTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"RenderTargetTex");
	//CONTEXT->CopyResource(m_RenderTargetCopyTex->GetTex2D().Get(), pRenderTargetTex->GetTex2D().Get());
}

void CRenderMgr::DataBind()
{
	
}

void CRenderMgr::DataClear()
{
	
}