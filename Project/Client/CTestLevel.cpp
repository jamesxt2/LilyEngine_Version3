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
	pCube->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());

	pLevel->AddObject(pCube);

	CGameObject* pCube2 = new CGameObject;
	pCube2->SetName(L"Cube2");
	pCube2->AddComponent(new CTransform);
	pCube2->AddComponent(new CMeshRender);

	pCube2->GetTransformComp()->SetRelativePosition(50.f, 10.f, 100.f);
	pCube2->GetTransformComp()->SetRelativeRotation(1.f, 1.f, 0.5f);
	pCube2->GetTransformComp()->SetRelativeScale(20.f, 20.f, 20.f);
	pCube2->GetTransformComp()->SetObjCBIndex(1);

	pCube2->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"CubeMesh"));
	pCube2->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());

	pLevel->AddObject(pCube2);

	CGameObject* pCube3 = new CGameObject;
	pCube3->SetName(L"Cube3");
	pCube3->AddComponent(new CTransform);
	pCube3->AddComponent(new CMeshRender);
	
	pCube3->GetTransformComp()->SetRelativePosition(-100.f, 10.f, 100.f);
	pCube3->GetTransformComp()->SetRelativeRotation(1.f, 1.f, 0.5f);
	pCube3->GetTransformComp()->SetRelativeScale(20.f, 20.f, 20.f);
	pCube3->GetTransformComp()->SetObjCBIndex(2);
	
	pCube3->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"CubeMesh"));
	pCube3->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());

	pLevel->AddObject(pCube3);

	pLevel->ChangeState(LEVEL_STATE::PLAY);
}
