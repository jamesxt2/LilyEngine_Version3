#include "pch.h"
#include "CWaveObject.h"

#include "CDevice.h"
#include "CTimeMgr.h"
#include "CRenderMgr.h"
#include "CAssetMgr.h"

CWaveObject::CWaveObject(int m, int n, float dx, float dt, float speed, float damping)
	: CGameObject(), GpuWaves(m, n, dx, dt, speed, damping)
{

	CRenderMgr::GetInst()->OnObjRenderStart.AddDynamic(this, &CWaveObject::UpdateWaveObject);
}

CWaveObject::~CWaveObject()
{
}

void CWaveObject::Render()
{
	CMDLIST->SetGraphicsRootDescriptorTable(4, m_CurrSolSrv);

	g_Object.DisplacementMapTexelSize.x = 1.f / m_NumCols;
	g_Object.DisplacementMapTexelSize.y = 1.f / m_NumRows;
	g_Object.GridSpatialStep = m_SpatialStep;

	CGameObject::Render();
}

void CWaveObject::UpdateWaveObject()
{
	CD3DX12_RESOURCE_BARRIER GR2UACurrBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_CurrSol.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	CMDLIST->ResourceBarrier(1, &GR2UACurrBarrier);

	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if (CTimeMgr::GetInst()->TotalTime() - t_base >= 0.25f)
	{
		t_base += 0.25f;

		int i = Rand(4, m_NumRows - 5);
		int j = Rand(4, m_NumCols - 5);
		float r = RandF(1.f, 2.f);

		Disturb(i, j, r);
	}
	Update();

	CD3DX12_RESOURCE_BARRIER UA2GRCurrBarrier(CD3DX12_RESOURCE_BARRIER::Transition(m_CurrSol.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
	CMDLIST->ResourceBarrier(1, &UA2GRCurrBarrier);
}
