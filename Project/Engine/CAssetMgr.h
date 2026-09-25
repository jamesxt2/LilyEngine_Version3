#pragma once
#include "singleton.h"

#include "CPathMgr.h"
#include "assets.h"
#include "MeshData.h"
#include "Waves.h"


class CAssetMgr : public CSingleton<CAssetMgr>
{
	SINGLE(CAssetMgr)

public:
	void Init();
	void PostInit();
	void Tick();

	template<typename T>
	Ptr<T> Load(const std::wstring& key, const std::wstring& relativePath);

	template<typename T>
	Ptr<T> FindAsset(const std::wstring& key);

	template<typename T>
	void AddAsset(const std::wstring& key, Ptr<T> pAsset);

	void GetAssetNames(ASSET_TYPE type, _Out_ std::vector<std::string>& vecNames);
	inline const std::unordered_map<std::wstring, Ptr<CAsset>>& GetAssets(ASSET_TYPE type) const { return m_AssetMap[(UINT)type]; }


private:

	///<summary>
	/// Creates a box centered at the origin with the given dimensions, where each
	/// face has m rows and n columns of vertices.
	///</summary>
	void CreateBox(const std::wstring& name, float width, float height, float depth, uint32 numSubdivisions = 0);

	///<summary>
	/// Creates a sphere centered at the origin with the given radius.  The
	/// slices and stacks parameters control the degree of tessellation.
	///</summary>
	void CreateSphere(const std::wstring& name, float radius, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// Creates a geosphere centered at the origin with the given radius.  The
	/// depth controls the level of tessellation.
	///</summary>
	void CreateGeosphere(const std::wstring& name, float radius, uint32 numSubdivisions);

	///<summary>
	/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
	/// The bottom and top radius can vary to form various cone shapes rather than true
	// cylinders.  The slices and stacks parameters control the degree of tessellation.
	///</summary>
	void CreateCylinder(const std::wstring& name, float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount);

	///<summary>
	/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
	/// at the origin with the specified width and depth.
	///</summary>
	void CreateGrid(const std::wstring& name, float width, float depth, uint32 m, uint32 n);

	///<summary>
	/// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
	///</summary>
	void CreateQuad(const std::wstring& name, float x, float y, float w, float h, float depth);

	void UpdateWaves();
	void AnimateMaterials();

	void CreateDefaultMesh();

	void CreateSceneMeshes();
	void CreateWaveMeshes();
	void CreateSkullMesh();
	void CreateRoomMeshes();
	void CreateBillboardMesh();

	void CreateDefaultTexture();

	void CreateDefaultMaterial();
	void CreateDefaultGraphicsShader();

	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);

	VertexMesh MidPoint(const VertexMesh& v0, const VertexMesh& v1);
	void Subdivide(MeshData& meshData);

	std::unordered_map<std::wstring, Ptr<CAsset>> m_AssetMap[(UINT)ASSET_TYPE::END];

	std::unordered_map<std::wstring, MeshData> m_MeshDataMap;

	Waves* m_Waves;

public:
	inline int GetWavesVertexCount() const { return m_Waves->VertexCount(); }
	inline UINT GetAssetSize(ASSET_TYPE type) const { return (UINT)m_AssetMap[(UINT)type].size(); }
};

template<typename T>
inline ASSET_TYPE GetAssetType()
{
	if constexpr (std::is_same_v<T, CMesh>)
		return ASSET_TYPE::MESH;
	if constexpr(std::is_same_v<T, CGraphicsShader>)
		return ASSET_TYPE::GRAPHICS_SHADER;
	if constexpr (std::is_same_v<T, CMaterial>)
		return ASSET_TYPE::MATERIAL;
	if constexpr (std::is_same_v<T, CTexture>)
		return ASSET_TYPE::TEXTURE;
}

template<typename T>
inline Ptr<T> CAssetMgr::Load(const std::wstring& key, const std::wstring& relativePath)
{
	Ptr<CAsset> pAsset = FindAsset<T>(key).Get();
	if (pAsset.Get() != nullptr)
	{
		return (T*)pAsset.Get();
	}

	std::wstring fullPath = CPathMgr::GetInst()->GetContentPath() + relativePath;
	pAsset = new T;
	if (FAILED(pAsset->Load(fullPath)))
	{
		MessageBox(nullptr, fullPath.c_str(), L"Error : Fail to load asset", MB_OK);
		return nullptr;
	}

	pAsset->m_RelativePath = relativePath;

	ASSET_TYPE type = GetAssetType<T>();

	m_AssetMap[(UINT)type].insert(std::make_pair(key, pAsset.Get()));
	pAsset->m_Key = key;

#if defined(_DEBUG) || defined(DEBUG)
	return dynamic_cast<T*>(pAsset.Get());
#else
	return (T*)pAsset.Get();
#endif
}

template<typename T>
inline Ptr<T> CAssetMgr::FindAsset(const std::wstring& key)
{
	ASSET_TYPE type = GetAssetType<T>();

	std::unordered_map<std::wstring, Ptr<CAsset>>::iterator iter =
		m_AssetMap[(UINT)type].find(key);

	if (iter == m_AssetMap[(UINT)type].end())
		return nullptr;

#if defined(_DEBUG) || defined(DEBUG)
	return dynamic_cast<T*>(iter->second.Get());
#else
	return (T*)iter->second.Get();
#endif
}

template<typename T>
inline void CAssetMgr::AddAsset(const std::wstring& key, Ptr<T> pAsset)
{
	Ptr<T> findAsset = FindAsset<T>(key);

	assert(findAsset.Get() == nullptr);

	ASSET_TYPE type = GetAssetType<T>();

	m_AssetMap[(UINT)type].insert(std::make_pair(key, pAsset.Get()));
	pAsset->m_Key = key;
}
