#include "pch.h"
#include "CAsset.h"

CAsset::CAsset(ASSET_TYPE type)
	: m_Type(type), m_RefCount(0)
{
}

CAsset::CAsset(const CAsset& _other)
	: CEntity(_other),
	m_Type(_other.m_Type), m_RefCount(0)
{
}

CAsset::~CAsset()
{
}

TAsset::TAsset(ASSET_TYPE type)
	: m_Type(type), m_RefCount(0)
{
}

TAsset::TAsset(const TAsset& _other)
	: m_Type(_other.m_Type), m_RefCount(0)
{
}

TAsset::~TAsset()
{
}
