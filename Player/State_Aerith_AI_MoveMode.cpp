#include "stdafx.h"
#include "Aerith/State/State_List_Aerith.h"
#include "Event_Manager.h"
#include "StatusComp.h"

using namespace STATE_AERITH;

CState_Aerith_AI_MoveMode::CState_Aerith_AI_MoveMode(shared_ptr<CGameObject> pActor, shared_ptr<class CStateMachine> pStatemachine)
	:CState_Aerith(pActor, pStatemachine)
{
}

HRESULT CState_Aerith_AI_MoveMode::Initialize_State(CState* pPreviousState)
{
	if (m_pBehaviorTree == nullptr)
		Build_BehaviorTree();

	m_pActor_TransformCom.lock()->Set_Move_AnimationPosition(true);
	m_pActor_ModelCom.lock()->Set_Animation(m_strBattleMode[0] + "B_Idle01_1", 1.f, true);
		
	m_TargetInfo = GET_SINGLE(CClient_Manager)->Find_TargetPlayer(m_pActor, CLOUD);

	return S_OK;
}

void CState_Aerith_AI_MoveMode::Priority_Tick(_cref_time fTimeDelta)
{
	__super::Priority_Tick(fTimeDelta);
	if (m_bPlayable) //AI모드 해제
	{
		m_pStateMachineCom.lock()->Set_State<CState_Aerith_Idle>();
		return;
	}

	eVirtualActionKey = VIRTUAL_ACTIONKEY_NONE;
	GET_SINGLE(CClient_Manager)->Update_TargetInfo(m_pActor, m_TargetInfo); //타겟인 메인플레이어 위치 정보 업데이트

	m_pBehaviorTree->getBlackboard()->m_vPosition = m_pActor_TransformCom.lock()->Get_State(CTransform::STATE_POSITION); //Actor(Aerith) 위치 정보 블랙보드에 등록

	m_pBehaviorTree->update(fTimeDelta);

	m_bIsExcute = Action_Key_Input(m_pGameInstance, m_pStateMachineCom, m_bPlayable); //이벤트,스킬,공격 커맨드 등 실행 
}

