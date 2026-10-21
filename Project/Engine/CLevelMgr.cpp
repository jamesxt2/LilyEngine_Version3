#include "pch.h"
#include "CLevelMgr.h"

#include "CLevel.h"
#include "CGameObject.h"
#include "CTransform.h"
#include "CCamera.h"
#include "CCameraMoveScript.h"
#include "CMeshRender.h"
#include "CAssetMgr.h"
#include "CRenderMgr.h"
#include "CKeyMgr.h"
#include "CDevice.h"
#include "CConstantBuffer.h"
#include "CWaveObject.h"
#include "CParticleSystem.h"

CLevelMgr::CLevelMgr()
	: m_CurLevel(nullptr)
{
}

CLevelMgr::~CLevelMgr()
{
	for (auto& level : m_LevelMap)
	{
		if (level.second != nullptr)
			delete level.second;
	}
}


void CLevelMgr::Init()
{
	/******************************************************************/
	// Wave Level
	/******************************************************************/
	CLevel* pWaveLevel = new CLevel;

	// Camera object
	CGameObject* pCamera = new CGameObject;
	pCamera->SetName(L"MainCamera");
	pCamera->AddComponent(new CTransform);
	pCamera->AddComponent(new CCamera);
	pCamera->AddComponent(new CCameraMoveScript);
	pCamera->GetCameraComp()->SetProjType(PROJ_TYPE::PERSPECTIVE);

	pCamera->GetCameraComp()->SetCameraPriority(0, L"WaveLevel");

	pCamera->GetTransformComp()->SetRelativePosition(0.f, 1.f, -1.f);

	pWaveLevel->AddObject(pCamera);

	// object

	CGameObject* pLandGrid = new CGameObject;
	pLandGrid->SetName(L"WaveGrid");
	pLandGrid->AddComponent(new CTransform);
	pLandGrid->AddComponent(new CMeshRender);

	pLandGrid->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pLandGrid->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pLandGrid->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pLandGrid->GetTransformComp()->SetObjCBIndex(0);
	pLandGrid->GetTransformComp()->SetTexTransform(Matrix::CreateScale(5.f, 5.f, 1.f));

	pLandGrid->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"LandMesh"));
	pLandGrid->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"GrassMaterial"));
	pLandGrid->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
	pLandGrid->GetMeshRenderComp()->SetSubMeshGeo(pLandGrid->GetMeshRenderComp()->GetMesh()->GetSubGeo(L"grid"));

	pWaveLevel->AddObject(pLandGrid);

	CWaveObject* pWave = new CWaveObject(256, 256, 0.25f, 0.03f, 2.0f, 0.2f);
	pWave->SetName(L"Wave");
	pWave->AddComponent(new CTransform);
	pWave->AddComponent(new CMeshRender);
	
	pWave->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pWave->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pWave->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pWave->GetTransformComp()->SetObjCBIndex(1);
	
	pWave->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->CreateGridMesh(L"WaveMesh", 160.f, 160.f, 256, 256));
	pWave->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"WaterMaterial"));
	pWave->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_WAVE_RENDER);
	
	pWaveLevel->AddObject(pWave);

	CGameObject* pWoodBox = new CGameObject;
	pWoodBox->SetName(L"WoodBox");
	pWoodBox->AddComponent(new CTransform);
	pWoodBox->AddComponent(new CMeshRender);

	pWoodBox->GetTransformComp()->SetRelativePosition(-6.f, 0.75f, -4.f);
	pWoodBox->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pWoodBox->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pWoodBox->GetTransformComp()->SetObjCBIndex(2);

	pWoodBox->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
	pWoodBox->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"WireFenceBoxMaterial"));
	pWoodBox->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_ALPHA_TESTED);
	pWoodBox->GetMeshRenderComp()->SetSubMeshGeo(pWoodBox->GetMeshRenderComp()->GetMesh()->GetSubGeo(L"box"));

	pWaveLevel->AddObject(pWoodBox);

	CGameObject* pTreeBillboard = new CGameObject;
	pTreeBillboard->SetName(L"WaveGrid");
	pTreeBillboard->AddComponent(new CTransform);
	pTreeBillboard->AddComponent(new CMeshRender);
	
	pTreeBillboard->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pTreeBillboard->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pTreeBillboard->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pTreeBillboard->GetTransformComp()->SetObjCBIndex(3);
	
	pTreeBillboard->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"TreeBillboardsMesh"));
	pTreeBillboard->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"TreeBillboardMaterial"));
	pTreeBillboard->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_BILLBOARD);

	pWaveLevel->AddObject(pTreeBillboard);

	CGameObject* pParticle = new CGameObject;
	pParticle->SetName(L"Particle");
	pParticle->AddComponent(new CTransform);
	
	pParticle->AddComponent(new CParticleSystem(10000, CAssetMgr::GetInst()->FindAsset<CTexture>(L"NoiseTexture")->GetPoolIndex()));
	
	pParticle->GetTransformComp()->SetRelativePosition(30.f, 60.f, 0.f);
	pParticle->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pParticle->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pParticle->GetTransformComp()->SetObjCBIndex(4);
	
	pParticle->GetRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"PointMesh"));
	pParticle->GetRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"AlphaCircleMaterial"));
	pParticle->GetRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_PARTICLE_RENDER);
	
	pWaveLevel->AddObject(pParticle);

	AddLevel(L"WaveLevel", pWaveLevel);
	/******************************************************************/
	// Wave Level
	/******************************************************************/



	/******************************************************************/
	// Scene Level
	/******************************************************************/
	int sceneLevelObjIdx = 0;

	CLevel* pSceneLevel = new CLevel;

	CGameObject* pCamera2 = new CGameObject;
	pCamera2->SetName(L"SceneMainCamera");
	pCamera2->AddComponent(new CTransform);
	pCamera2->AddComponent(new CCamera);
	pCamera2->AddComponent(new CCameraMoveScript);
	pCamera2->GetCameraComp()->SetProjType(PROJ_TYPE::PERSPECTIVE);
	
	pCamera2->GetCameraComp()->SetCameraPriority(0, L"SceneLevel");
	
	pCamera2->GetTransformComp()->SetRelativePosition(0.f, 5.f, -15.f);
	
	pSceneLevel->AddObject(pCamera2);

	CGameObject* pSceneGrid = new CGameObject;
	pSceneGrid->SetName(L"SceneGrid");
	pSceneGrid->AddComponent(new CTransform);
	pSceneGrid->AddComponent(new CMeshRender);

	pSceneGrid->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pSceneGrid->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pSceneGrid->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pSceneGrid->GetTransformComp()->SetObjCBIndex(sceneLevelObjIdx++);
	pSceneGrid->GetTransformComp()->SetTexTransform(Matrix::CreateScale(5.f, 5.f, 1.f));

	pSceneGrid->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
	pSceneGrid->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"TileMaterial"));
	pSceneGrid->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
	pSceneGrid->GetMeshRenderComp()->SetSubMeshGeo(pSceneGrid->GetMeshRenderComp()->GetMesh()->GetSubGeo(L"grid"));

	pSceneLevel->AddObject(pSceneGrid);

	CGameObject* pBox = new CGameObject;
	pBox->SetName(L"Box");
	pBox->AddComponent(new CTransform);
	pBox->AddComponent(new CMeshRender);

	pBox->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pBox->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pBox->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pBox->GetTransformComp()->SetObjCBIndex(sceneLevelObjIdx++);

	pBox->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
	pBox->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"WoodBoxMaterial"));
	pBox->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
	pBox->GetMeshRenderComp()->SetSubMeshGeo(pBox->GetMeshRenderComp()->GetMesh()->GetSubGeo(L"box"));

	pSceneLevel->AddObject(pBox);

	CGameObject* pSkull = new CGameObject;
	pSkull->SetName(L"Skull");
	pSkull->AddComponent(new CTransform);
	pSkull->AddComponent(new CMeshRender);

	pSkull->GetTransformComp()->SetRelativePosition(0.f, 1.f, 0.f);
	pSkull->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pSkull->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pSkull->GetTransformComp()->SetObjCBIndex(sceneLevelObjIdx++);

	pSkull->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SkullMesh"));
	pSkull->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"SkullMaterial"));
	pSkull->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);

	pSceneLevel->AddObject(pSkull);

	CGameObject* pSky = new CGameObject;
	pSky->SetName(L"Sky");
	pSky->AddComponent(new CTransform);
	pSky->AddComponent(new CMeshRender);

	pSky->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pSky->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pSky->GetTransformComp()->SetRelativeScale(5000.f, 5000.f, 5000.f);
	pSky->GetTransformComp()->SetObjCBIndex(sceneLevelObjIdx++);

	pSky->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
	pSky->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"SkyMaterial"));
	pSky->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_SKY);
	pSky->GetMeshRenderComp()->SetSubMeshGeo(pSky->GetMeshRenderComp()->GetMesh()->GetSubGeo(L"sphere"));

	pSceneLevel->AddObject(pSky);

	for (int i = 0; i < 5; ++i)
	{
		// left cylinder
		CGameObject* pCylinder = new CGameObject;
		pCylinder->SetName(L"Cylinder");
		pCylinder->AddComponent(new CTransform);
		pCylinder->AddComponent(new CMeshRender);

		pCylinder->GetTransformComp()->SetRelativePosition(-8.f, 1.5f, 6.f - 3 * i);
		pCylinder->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pCylinder->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pCylinder->GetTransformComp()->SetObjCBIndex(sceneLevelObjIdx++);

		pCylinder->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
		if (i % 2 == 0)
			pCylinder->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"BricksMaterial"));
		else
			pCylinder->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"BricksNoNormalMaterial"));
		pCylinder->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
		pCylinder->GetMeshRenderComp()->SetSubMeshGeo(pCylinder->GetMeshRenderComp()->GetMesh()->GetSubGeo(L"cylinder"));

		pSceneLevel->AddObject(pCylinder);

		// right cylinder
		CGameObject* pCylinder2 = new CGameObject;
		pCylinder2->SetName(L"Cylinder");
		pCylinder2->AddComponent(new CTransform);
		pCylinder2->AddComponent(new CMeshRender);

		pCylinder2->GetTransformComp()->SetRelativePosition(8.f, 1.5f, 6.f - 3 * i);
		pCylinder2->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pCylinder2->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pCylinder2->GetTransformComp()->SetObjCBIndex(sceneLevelObjIdx++);

		pCylinder2->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
		pCylinder2->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"BricksMaterial"));
		pCylinder2->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
		pCylinder2->GetMeshRenderComp()->SetSubMeshGeo(pCylinder2->GetMeshRenderComp()->GetMesh()->GetSubGeo(L"cylinder"));

		pSceneLevel->AddObject(pCylinder2);

		// left sphere
		CGameObject* pSphere = new CGameObject;
		pSphere->SetName(L"Cylinder");
		pSphere->AddComponent(new CTransform);
		pSphere->AddComponent(new CMeshRender);

		pSphere->GetTransformComp()->SetRelativePosition(-8.f, 3.5f, 6.f - 3 * i);
		pSphere->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pSphere->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pSphere->GetTransformComp()->SetObjCBIndex(sceneLevelObjIdx++);

		pSphere->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
		pSphere->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"MirrorPlaneMaterial"));
		pSphere->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
		pSphere->GetMeshRenderComp()->SetSubMeshGeo(pSphere->GetMeshRenderComp()->GetMesh()->GetSubGeo(L"sphere"));

		pSceneLevel->AddObject(pSphere);

		// right sphere
		CGameObject* pSphere2 = new CGameObject;
		pSphere2->SetName(L"Cylinder");
		pSphere2->AddComponent(new CTransform);
		pSphere2->AddComponent(new CMeshRender);

		pSphere2->GetTransformComp()->SetRelativePosition(8.f, 3.5f, 6.f - 3 * i);
		pSphere2->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pSphere2->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pSphere2->GetTransformComp()->SetObjCBIndex(sceneLevelObjIdx++);

		pSphere2->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
		pSphere2->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"StoneMaterial"));
		pSphere2->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
		pSphere2->GetMeshRenderComp()->SetSubMeshGeo(pSphere2->GetMeshRenderComp()->GetMesh()->GetSubGeo(L"sphere"));

		pSceneLevel->AddObject(pSphere2);
	}

	AddLevel(L"SceneLevel", pSceneLevel);
	
	/******************************************************************/
	// Scene Level
	/******************************************************************/


	

	

	

	
	


	/******************************************************************/
	// Tess Level
	/******************************************************************/

	CLevel* pTessLevel = new CLevel;

	CGameObject* pCamera4 = new CGameObject;
	pCamera4->SetName(L"TessMainCamera");
	pCamera4->AddComponent(new CTransform);
	pCamera4->AddComponent(new CCamera);
	pCamera4->AddComponent(new CCameraMoveScript);
	pCamera4->GetCameraComp()->SetProjType(PROJ_TYPE::PERSPECTIVE);
		   
	pCamera4->GetCameraComp()->SetCameraPriority(0, L"TessLevel");
		   
	pCamera4->GetTransformComp()->SetRelativePosition(0.f, 5.f, -15.f);

	pTessLevel->AddObject(pCamera4);

	CGameObject* pTessGrid = new CGameObject;
	pTessGrid->SetName(L"TessGrid");
	pTessGrid->AddComponent(new CTransform);
	pTessGrid->AddComponent(new CMeshRender);

	pTessGrid->GetTransformComp()->SetRelativePosition(0.f, -5.f, 0.f);
	pTessGrid->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pTessGrid->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pTessGrid->GetTransformComp()->SetObjCBIndex(0);

	pTessGrid->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->CreateTessQuadPatchMesh(L"TessQuadPatch"));
	pTessGrid->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_TESS_QUAD_PATCH);

	pTessLevel->AddObject(pTessGrid);

	AddLevel(L"TessLevel", pTessLevel);
	/******************************************************************/
	// Tess Level
	/******************************************************************/

	ChangeLevel(L"WaveLevel");
	CRenderMgr::GetInst()->SetCurrentLevel(L"WaveLevel");
}

