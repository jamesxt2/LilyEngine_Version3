#include "pch.h"
#include "CMaterial.h"

#include "CDevice.h"
#include "CRenderMgr.h"
#include "CLevelMgr.h"

CMaterial::CMaterial()
	: CAsset(ASSET_TYPE::MATERIAL),
	m_MtrlCBIndex(-1),
	m_DiffuseAlbedo{ 1.f, 1.f, 1.f, 1.f },
	m_FresnelR0{ 0.01f, 0.01f, 0.01f },
	m_Roughness(0.25f),
	m_Texture(nullptr)
{
	CRenderMgr::GetInst()->OnObjRenderFinish.AddDynamic(this, &CMaterial::DecreaseNumFramesDirty);
	CLevelMgr::GetInst()->OnLevelChange.AddDynamic(this, &CMaterial::ResetNumFramesDirty);
}

CMaterial::~CMaterial()
{

}

void CMaterial::Bind(UINT texRootParamIndex, UINT mtrlCBRootParamIndex)
{
	if (m_Texture != nullptr)
		m_Texture->Bind_Graphics_Table(texRootParamIndex);

	std::shared_ptr<CConstantBuffer> pMtrlCB = CDevice::GetInst()->GetConstBuffer(CB_TYPE::MATERIAL);
	if (m_NumFramesDirty > 0)
	{
		TMaterial mtrl;
		mtrl.DiffuseAlbedo = m_DiffuseAlbedo;
		mtrl.FresnelR0 = m_FresnelR0;
		mtrl.Roughness = m_Roughness;
		mtrl.bUseTecture = m_Texture == nullptr ? 0 : 1;
		mtrl.MtrlTransform = m_MtrlTransform;

		pMtrlCB->CopyData(m_MtrlCBIndex, &mtrl);
	}
	pMtrlCB->Bind_Graphics(m_MtrlCBIndex, mtrlCBRootParamIndex);
}
