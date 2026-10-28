#pragma once
#include "CComponent.h"

#include "CAnimInst.h"

class CAnimator : public CComponent
{
public:
	CAnimator();
	CAnimator(const CAnimator& _other);
	~CAnimator();
	friend class CAssetMgr;
	friend class CLevelMgr;
	CLONE(CAnimator)

	virtual void Tick() override;
	virtual void FinalTick() override;
	void Bind(UINT animCBIdx = 0);

	void GetFinalTransforms(const std::wstring& clipName, float timePos,
		std::vector<Matrix>& finalTransforms, std::vector<Matrix>& finalInvTranspose) const;

private:
	UINT m_SkinnedCBIndex = 0;
	Ptr<CAnimInst> m_AnimInst;

	std::vector<Matrix> m_FinalTransforms;
	std::vector<Matrix> m_FinalInvTranspose;
	std::wstring m_ClipName;
	float m_TimePos = 0.f;

public:
	void SetAnimInst(Ptr<CAnimInst> inst, const std::wstring& clipName, float timePos = 0.f);
	Ptr<CAnimInst> GetAnimInst() const { return m_AnimInst; }

	inline void SetSkinnedCBIndex(UINT index) { m_SkinnedCBIndex = index; }
	inline UINT GetSkinnedCBIndex() const { return m_SkinnedCBIndex; }
};

