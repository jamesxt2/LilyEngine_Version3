#include "pch.h"
#include "CGameObject.h"

#include "CComponent.h"
#include "CLevelMgr.h"
#include "CLevel.h"
#include "CRenderComponent.h"
#include "CTransform.h"
#include "CScript.h"
#include "MeshData.h"
#include "CDevice.h"

CGameObject::CGameObject()
	: m_arrComp{}, m_RenderComp(nullptr), m_Parent(nullptr), m_Dead(false)
{
}

CGameObject::CGameObject(const CGameObject& other)
	: CEntity(other), m_RenderComp(nullptr), m_arrComp{}, m_Parent(nullptr), m_Dead(false)
{
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if (other.m_arrComp[i])
			AddComponent(other.m_arrComp[i]->Clone());
	}
	for (size_t i = 0; i < other.m_vecScript.size(); ++i)
	{
		AddComponent(other.m_vecScript[i]->Clone());
	}
	for (size_t i = 0; i < other.m_vecChild.size(); ++i)
	{
		AddChild(other.m_vecChild[i]->Clone());
	}
}

CGameObject::~CGameObject()
{
	Safe_Del_Array(m_arrComp);
	Safe_Del_Vector(m_vecChild);
}

void CGameObject::Begin()
{
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if (m_arrComp[i] != nullptr)
			m_arrComp[i]->Begin();
	}
	for (size_t i = 0; i < m_vecScript.size(); ++i)
	{
		m_vecScript[i]->Begin();
	}
	for (size_t i = 0; i < m_vecChild.size(); ++i)
	{
		m_vecChild[i]->Begin();
	}
}

void CGameObject::Tick()
{
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if (m_arrComp[i] != nullptr)
			m_arrComp[i]->Tick();
	}
	for (size_t i = 0; i < m_vecScript.size(); ++i)
	{
		m_vecScript[i]->Tick();
	}
	for (size_t i = 0; i < m_vecChild.size(); ++i)
	{
		m_vecChild[i]->Tick();
	}
}

void CGameObject::FinalTick()
{
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if (m_arrComp[i] != nullptr)
			m_arrComp[i]->FinalTick();
	}

	CLevelMgr::GetInst()->GetCurrentLevel()->RegisterObject(this);

	for (size_t i = 0; i < m_vecChild.size(); ++i)
	{
		m_vecChild[i]->FinalTick();
	}
}

void CGameObject::Render()
{
	if (m_RenderComp)
	{
		if (GetTransformComp())
			GetTransformComp()->Bind();

		m_RenderComp->Render();

		if (m_SubMeshGeo)
			CMDLIST->DrawIndexedInstanced(m_SubMeshGeo->IndexCount, 1, m_SubMeshGeo->StartIndexLocation, m_SubMeshGeo->BaseVertexLocation, 0);
	}
}

void CGameObject::AddComponent(CComponent* component)
{
	if (component == nullptr) return;

	COMPONENT_TYPE type = component->GetComponentType();

	if (type == COMPONENT_TYPE::SCRIPT)
	{
		m_vecScript.push_back((CScript*)component);
	}
	else
	{
		assert(!m_arrComp[(UINT)type]);

		CRenderComponent* pRenderCom = dynamic_cast<CRenderComponent*>(component);
		if (pRenderCom != nullptr)
		{
			assert(!m_RenderComp);
			m_RenderComp = pRenderCom;
		}

		m_arrComp[(UINT)type] = component;
	}

	component->m_Owner = this;
}

void CGameObject::AddChild(CGameObject* obj)
{
	obj->m_Parent = this;
	m_vecChild.push_back(obj);
}

void CGameObject::Destroy()
{
	//TTask task = {};
	//task.type = TASK_TYPE::DESTROY_OBJECT;
	//task.dwParam_0 = (DWORD_PTR)this;
	//CTaskMgr::GetInst()->AddTask(task);
}