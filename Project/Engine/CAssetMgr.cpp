#include "pch.h"
#include "CAssetMgr.h"

#include "d3dUtils.h"
#include "CGraphicsShader.h"
#include "CDevice.h"

CAssetMgr::CAssetMgr()
{
}

CAssetMgr::~CAssetMgr()
{

}

void CAssetMgr::GetAssetNames(ASSET_TYPE type, _Out_ std::vector<std::string>& vecNames)
{
	vecNames.clear();
	for (const auto& pair : m_AssetMap[(UINT)type])
	{
		std::string strName;
		WStringToString(pair.first, strName);
		vecNames.push_back(strName);
	}
}

void CAssetMgr::Init()
{
	CDevice::GetInst()->Reset();
	CreateDefaultMesh();
	CDevice::GetInst()->Close();

	CreateDefaultGraphicsShader();
}

void CAssetMgr::CreateDefaultMesh()
{
	Ptr<CMesh> pMesh = nullptr;

	std::vector<Vertex> vecVtx;
	std::vector<UINT> vecIdx;
	Vertex v;

	/***************/
	// Point Mesh
	/***************/
	{
		v.Pos = XMFLOAT3(0.f, 0.f, 0.f);
		v.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
		v.UV = XMFLOAT2(0.f, 0.f);

		UINT idx = 0;

		pMesh = new CMesh;
		pMesh->CreateVertexBuffer(&v, 1);
		pMesh->CreateIndexBuffer(&idx, 1);
		AddAsset(L"PointMesh", pMesh);
	}

	/***************/
	// Rect Mesh
	/***************/
	{
		v.Pos = XMFLOAT3(-0.5f, 0.5f, 0.f);
		v.Color = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
		v.UV = XMFLOAT2(0.f, 0.f);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(0.5f, 0.5f, 0.f);
		v.Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f);
		v.UV = XMFLOAT2(1.f, 0.f);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(0.5f, -0.5f, 0.f);
		v.Color = XMFLOAT4(0.f, 0.f, 1.f, 1.f);
		v.UV = XMFLOAT2(1.f, 1.f);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(-0.5f, -0.5f, 0.f);
		v.Color = XMFLOAT4(0.f, 1.f, 0.f, 1.f);
		v.UV = XMFLOAT2(0.f, 1.f);
		vecVtx.push_back(v);

		vecIdx.push_back(0);
		vecIdx.push_back(3);
		vecIdx.push_back(2);

		vecIdx.push_back(0);
		vecIdx.push_back(1);
		vecIdx.push_back(2);

		pMesh = new CMesh;
		pMesh->CreateVertexBuffer(vecVtx.data(), (UINT)vecVtx.size());
		pMesh->CreateIndexBuffer(vecIdx.data(), (UINT)vecIdx.size());
		AddAsset(L"RectMesh", pMesh);

		vecIdx.clear();

		vecIdx.push_back(0);
		vecIdx.push_back(1);
		vecIdx.push_back(2);
		vecIdx.push_back(3);
		vecIdx.push_back(0);

		pMesh = new CMesh;
		pMesh->CreateVertexBuffer(vecVtx.data(), (UINT)vecVtx.size());
		pMesh->CreateIndexBuffer(vecIdx.data(), (UINT)vecIdx.size());
		AddAsset(L"RectMesh_Debug", pMesh);

		vecVtx.clear();
		vecIdx.clear();
	}

	/***************/
	// Circle Mesh
	/***************/
	{
		v.Pos = XMFLOAT3(0.f, 0.f, 0.f);
		v.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
		vecVtx.push_back(v);

		float Radius = 0.5f;
		UINT Slice = 60;
		float AngleStep = 2 * XM_PI / Slice;

		float Angle = 0.f;
		for (UINT i = 0; i <= Slice; ++i, Angle += AngleStep)
		{
			v.Pos = XMFLOAT3(cosf(Angle) * Radius, sinf(Angle) * Radius, 0.f);
			v.Color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
			vecVtx.push_back(v);
		}

		for (UINT i = 0; i < Slice; ++i)
		{
			vecIdx.push_back(0);
			vecIdx.push_back(i + 2);
			vecIdx.push_back(i + 1);
		}

		pMesh = new CMesh;
		pMesh->CreateVertexBuffer(vecVtx.data(), (UINT)vecVtx.size());
		pMesh->CreateIndexBuffer(vecIdx.data(), (UINT)vecIdx.size());
		AddAsset(L"CircleMesh", pMesh);

		vecIdx.clear();

		for (UINT i = 0; i < Slice; ++i)
		{
			vecIdx.push_back(i + 1);
		}
		vecIdx.push_back(1);

		pMesh = new CMesh;
		pMesh->CreateVertexBuffer(vecVtx.data(), (UINT)vecVtx.size());
		pMesh->CreateIndexBuffer(vecIdx.data(), (UINT)vecIdx.size());
		AddAsset(L"CircleMesh_Debug", pMesh);

		vecVtx.clear();
		vecIdx.clear();
	}

	/***************/
	// Cube Mesh
	/***************/
	{
		v.Pos = XMFLOAT3(-1.f, -1.f, -1.f);
		v.Color = XMFLOAT4(Colors::White);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(-1.f, +1.f, -1.f);
		v.Color = XMFLOAT4(Colors::Black);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(+1.f, +1.f, -1.f);
		v.Color = XMFLOAT4(Colors::Red);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(+1.f, -1.f, -1.f);
		v.Color = XMFLOAT4(Colors::Green);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(-1.f, -1.f, +1.f);
		v.Color = XMFLOAT4(Colors::Blue);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(-1.f, +1.f, +1.f);
		v.Color = XMFLOAT4(Colors::Yellow);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(+1.f, +1.f, +1.f);
		v.Color = XMFLOAT4(Colors::Cyan);
		vecVtx.push_back(v);

		v.Pos = XMFLOAT3(+1.f, -1.f, +1.f);
		v.Color = XMFLOAT4(Colors::Magenta);
		vecVtx.push_back(v);

		// front
		vecIdx.push_back(0); vecIdx.push_back(1); vecIdx.push_back(2);
		vecIdx.push_back(0); vecIdx.push_back(2); vecIdx.push_back(3);

		// back
		vecIdx.push_back(4); vecIdx.push_back(6); vecIdx.push_back(5);
		vecIdx.push_back(4); vecIdx.push_back(7); vecIdx.push_back(6);

		// left
		vecIdx.push_back(4); vecIdx.push_back(5); vecIdx.push_back(1);
		vecIdx.push_back(4); vecIdx.push_back(1); vecIdx.push_back(0);

		// right
		vecIdx.push_back(3); vecIdx.push_back(2); vecIdx.push_back(6);
		vecIdx.push_back(3); vecIdx.push_back(6); vecIdx.push_back(7);

		// top
		vecIdx.push_back(1); vecIdx.push_back(5); vecIdx.push_back(6);
		vecIdx.push_back(1); vecIdx.push_back(6); vecIdx.push_back(2);

		// bottom
		vecIdx.push_back(4); vecIdx.push_back(0); vecIdx.push_back(3);
		vecIdx.push_back(4); vecIdx.push_back(3); vecIdx.push_back(7);

		pMesh = new CMesh;
		pMesh->CreateVertexBuffer(vecVtx.data(), (UINT)vecVtx.size());
		pMesh->CreateIndexBuffer(vecIdx.data(), (UINT)vecIdx.size());
		AddAsset(L"CubeMesh", pMesh);

		vecVtx.clear();
		vecIdx.clear();
	}

}

