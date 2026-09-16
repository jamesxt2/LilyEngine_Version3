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

	pGrid->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"DefaultGeoMesh"));
	pGrid->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());
	pGrid->SetSubMeshGeo(pGrid->GetMeshRenderComp()->GetMesh()->GetSubGeo("grid"));

	pLevel->AddObject(pGrid);

	CGameObject* pBox = new CGameObject;
	pBox->SetName(L"Box");
	pBox->AddComponent(new CTransform);
	pBox->AddComponent(new CMeshRender);

	pBox->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pBox->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pBox->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pBox->GetTransformComp()->SetObjCBIndex(1);

	pBox->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"DefaultGeoMesh"));
	pBox->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());
	pBox->SetSubMeshGeo(pBox->GetMeshRenderComp()->GetMesh()->GetSubGeo("box"));

	pLevel->AddObject(pBox);

	for (int i = 0; i < 5; ++i)
	{
		// left cylinder
		CGameObject* pCylinder = new CGameObject;
		pCylinder->SetName(L"Cylinder");
		pCylinder->AddComponent(new CTransform);
		pCylinder->AddComponent(new CMeshRender);

		pCylinder->GetTransformComp()->SetRelativePosition(-4.f, 1.5f, 6.f - 3 * i);
		pCylinder->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pCylinder->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pCylinder->GetTransformComp()->SetObjCBIndex(2 + 4 * i);

		pCylinder->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"DefaultGeoMesh"));
		pCylinder->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());
		pCylinder->SetSubMeshGeo(pCylinder->GetMeshRenderComp()->GetMesh()->GetSubGeo("cylinder"));

		pLevel->AddObject(pCylinder);

		// right cylinder
		CGameObject* pCylinder2 = new CGameObject;
		pCylinder2->SetName(L"Cylinder");
		pCylinder2->AddComponent(new CTransform);
		pCylinder2->AddComponent(new CMeshRender);

		pCylinder2->GetTransformComp()->SetRelativePosition(4.f, 1.5f, 6.f - 3 * i);
		pCylinder2->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pCylinder2->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pCylinder2->GetTransformComp()->SetObjCBIndex(2 + 4 * i + 1);

		pCylinder2->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"DefaultGeoMesh"));
		pCylinder2->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());
		pCylinder2->SetSubMeshGeo(pCylinder2->GetMeshRenderComp()->GetMesh()->GetSubGeo("cylinder"));

		pLevel->AddObject(pCylinder2);

		// left sphere
		CGameObject* pSphere = new CGameObject;
		pSphere->SetName(L"Cylinder");
		pSphere->AddComponent(new CTransform);
		pSphere->AddComponent(new CMeshRender);

		pSphere->GetTransformComp()->SetRelativePosition(-4.f, 3.5f, 6.f - 3 * i);
		pSphere->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pSphere->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pSphere->GetTransformComp()->SetObjCBIndex(2 + 4 * i + 2);

		pSphere->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"DefaultGeoMesh"));
		pSphere->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());
		pSphere->SetSubMeshGeo(pSphere->GetMeshRenderComp()->GetMesh()->GetSubGeo("sphere"));

		pLevel->AddObject(pSphere);

		// right sphere
		CGameObject* pSphere2 = new CGameObject;
		pSphere2->SetName(L"Cylinder");
		pSphere2->AddComponent(new CTransform);
		pSphere2->AddComponent(new CMeshRender);

		pSphere2->GetTransformComp()->SetRelativePosition(4.f, 3.5f, 6.f - 3 * i);
		pSphere2->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
		pSphere2->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
		pSphere2->GetTransformComp()->SetObjCBIndex(2 + 4 * i + 3);

		pSphere2->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"DefaultGeoMesh"));
		pSphere2->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"ColorShader").Get());
		pSphere2->SetSubMeshGeo(pSphere2->GetMeshRenderComp()->GetMesh()->GetSubGeo("sphere"));

		pLevel->AddObject(pSphere2);
	}
	

	pLevel->ChangeState(LEVEL_STATE::PLAY);
}
