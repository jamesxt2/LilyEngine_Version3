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

	void RegisterLevelCamera(const std::wstring& levelname, CCamera* camera, int priority);
	void SetCurrentLevel(const std::wstring& levalname);
	inline void RegisterEditorCamera(CCamera* editorCam) { m_EditorCam = editorCam; }

	MulticastDelegate<> OnObjRenderFinish;

private:
	void Render_Play();

	std::wstring m_CurrLevelName;

private:
	std::map<std::wstring, std::vector<CCamera*>> m_LevelCameraMap;
	CCamera* m_EditorCam;

	void(CRenderMgr::* Render_Func)(void);

};

