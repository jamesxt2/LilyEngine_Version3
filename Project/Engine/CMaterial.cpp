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
	CLevelMgr::GetInst()->OnLevelChange.AddDynamic(this, &CMaterial::ResetNumFramesDirty);
}

CMaterial::~CMaterial()
{

}

