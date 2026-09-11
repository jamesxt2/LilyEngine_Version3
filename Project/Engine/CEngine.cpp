#include "pch.h"
#include "CEngine.h"

#include "CDevice.h"

#include "CTimeMgr.h"

CEngine::CEngine()
    : m_MainWnd(nullptr), m_Resolution{}
{
}

CEngine::~CEngine()
{
}

int CEngine::Init(HWND _MainWnd, POINT _Resolution)
{
    m_MainWnd = _MainWnd;
    m_Resolution = _Resolution;

    RECT rect = { 0, 0, m_Resolution.x, m_Resolution.y };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    SetWindowPos(m_MainWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0);

	/*******************************************************/
	// Create device
	/*******************************************************/
    if (FAILED(CDevice::GetInst()->Init(m_MainWnd, m_Resolution)))
    {
        MessageBox(m_MainWnd, L"Fail to initiate Device!", L"Error", MB_OK);
        return E_FAIL;
    }

	/*******************************************************/
	// Reset Timer
	/*******************************************************/
	CTimeMgr::GetInst()->Reset();

    return S_OK;
}

void CEngine::Run()
{
	CTimeMgr::GetInst()->Tick();
    if (!m_Paused)
    {
		Draw();
    }
    else
        Sleep(100);
}

LRESULT CEngine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_Paused = true;
			CTimeMgr::GetInst()->Stop();
		}
		else
		{
			m_Paused = false;
			CTimeMgr::GetInst()->Start();
		}
		return 0;
	}
    
	case WM_SIZE:
	{
		// Save the new client area dimensions.
		m_Resolution.x = LOWORD(lParam);
		m_Resolution.y = HIWORD(lParam);
		if (CDevice::GetInst())
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_Paused = true;
				m_Minimized = true;
				m_Maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_Paused = false;
				m_Minimized = false;
				m_Maximized = true;
				CDevice::GetInst()->OnResize(m_Resolution);
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (m_Minimized)
				{
					m_Paused = false;
					m_Minimized = false;
					CDevice::GetInst()->OnResize(m_Resolution);
				}

				// Restoring from maximized state?
				else if (m_Maximized)
				{
					m_Paused = false;
					m_Maximized = false;
					CDevice::GetInst()->OnResize(m_Resolution);
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					CDevice::GetInst()->OnResize(m_Resolution);
				}
			}
		}
		return 0;
	}

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
	{
		m_Paused = true;
		m_Resizing = true;
		CTimeMgr::GetInst()->Stop();
		return 0;
	}

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_Paused = false;
		m_Resizing = false;
		CTimeMgr::GetInst()->Start();
		CDevice::GetInst()->OnResize(m_Resolution);
		return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void CEngine::Draw()
{
	CDevice::GetInst()->Draw();
}