void CAssetMgr::CreateDefaultGraphicsShader()
{
	std::wstring strPath = CPathMgr::GetInst()->GetContentPath();

	Ptr<CGraphicsShader> pShader = nullptr;

	// Std2DShader
	pShader = new CGraphicsShader;;
	pShader->BuildVertexShaderAndInputLayout(strPath + L"shader\\color.fx", "VS");
	pShader->BuildPixelShader(strPath + L"shader\\color.fx", "PS");
	pShader->BuildPSO();
	//pShader->SetRSType(RS_TYPE::CULL_NONE);
	//pShader->SetDSType(DS_TYPE::LESS);
	//pShader->SetBSType(BS_TYPE::DEFAULT);

	AddAsset<CGraphicsShader>(L"ColorShader", pShader);
}

void CAssetMgr::CreateCylinderMesh(const std::wstring& name, float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount)
{
	float stackHeight = height / stackCount;

	float radiusStep = (topRadius - bottomRadius) / stackCount;
	UINT ringCount = stackCount + 1;

	Ptr<CMesh> pMesh = nullptr;
	std::vector<Vertex> vecVtx;
	std::vector<UINT> vecIdx;

	for (UINT i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;
		float dTheta = 2 * XM_PI / sliceCount;
		for (UINT j = 0; j <= sliceCount; ++j)
		{

		}
	}
}
