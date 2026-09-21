#pragma once
#include "CAsset.h"

#include "CTexture.h"

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

	Ptr<CTexture> m_Texture;

	Matrix m_MtrlTransform;

	int m_NumFramesDirty = g_NumFrameResources;

public:
	inline int GetMtrlCBIndex() const { return m_MtrlCBIndex; }

	void DecreaseNumFramesDirty() 
	{
		if (m_NumFramesDirty >= 0)
			--m_NumFramesDirty; 
	}
	void ResetNumFramesDirty() { m_NumFramesDirty = g_NumFrameResources; }
};

