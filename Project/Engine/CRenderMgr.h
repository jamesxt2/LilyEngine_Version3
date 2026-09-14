#pragma once
#include "singleton.h"

class CCamera;

class CRenderMgr : public CSingleton<CRenderMgr>
{
	SINGLE(CRenderMgr)

public:
	void Init();
	void Tick();
	void Render();

	void RegisterCamera(CCamera* camera, int priority);
	inline void RegisterEditorCamera(CCamera* editorCam) { m_EditorCam = editorCam; }

	void CopyRenderTarget();

private:
	void Render_Play();

private:
	std::vector<CCamera*> m_vecCam;
	CCamera* m_EditorCam;

	void(CRenderMgr::* Render_Func)(void);

public:
	inline std::vector<CCamera*>& GetRegisteredCamera() { return m_vecCam; }
};

