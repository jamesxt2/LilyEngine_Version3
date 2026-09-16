#include "pch.h"
#include "CCameraMoveScript.h"

#include "CCamera.h"
#include "CGameObject.h"
#include "CKeyMgr.h"
#include "CTransform.h"
#include "CTimeMgr.h"

CCameraMoveScript::CCameraMoveScript()
	: m_Speed(10.f)
{
}

CCameraMoveScript::~CCameraMoveScript()
{
}

void CCameraMoveScript::Tick()
{
	CCamera* camera = GetOwner()->GetCameraComp();

	if (KEY_TAP(KEY::P))
	{
		PROJ_TYPE type = camera->GetProjType();
		camera->SetProjType(type == PROJ_TYPE::ORTHOGRAPHIC ? PROJ_TYPE::PERSPECTIVE : PROJ_TYPE::ORTHOGRAPHIC);

		if (type == PROJ_TYPE::PERSPECTIVE)
		{
			CTransform* transComp = GetOwner()->GetTransformComp();
			Vector3 pos = transComp->GetRelativePosition();
			pos.z = 0.f;
			transComp->SetRelativePosition(pos);
			transComp->SetRelativeRotation(Vector3(0.f, 0.f, 0.f));
		}

		MarkDirty();
	}

	if (camera->GetProjType() == PROJ_TYPE::PERSPECTIVE)
		MoveByPerspective();
	else
		MoveByOrthographic();

}

void CCameraMoveScript::MoveByPerspective()
{
	CTransform* transComp = GetOwner()->GetTransformComp();
	if (!transComp) return;

	bool bMove = false;

	Vector3 vCurPos = transComp->GetRelativePosition();
	Vector3 vForward = transComp->GetRelativeDir(DIR_TYPE::FORWARD);
	Vector3 vRight = transComp->GetRelativeDir(DIR_TYPE::RIGHT);
	Vector3 vUp = transComp->GetRelativeDir(DIR_TYPE::UP);

	if (KEY_PRESSED(KEY::W))
	{
		vCurPos += vForward * m_Speed * DT;
		bMove = true;
	}
	if (KEY_PRESSED(KEY::S))
	{
		vCurPos += -vForward * m_Speed * DT;
		bMove = true;
	}
	if (KEY_PRESSED(KEY::A))
	{
		vCurPos += -vRight * m_Speed * DT;
		bMove = true;
	}
	if (KEY_PRESSED(KEY::D))
	{
		vCurPos += vRight * m_Speed * DT;
		bMove = true;
	}
	if (KEY_PRESSED(KEY::E))
	{
		vCurPos += vUp * m_Speed * DT;
		bMove = true;
	}
	if (KEY_PRESSED(KEY::Q))
	{
		vCurPos += -vUp * m_Speed * DT;
		bMove = true;
	}
	transComp->SetRelativePosition(vCurPos);

	Vector3 vCurRot = transComp->GetRelativeRotation();
	if (KEY_PRESSED(KEY::RBUTTON))
	{
		vCurRot.y += CKeyMgr::GetInst()->GetMouseMoveDir().x * XM_PI * DT * 3.f;
		vCurRot.x -= CKeyMgr::GetInst()->GetMouseMoveDir().y * XM_PI * DT * 3.f;
		bMove = true;
	}
	transComp->SetRelativeRotation(vCurRot);

	if (bMove)
		MarkDirty();
}

void CCameraMoveScript::MoveByOrthographic()
{
	CTransform* transComp = GetOwner()->GetTransformComp();
	if (!transComp) return;

	bool bMove = false;

	Vector3 vCurPos = transComp->GetRelativePosition();

	if (KEY_PRESSED(KEY::W))
	{
		vCurPos.y += m_Speed * DT;
		bMove = true;
	}
	if (KEY_PRESSED(KEY::S))
	{
		vCurPos.y -= m_Speed * DT;
		bMove = true;
	}
	if (KEY_PRESSED(KEY::A))
	{
		vCurPos.x -= m_Speed * DT;
		bMove = true;
	}
	if (KEY_PRESSED(KEY::D))
	{
		vCurPos.x += m_Speed * DT;
		bMove = true;
	}
	if (KEY_PRESSED(KEY::E))
	{
		CCamera* camera = GetOwner()->GetCameraComp();
		float scale = camera->GetScale() - DT;
		camera->SetScale(scale < 0.01f ? 0.01f : scale);
		bMove = true;
	}
	if (KEY_PRESSED(KEY::Q))
	{
		CCamera* camera = GetOwner()->GetCameraComp();
		camera->SetScale(camera->GetScale() + DT);
		bMove = true;
	}
	transComp->SetRelativePosition(vCurPos);

	if (bMove)
		MarkDirty();
}

void CCameraMoveScript::MarkDirty()
{
	if (GetOwner()->GetCameraComp())
		GetOwner()->GetCameraComp()->SetDirty();
}