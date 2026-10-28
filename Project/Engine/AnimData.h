#pragma once

struct Keyframe
{
	float TimePos;
	Vector3 Translation;
	Vector3 Scale;
	Quaternion RotationQuat;

	Keyframe()
		: TimePos(0.f), Translation(0.f), Scale(1.f),
		RotationQuat(0.f, 0.f, 0.f, 1.f)
	{
	}
	Keyframe(const Keyframe& _other)
		: TimePos(_other.TimePos), Translation(_other.Translation),
		Scale(_other.Scale), RotationQuat(_other.RotationQuat)
	{
	}

};

struct BoneAnimation
{
	float GetStartTime() const;
	float GetEndTime() const;
	void Interpolate(float t, _Out_ Matrix& M) const;
	void Interpolate(float t, _Out_ Vector3& outPos, _Out_ Quaternion& outRotQ, _Out_ Vector3& outScale);

	std::vector<Keyframe> Keyframes;

	BoneAnimation() {}
	BoneAnimation(const BoneAnimation& _other)
		: Keyframes(_other.Keyframes)
	{
	}
};

struct AnimationClip
{
	float GetClipStartTime() const;
	float GetClipEndTime() const;
	void Interpolate(float t, std::vector<Matrix>& boneTransforms) const;

	std::vector<BoneAnimation> BoneAnimations;
};

struct SkinnedData
{
	std::vector<int> m_BoneHierarchy;
	std::vector<Matrix> m_BoneOffsets;
	std::unordered_map<std::wstring, AnimationClip> m_Animations;

	void Set(std::vector<int>& boneHierarchy,
		std::vector<Matrix>& boneOffsets,
		std::unordered_map<std::wstring, AnimationClip>& animations);
};
