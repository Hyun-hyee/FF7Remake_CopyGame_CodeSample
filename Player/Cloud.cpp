#include "stdafx.h"
#include "Cloud.h"
#include "GameInstance.h"
#include "PhysX_Manager.h"
#include "Cloud_Weapon.h"
#include "Camera_Manager.h"
#include "State_List_Cloud.h"
#include "StatusComp.h"
#include "Map_Laser.h"

#include "Level_Test_Defines.h"

IMPLEMENT_CREATE(CCloud)
IMPLEMENT_CLONE(CCloud, CGameObject)

CCloud::CCloud(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	:CPlayer(pDevice, pContext)
{
}

CCloud::CCloud(const CCloud& rhs)
	:CPlayer(rhs)
{
	m_eObjectType = OBJTYPE::PLAYER;
	m_ePlayerType = CLOUD;
}

HRESULT CCloud::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		RETURN_EFAIL;
	return S_OK;
}

HRESULT CCloud::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		RETURN_EFAIL;

	if (FAILED(Ready_Components(pArg)))
		RETURN_EFAIL;

	if (FAILED(Ready_PartObjects()))
		RETURN_EFAIL;


#pragma region 메인 상태 등록
	m_pStateMachineCom->Add_State<CState_Cloud_Idle>();
	m_pStateMachineCom->Add_State<CState_Cloud_Run>();
	m_pStateMachineCom->Add_State<CState_Cloud_Walk>();
	m_pStateMachineCom->Add_State<CState_Cloud_WalkB>();
	m_pStateMachineCom->Add_State<CState_Cloud_WalkL>();
	m_pStateMachineCom->Add_State<CState_Cloud_WalkR>();
	m_pStateMachineCom->Add_State<CState_Cloud_AssertAttack01>();
	m_pStateMachineCom->Add_State<CState_Cloud_AssertAttack02>();
	m_pStateMachineCom->Add_State<CState_Cloud_AssertAttack03>();
	m_pStateMachineCom->Add_State<CState_Cloud_AssertAttack04>();
	m_pStateMachineCom->Add_State<CState_Cloud_AssertAttack05>();
	m_pStateMachineCom->Add_State<CState_Cloud_BraveAttack01>();
	m_pStateMachineCom->Add_State<CState_Cloud_BraveAttack02>();
	m_pStateMachineCom->Add_State<CState_Cloud_BraveAttack03>();
	m_pStateMachineCom->Add_State<CState_Cloud_BraveAttack04>();
	m_pStateMachineCom->Add_State<CState_Cloud_AbilityAttack_Infinity>();
	m_pStateMachineCom->Add_State<CState_Cloud_AbilityAttack_Slash>();
	m_pStateMachineCom->Add_State<CState_Cloud_AbilityAttack_Blade>();
	m_pStateMachineCom->Add_State<CState_Cloud_LimitAttack>();

	m_pStateMachineCom->Add_State<CState_Cloud_Avoid>();
	m_pStateMachineCom->Add_State<CState_Cloud_Abnormal>();
	m_pStateMachineCom->Add_State<CState_Cloud_Damage_Back>();
	m_pStateMachineCom->Add_State<CState_Cloud_Damage_Catch>();
	m_pStateMachineCom->Add_State<CState_Cloud_Damage_Front>();
	m_pStateMachineCom->Add_State<CState_Cloud_Die>();
	m_pStateMachineCom->Add_State<CState_Cloud_Event_PassOver>();
	m_pStateMachineCom->Add_State<CState_Cloud_Guard>();
	m_pStateMachineCom->Add_State<CState_Cloud_GuardRelease>();
	m_pStateMachineCom->Add_State<CState_Cloud_Item_Potion>();
	m_pStateMachineCom->Add_State<CState_Cloud_LadderDown>();
	m_pStateMachineCom->Add_State<CState_Cloud_LadderUp>();
	m_pStateMachineCom->Add_State<CState_Cloud_LadderDownFinished>();
	m_pStateMachineCom->Add_State<CState_Cloud_LadderUpFinished>();
	m_pStateMachineCom->Add_State<CState_Cloud_LeverDown>();
	m_pStateMachineCom->Add_State<CState_Cloud_Switch>();
	m_pStateMachineCom->Add_State<CState_Cloud_SneakIn>();
	m_pStateMachineCom->Add_State<CState_Cloud_AI_AttackMode>();
	m_pStateMachineCom->Add_State<CState_Cloud_Dance>();
	m_pStateMachineCom->Set_State<CState_Cloud_Idle>();
#pragma endregion

