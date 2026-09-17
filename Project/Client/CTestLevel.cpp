#include "pch.h"
#include "CTestLevel.h"

#include <Engine/CLevel.h>
#include <Engine/CGameObject.h>
#include <Engine/CTransform.h>
#include <Engine/CCamera.h>
#include <Engine/CMeshRender.h>
#include <Engine/CAssetMgr.h>
#include <Engine/CLevelMgr.h>
#include <Engine/CCameraMoveScript.h>

void CTestLevel::CreateTestLevel()
{
	CLevel* pLevel = new CLevel;

	CLevelMgr::GetInst()->ChangeLevel(pLevel);

	// Camera object
	CGameObject* pCamera = new CGameObject;
	pCamera->SetName(L"MainCamera");
	pCamera->AddComponent(new CTransform);
	pCamera->AddComponent(new CCamera);
	pCamera->AddComponent(new CCameraMoveScript);
	pCamera->GetCameraComp()->SetProjType(PROJ_TYPE::PERSPECTIVE);

	pCamera->GetCameraComp()->SetCameraPriority(0);

	pCamera->GetTransformComp()->SetRelativePosition(0.f, 1.f, -1.f);

	pLevel->AddObject(pCamera);

	// object

	CGameObject* pGrid = new CGameObject;
	pGrid->SetName(L"Grid");
	pGrid->AddComponent(new CTransform);
	pGrid->AddComponent(new CMeshRender);

	pGrid->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pGrid->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pGrid->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pGrid->GetTransformComp()->SetObjCBIndex(0);

	pGrid->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"LandMesh"));
	pGrid->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());
	pGrid->SetSubMeshGeo(pGrid->GetMeshRenderComp()->GetMesh()->GetSubGeo("grid"));

	pLevel->AddObject(pGrid);



	CGameObject* pWave = new CGameObject;
	pWave->SetName(L"Wave");
	pWave->AddComponent(new CTransform);
	pWave->AddComponent(new CMeshRender);

	pWave->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pWave->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pWave->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pWave->GetTransformComp()->SetObjCBIndex(1);

	pWave->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"WaveMesh"));
	pWave->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());
	pWave->SetSubMeshGeo(pWave->GetMeshRenderComp()->GetMesh()->GetSubGeo("grid"));

	pLevel->AddObject(pWave);


	pLevel->ChangeState(LEVEL_STATE::PLAY);
}
