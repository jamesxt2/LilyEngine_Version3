#include "pch.h"
#include "CCamera.h"

#include "CDevice.h"
#include "CGameObject.h"
#include "CTransform.h"
#include "CRenderMgr.h"
#include "CAssetMgr.h"
#include "CLevelMgr.h"
#include "CLevel.h"
#include "CRenderComponent.h"
#include "CTexturePool.h"
#include "CShadowMap.h"

CCamera::CCamera()
	: CComponent(COMPONENT_TYPE::CAMERA),
	m_ProjType(PROJ_TYPE::PERSPECTIVE), m_CamPriority(-1),
	m_FOV(XM_PI / 3.f), m_Far(10000.f), m_Width(0.f), m_AspectRatio(1.f),
	m_Scale(1.f), m_IsDirty(false)
{
	m_Width = CDevice::GetInst()->GetRenderResolution().x;
	m_AspectRatio = CDevice::GetInst()->GetAspectRatio();
}

CCamera::CCamera(const CCamera& other)
	: CComponent(other),
	m_ProjType(other.m_ProjType),
	m_CamPriority(-1),
	m_FOV(other.m_FOV),
	m_Far(other.m_Far),
	m_Width(other.m_Width),
	m_AspectRatio(other.m_AspectRatio),
	m_Scale(other.m_Scale),
	m_IsDirty(false)
{
}

CCamera::~CCamera()
{
}

void CCamera::Begin()
{

}

void CCamera::Tick()
{
	g_Global.EyePosW = GetOwner()->GetTransformComp()->GetWorldPosition();
}

void CCamera::FinalTick()
{
	// View
	Vector3 vCamWorldPos = GetOwner()->GetTransformComp()->GetRelativePosition();
	Matrix matViewTrans = XMMatrixTranslation(-vCamWorldPos.x, -vCamWorldPos.y, -vCamWorldPos.z);

	Vector3 vR = GetOwner()->GetTransformComp()->GetRelativeDir(DIR_TYPE::RIGHT);
	Vector3 vU = GetOwner()->GetTransformComp()->GetRelativeDir(DIR_TYPE::UP);
	Vector3 vF = GetOwner()->GetTransformComp()->GetRelativeDir(DIR_TYPE::FORWARD);

	Matrix matViewRot = XMMatrixIdentity();
	matViewRot._11 = vR.x; matViewRot._12 = vU.x; matViewRot._13 = vF.x;
	matViewRot._21 = vR.y; matViewRot._22 = vU.y; matViewRot._23 = vF.y;
	matViewRot._31 = vR.z; matViewRot._32 = vU.z; matViewRot._33 = vF.z;

	m_matView = matViewTrans * matViewRot;
	m_InvView = m_matView.Invert();

	// Projection
	if (m_ProjType == PROJ_TYPE::PERSPECTIVE)
		m_matProj = XMMatrixPerspectiveFovLH(m_FOV, m_AspectRatio, 0.1f, m_Far);
	else
		m_matProj = XMMatrixOrthographicLH(m_Scale * m_Width, m_Scale * m_Width / m_AspectRatio, 1.f, m_Far);

	BoundingFrustum::CreateFromMatrix(m_CamFrustum, m_matProj);

	g_Global.View = m_matView;
	g_Global.Proj = m_matProj;
	g_Global.ViewProj = m_matView * m_matProj;
}

void CCamera::Render()
{
	CLevelMgr::GetInst()->GetCurrentLevel()->SortObjects();
	std::unordered_map<OBJ_PSO_TYPE, std::vector<CGameObject*>>& sortedObjs = CLevelMgr::GetInst()->GetCurrentLevel()->GetSortedObjects();

	if (m_IsDirty)
	{
		MarkDirty();
		m_IsDirty = false;
	}

	//CMDLIST->SetGraphicsRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"Default").Get());

	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->Bind_Graphics(0, 0);
	//CDevice::GetInst()->GetCurrFrameResource()->GetStructuredBuffer(SB_TYPE::MATERIAL)->Bind_Graphics_SRV_Table(CMDLIST.Get(), 2);
	//CTexturePool::GetInst()->Bind_Graphics_Table_2D(CMDLIST.Get(), 3);
	//CTexturePool::GetInst()->Bind_Graphics_Table_Array(CMDLIST.Get(), 4);
	//CTexturePool::GetInst()->Bind_Graphics_Table_Cube(CMDLIST.Get(), 6);

	CMDLIST->SetGraphicsRootDescriptorTable(7, CShadowMap::GetInst()->GetSrvGpuHandle());

	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_DEFAULT).Get());
	for (const auto& obj : sortedObjs[OBJ_PSO_TYPE::PSO_DEFAULT])
		TestFrustumAndRender(obj);

	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_SKY).Get());
	for (const auto& obj : sortedObjs[OBJ_PSO_TYPE::PSO_SKY])
		obj->Render();

	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_ALPHA_TESTED).Get());
	for (const auto& obj : sortedObjs[OBJ_PSO_TYPE::PSO_ALPHA_TESTED])
		TestFrustumAndRender(obj);

	
	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_TESS_QUAD_PATCH).Get());
	for (const auto& obj : sortedObjs[OBJ_PSO_TYPE::PSO_TESS_QUAD_PATCH])
		TestFrustumAndRender(obj);

	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_BILLBOARD).Get());
	for (const auto& obj : sortedObjs[OBJ_PSO_TYPE::PSO_BILLBOARD])
		TestFrustumAndRender(obj);

	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->Bind_Graphics(0, 0);

	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_TRANSPARENT).Get());
	for (const auto& obj : sortedObjs[OBJ_PSO_TYPE::PSO_TRANSPARENT])
		obj->Render();
		
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_WAVE_RENDER).Get());
	for (const auto& obj : sortedObjs[OBJ_PSO_TYPE::PSO_WAVE_RENDER])
		TestFrustumAndRender(obj);

	CMDLIST->SetGraphicsRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"Particle").Get());
		
	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_PARTICLE_RENDER).Get());
	for (const auto& obj : sortedObjs[OBJ_PSO_TYPE::PSO_PARTICLE_RENDER])
		obj->Render();
}

void CCamera::TestFrustumAndRender(CGameObject* obj)
{
	Matrix viewToLocal = m_InvView * obj->GetTransformComp()->GetInvWorld();
	BoundingFrustum localSpaceFrustum;
	m_CamFrustum.Transform(localSpaceFrustum, viewToLocal);
	if (localSpaceFrustum.Contains(obj->GetRenderComp()->GetBounds()) != DISJOINT)
		obj->Render();
}

void CCamera::MarkDirty()
{
	for (const auto& obj : m_vecObjs)
	{
		if (obj->GetTransformComp() != nullptr)
			obj->GetTransformComp()->ResetDirty();
	}
}

void CCamera::SetDirty()
{
	m_IsDirty = true;
}

void CCamera::SetCameraPriority(int priority, const std::wstring& levelname)
{
	m_CamPriority = priority;

	CRenderMgr::GetInst()->RegisterLevelCamera(levelname, this, m_CamPriority);
}