#include "pch.h"
#include "CMeshRender.h"

#include "CGameObject.h"
#include "CTransform.h"
#include "CDevice.h"

CMeshRender::CMeshRender()
	: CRenderComponent(COMPONENT_TYPE::MESHRENDER)
{
}

CMeshRender::~CMeshRender()
{
}

void CMeshRender::FinalTick()
{
}

void CMeshRender::Render()
{
	if (GetMesh() == nullptr || GetMaterial() == nullptr) return;

	CDevice::GetInst()->GetConstBuffer(CB_TYPE::MATERIAL)->Bind(GetMaterial()->GetMtrlCBIndex(), 1);
	GetMaterial()->Bind();

	GetMesh()->Render();
}
