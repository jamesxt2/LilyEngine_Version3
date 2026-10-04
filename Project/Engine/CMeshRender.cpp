#include "pch.h"
#include "CMeshRender.h"

#include "CGameObject.h"
#include "CTransform.h"
#include "CDevice.h"

CMeshRender::CMeshRender()
	: CRenderComponent(COMPONENT_TYPE::MESHRENDER), m_SubMeshGeo(nullptr)
{
}

CMeshRender::CMeshRender(const CMeshRender& _other)
	: CRenderComponent(_other), m_SubMeshGeo(_other.m_SubMeshGeo)
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

	GetMaterial()->Bind();

	GetMesh()->Bind();

	if (m_SubMeshGeo)
		CMDLIST->DrawIndexedInstanced(m_SubMeshGeo->IndexCount, 1, m_SubMeshGeo->StartIndexLocation, m_SubMeshGeo->BaseVertexLocation, 0);
	else
		CMDLIST->DrawIndexedInstanced(GetMesh()->GetIndexCount(), 1, 0, 0, 0);
}
