#include "pch.h"
#include "CMaterial.h"

#include "CDevice.h"

CMaterial::CMaterial()
	: CAsset(ASSET_TYPE::MATERIAL),
	m_MtrlCBIndex(-1),
	m_DiffuseAlbedo{ 1.f, 1.f, 1.f, 1.f },
	m_FresnelR0{ 0.01f, 0.01f, 0.01f },
	m_Roughness(0.25f)
{

}

CMaterial::~CMaterial()
{
}

void CMaterial::Bind()
{
	if (m_NumFramesDirty > 0)
	{
		std::shared_ptr<CConstantBuffer> pMtrlCB = CDevice::GetInst()->GetConstBuffer(CB_TYPE::MATERIAL);

		TMaterial mtrl;
		mtrl.DiffuseAlbedo = m_DiffuseAlbedo;
		mtrl.FresnelR0 = m_FresnelR0;
		mtrl.Roughness = m_Roughness;

		pMtrlCB->CopyData(m_MtrlCBIndex, &mtrl);

		--m_NumFramesDirty;
	}
}
