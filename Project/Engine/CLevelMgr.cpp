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
	pLandGrid->SetSubMeshGeo(pLandGrid->GetMeshRenderComp()->GetMesh()->GetSubGeo("grid"));

	pWaveLevel->AddObject(pLandGrid);



	//CGameObject* pWave = new CGameObject;
	//pWave->SetName(L"Wave");
	//pWave->AddComponent(new CTransform);
	//pWave->AddComponent(new CMeshRender);
	//
	//pWave->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	//pWave->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	//pWave->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	//pWave->GetTransformComp()->SetObjCBIndex(1);
	//
	//pWave->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"WaveMesh"));
	//pWave->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"WaterMaterial"));
	//pWave->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_TRANSPARENT);
	//pWave->SetSubMeshGeo(pWave->GetMeshRenderComp()->GetMesh()->GetSubGeo("grid"));
	//
	//pWaveLevel->AddObject(pWave);

	CWaveObject* pWave = CAssetMgr::GetInst()->GetWaveObject();
	pWave->SetName(L"Wave");
	pWave->AddComponent(new CTransform);
	pWave->AddComponent(new CMeshRender);

	pWave->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pWave->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pWave->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pWave->GetTransformComp()->SetObjCBIndex(1);

	pWave->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"WaveMesh"));
	pWave->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"WaterMaterial"));
	pWave->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_WAVE_RENDER);
	pWave->SetSubMeshGeo(pWave->GetMeshRenderComp()->GetMesh()->GetSubGeo("grid"));

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
	pWoodBox->SetSubMeshGeo(pWoodBox->GetMeshRenderComp()->GetMesh()->GetSubGeo("box"));

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
	//pTreeBillboard->SetSubMeshGeo(pTreeBillboard->GetMeshRenderComp()->GetMesh()->GetSubGeo("grid"));

	pWaveLevel->AddObject(pTreeBillboard);

	AddLevel(L"WaveLevel", pWaveLevel);
	/******************************************************************/
	// Wave Level
	/******************************************************************/



	/******************************************************************/
	// Scene Level
	/******************************************************************/
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
	pSceneGrid->GetTransformComp()->SetObjCBIndex(0);
	pSceneGrid->GetTransformComp()->SetTexTransform(Matrix::CreateScale(5.f, 5.f, 1.f));

	pSceneGrid->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
	pSceneGrid->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"TileMaterial"));
	pSceneGrid->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
	pSceneGrid->SetSubMeshGeo(pSceneGrid->GetMeshRenderComp()->GetMesh()->GetSubGeo("grid"));

	pSceneLevel->AddObject(pSceneGrid);

	CGameObject* pBox = new CGameObject;
	pBox->SetName(L"Box");
	pBox->AddComponent(new CTransform);
	pBox->AddComponent(new CMeshRender);

	pBox->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pBox->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pBox->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pBox->GetTransformComp()->SetObjCBIndex(1);

	pBox->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
	pBox->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"WoodBoxMaterial"));
	pBox->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
	pBox->SetSubMeshGeo(pBox->GetMeshRenderComp()->GetMesh()->GetSubGeo("box"));

	pSceneLevel->AddObject(pBox);

	CGameObject* pSkull = new CGameObject;
	pSkull->SetName(L"Skull");
	pSkull->AddComponent(new CTransform);
	pSkull->AddComponent(new CMeshRender);

	pSkull->GetTransformComp()->SetRelativePosition(0.f, 1.f, 0.f);
	pSkull->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pSkull->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pSkull->GetTransformComp()->SetObjCBIndex(2);

	pSkull->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SkullMesh"));
	pSkull->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"SkullMaterial"));
	pSkull->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);

	pSceneLevel->AddObject(pSkull);

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
		pCylinder->GetTransformComp()->SetObjCBIndex(3 + 4 * i);

		pCylinder->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
		pCylinder->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"BricksMaterial"));
		pCylinder->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
		pCylinder->SetSubMeshGeo(pCylinder->GetMeshRenderComp()->GetMesh()->GetSubGeo("cylinder"));

		pSceneLevel->AddObject(pCylinder);

		// right cylinder
		CGameObject* pCylinder2 = new CGameObject;
		pCylinder2->SetName(L"Cylinder");
		pCylinder2->AddComponent(new CTransform);
		pCylinder2->AddComponent(new CMeshRender);

		pCylinder2->GetTransformComp()->SetRelativePosition(8.f, 1.5f, 6.f - 3 * i);
		pCylinder2->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pCylinder2->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pCylinder2->GetTransformComp()->SetObjCBIndex(3 + 4 * i + 1);

		pCylinder2->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
		pCylinder2->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"BricksMaterial"));
		pCylinder2->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
		pCylinder2->SetSubMeshGeo(pCylinder2->GetMeshRenderComp()->GetMesh()->GetSubGeo("cylinder"));

		pSceneLevel->AddObject(pCylinder2);

		// left sphere
		CGameObject* pSphere = new CGameObject;
		pSphere->SetName(L"Cylinder");
		pSphere->AddComponent(new CTransform);
		pSphere->AddComponent(new CMeshRender);

		pSphere->GetTransformComp()->SetRelativePosition(-8.f, 3.5f, 6.f - 3 * i);
		pSphere->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pSphere->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pSphere->GetTransformComp()->SetObjCBIndex(3 + 4 * i + 2);

		pSphere->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
		pSphere->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"StoneMaterial"));
		pSphere->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
		pSphere->SetSubMeshGeo(pSphere->GetMeshRenderComp()->GetMesh()->GetSubGeo("sphere"));

		pSceneLevel->AddObject(pSphere);

		// right sphere
		CGameObject* pSphere2 = new CGameObject;
		pSphere2->SetName(L"Cylinder");
		pSphere2->AddComponent(new CTransform);
		pSphere2->AddComponent(new CMeshRender);

		pSphere2->GetTransformComp()->SetRelativePosition(8.f, 3.5f, 6.f - 3 * i);
		pSphere2->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pSphere2->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pSphere2->GetTransformComp()->SetObjCBIndex(3 + 4 * i + 3);

		pSphere2->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SceneGeoMesh"));
		pSphere2->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"StoneMaterial"));
		pSphere2->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
		pSphere2->SetSubMeshGeo(pSphere2->GetMeshRenderComp()->GetMesh()->GetSubGeo("sphere"));

		pSceneLevel->AddObject(pSphere2);
	}

	AddLevel(L"SceneLevel", pSceneLevel);
	/******************************************************************/
	// Scene Level
	/******************************************************************/


	/******************************************************************/
	// Room Level
	/******************************************************************/
	CLevel* pRoomLevel = new CLevel;

	CGameObject* pCamera3 = new CGameObject;
	pCamera3->SetName(L"RoomMainCamera");
	pCamera3->AddComponent(new CTransform);
	pCamera3->AddComponent(new CCamera);
	pCamera3->AddComponent(new CCameraMoveScript);
	pCamera3->GetCameraComp()->SetProjType(PROJ_TYPE::PERSPECTIVE);

	pCamera3->GetCameraComp()->SetCameraPriority(0, L"RoomLevel");

	pCamera3->GetTransformComp()->SetRelativePosition(0.f, 5.f, -15.f);

	pRoomLevel->AddObject(pCamera3);

	CGameObject* pRoomFloor = new CGameObject;
	pRoomFloor->SetName(L"RoomFloor");
	pRoomFloor->AddComponent(new CTransform);
	pRoomFloor->AddComponent(new CMeshRender);

	pRoomFloor->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pRoomFloor->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pRoomFloor->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pRoomFloor->GetTransformComp()->SetObjCBIndex(0);

	pRoomFloor->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RoomMesh"));
	pRoomFloor->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"TileMaterial"));
	pRoomFloor->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
	pRoomFloor->SetSubMeshGeo(pRoomFloor->GetMeshRenderComp()->GetMesh()->GetSubGeo("floor"));

	pRoomLevel->AddObject(pRoomFloor);

	CGameObject* pRoomWall = new CGameObject;
	pRoomWall->SetName(L"RoomFloor");
	pRoomWall->AddComponent(new CTransform);
	pRoomWall->AddComponent(new CMeshRender);

	pRoomWall->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pRoomWall->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pRoomWall->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pRoomWall->GetTransformComp()->SetObjCBIndex(1);

	pRoomWall->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RoomMesh"));
	pRoomWall->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"BricksMaterial"));
	pRoomWall->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);
	pRoomWall->SetSubMeshGeo(pRoomWall->GetMeshRenderComp()->GetMesh()->GetSubGeo("wall"));

	pRoomLevel->AddObject(pRoomWall);

	CGameObject* pRoomSkull = new CGameObject;
	pRoomSkull->SetName(L"Skull");
	pRoomSkull->AddComponent(new CTransform);
	pRoomSkull->AddComponent(new CMeshRender);

	pRoomSkull->GetTransformComp()->SetRelativePosition(1.f, 1.5f, -3.f);
	pRoomSkull->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pRoomSkull->GetTransformComp()->SetRelativeScale(0.3f, 0.3f, 0.3f);
	pRoomSkull->GetTransformComp()->SetObjCBIndex(2);

	pRoomSkull->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SkullMesh"));
	pRoomSkull->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"SkullMaterial"));
	pRoomSkull->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_DEFAULT);

	pRoomLevel->AddObject(pRoomSkull);

	CGameObject* pReflectedSkull = pRoomSkull->Clone();
	pReflectedSkull->GetTransformComp()->SetObjCBIndex(3);
	pReflectedSkull->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_REFLECTIONS);

	pRoomLevel->AddObject(pReflectedSkull);

	CGameObject* pShadowedSkull = pRoomSkull->Clone();
	pShadowedSkull->GetTransformComp()->SetObjCBIndex(4);
	pShadowedSkull->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"ShadowMaterial"));
	pShadowedSkull->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_SHADOW);

	pRoomLevel->AddObject(pShadowedSkull);

	CGameObject* pRoomMirror = new CGameObject;
	pRoomMirror->SetName(L"RoomMirror");
	pRoomMirror->AddComponent(new CTransform);
	pRoomMirror->AddComponent(new CMeshRender);

	pRoomMirror->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pRoomMirror->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pRoomMirror->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pRoomMirror->GetTransformComp()->SetObjCBIndex(5);

	pRoomMirror->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RoomMesh"));
	pRoomMirror->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"MirrorMaterial"));
	pRoomMirror->GetMeshRenderComp()->SetObjPSOType(OBJ_PSO_TYPE::PSO_MIRRORS | OBJ_PSO_TYPE::PSO_TRANSPARENT);
	pRoomMirror->SetSubMeshGeo(pRoomMirror->GetMeshRenderComp()->GetMesh()->GetSubGeo("mirror"));

	pRoomLevel->AddObject(pRoomMirror);

	AddLevel(L"RoomLevel", pRoomLevel);
	/******************************************************************/
	// Room Level
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
		ChangeLevel(L"RoomLevel");
		CRenderMgr::GetInst()->SetCurrentLevel(L"RoomLevel");
		OnLevelChange.Broadcast();
	}

	if (m_CurLevel != nullptr)
	{
		if (m_CurLevel->GetState() == LEVEL_STATE::PLAY)
			m_CurLevel->Tick();
		m_CurLevel->RegisterClear();
		m_CurLevel->FinalTick();
	}

	//CMDLIST->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//CMDLIST->SetPipelineState(m_PSOGroup[m_CurrPSOType].Get());
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



