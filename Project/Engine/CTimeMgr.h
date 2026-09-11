#pragma once
#include "singleton.h"

class CTimeMgr : public CSingleton<CTimeMgr>
{
	SINGLE(CTimeMgr)

public:
	float TotalTime() const;
	float DeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	double						m_SecondsPerCount;
	double						m_DeltaTime;

	int64_t						m_BaseTime;
	int64_t						m_PausedTime;
	int64_t						m_StopTime;
	int64_t						m_PrevTime;
	int64_t						m_CurrTime;

	bool						m_Stopped;

	double						m_AccTime;
	UINT						m_FrmCount;
};

