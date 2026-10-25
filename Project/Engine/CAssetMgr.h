#pragma once
#include "singleton.h"

#include "CPathMgr.h"
#include "assets.h"
#include "MeshData.h"
#include "CWaveObject.h"

class CAssetMgr : public CSingleton<CAssetMgr>
{
	SINGLE(CAssetMgr)

public:
	void Init();
	void Tick();

	template<typename T>
	Ptr<T> FindAsset(const std::wstring& name);

	template<typename T>
	void AddAsset(const std::wstring& name, Ptr<T> pAsset);

	void GetAssetNames(ASSET_TYPE type, _Out_ std::vector<std::string>& vecNames);
	inline const std::unordered_map<std::wstring, Ptr<CAsset>>& GetAssets(ASSET_TYPE type) const { return m_AssetMap[(UINT)type]; }

	Ptr<CMesh> CreateGridMesh(const std::wstring& name, float width, float depth, UINT rows, UINT columns);
	Ptr<CMesh> CreateTessQuadPatchMesh(const std::wstring& name);

private:

	///<summary>
	/// Creates a box centered at the origin with the given dimensions, where each
	/// face has m rows and n columns of vertices.
	///</summary>
	std::shared_ptr<MeshData> CreateBox(float width, float height, float depth, uint32 numSubdivisions = 0);

	///<summary>
	/// Creates a sphere centered at the origin with the given radius.  The
	/// slices and stacks parameters control the degree of tessellation.
	///</summary>
	std::shared_ptr<MeshData> CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// Creates a geosphere centered at the origin with the given radius.  The
	/// depth controls the level of tessellation.
	///</summary>
	std::shared_ptr<MeshData> CreateGeosphere(float radius, uint32 numSubdivisions);

	///<summary>
	/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
	/// The bottom and top radius can vary to form various cone shapes rather than true
	// cylinders.  The slices and stacks parameters control the degree of tessellation.
	///</summary>
	std::shared_ptr<MeshData> CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount);

	///<summary>
	/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
	/// at the origin with the specified width and depth.
	///</summary>
	std::shared_ptr<MeshData> CreateGrid(float width, float depth, uint32 m, uint32 n);

	///<summary>
	/// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
	///</summary>
	std::shared_ptr<MeshData> CreateQuad(float x, float y, float w, float h, float depth);

	//void UpdateWaves();
	void AnimateMaterials();
	void UpdateAndUploadMaterials();

	template<typename T>
	BoundingBox GetGeomertyBoundingBox(const std::vector<T>& vertices);

	void CreateMeshes();

	void CreateDefaultMeshes();
	void CreateSceneMeshes();
	void CreateWaveSceneMeshes();
	void CreateSkullMesh();
	void CreateRoomMeshes();
	void CreateBillboardMesh();

	void CreateTextures();

	void CreateMaterials();

	void CreateGraphicsShaders();
	void CreateComputeShaders();

	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, std::shared_ptr<MeshData> meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, std::shared_ptr<MeshData> meshData);

	VertexMesh MidPoint(const VertexMesh& v0, const VertexMesh& v1);
	void Subdivide(std::shared_ptr<MeshData> meshData);

	std::unordered_map<std::wstring, Ptr<CAsset>> m_AssetMap[(UINT)ASSET_TYPE::END];

	std::unordered_map<std::wstring, ComPtr<ID3D12RootSignature>> m_RootSignatureMap;
	void BuildRootSignatures();
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

	void BuildPSO();
	std::unordered_map<PSO_TYPE, ComPtr<ID3D12PipelineState>>	m_PSOGroup;

public:

	inline UINT GetAssetSize(ASSET_TYPE type) const { return (UINT)m_AssetMap[(UINT)type].size(); }

	ComPtr<ID3D12PipelineState> GetPSO(PSO_TYPE type) const;
	ComPtr<ID3D12RootSignature> GetRootSignature(const std::wstring& key) const;
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
	if constexpr (std::is_same_v<T, CComputeShader>)
		return ASSET_TYPE::COMPUTE_SHADER;
}

template<typename T>
inline Ptr<T> CAssetMgr::FindAsset(const std::wstring& name)
{
	ASSET_TYPE type = GetAssetType<T>();

	std::unordered_map<std::wstring, Ptr<CAsset>>::iterator iter =
		m_AssetMap[(UINT)type].find(name);

	if (iter == m_AssetMap[(UINT)type].end())
		return nullptr;

#if defined(_DEBUG) || defined(DEBUG)
	return dynamic_cast<T*>(iter->second.Get());
#else
	return (T*)iter->second.Get();
#endif
}

template<typename T>
inline void CAssetMgr::AddAsset(const std::wstring& name, Ptr<T> pAsset)
{
	Ptr<T> findAsset = FindAsset<T>(name);

	assert(findAsset.Get() == nullptr);

	ASSET_TYPE type = GetAssetType<T>();

	m_AssetMap[(UINT)type].insert(std::make_pair(name, pAsset.Get()));
	pAsset->m_Name = name;
}

template<typename T>
inline BoundingBox CAssetMgr::GetGeomertyBoundingBox(const std::vector<T>& vertices)
{
	if (vertices.empty())
		return BoundingBox();
	Vector3 vMax = vertices[0].Position;
	Vector3 vMin = vertices[0].Position;
	for (size_t i = 1; i < vertices.size(); ++i)
	{
		vMax.x = std::max<float>(vMax.x, vertices[i].Position.x);
		vMax.y = std::max<float>(vMax.y, vertices[i].Position.y);
		vMax.z = std::max<float>(vMax.z, vertices[i].Position.z);
		vMin.x = std::min<float>(vMin.x, vertices[i].Position.x);
		vMin.y = std::min<float>(vMin.y, vertices[i].Position.y);
		vMin.z = std::min<float>(vMin.z, vertices[i].Position.z);
	}
	BoundingBox bounds;
	bounds.Center = 0.5f * (vMin + vMax);
	bounds.Extents = 0.5f * (vMax - vMin);
	return bounds;
}
