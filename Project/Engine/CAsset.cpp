#include "pch.h"
#include "CAsset.h"

CAsset::CAsset(ASSET_TYPE type)
	: m_Type(type), m_RefCount(0)
{
}

CAsset::CAsset(const CAsset& _other)
	: CEntity(_other), m_Key(_other.m_Key), m_RelativePath(_other.m_RelativePath),
	m_Type(_other.m_Type), m_RefCount(0)
{
}

CAsset::~CAsset()
{
}
