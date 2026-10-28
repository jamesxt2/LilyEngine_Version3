#pragma once
#include "CAsset.h"

#include "M3DLoader.h"
#include "AnimData.h"

class CAnimInst : public CAsset
{
public:
	CAnimInst();
	CAnimInst(const CAnimInst& other) = delete;
	~CAnimInst();
	CLONE_DISABLE(CAnimInst)
	friend class CAssetMgr;
	friend class CAnimator;
	friend class CLevelMgr;

private:

	std::vector<M3DLoader::Subset> m_SkinnedSubsets;
	std::vector<M3DLoader::M3dMaterial> m_SkinnedMtrl;
	SkinnedData m_SkinnedData;


public:
	inline UINT GetBoneCount() const { return (UINT)m_SkinnedData.m_BoneHierarchy.size(); }
	float GetClipStartTime(const std::wstring& clipName) const;
	float GetClipEndTime(const std::wstring& clipName) const;
};
