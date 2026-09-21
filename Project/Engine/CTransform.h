#pragma once
#include "CComponent.h"

class CTransform : public CComponent
{
public:
	CTransform();
	~CTransform();
	CLONE(CTransform)

	virtual void Tick() override;
	virtual void FinalTick() override;
	void Bind();

private:
	Vector3		m_RelativePosition;
	Vector3		m_RelativeRotation;
	Vector3		m_RelativeScale;
	Matrix		m_matWorld;

	Vector3		m_WorldDir[3];
	Vector3		m_RelativeDir[3];

	bool		m_Absolute;

	UINT		m_NumFramesDirty = g_NumFrameResources;
	int			m_ObjCBIndex = 0;

	Matrix		m_matTexTransform;

public:
	inline Vector3 GetRelativePosition() const { return m_RelativePosition; }
	inline Vector3 GetRelativeRotation() const { return m_RelativeRotation; }
	inline Vector3 GetRelativeScale() const { return m_RelativeScale; }

	Vector3 GetWorldPosition() const;
	Vector3 GetWorldScale();

	inline void SetRelativePosition(const Vector3& position) { m_RelativePosition = position; }
	inline void SetRelativeRotation(const Vector3& rotation) { m_RelativeRotation = rotation; }
	inline void SetRelativeScale(const Vector3& scale) { m_RelativeScale = scale; }

	inline void SetRelativePosition(float x, float y, float z) { m_RelativePosition = Vector3(x, y, z); }
	inline void SetRelativeRotation(float x, float y, float z) { m_RelativeRotation = Vector3(x, y, z); }
	inline void SetRelativeScale(float x, float y, float z) { m_RelativeScale = Vector3(x, y, z); }

	inline void SetWorldMat(const Matrix& mat) { m_matWorld = mat; }
	inline const Matrix& GetWorldMat() const { return m_matWorld; }

	inline Vector3 GetRelativeDir(DIR_TYPE type) const { return m_RelativeDir[(UINT)type]; }
	inline Vector3 GetWorldDir(DIR_TYPE type) const { return m_WorldDir[(UINT)type]; }

	inline void SetAbsolute(bool bAbsolute) { m_Absolute = bAbsolute; }
	inline bool IsAbsolute() const { return m_Absolute; }

	inline void SetObjCBIndex(int index) { m_ObjCBIndex = index; }
	inline int GetObjCBIndex() const { return m_ObjCBIndex; }

	inline void ResetDirty() { m_NumFramesDirty = g_NumFrameResources; }

	inline void SetTexTransform(const Matrix& mat) { m_matTexTransform = mat; }
};