void CLevelMgr::Tick()
{
	if (KEY_TAP(KEY::_1))
	{
		ChangeLevel(L"WaveLevel");
		CRenderMgr::GetInst()->SetCurrentLevel(L"WaveLevel");
		OnLevelChange.Broadcast();
	}
	if (KEY_TAP(KEY::_2))
	{
		ChangeLevel(L"SceneLevel");
		CRenderMgr::GetInst()->SetCurrentLevel(L"SceneLevel");
		OnLevelChange.Broadcast();
	}
	if (KEY_TAP(KEY::_3))
	{
		ChangeLevel(L"TessLevel");
		CRenderMgr::GetInst()->SetCurrentLevel(L"TessLevel");
		OnLevelChange.Broadcast();
	}

	if (m_CurLevel != nullptr)
	{
		if (m_CurLevel->GetState() == LEVEL_STATE::PLAY)
			m_CurLevel->Tick();
		m_CurLevel->RegisterClear();
		m_CurLevel->FinalTick();
	}
}

void CLevelMgr::ChangeLevel(const std::wstring& name)
{
	std::unordered_map<std::wstring, CLevel*>::iterator iter = m_LevelMap.find(name);
	if (iter == m_LevelMap.end())
		return;
	if (m_CurLevel != nullptr)
	{
		m_CurLevel->ChangeState(LEVEL_STATE::STOP);
	}
	m_CurLevel = iter->second;
	iter->second->ChangeState(LEVEL_STATE::PLAY);
	CRenderMgr::GetInst()->SetCurrentLevel(name);
}



