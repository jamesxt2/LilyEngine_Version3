#include "pch.h"
#include "CLevel.h"

#include "CGameObject.h"
#include "CDevice.h"
#include "CKeyMgr.h"
#include "CTimeMgr.h"
#include "CRenderComponent.h"
#include "CShadowMap.h"

CLevel::CLevel()
	: m_State(LEVEL_STATE::STOP)
{
	m_SceneBounds.Center = XMFLOAT3(0.f, 0.f, 0.f);
	m_SceneBounds.Radius = sqrtf(10.f * 10.f + 15.f * 15.f);
}

CLevel::~CLevel()
{
	Utilities::Safe_Del_Vector(m_vecParent);
}

void CLevel::Begin()
{
	for (size_t i = 0; i < m_vecParent.size(); ++i)
		m_vecParent[i]->Begin();
}

void CLevel::Tick()
{
	if (KEY_PRESSED(KEY::LEFT))
		m_SunTheta -= CTimeMgr::GetInst()->DeltaTime();
	if (KEY_PRESSED(KEY::RIGHT))
		m_SunTheta += CTimeMgr::GetInst()->DeltaTime();
	if (KEY_PRESSED(KEY::UP))
		m_SunPhi += CTimeMgr::GetInst()->DeltaTime();
	if (KEY_PRESSED(KEY::DOWN))
		m_SunPhi -= CTimeMgr::GetInst()->DeltaTime();

	for (size_t i = 0; i < m_vecParent.size(); ++i)
	{
		m_vecParent[i]->Tick();
	}

	

	// global CB
	g_Global.AmbientLight = Vector4(0.35f, 0.35f, 0.45f, 1.0f);

	XMVECTOR lightDir = -Utilities::SphericalToCartesian(1.0f, m_SunTheta, m_SunPhi);

	Matrix Rot = XMMatrixRotationY(m_LightRotationAngle);

	lightDir = XMVector3TransformNormal(lightDir, Rot);

	g_Global.Lights[0].Direction = lightDir;
	g_Global.Lights[0].Strength = Vector3(1.f, 1.f, 1.f);

	lightDir = -Utilities::SphericalToCartesian(1.0f, m_SunTheta + XM_PI, m_SunPhi);


	UpdateShadowTransform();

	g_Global.Lights[1].Direction = lightDir;
	g_Global.Lights[1].Strength = Vector3(0.8f, 0.8f, 0.8f);

	g_Global.FogColor = Vector4(0.7f, 0.7f, 0.7f, 1.0f);
	g_Global.FogStart = 5.f;
	g_Global.FogRange = 150.f;

	g_Global.DeltaTime = CTimeMgr::GetInst()->DeltaTime();
	g_Global.TotalTime = CTimeMgr::GetInst()->TotalTime();

	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->CopyData(0, &g_Global);

}

void CLevel::FinalTick()
{
	m_LightRotationAngle += 0.1f * CTimeMgr::GetInst()->DeltaTime();

	std::vector<CGameObject*>::iterator iter = m_vecParent.begin();
	for (; iter != m_vecParent.end(); )
	{
		(*iter)->FinalTick();
		if ((*iter)->IsDead())
			iter = m_vecParent.erase(iter);
		else
			++iter;
	}
}

void CLevel::UpdateShadowTransform()
{
	Vector3 lightDir = g_Global.Lights[0].Direction;
	Vector3 lightPos = -2.f * m_SceneBounds.Radius * lightDir;
	Vector3 targetPos = m_SceneBounds.Center;
	Vector3 lightUp(0.f, 1.f, 0.f);
	Matrix lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	m_LightPosW = lightPos;

	Vector3 SphereCenterLS = XMVector3TransformCoord(targetPos, lightView);

	float l = SphereCenterLS.x - m_SceneBounds.Radius;
	float b = SphereCenterLS.y - m_SceneBounds.Radius;
	float n = SphereCenterLS.z - m_SceneBounds.Radius;
	float r = SphereCenterLS.x + m_SceneBounds.Radius;
	float t = SphereCenterLS.y + m_SceneBounds.Radius;
	float f = SphereCenterLS.z + m_SceneBounds.Radius;

	m_LightNearZ = n;
	m_LightFarZ = f;
	Matrix lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// change coord from [-1, 1] NDC to [0, 1] texture space
	Matrix T(0.5f, 0.f, 0.f, 0.f,
		0.f, -0.5f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.5f, 0.5f, 0.f, 1.f);

	Matrix S = lightView * lightProj * T;
	m_LightView = lightView;
	m_LightProj = lightProj;
	m_ShadowTransform = S;

	g_Global.ShadowTransform = m_ShadowTransform;

	TGlobal Shadow_Global;
	Shadow_Global.EyePosW = m_LightPosW;
	Shadow_Global.View = m_LightView;
	Shadow_Global.Proj = m_LightProj;
	Shadow_Global.ViewProj = m_LightView * m_LightProj;
	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->CopyData(1, &Shadow_Global);
}

void CLevel::RegisterClear()
{
	m_vecObject.clear();
}

void CLevel::AddObject(CGameObject* object, bool bChildMove)
{
	if (!object->GetParent())
		m_vecParent.push_back(object);
}

CGameObject* CLevel::FindObjectByName(const std::wstring& name)
{
	for (size_t j = 0; j < m_vecObject.size(); ++j)
	{
		if (m_vecObject[j]->GetName() == name)
			return m_vecObject[j];
	}

	return nullptr;
}

void CLevel::ChangeState(LEVEL_STATE nextState)
{
	if (m_State == nextState) return;
	assert(!(m_State == LEVEL_STATE::STOP && nextState == LEVEL_STATE::PAUSE));

	if (m_State == LEVEL_STATE::STOP && nextState == LEVEL_STATE::PLAY)
		Begin();

	m_State = nextState;
}

void CLevel::SortObjects()
{
	for (auto& pair : m_SortedObjs)
		pair.second.clear();
	for (const auto& obj : m_vecObject)
	{
		if (obj->GetRenderComp() == nullptr)
			continue;
		OBJ_PSO_TYPE type = obj->GetRenderComp()->GetObjPSOType();
		if ((uint32)(OBJ_PSO_TYPE::PSO_DEFAULT & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_DEFAULT].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_ALPHA_TESTED & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_ALPHA_TESTED].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_TRANSPARENT & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_TRANSPARENT].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_WIREFRAME & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_WIREFRAME].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_BILLBOARD & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_BILLBOARD].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_WAVE_RENDER & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_WAVE_RENDER].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_PARTICLE_RENDER & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_PARTICLE_RENDER].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_TESS_QUAD_PATCH & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_TESS_QUAD_PATCH].push_back(obj);
		if ((uint32)(OBJ_PSO_TYPE::PSO_SKY & type))
			m_SortedObjs[OBJ_PSO_TYPE::PSO_SKY].push_back(obj);
	}
}

void CLevel::BuildLevelShadowMap()
{
	SortObjects();
	CShadowMap::GetInst()->DrawSceneToShadowMap(m_SortedObjs[OBJ_PSO_TYPE::PSO_DEFAULT]);
}
