#include "pch.h"
#include "CEngine.h"

#include "CDevice.h"

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

    if (FAILED(CDevice::GetInst()->Init(m_MainWnd, m_Resolution)))
    {
        MessageBox(m_MainWnd, L"Fail to initiate Device!", L"Error", MB_OK);
        return E_FAIL;
    }

    return S_OK;
}