#pragma region 상체 상태머신(Add Anim전용) 클론
	shared_ptr<CStateMachine> pUpperBodyFSM = { nullptr };
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_StateMachine"),
		TEXT("Com_StateMachineCom_UpperBody"), &pUpperBodyFSM)))
		RETURN_EFAIL;

	m_pAdditionStateCom.emplace("UpperBody", pUpperBodyFSM);
	pUpperBodyFSM->Add_State<CStateUpperBody_Cloud_Idle>();
	pUpperBodyFSM->Add_State<CStateUpperBody_Cloud_Guard>();

	pUpperBodyFSM->Set_State<CStateUpperBody_Cloud_Idle>();
#pragma endregion

#pragma region 상체 상태머신(Add Anim전용) - 메인 상태머신 연결
	pUpperBodyFSM->Add_RefStateMachine("Main", m_pStateMachineCom);
	m_pStateMachineCom->Add_RefStateMachine("UpperBody", pUpperBodyFSM);
#pragma endregion

#pragma region 상체 ADD Anim 생성
	m_pModelCom->Get_AnimationComponent()->Create_Add_Animation();
	m_pModelCom->Get_AnimationComponent()->Create_Add_Animation();
#pragma endregion

	GET_SINGLE(CCamera_Manager)->Set_CamType(CCamera_Manager::CAM_THIRD);

	m_pStatusCom->Init_LimitBreak(100.f);

	m_TimeChecker = FTimeChecker(0.3f);
	
	GET_SINGLE(CClient_Manager)->Register_Player(PLAYER_TYPE::CLOUD, static_pointer_cast<CPlayer>(shared_from_this()));

	return S_OK;
}

void CCloud::Begin_Play(_cref_time fTimeDelta)
{
	__super::Begin_Play(fTimeDelta);

	if (m_pModelCom)
	{
		//Controller 추가
		PHYSXCONTROLLER_DESC ControllerDesc = {};
		PhysXControllerDesc::Setting_Controller(ControllerDesc, m_pTransformCom, CL_PLAYER_CONTROLLER, 1.0f, 0.3f);

		if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_PhysX_Controller"),
			TEXT("Com_PhysXControllerCom"), &(m_pPhysXControllerCom), &ControllerDesc)))
			return;

		m_pTransformCom->Set_PhysXControllerCom(m_pPhysXControllerCom);

		//몸체 데미지용 충돌체(Dynamic-Kinematic) 추가 
		PHYSXCOLLIDER_DESC ColliderDesc = {};
		PhysXColliderDesc::Setting_DynamicCollider_WithScale(ColliderDesc, PHYSXCOLLIDER_TYPE::BOX, CL_PLAYER_BODY, m_pTransformCom, { 0.3f,1.5f,0.3f }, false, nullptr, true);
		m_vPhysXColliderLocalOffset = { 0.f,0.9f,0.f };

		if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_PhysX_Collider"),
			TEXT("Com_PhysXColliderCom"), &(m_pPhysXColliderCom), &ColliderDesc)))
			return;
	}

	// 스탯 설정
	m_pStatusCom->Init_StatsByTable(CStatusComp::ETableType::Character, "Cloud");

	GET_SINGLE(CCamera_Manager)->Set_Target_ThirdPersonCamera(CLOUD, m_pTransformCom);

	m_pModelCom->Get_AnimationComponent()->Create_MaskAnim();
	m_pModelCom->Set_Animation(0, 1.f, true);
}

void CCloud::Priority_Tick(_cref_time fTimeDelta)
{
	__super::Priority_Tick(fTimeDelta);

	// 상체 상태 머신(Add Anim전용) Tick
	for (auto iter = m_pAdditionStateCom.begin(); iter != m_pAdditionStateCom.end(); ++iter)
		(*iter).second->Priority_Tick(fTimeDelta);
}

void CCloud::Tick(_cref_time fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CCloud::Late_Tick(_cref_time fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

void CCloud::Before_Render(_cref_time fTimeDelta)
{
	__super::Before_Render(fTimeDelta);

	//캐릭터 렌더
	if (FAILED(m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND_CHARACTER, shared_from_this())))
		return;

	//셰도우 렌더
	if (FAILED(m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, shared_from_this())))
		return;
}

void CCloud::End_Play(_cref_time fTimeDelta)
{
	__super::End_Play(fTimeDelta);
}

