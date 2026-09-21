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

CLevelMgr::CLevelMgr()
	: m_CurLevel(nullptr), m_CurrPSOType(OBJ_PSO_TYPE::COLOR_DEFAULT)
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

void CLevelMgr::BuildPSO()
{
	ComPtr<ID3D12RootSignature> rootSignature(CDevice::GetInst()->GetRootSignature());

	assert(rootSignature);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	Ptr<CGraphicsShader> shader = CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader");
	psoDesc.InputLayout = { shader->GetInputLayout().data(), (UINT)shader->GetInputLayout().size()};
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(shader->GetVsByteCode()->GetBufferPointer()),
		shader->GetVsByteCode()->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(shader->GetPsByteCode()->GetBufferPointer()),
		shader->GetPsByteCode()->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = CDevice::GetInst()->EnableMSAA() ? 4 : 1;
	psoDesc.SampleDesc.Quality = CDevice::GetInst()->EnableMSAA() ? CDevice::GetInst()->Get4xMSAAQuality() - 1 : 0;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::COLOR_DEFAULT])));

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::COLOR_WIREFRAME])));
}



void CLevelMgr::Init()
{
	BuildPSO();

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
	pLandGrid->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());
	pLandGrid->SetSubMeshGeo(pLandGrid->GetMeshRenderComp()->GetMesh()->GetSubGeo("grid"));

	pWaveLevel->AddObject(pLandGrid);



	CGameObject* pWave = new CGameObject;
	pWave->SetName(L"Wave");
	pWave->AddComponent(new CTransform);
	pWave->AddComponent(new CMeshRender);

	pWave->GetTransformComp()->SetRelativePosition(0.f, 0.f, 0.f);
	pWave->GetTransformComp()->SetRelativeRotation(0.f, 0.f, 0.f);
	pWave->GetTransformComp()->SetRelativeScale(1.f, 1.f, 1.f);
	pWave->GetTransformComp()->SetObjCBIndex(1);

	pWave->GetMeshRenderComp()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"WaveMesh"));
	pWave->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"WaterMaterial"));
	pWave->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());
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
	pWoodBox->GetMeshRenderComp()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"WoodBoxMaterial"));
	pWoodBox->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());
	pWoodBox->SetSubMeshGeo(pWoodBox->GetMeshRenderComp()->GetMesh()->GetSubGeo("box"));

	pWaveLevel->AddObject(pWoodBox);

	AddLevel(L"WaveLevel", pWaveLevel);





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
	pSceneGrid->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());
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
	pBox->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());
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
	pSkull->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());

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
		pCylinder->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());
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
		pCylinder2->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());
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
		pSphere->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());
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
		pSphere2->GetMeshRenderComp()->GetMesh()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader").Get());
		pSphere2->SetSubMeshGeo(pSphere2->GetMeshRenderComp()->GetMesh()->GetSubGeo("sphere"));

		pSceneLevel->AddObject(pSphere2);
	}

	AddLevel(L"SceneLevel", pSceneLevel);

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
		if (m_CurrPSOType == OBJ_PSO_TYPE::COLOR_DEFAULT)
			m_CurrPSOType = OBJ_PSO_TYPE::COLOR_WIREFRAME;
		else
			m_CurrPSOType = OBJ_PSO_TYPE::COLOR_DEFAULT;
	}

	if (m_CurLevel != nullptr)
	{
		if (m_CurLevel->GetState() == LEVEL_STATE::PLAY)
			m_CurLevel->Tick();
		m_CurLevel->RegisterClear();
		m_CurLevel->FinalTick();
	}

	CMDLIST->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CMDLIST->SetPipelineState(m_PSOGroup[m_CurrPSOType].Get());
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

