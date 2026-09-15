#pragma once
#include "singleton.h"

#include "CPathMgr.h"
#include "assets.h"

class CAssetMgr : public CSingleton<CAssetMgr>
{
	SINGLE(CAssetMgr)

public:
	void Init();

	template<typename T>
	Ptr<T> Load(const std::wstring& key, const std::wstring& relativePath);

	template<typename T>
	Ptr<T> FindAsset(const std::wstring& key);

	template<typename T>
	void AddAsset(const std::wstring& key, Ptr<T> pAsset);

	void GetAssetNames(ASSET_TYPE type, _Out_ std::vector<std::string>& vecNames);
	inline const std::map<std::wstring, Ptr<CAsset>>& GetAssets(ASSET_TYPE type) const { return m_AssetMap[(UINT)type]; }

	void CreateCylinderMesh(const std::wstring& name, float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount);

private:

	void CreateDefaultMesh();
	void CreateDefaultGraphicsShader();

	std::map<std::wstring, Ptr<CAsset>> m_AssetMap[(UINT)ASSET_TYPE::END];
};

template<typename T>
inline ASSET_TYPE GetAssetType()
{
	if constexpr (std::is_same_v<T, CMesh>)
		return ASSET_TYPE::MESH;
	if constexpr(std::is_same_v<T, CGraphicsShader>)
		return ASSET_TYPE::GRAPHICS_SHADER;
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

	std::map<std::wstring, Ptr<CAsset>>::iterator iter =
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