HRESULT CCloud::Render()
{
	if (m_pModelCom)
	{
		if (FAILED(Bind_ShaderResources()))
			RETURN_EFAIL;

		auto ActiveMeshes = m_pModelCom->Get_ActiveMeshes();
		for (_uint i = 0; i < ActiveMeshes.size(); ++i)
		{
			// 뼈 바인딩
			if (FAILED(m_pModelCom->Bind_BoneToShader(ActiveMeshes[i], "g_BoneMatrices")))
				RETURN_EFAIL;

			// 텍스처 바인딩
			if (FAILED(m_pModelCom->Bind_MeshMaterialToShader(ActiveMeshes[i], TextureType_DIFFUSE, "g_DiffuseTexture")))
				RETURN_EFAIL;
			if (FAILED(m_pModelCom->Bind_MeshMaterialToShader(ActiveMeshes[i], TextureType_NORMALS, "g_NormalTexture")))
				RETURN_EFAIL;
			if (FAILED(m_pModelCom->Bind_MeshMaterialToShader(ActiveMeshes[i], TextureType_DIFFUSE_ROUGHNESS, "g_MRVTexture")))
				RETURN_EFAIL;
			if (FAILED(m_pModelCom->Bind_MeshMaterialToShader(ActiveMeshes[i], TextureType_METALNESS, "g_MTexture")))
				RETURN_EFAIL;
			if (FAILED(m_pModelCom->Bind_MeshMaterialToShader(ActiveMeshes[i], TextureType_EMISSION_COLOR, "g_EmissiveTexture")))
				RETURN_EFAIL;
			if (FAILED(m_pModelCom->Bind_MeshMaterialToShader(ActiveMeshes[i], TextureType_AMBIENT_OCCLUSION, "g_AOTexture")))
				RETURN_EFAIL;
			if (FAILED(m_pModelCom->Bind_MeshMaterialToShader(ActiveMeshes[i], TextureType_OPACITY, "g_AlphaTexture")))
				RETURN_EFAIL;

			// 패스, 버퍼 바인딩
			if (FAILED(m_pModelCom->ShaderComp()->Begin(0)))
				RETURN_EFAIL;
			//버퍼 렌더
			if (FAILED(m_pModelCom->BindAndRender_Mesh(ActiveMeshes[i])))
				RETURN_EFAIL;
		}
	}
	return S_OK;
}

HRESULT CCloud::Ready_Components(void* pArg)
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_AnimCommonModel"),
		TEXT("Com_ModelCom"), &(m_pModelCom), nullptr)))
		RETURN_EFAIL;

	if (m_pModelCom)
	{
		m_pModelCom->Link_Model(CCommonModelComp::TYPE_ANIM, TEXT("PC_0000_00_Cloud"));
		if (FAILED(m_pModelCom->Link_Shader(L"Shader_AnimModel", VTXMESHANIM::Elements, VTXMESHANIM::iNumElements)))
			RETURN_EFAIL;

		m_pModelCom->Set_PreRotate(
			XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-90.f)));

		// 노티파이 프로토타입 클로닝 + 모델 컴포넌트 전달(Weak)
		m_pModelCom->Clone_NotifyFromPrototype(TEXT("Prototype_Component_Notify_Cloud"));
		m_pModelCom->Regist_ModelCompToNotify(m_pModelCom);
	}

	return S_OK;
}

HRESULT CCloud::Bind_ShaderResources()
{
	// 공통 행렬 바인딩
	if (FAILED(m_pModelCom->ShaderComp()->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::TS_VIEW))))
		RETURN_EFAIL;
	if (FAILED(m_pModelCom->ShaderComp()->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::TS_PROJ))))
		RETURN_EFAIL;

	// 월드 행렬 바인딩
	if (FAILED(m_pModelCom->ShaderComp()->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4())))
		RETURN_EFAIL;

	return S_OK;
}

HRESULT CCloud::Ready_PartObjects()
{
	/*무기 PartObject 추가*/
	CPartObject::PARTOBJECT_DESC Desc;
	Desc.pParentTransform = m_pTransformCom;
	Desc.pBoneGroup = m_pModelCom->Get_BoneGroup();
	Desc.strBoneName = TEXT("R_Weapon_a");

	if (FAILED(Add_PartObject<CCloud_Weapon>(TEXT("Prototype_GameObject_Cloud_Weapon"), TEXT("Part_Weapon"), &Desc, nullptr)))
		RETURN_EFAIL;

	return S_OK;
}

void CCloud::PhysX_OnCollision_Enter(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
	__super::PhysX_OnCollision_Enter(pThisCol, pOtherCol, ContactInfo);

	if (pOtherCol->Get_ColliderDesc().iFilterType == CL_MONSTER_ATTACK || pOtherCol->Get_ColliderDesc().iFilterType == CL_LASER)
	{
		CState* pCurState = m_pStateMachineCom->Get_CurState();
		if (typeid(*pCurState) != typeid(CState_Cloud_Avoid)
			&& typeid(*pCurState) != typeid(CState_Cloud_AbilityAttack_Infinity)
			&& typeid(*pCurState) != typeid(CState_Cloud_AbilityAttack_Blade)
			&& typeid(*pCurState) != typeid(CState_Cloud_AbilityAttack_Slash)
			&& typeid(*pCurState) != typeid(CState_Cloud_LimitAttack)
			&& typeid(*pCurState) != typeid(CState_Cloud_Damage_Front)
			&& typeid(*pCurState) != typeid(CState_Cloud_Damage_Back))
		{
			PxContactPairPoint* pContactPoint = new PxContactPairPoint[ContactInfo.contactCount];
			PxU32 nbContacts = ContactInfo.extractContacts(pContactPoint, ContactInfo.contactCount);

			_vector vHitDir = XMVector3Normalize(XMVectorSetY(Convert_Vector(pContactPoint[0].normal), 0.f));
			_vector vLookDir = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));

			//데미지 방향 판단
			if (XMVectorGetX(XMVector3Length(vHitDir - vLookDir)) <= XMVectorGetX(XMVector3Length(vHitDir + vLookDir)) || typeid(*(pOtherCol->Get_Owner().lock().get())) == typeid(CMap_Laser))
			{
				m_pStateMachineCom->Set_State<CState_Cloud_Damage_Front>();
			}
			else
			{
				m_pStateMachineCom->Set_State<CState_Cloud_Damage_Back>();
			}

			Safe_Delete(pContactPoint);
		}
	}
}

