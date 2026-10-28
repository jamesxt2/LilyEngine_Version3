#include "pch.h"
#include "CTransform.h"

#include "CDevice.h"
#include "CConstantBuffer.h"
#include "CGameObject.h"
#include "CRenderComponent.h"
#include "CLevelMgr.h"
#include "CRenderMgr.h"

CTransform::CTransform()
	: CComponent(COMPONENT_TYPE::TRANSFORM),
	m_RelativeScale(XMFLOAT3(1.f, 1.f, 1.f)), m_Absolute(false)
{
	CLevelMgr::GetInst()->OnLevelChange.AddDynamic(this, &CTransform::ResetDirty);
	CRenderMgr::GetInst()->OnObjRenderFinish.AddDynamic(this, &CTransform::DecreaseNumFramesDirty);
}

CTransform::CTransform(const CTransform& _other)
	: CComponent(_other),
	m_RelativePosition(_other.m_RelativePosition),
	m_RelativeRotation(_other.m_RelativeRotation),
	m_RelativeScale(_other.m_RelativeScale),
	m_Absolute(_other.m_Absolute),
	m_matTexTransform(_other.m_matTexTransform)
{
}

CTransform::~CTransform()
{
}

void CTransform::Tick()
{
	
}

void CTransform::FinalTick()
{
	m_matWorld = XMMatrixIdentity();

	Matrix matScale = XMMatrixScaling(m_RelativeScale.x, m_RelativeScale.y, m_RelativeScale.z);

	Matrix matRot = XMMatrixRotationQuaternion(m_RelativeRotationQuat);

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

	m_InvWorld = m_matWorld.Invert();
}

void CTransform::Bind()
{
	if (m_NumFramesDirty > 0)
	{
		std::shared_ptr<CConstantBuffer> pObjCB = CDevice::GetInst()->GetConstBuffer(CB_TYPE::OBJECT);

		g_Object.World = m_matWorld;
		g_Object.WorldInvTranspose = m_matWorld.Invert().Transpose();
		g_Object.TexTransform = m_matTexTransform;
		if (GetOwner()->GetRenderComp())
			g_Object.MaterialIndex = GetOwner()->GetRenderComp()->GetMaterialIndex();

		pObjCB->CopyData(m_ObjCBIndex, &g_Object);
	}
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

void CTransform::SetRelativeRotation(const Vector3& rotation)
{
	m_RelativeRotation = rotation;
	m_RelativeRotationQuat = Quaternion::CreateFromYawPitchRoll(rotation.y, rotation.x, rotation.z);
}