void CState_Aerith_AI_MoveMode::Tick(_cref_time fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CState_Aerith_AI_MoveMode::Late_Tick(_cref_time fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

void CState_Aerith_AI_MoveMode::Transition_State(CState* pNextState)
{
	__super::Transition_State(pNextState);
}

bool CState_Aerith_AI_MoveMode::isValid_NextState(CState* state)
{
	return true;
}

void CState_Aerith_AI_MoveMode::Build_BehaviorTree()
{
	FUNCTION_NODE Condition_Idle
		= FUNCTION_NODE_MAKE
	{
		if (m_TargetInfo.fDistance <= 1.5f)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};

	FUNCTION_NODE Condition_Teleport
		= FUNCTION_NODE_MAKE
	{
		if (m_TargetInfo.fDistance >= 10.f || fabs(m_TargetInfo.vTargetPos.y - m_pActor_TransformCom.lock()->Get_State(CTransform::STATE_POSITION).m128_f32[1]) >= 2.f)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};

	FUNCTION_NODE Condition_Backward_Distance
		= FUNCTION_NODE_MAKE
	{
		if (m_TargetInfo.fDistance <= 0.5f)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};

	FUNCTION_NODE Call_Teleport
		= FUNCTION_NODE_MAKE
	{
		_float3 vPosition = GET_SINGLE(CCamera_Manager)->Get_ThirdPersonCamera(CLOUD).lock()->Get_TransformCom().lock()->Get_State(CTransform::STATE_POSITION);
		GET_SINGLE(CCamera_Manager)->Get_ThirdPersonCamera(AERITH).lock()->Get_TransformCom().lock()->Set_Position(1.f,vPosition);

		vPosition.y = m_TargetInfo.vTargetPos.y;
		m_pActor_TransformCom.lock()->Set_Position(1.f, vPosition);

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Idle
		= FUNCTION_NODE_MAKE
	{
		Reset_VirtualKey();
		if (!m_pActor_ModelCom.lock()->IsCurrentAnimation(m_strBattleMode[0] + "B_Idle01_1"))
		{
			m_pActor_ModelCom.lock()->Set_Animation(m_strBattleMode[0] + "B_Idle01_1", 1.f, true);
			m_pActor_TransformCom.lock()->Set_Move_AnimationPosition(false);
		}
		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_TurnToTarget_WithAnim
		= FUNCTION_NODE_MAKE
	{
		_bool bRun = eVirtualMoveKey == RUN ? true : false;
		m_DirKeyInfo = STATE_AERITH::eDirKeyInfo;

		if (m_pActor_ModelCom.lock()->Get_CurrentAnimationName().find("Run") == string::npos && m_pActor_ModelCom.lock()->Get_CurrentAnimationName().find("Walk") == string::npos)
		{
			m_pActor_TransformCom.lock()->Set_BaseDir(XMVector3Normalize(m_TargetInfo.vTargetPos - pBlackboard->m_vPosition));
			_float fAngle = m_pActor_TransformCom.lock()->Check_Look_On_BaseDir(m_DirKeyInfo.fRadianFromBaseDir);
			_float fDegree = XMConvertToDegrees(fAngle);

			m_pActor_TransformCom.lock()->Set_Move_AnimationPosition(true);

			if (InRange(fAngle, 0.35f * PI, 0.65f * PI, "[]"))
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0_R90", 2.0f, false) : m_pActor_ModelCom.lock()->Set_Animation(m_strCurBattleMode + "B_Walk01_0_R90", 2.0f, false);
				pBlackboard->setBool("TurnDir", false);
			}
			else if (InRange(fAngle, 1.35f * PI, 1.65f * PI, "[]"))
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0_L90", 2.0f, false) : m_pActor_ModelCom.lock()->Set_Animation(m_strCurBattleMode + "B_Walk01_0_L90", 2.0f, false);
				pBlackboard->setBool("TurnDir", false);
			}
			else if (InRange(fAngle, 0.75f * PI, 1.f * PI, "[)"))
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0_R180", 2.0f, false) : m_pActor_ModelCom.lock()->Set_Animation(m_strCurBattleMode + "B_Walk01_0_R180", 2.0f, false);
				pBlackboard->setBool("TurnDir", false);
			}
			else if (InRange(fAngle, 1.f * PI, 1.25f * PI, "[)"))
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0_L180", 2.0f, false) : m_pActor_ModelCom.lock()->Set_Animation(m_strCurBattleMode + "B_Walk01_0_L180", 2.0f, false);
				pBlackboard->setBool("TurnDir", false);
			}
			else
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0", 1.3f, false) : m_pActor_ModelCom.lock()->Set_Animation(m_strCurBattleMode + "B_Walk01_0", 1.3f, false);
				pBlackboard->setBool("TurnDir", true);
			}
		}
		else
		{
			if (m_pActor_ModelCom.lock()->IsAnimation_Finished())
				return BT_STATUS::SuccessForRepeater;
			else
				return BT_STATUS::Running;
		}

		if (pBlackboard->getBool("TurnDir"))
			Turn_Dir(fTimeDelta * 1.5f);

		return BT_STATUS::Running;
	};


	FUNCTION_NODE Call_Dir_UP
		= FUNCTION_NODE_MAKE
	{
		DirToRadian(UP);
		STATE_AERITH::eDirKeyInfo = m_DirKeyInfo;
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_MoveToTarget_M
		= FUNCTION_NODE_MAKE
	{
		if (m_TargetInfo.fDistance >= 1.8f)
			eVirtualMoveKey = RUN;
		else
			eVirtualMoveKey = WALK;

		eVirtualMoveKey == RUN ? m_pStateMachineCom.lock()->Set_State<CState_Aerith_Run>() : m_pStateMachineCom.lock()->Set_State<CState_Aerith_Walk>();

		m_pGameInstance->Add_TickEvent(m_pActor.lock(), TICKEVENT_MAKE
		{
			if (m_bPlayable || GET_SINGLE(CClient_Manager)->Get_AttackMode())
				return false;

			GET_SINGLE(CClient_Manager)->Update_TargetInfo(m_pActor, m_TargetInfo);

			if (m_TargetInfo.fDistance >= 1.8f)
				eVirtualMoveKey = RUN;
			else
				eVirtualMoveKey = WALK;

			if (m_TargetInfo.fDistance <= 1.0f)
			{
				Set_VirtualActionKey(VIRTUAL_ACTIONKEY_NONE);
				return false;
			}
			else
			{
				CState* pCurState = m_pStateMachineCom.lock()->Get_CurState();
				if (typeid(*pCurState) == typeid(CState_Aerith_AI_AttackMode))
					eVirtualMoveKey == RUN ? m_pStateMachineCom.lock()->Set_State<CState_Aerith_Run>() : m_pStateMachineCom.lock()->Set_State<CState_Aerith_Walk>();

				m_pActor_TransformCom.lock()->Set_BaseDir(m_TargetInfo.vDirToTarget);
				return true;
			}
		});

		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_MoveToTarget_L
		= FUNCTION_NODE_MAKE
	{
		m_pStateMachineCom.lock()->Set_State<CState_Aerith_WalkB>();

		m_pGameInstance->Add_TickEvent(m_pActor.lock(), TICKEVENT_MAKE
		{
			if (m_bPlayable || GET_SINGLE(CClient_Manager)->Get_AttackMode())
				return false;

			GET_SINGLE(CClient_Manager)->Update_TargetInfo(m_pActor, m_TargetInfo);

			if (m_TargetInfo.fDistance >= 0.7f)
			{
				Set_VirtualActionKey(VIRTUAL_ACTIONKEY_NONE);
				return false;
			}
			else
			{
				CState* pCurState = m_pStateMachineCom.lock()->Get_CurState();
				if (typeid(*pCurState) == typeid(CState_Aerith_AI_AttackMode))
					m_pStateMachineCom.lock()->Set_State<CState_Aerith_WalkB>();
	
				m_pActor_TransformCom.lock()->Set_BaseDir(m_TargetInfo.vDirToTarget);
				return true;
			}
		});

		return BT_STATUS::Success;
	};


	m_pBehaviorTree = Builder()
		.composite<Selector>()
			.composite<Sequence>()
				.leaf<FunctionNode>(Condition_Teleport)
				.leaf<FunctionNode>(Call_Teleport)
			.end()
			.composite<Sequence>()
				.leaf<FunctionNode>(Condition_Backward_Distance)
				.leaf<FunctionNode>(Call_Dir_UP)
				.leaf<FunctionNode>(Call_MoveToTarget_L)
			.end()
			.composite<Sequence>()
				.leaf<FunctionNode>(Condition_Idle)
				.leaf<FunctionNode>(Call_Idle)
			.end()			
			.composite<Sequence>()
				.leaf<FunctionNode>(Call_Dir_UP)
				.decorator<Repeater>()
					.leaf<FunctionNode>(Call_TurnToTarget_WithAnim)
				.end()
				.leaf<FunctionNode>(Call_MoveToTarget_M)
			.end()
		.end()
	.build();
}

void CState_Aerith_AI_MoveMode::Free()
{
	__super::Free();
}
