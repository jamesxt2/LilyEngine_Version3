#pragma once
#include "CComponent.h"

class CGameObject;

class CCamera : public CComponent
{
public:
	CCamera();
	CCamera(const CCamera& other);
	~CCamera();
	CLONE(CCamera)

	virtual void Begin() override;
	virtual void FinalTick() override;
	void Render();

private:
	PROJ_TYPE		m_ProjType;

	int				m_CamPriority;

	float			m_FOV;
	float			m_Far;

	float			m_Width;
	float			m_AspectRatio;

	float			m_Scale; // For orthographic

	Matrix			m_matView;
	Matrix			m_matProj;

	std::vector<CGameObject*> m_vecObjects;

public:
	inline void SetProjType(PROJ_TYPE type) { m_ProjType = type; }
	inline PROJ_TYPE GetProjType() const { return m_ProjType; }

	void SetCameraPriority(int priority);
	inline int GetCameraPriority() const { return m_CamPriority; }

	inline void SetFar(float _far) { m_Far = _far; }
	inline float GetFar() const { return m_Far; }
	inline void SetFOV(float _FOV) { m_FOV = _FOV; }
	inline float GetFOV() const { return m_FOV; }

	inline void SetWidth(float width) { m_Width = width; }
	inline float GetWidth() const { return m_Width; }
	inline void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }
	inline float GetAspectRatio() const { return m_AspectRatio; }

	inline void SetScale(float scale) { m_Scale = scale; }
	inline float GetScale() const { return m_Scale; }
};

