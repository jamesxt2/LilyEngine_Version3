#include "pch.h"
#include "CAnimator.h"

#include "CTimeMgr.h"
#include "CGameObject.h"
#include "CTransform.h"
#include "CDevice.h"



CAnimator::CAnimator()
    : CComponent(COMPONENT_TYPE::ANIMATOR)
{
    
}

CAnimator::CAnimator(const CAnimator& _other)
    : CComponent(_other)
{
}

CAnimator::~CAnimator()
{
}

void CAnimator::Tick()
{
    m_TimePos += CTimeMgr::GetInst()->DeltaTime();

    if (m_TimePos > m_AnimInst->GetClipEndTime(m_ClipName))
        m_TimePos = 0.f;

    GetFinalTransforms(m_ClipName, m_TimePos, m_FinalTransforms, m_FinalInvTranspose);
}

void CAnimator::FinalTick()
{
}

void CAnimator::Bind(UINT animCBIdx)
{
    TSkinned skinnedData;
    std::copy(std::begin(m_FinalTransforms), std::end(m_FinalTransforms),
        &skinnedData.BoneTransforms[0]);
    std::copy(std::begin(m_FinalInvTranspose), std::end(m_FinalInvTranspose),
        &skinnedData.BoneInvTranspose[0]);

    CDevice::GetInst()->GetConstBuffer(CB_TYPE::SKINNED)->CopyData(animCBIdx, (const void*)&skinnedData);
}



void CAnimator::GetFinalTransforms(const std::wstring& clipName, float timePos, 
    std::vector<Matrix>& finalTransforms, std::vector<Matrix>& finalInvTranspose) const
{
    UINT numBones = (UINT)m_AnimInst->m_SkinnedData.m_BoneOffsets.size();

    std::vector<Matrix> toParentTransforms(numBones);

    auto clip = m_AnimInst->m_SkinnedData.m_Animations.find(clipName);
    clip->second.Interpolate(timePos, toParentTransforms);

    std::vector<Matrix> toRootTransforms(numBones);
    toRootTransforms[0] = toParentTransforms[0];

    for (UINT i = 1; i < numBones; ++i)
    {
        Matrix toParent = toParentTransforms[i];

        int parentIndex = m_AnimInst->m_SkinnedData.m_BoneHierarchy[i];
        Matrix parentToRoot = toRootTransforms[parentIndex];

        Matrix toRoot = toParent * parentToRoot;

        toRootTransforms[i] = toRoot;
    }

    for (UINT i = 0; i < numBones; ++i)
    {
        Matrix offset = m_AnimInst->m_SkinnedData.m_BoneOffsets[i];
        Matrix toRoot = toRootTransforms[i];
        finalTransforms[i] = offset * toRoot;
        finalInvTranspose[i] = finalTransforms[i].Invert().Transpose();
    }
}

void CAnimator::SetAnimInst(Ptr<CAnimInst> inst, const std::wstring& clipName, float timePos)
{
    m_AnimInst = inst;

    m_FinalTransforms.resize(inst->GetBoneCount());
    m_FinalInvTranspose.resize(inst->GetBoneCount());
    m_ClipName = clipName;
    m_TimePos = timePos;
}



