#include "pch.h"
#include "CTransform.h"

#include "CDevice.h"
#include "CConstantBuffer.h"
#include "CGameObject.h"

CTransform::CTransform()
	: CComponent(COMPONENT_TYPE::TRANSFORM),
	m_RelativeScale(XMFLOAT3(1.f, 1.f, 1.f)), m_Absolute(false)
{
}

CTransform::~CTransform()
{
}

void CTransform::FinalTick()
{
	m_matWorld = XMMatrixIdentity();

	Matrix matScale = XMMatrixScaling(m_RelativeScale.x, m_RelativeScale.y, m_RelativeScale.z);

	Matrix matRot = XMMatrixRotationX(m_RelativeRotation.x)
		* XMMatrixRotationY(m_RelativeRotation.y)
		* XMMatrixRotationZ(m_RelativeRotation.z);

	Matrix matTranslation = XMMatrixTranslation(m_RelativePosition.x, m_RelativePosition.y, m_RelativePosition.z);

	m_matWorld = matScale * matRot * matTranslation;

	m_WorldDir[(UINT)DIR_TYPE::RIGHT] = m_RelativeDir[(UINT)DIR_TYPE::RIGHT] = XAxis;
	m_WorldDir[(UINT)DIR_TYPE::UP] = m_RelativeDir[(UINT)DIR_TYPE::UP] = YAxis;
	m_WorldDir[(UINT)DIR_TYPE::FORWARD] = m_RelativeDir[(UINT)DIR_TYPE::FORWARD] = ZAxis;

	for (int i = 0; i < 3; ++i)
	{
		m_RelativeDir[i] = XMVector3TransformNormal(m_RelativeDir[i], matRot);
		m_RelativeDir[i].Normalize();
	}

	if (GetOwner()->GetParent())
	{
		const Matrix& matParent = GetOwner()->GetParent()->GetTransformComp()->GetWorldMat();

		if (m_Absolute)
		{
			Vector3 parentScale = GetOwner()->GetParent()->GetTransformComp()->GetWorldScale();
			Matrix matScaleInv = XMMatrixInverse(nullptr, XMMatrixScaling(parentScale.x, parentScale.y, parentScale.z));
			m_matWorld = m_matWorld * matScaleInv * matParent;
		}
		else
			m_matWorld *= matParent;

		for (int i = 0; i < 3; ++i)
		{
			m_WorldDir[i] = XMVector3TransformNormal(m_RelativeDir[i], matParent);
			m_WorldDir[i].Normalize();
		}
	}
	else
	{
		for (int i = 0; i < 3; ++i)
			m_WorldDir[i] = m_RelativeDir[i];
	}

}

void CTransform::Bind()
{
	// System memory -> GPU
	CConstantBuffer* pCB = CDevice::GetInst()->GetConstBuffer(CB_TYPE::TRANSFORM);

	g_Trans.matModel = m_matWorld;
	g_Trans.matMV = g_Trans.matModel * g_Trans.matView;
	g_Trans.matMVP = g_Trans.matMV * g_Trans.matProj;

	pCB->CopyData(&g_Trans);
	pCB->Bind();
}

Vector3 CTransform::GetWorldPosition() const
{
	return m_matWorld.Translation();
}

Vector3 CTransform::GetWorldScale()
{
	CGameObject* pObj = GetOwner();
	Vector3 worldScale = Vector3(1.f, 1.f, 1.f);

	while (pObj)
	{
		worldScale *= pObj->GetTransformComp()->GetRelativeScale();
		if (pObj->GetTransformComp()->IsAbsolute())
			break;
		pObj = pObj->GetParent();
	}

	return worldScale;
}