void CCloud::PhysX_OnCollision_Stay(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
	__super::PhysX_OnCollision_Stay(pThisCol, pOtherCol, ContactInfo);


}

void CCloud::PhysX_OnCollision_Exit(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo)
{
	__super::PhysX_OnCollision_Exit(pThisCol, pOtherCol, ContactInfo);
}

void CCloud::PhysX_Raycast_Stay(weak_ptr<CGameObject> pOther, _uint iOtherColLayer, _float4 vOrigin, _float4 vDirection, _float4 vColPosition)
{
	__super::PhysX_Raycast_Stay(pOther, iOtherColLayer, vOrigin, vDirection, vColPosition);

	if (iOtherColLayer == CL_MONSTER_ATTACK)
	{
		CState* pCurState = m_pStateMachineCom->Get_CurState();
		if (typeid(*pCurState) != typeid(CState_Cloud_Avoid)
			&& typeid(*pCurState) != typeid(CState_Cloud_AbilityAttack_Infinity)
			&& typeid(*pCurState) != typeid(CState_Cloud_AbilityAttack_Blade)
			&& typeid(*pCurState) != typeid(CState_Cloud_AbilityAttack_Slash)
			&& typeid(*pCurState) != typeid(CState_Cloud_LimitAttack)
			&& typeid(*pCurState) != typeid(CState_Cloud_Damage_Front)
			&& typeid(*pCurState) != typeid(CState_Cloud_Damage_Back))
		{
			_vector vHitDir = XMVector3Normalize(XMVectorSetY(vDirection, 0.f));
			_vector vLookDir = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
			if (XMVectorGetX(XMVector3Length(vHitDir - vLookDir)) <= XMVectorGetX(XMVector3Length(vHitDir + vLookDir)))
				m_pStateMachineCom->Set_State<CState_Cloud_Damage_Front>();
			else
				m_pStateMachineCom->Set_State<CState_Cloud_Damage_Back>();
		}
	}
}

void CCloud::Status_Damaged(_uint iStatusDamaged, _uint iHitPower, _uint iAddHitPower)
{
	//상대 HitPower에 따라 Damage 애니메이션 다르게 적용
	m_pStateMachineCom->Find_State<CState_Cloud_Damage_Front>()->Set_DamageType(iHitPower);
	m_pStateMachineCom->Find_State<CState_Cloud_Damage_Back>()->Set_DamageType(iHitPower);

	CState* pCurState = m_pStateMachineCom->Get_CurState();
	if (typeid(*pCurState) != typeid(CState_Cloud_Avoid)
		&& typeid(*pCurState) != typeid(CState_Cloud_AbilityAttack_Infinity)
		&& typeid(*pCurState) != typeid(CState_Cloud_AbilityAttack_Blade)
		&& typeid(*pCurState) != typeid(CState_Cloud_AbilityAttack_Slash)
		&& typeid(*pCurState) != typeid(CState_Cloud_LimitAttack)
		&& typeid(*pCurState) != typeid(CState_Cloud_Damage_Front)
		&& typeid(*pCurState) != typeid(CState_Cloud_Damage_Back))
	{
		switch (iHitPower)
		{
		case 5:
			m_pModelCom->AnimationComp()->Set_ADD_Animation(0, L"Battle00|B_Dmg01_Add", L"C_Spine_a", { L"C_Spine_c" }
			, 4, 1.f, 1.f, false);
		case 0:
			m_pStateMachineCom->Set_NextState(m_pStateMachineCom->Get_CurState());
			break;
		defualt:
			break;
		}
	}

}

void CCloud::Set_State_AIMode(_bool bAIMode)
{
	if (bAIMode)
		m_pStateMachineCom->Set_State<CState_Cloud_AI_AttackMode>();
	else
		m_pStateMachineCom->Wait_State<CState_Cloud_Idle>();
}

void CCloud::Free()
{
	__super::Free();
}