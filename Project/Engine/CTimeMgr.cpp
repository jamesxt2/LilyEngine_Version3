#include "pch.h"
#include "CTimeMgr.h"

#include "CEngine.h"

CTimeMgr::CTimeMgr()
	: m_SecondsPerCount(0.0), m_DeltaTime(-1.0), m_BaseTime(0),
	m_PausedTime(0), m_StopTime(0), m_PrevTime(0), m_CurrTime(0), 
	m_Stopped(false), m_AccTime(0.0), m_FrmCount(0)
{
	int64_t countsPerSecond;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	m_SecondsPerCount = 1.0 / (double)countsPerSecond;
}

CTimeMgr::~CTimeMgr()
{
}

void CTimeMgr::Tick()
{
	if (m_Stopped)
	{
		m_DeltaTime = 0.0;
		return;
	}

	int64_t currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_CurrTime = currTime;

	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;

	m_PrevTime = m_CurrTime;

	if (m_DeltaTime < 0.0)
		m_DeltaTime = 0.0;

	++m_FrmCount;
	m_AccTime += m_DeltaTime;
	if (m_AccTime > 1.0)
	{
		HWND hMainWnd = CEngine::GetInst()->GetMainWnd();
		wchar_t szText[255] = {};
		swprintf_s(szText, L"FPS : %d Delta Time : %f", m_FrmCount, m_DeltaTime);
		SetWindowText(hMainWnd, szText);

		m_AccTime -= 1.0;
		m_FrmCount = 0;
	}
}

float CTimeMgr::DeltaTime() const
{
	return (float)m_DeltaTime;
}

void CTimeMgr::Reset()
{
	int64_t currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_Stopped = false;
}

void CTimeMgr::Stop()
{
	if (m_Stopped) return;

	int64_t currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_StopTime = currTime;
	m_Stopped = true;
}

void CTimeMgr::Start()
{
	if (!m_Stopped) return;

	int64_t startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	m_PausedTime += (startTime - m_StopTime);
	m_PrevTime = startTime;
	m_StopTime = 0;
	m_Stopped = false;
}

float CTimeMgr::TotalTime() const
{
	return m_Stopped ?
		(float)((m_StopTime - m_PausedTime - m_BaseTime) * m_SecondsPerCount) :
		(float)((m_CurrTime - m_PausedTime - m_BaseTime) * m_SecondsPerCount);
}