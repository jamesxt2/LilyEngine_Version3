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

	// Projection
	if (m_ProjType == PROJ_TYPE::PERSPECTIVE)
		m_matProj = XMMatrixPerspectiveFovLH(m_FOV, m_AspectRatio, 1.f, m_Far);
	else
		m_matProj = XMMatrixOrthographicLH(m_Scale * m_Width, m_Scale * m_Width / m_AspectRatio, 1.f, m_Far);
}

void CCamera::Render()
{
	SortObjects();

	if (m_IsDirty)
	{
		MarkDirty();
		m_IsDirty = false;
	}

	g_Object.View = m_matView;
	g_Object.Proj = m_matProj;
	g_Object.ViewProj = m_matView * m_matProj;

	CMDLIST->SetGraphicsRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"Default").Get());

	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->Bind_Graphics(0, 3);

	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_DEFAULT).Get());
	for (const auto& obj : m_SortedObjs[OBJ_PSO_TYPE::PSO_DEFAULT])
		obj->Render();

	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_ALPHA_TESTED).Get());
	for (const auto& obj : m_SortedObjs[OBJ_PSO_TYPE::PSO_ALPHA_TESTED])
		obj->Render();

	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_BILLBOARD).Get());
	for (const auto& obj : m_SortedObjs[OBJ_PSO_TYPE::PSO_BILLBOARD])
		obj->Render();

	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// the mirror area's pixel's stencils are set to 1
	CMDLIST->OMSetStencilRef(1);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_MIRRORS).Get());
	for (const auto& obj : m_SortedObjs[OBJ_PSO_TYPE::PSO_MIRRORS])
		obj->Render();

	// stencil still 1 for mirror, 0 for other, if 1, render to the mirror area
	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->Bind_Graphics(1, 3);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_REFLECTIONS).Get());
	for (const auto& obj : m_SortedObjs[OBJ_PSO_TYPE::PSO_REFLECTIONS])
		obj->Render();

	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->Bind_Graphics(0, 3);
	CMDLIST->OMSetStencilRef(0);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_TRANSPARENT).Get());
	for (const auto& obj : m_SortedObjs[OBJ_PSO_TYPE::PSO_TRANSPARENT])
		obj->Render();

	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_WAVE_RENDER).Get());
	for (const auto& obj : m_SortedObjs[OBJ_PSO_TYPE::PSO_WAVE_RENDER])
		obj->Render();

	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_SHADOW).Get());
	for (const auto& obj : m_SortedObjs[OBJ_PSO_TYPE::PSO_SHADOW])
		obj->Render();

	CMDLIST->SetGraphicsRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"Particle").Get());

	CMDLIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	CMDLIST->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_PARTICLE_RENDER).Get());
	for (const auto& obj : m_SortedObjs[OBJ_PSO_TYPE::PSO_PARTICLE_RENDER])
		obj->Render();

	for (auto& obj : m_SortedObjs)
		obj.second.clear();
}

void CCamera::SortObjects()
{
	m_vecObjs = CLevelMgr::GetInst()->GetCurrentLevel()->GetObjects();
	for (const auto& obj : m_vecObjs)
	{
		if (obj->GetRenderComp() == nullptr)
			continue;
		OBJ_PSO_TYPE type = obj->GetRenderComp()->GetObjPSOType();
		if ((uint32)(OBJ_PSO_TYPE::PSO_DEFAULT & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_DEFAULT].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_ALPHA_TESTED & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_ALPHA_TESTED].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_MIRRORS & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_MIRRORS].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_REFLECTIONS & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_REFLECTIONS].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_TRANSPARENT & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_TRANSPARENT].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_WIREFRAME & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_WIREFRAME].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_SHADOW & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_SHADOW].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_BILLBOARD & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_BILLBOARD].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_HORIZONTAL_BLUR & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_HORIZONTAL_BLUR].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_WAVE_RENDER & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_WAVE_RENDER].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_PARTICLE_RENDER & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_PARTICLE_RENDER].push_back(obj);
	}
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