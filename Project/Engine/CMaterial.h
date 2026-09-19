#pragma once
#include "CAsset.h"

class CMaterial : public CAsset
{
public:
	CMaterial();
	CMaterial(const CMaterial& other) = delete;
	~CMaterial();
	CLONE_DISABLE(CMaterial)
	friend class CAssetMgr;

	void Bind();

private:
	int m_MtrlCBIndex;
	Vector4 m_DiffuseAlbedo;
	Vector3 m_FresnelR0;
	float m_Roughness;

	int m_NumFramesDirty = g_NumFrameResources;

public:
	inline int GetMtrlCBIndex() const { return m_MtrlCBIndex; }
};

