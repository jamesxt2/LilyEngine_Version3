#include "pch.h"
#include "CTestLevel.h"

#include <Engine/CLevel.h>
#include <Engine/CGameObject.h>
#include <Engine/CTransform.h>
#include <Engine/CCamera.h>
#include <Engine/CMeshRender.h>
#include <Engine/CAssetMgr.h>
#include <Engine/CLevelMgr.h>

void CTestLevel::CreateTestLevel()
{
	CLevel* pLevel = new CLevel;

	CLevelMgr::GetInst()->ChangeLevel(pLevel);

	// Camera object
	CGameObject* pCamera = new CGameObject;
	pCamera->SetName(L"MainCamera");
	pCamera->AddComponent(new CTransform);
	pCamera->AddComponent(new CCamera);
	pCamera->GetCameraComp()->SetProjType(PROJ_TYPE::PERSPECTIVE);

	pCamera->GetCameraComp()->SetCameraPriority(0);

	pLevel->AddObject(pCamera);

	// Cube object

	CGameObject* pCube = new CGameObject;
	pCube->SetName(L"Cube");
	pCube->AddComponent(new CTransform);
	pCube->AddComponent(new CMeshRender);

	pCube->GetTransformComp()->SetRelativePosition(10.f, 10.f, 100.f);
	pCube->GetTransformComp()->SetRelativeRotation(1.f, 1.f, 0.5f);
	pCube->GetTransformComp()->SetRelativeScale(20.f, 20.f, 20.f);

	pCube->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"CubeMesh"));
	pCube->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader"));

	pLevel->AddObject(pCube);
	//pLevel->RegisterObject(pCube);

	pLevel->ChangeState(LEVEL_STATE::PLAY);
}
