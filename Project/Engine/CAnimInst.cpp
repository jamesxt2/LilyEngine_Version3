#include "pch.h"
#include "CAnimInst.h"

CAnimInst::CAnimInst()
    : CAsset(ASSET_TYPE::ANIM_INST)
{
}

CAnimInst::~CAnimInst()
{
}

float CAnimInst::GetClipStartTime(const std::wstring& clipName) const
{
    auto clip = m_SkinnedData.m_Animations.find(clipName);
    return clip->second.GetClipStartTime();
}

float CAnimInst::GetClipEndTime(const std::wstring& clipName) const
{
    auto clip = m_SkinnedData.m_Animations.find(clipName);
    return clip->second.GetClipEndTime();
}
