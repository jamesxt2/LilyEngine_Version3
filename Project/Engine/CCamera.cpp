#include "pch.h"
#include "CCamera.h"

#include "CDevice.h"
#include "CGameObject.h"
#include "CTransform.h"
#include "CRenderMgr.h"
#include "CLevelMgr.h"
#include "CLevel.h"

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
	m_vecObjects = CLevelMgr::GetInst()->GetCurrentLevel()->GetObjects();

	if (m_IsDirty)
	{
		MarkDirty();
		m_IsDirty = false;
	}

	g_Trans.ViewProj = m_matView * m_matProj;

	for (size_t i = 0; i < m_vecObjects.size(); ++i)
	{
		if (m_vecObjects[i]->GetRenderComp() == nullptr)
			continue;
		m_vecObjects[i]->Render();
	}

	m_vecObjects.clear();
}

void CCamera::MarkDirty()
{
	for (size_t i = 0; i < m_vecObjects.size(); ++i)
	{
		if (m_vecObjects[i] == nullptr)
			continue;
		if (m_vecObjects[i]->GetRenderComp() == nullptr)
			continue;
		if (m_vecObjects[i]->GetTransformComp() != nullptr)
			m_vecObjects[i]->GetTransformComp()->ResetDirty();
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