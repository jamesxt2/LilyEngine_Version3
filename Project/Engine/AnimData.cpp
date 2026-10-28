#include "pch.h"
#include "AnimData.h"

float BoneAnimation::GetStartTime() const
{
    return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime() const
{
    return Keyframes.back().TimePos;
}

void BoneAnimation::Interpolate(float t, _Out_ Matrix& M) const
{
    M = XMMatrixIdentity();
    if (t <= Keyframes.front().TimePos)
    {
        Vector3 S = Keyframes.front().Scale;
        Vector3 P = Keyframes.front().Translation;
        Quaternion Q = Keyframes.front().RotationQuat;

        Vector4 zero = Vector4(0.f, 0.f, 0.f, 1.f);
        M = XMMatrixAffineTransformation(S, zero, Q, P);
    }
    else if (t >= Keyframes.back().TimePos)
    {
        Vector3 S = Keyframes.back().Scale;
        Vector3 P = Keyframes.back().Translation;
        Quaternion Q = Keyframes.back().RotationQuat;

        Vector4 zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
        XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
    }
    else
    {
        for (UINT i = 0; i < (UINT)Keyframes.size() - 1; ++i)
        {
            if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
            {
                float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

                Vector3 s0 = Keyframes[i].Scale;
                Vector3 s1 = Keyframes[i + 1].Scale;

                Vector3 p0 = Keyframes[i].Translation;
                Vector3 p1 = Keyframes[i + 1].Translation;

                Quaternion q0 = Keyframes[i].RotationQuat;
                Quaternion q1 = Keyframes[i + 1].RotationQuat;

                Vector3 S = XMVectorLerp(s0, s1, lerpPercent);
                Vector3 P = XMVectorLerp(p0, p1, lerpPercent);
                Quaternion Q = XMQuaternionSlerp(q0, q1, lerpPercent);

                Vector4 zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
                XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

                break;
            }
        }
    }
}

void BoneAnimation::Interpolate(float t, _Out_ Vector3& outPos, _Out_ Quaternion& outRotQ, _Out_ Vector3& outScale)
{
    outPos = Vector3(0.f);
    outRotQ = Quaternion(0.f, 0.f, 0.f, 0.f);
    outScale = Vector3(1.f);
    if (t <= Keyframes.front().TimePos)
    {
        outScale = Keyframes.front().Scale;
        outPos = Keyframes.front().Translation;
        outRotQ = Keyframes.front().RotationQuat;
    }
    else if (t >= Keyframes.back().TimePos)
    {
        outScale = Keyframes.back().Scale;
        outPos = Keyframes.back().Translation;
        outRotQ = Keyframes.back().RotationQuat;
    }
    else
    {
        for (UINT i = 0; i < (UINT)Keyframes.size() - 1; ++i)
        {
            if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
            {
                float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

                outScale = XMVectorLerp(Keyframes[i].Scale, Keyframes[i + 1].Scale, lerpPercent);
                outPos = XMVectorLerp(Keyframes[i].Translation, Keyframes[i + 1].Translation, lerpPercent);
                outRotQ = XMQuaternionSlerp(Keyframes[i].RotationQuat, Keyframes[i + 1].RotationQuat, lerpPercent);

                break;
            }
        }
    }
}

float AnimationClip::GetClipStartTime() const
{
    // Find smallest start time over all bones in this clip.
    float t = Utilities::Infinity;
    for (UINT i = 0; i < BoneAnimations.size(); ++i)
    {
        t = std::min<float>(t, BoneAnimations[i].GetStartTime());
    }

    return t;
}

float AnimationClip::GetClipEndTime() const
{
    // Find largest end time over all bones in this clip.
    float t = 0.0f;
    for (UINT i = 0; i < BoneAnimations.size(); ++i)
    {
        t = std::max<float>(t, BoneAnimations[i].GetEndTime());
    }

    return t;
}

void AnimationClip::Interpolate(float t, std::vector<Matrix>& boneTransforms) const
{
    for (UINT i = 0; i < BoneAnimations.size(); ++i)
    {
        BoneAnimations[i].Interpolate(t, boneTransforms[i]);
    }
}

void SkinnedData::Set(std::vector<int>& boneHierarchy,
    std::vector<Matrix>& boneOffsets,
    std::unordered_map<std::wstring, AnimationClip>& animations)
{
    m_BoneHierarchy = boneHierarchy;
    m_BoneOffsets = boneOffsets;
    m_Animations = animations;
}