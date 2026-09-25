#include "pch.h"
#include "CLevel.h"

#include "CGameObject.h"
#include "CDevice.h"
#include "CKeyMgr.h"
#include "CTimeMgr.h"

CLevel::CLevel()
	: m_State(LEVEL_STATE::STOP)
{

}

CLevel::~CLevel()
{
	Safe_Del_Vector(m_vecParent);
}

void CLevel::Begin()
{
	
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

	XMVECTOR lightDir = -SphericalToCartesian(1.0f, m_SunTheta, m_SunPhi);

	g_Global.Lights[0].Direction = lightDir;
	g_Global.Lights[0].Strength = Vector3(1.f, 1.f, 0.9f);

	lightDir = -SphericalToCartesian(1.0f, m_SunTheta + XM_PI, m_SunPhi);

	g_Global.Lights[1].Direction = lightDir;
	g_Global.Lights[1].Strength = Vector3(0.7f, 0.7f, 0.6f);

	g_Global.FogColor = Vector4(0.7f, 0.7f, 0.7f, 1.0f);
	g_Global.FogStart = 5.f;
	g_Global.FogRange = 150.f;

	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->Bind(0, 3);
	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->CopyData(0, &g_Global);

	TGlobal globalReflected = g_Global;

	Plane mirrorPlane(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
	Matrix R = Matrix::CreateReflection(mirrorPlane);

	// Reflect the lighting.
	for (int i = 0; i < 2; ++i)
	{
		Vector3 lightDir = g_Global.Lights[i].Direction;
		Vector3 reflectedLightDir = Vector3::TransformNormal(lightDir, R);
		globalReflected.Lights[i].Direction = reflectedLightDir;
	}

	// Reflected pass stored in index 1
	CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->CopyData(1, &globalReflected);
}

void CLevel::FinalTick()
{
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
	//if (nextState == LEVEL_STATE::STOP || nextState == LEVEL_STATE::PAUSE)
		//CRenderMgr::GetInst()->ChangeRenderMode(RENDER_MODE::EDITOR);

	if (m_State == nextState) return;
	assert(!(m_State == LEVEL_STATE::STOP && nextState == LEVEL_STATE::PAUSE));

	if (m_State == LEVEL_STATE::STOP && nextState == LEVEL_STATE::PLAY)
		Begin();

	m_State = nextState;
}
