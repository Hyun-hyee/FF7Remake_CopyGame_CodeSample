#include "stdafx.h"
#include "State_List_Cloud.h"
#include "Event_Manager.h"
#include "StatusComp.h"
using namespace STATE_CLOUD;

void CState_Cloud_AI_AttackMode::Free()
{
}

CState_Cloud_AI_AttackMode::CState_Cloud_AI_AttackMode(shared_ptr<class CGameObject> pActor, shared_ptr<class CStateMachine> pStatemachine)
	:CState_Cloud(pActor, pStatemachine)
{
}

HRESULT CState_Cloud_AI_AttackMode::Initialize_State(CState* pPreviousState)
{
	if (m_pBehaviorTree == nullptr)
		Build_BehaviorTree();

	if (pPreviousState && (typeid(*pPreviousState) == typeid(CState_Cloud_Damage_Back) || typeid(*pPreviousState) == typeid(CState_Cloud_Damage_Front)))
		++m_iDamageCount;

	m_pActor_TransformCom.lock()->Set_Move_AnimationPosition(true);
	m_pActor_ModelCom.lock()->Set_Animation(m_strBattleMode[0] + "Idle01_1", 1.f, true);

	return S_OK;
}

void CState_Cloud_AI_AttackMode::Priority_Tick(_cref_time fTimeDelta)
{
	__super::Priority_Tick(fTimeDelta);

	if (m_bPlayable) //AI모드 해제
	{
		m_pStateMachineCom.lock()->Set_State<CState_Cloud_Idle>();
		return;
	}

	STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::VIRTUAL_ACTIONKEY_NONE;
	
	if (!m_TargetInfo.pTarget.lock() || m_TargetInfo.pTarget.lock()->IsState(OBJSTATE::DeadAnim) || !m_TargetInfo.pTarget.lock()->IsState(OBJSTATE::Active))
	{
		m_TargetInfo = GET_SINGLE(CClient_Manager)->Find_TargetMonster(CLOUD, 0); //타겟 변경
	}
	else
		GET_SINGLE(CClient_Manager)->Update_TargetInfo(m_pActor,m_TargetInfo); //타겟 정보 업데이트

	m_pBehaviorTree->getBlackboard()->m_vPosition = m_pActor_TransformCom.lock()->Get_State(CTransform::STATE_POSITION); //Actor(Cloud) 위치 정보 블랙보드에 등록

	m_pBehaviorTree->update(fTimeDelta);
	
	m_bIsExcute = STATE_CLOUD::Action_Key_Input(m_pGameInstance, m_pStateMachineCom,m_bPlayable); //이벤트,스킬,공격 커맨드 등 실행
}

void CState_Cloud_AI_AttackMode::Tick(_cref_time fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CState_Cloud_AI_AttackMode::Late_Tick(_cref_time fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

void CState_Cloud_AI_AttackMode::Transition_State(CState* pNextState)
{
	__super::Transition_State(pNextState);
}

bool CState_Cloud_AI_AttackMode::isValid_NextState(CState* state)
{
	return true;
}



void CState_Cloud_AI_AttackMode::Build_BehaviorTree()
{
	FUNCTION_NODE Condition_Heal
		= FUNCTION_NODE_MAKE
	{
		if (m_pStatusCom.lock()->Get_HP_Percent() <= 0.4 )
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};

	FUNCTION_NODE Call_Heal
		= FUNCTION_NODE_MAKE
	{
		//HealState추가	
		Set_VirtualActionKey(STATE_CLOUD::ITEM_POTION);
		return BT_STATUS::Success;
	};


	FUNCTION_NODE Condition_AttackMode
		= FUNCTION_NODE_MAKE
	{
		if ((m_TargetInfo.pTarget.lock() == nullptr) || (m_TargetInfo.pTarget.lock()->IsState(OBJSTATE::DeadAnim) || m_TargetInfo.fDistance > 15.f || m_TargetInfo.pTarget.lock()->Get_ObjectType() == OBJTYPE::PLAYER))
			m_TargetInfo = GET_SINGLE(CClient_Manager)->Find_TargetMonster(CLOUD, 0);
			
		if(m_TargetInfo.pTarget.lock() != nullptr)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};

	FUNCTION_NODE Condition_FollowPlayer
		= FUNCTION_NODE_MAKE
	{
		TARGETINFO TargetInfo = GET_SINGLE(CClient_Manager)->Find_TargetPlayer(m_pActor,AERITH);

		if (TargetInfo.fDistance >= 10.f)
		{
			m_TargetInfo = TargetInfo;
			return BT_STATUS::Success;
		}
		else
		{
			return BT_STATUS::Failure;
		}
	};
	
	FUNCTION_NODE Condition_ActionPower_Minus10
		= FUNCTION_NODE_MAKE
	{
		_bool bActionPower = DynPtrCast<IStatusInterface>(m_TargetInfo.pTarget.lock())->Get_StatusCom().lock()->IsActionPowerEqual(-10);

		if (bActionPower)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};


	FUNCTION_NODE Condition_ActionPower_0
		= FUNCTION_NODE_MAKE
	{
		_bool bActionPower = DynPtrCast<IStatusInterface>(m_TargetInfo.pTarget.lock())->Get_StatusCom().lock()->IsActionPowerEqual(0);

		if (bActionPower)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};


	FUNCTION_NODE Condition_ActionPower_5
		= FUNCTION_NODE_MAKE
	{
		_bool bActionPower = DynPtrCast<IStatusInterface>(m_TargetInfo.pTarget.lock())->Get_StatusCom().lock()->IsActionPowerEqual(5);

		if (bActionPower)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};


	FUNCTION_NODE Condition_ActionPower_10
		= FUNCTION_NODE_MAKE
	{
		_bool bActionPower = DynPtrCast<IStatusInterface>(m_TargetInfo.pTarget.lock())->Get_StatusCom().lock()->IsActionPowerEqual(10);

		if (bActionPower)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};


	FUNCTION_NODE Condition_ActionPower_20
		= FUNCTION_NODE_MAKE
	{
		if (!m_TargetInfo.pTarget.lock())
			return BT_STATUS::Success;

		_bool bActionPower = DynPtrCast<IStatusInterface>(m_TargetInfo.pTarget.lock())->Get_StatusCom().lock()->IsActionPowerEqual(20);

		if (bActionPower)
		{
			m_TargetInfo = TARGETINFO();
		}
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Idle
		= FUNCTION_NODE_MAKE
	{
		if (!m_pActor_ModelCom.lock()->IsCurrentAnimation(m_strBattleMode[0] + "Idle01_1"))
		{
			m_pActor_ModelCom.lock()->Set_Animation(m_strBattleMode[0] + "Idle01_1", 1.f, true);
			m_pActor_TransformCom.lock()->Set_Move_AnimationPosition(false);
		}

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Random_SkillType
		= FUNCTION_NODE_MAKE
	{
		_int iRandomNum = m_pGameInstance->RandomInt(0,3);
		pBlackboard->setInt("SkillType", iRandomNum);
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Condition_Forward_Distance
		= FUNCTION_NODE_MAKE
	{		
		if (m_TargetInfo.fDistance >= 5.f)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};

	FUNCTION_NODE Condition_Backward_Distance
		= FUNCTION_NODE_MAKE
	{
		if (m_TargetInfo.fDistance <= 0.7f)
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};

	FUNCTION_NODE Call_LookToTarget
		= FUNCTION_NODE_MAKE
	{
		m_pActor_TransformCom.lock()->Set_Look(m_TargetInfo.vDirToTarget);

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_LookToTarget_Inv
		= FUNCTION_NODE_MAKE
	{
		m_pActor_TransformCom.lock()->Set_Look(-m_TargetInfo.vDirToTarget);

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_TurnToTarget
		= FUNCTION_NODE_MAKE
	{
		m_pActor_TransformCom.lock()->Set_BaseDir(m_TargetInfo.vDirToTarget);

		if (m_pActor_TransformCom.lock()->Rotate_On_BaseDir(fTimeDelta, 0.f))
			return BT_STATUS::Success;
		else
			return BT_STATUS::Failure;
	};



	FUNCTION_NODE Call_TurnToTarget_WithAnim
		= FUNCTION_NODE_MAKE
	{
		_bool bRun = STATE_CLOUD::eVirtualMoveKey == STATE_CLOUD::RUN ? true : false;
		m_DirKeyInfo = STATE_CLOUD::eDirKeyInfo;

		if (m_pActor_ModelCom.lock()->Get_CurrentAnimationName().find("Run") == string::npos && m_pActor_ModelCom.lock()->Get_CurrentAnimationName().find("Walk") == string::npos)
		{
			m_pActor_TransformCom.lock()->Set_BaseDir(XMVector3Normalize(m_TargetInfo.vTargetPos - pBlackboard->m_vPosition));
			_float fAngle = m_pActor_TransformCom.lock()->Check_Look_On_BaseDir(m_DirKeyInfo.fRadianFromBaseDir);
			_float fDegree = XMConvertToDegrees(fAngle);

			m_pActor_TransformCom.lock()->Set_Move_AnimationPosition(true);

			if (InRange(fAngle, 0.35f * PI, 0.65f * PI, "[]"))
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0_R90", 2.0f, false) : m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Walk01_0_R90", 2.0f, false);
				pBlackboard->setBool("TurnDir", false);
			}
			else if (InRange(fAngle, 1.35f * PI, 1.65f * PI, "[]"))
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0_L90", 2.0f, false) : m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Walk01_0_L90", 2.0f, false);
				pBlackboard->setBool("TurnDir", false);
			}
			else if (InRange(fAngle, 0.75f * PI, 1.f * PI, "[)"))
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0_R180", 2.0f, false) : m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Walk01_0_R180", 2.0f, false);
				pBlackboard->setBool("TurnDir", false);
			}
			else if (InRange(fAngle, 1.f * PI, 1.25f * PI, "[)"))
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0_L180", 2.0f, false) : m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Walk01_0_L180", 2.0f, false);
				pBlackboard->setBool("TurnDir", false);
			}
			else
			{
				bRun ? m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Run01_0", 1.3f, false) : m_pActor_ModelCom.lock()->Set_Animation("Battle00|B_Walk01_0", 1.3f, false);
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
		STATE_CLOUD::eDirKeyInfo = m_DirKeyInfo;
		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_Dir_DOWN
		= FUNCTION_NODE_MAKE
	{
		DirToRadian(DOWN);
		STATE_CLOUD::eDirKeyInfo = m_DirKeyInfo;
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Dir_DOWNLEFT
		= FUNCTION_NODE_MAKE
	{
		DirToRadian(DOWN | LEFT);
		STATE_CLOUD::eDirKeyInfo = m_DirKeyInfo;
		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_Dir_RIGHT
		= FUNCTION_NODE_MAKE
	{
		DirToRadian(RIGHT);
		STATE_CLOUD::eDirKeyInfo = m_DirKeyInfo;
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Dir_LEFT
		= FUNCTION_NODE_MAKE
	{
		DirToRadian(LEFT);
		STATE_CLOUD::eDirKeyInfo = m_DirKeyInfo;
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Move_Run
		= FUNCTION_NODE_MAKE
	{
		Set_VirtualMoveKey(STATE_CLOUD::RUN);
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Move_Walk
		= FUNCTION_NODE_MAKE
	{
		Set_VirtualMoveKey(STATE_CLOUD::WALK);
		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_Move_WalkB
		= FUNCTION_NODE_MAKE
	{
		Set_VirtualMoveKey(STATE_CLOUD::WALKB);
		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_Move_WalkL
		= FUNCTION_NODE_MAKE
	{
		Set_VirtualMoveKey(STATE_CLOUD::WALKL);
		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_Move_WalkR
		= FUNCTION_NODE_MAKE
	{
		Set_VirtualMoveKey(STATE_CLOUD::WALKR);
		return BT_STATUS::Success;
	};



	FUNCTION_NODE Call_MoveToTarget_M5F
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualMoveKey == STATE_CLOUD::RUN ? m_pStateMachineCom.lock()->Set_State<CState_Cloud_Run>() : m_pStateMachineCom.lock()->Set_State<CState_Cloud_Walk>();

		m_pGameInstance->Add_TickEvent(m_pActor.lock(), TICKEVENT_MAKE
		{
			if (m_bPlayable)
				return false;
			
			GET_SINGLE(CClient_Manager)->Update_TargetInfo(m_pActor, m_TargetInfo);
			
			if (m_TargetInfo.fDistance >= 8.f)
				STATE_CLOUD::eVirtualMoveKey = STATE_CLOUD::RUN;
			else
				STATE_CLOUD::eVirtualMoveKey = STATE_CLOUD::WALK;

			if (m_TargetInfo.fDistance <= 5.f)
			{
				if(m_TargetInfo.pTarget.lock() && m_TargetInfo.pTarget.lock()->Get_ObjectType() != OBJTYPE::PLAYER)
					Set_VirtualActionKey(ASSERT_ATTACK1);
				else
					Set_VirtualActionKey(STATE_CLOUD::VIRTUAL_ACTIONKEY_NONE);

				return false;
			}
			else
			{
				CState* pCurState = m_pStateMachineCom.lock()->Get_CurState();
				if(typeid(*pCurState) == typeid(CState_Cloud_AI_AttackMode))
					STATE_CLOUD::eVirtualMoveKey == STATE_CLOUD::RUN ? m_pStateMachineCom.lock()->Set_State<CState_Cloud_Run>() : m_pStateMachineCom.lock()->Set_State<CState_Cloud_Walk>();

				m_pActor_TransformCom.lock()->Set_BaseDir(m_TargetInfo.vDirToTarget);
				return true;
			}
		});

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_MoveToTarget_L5F
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualMoveKey == STATE_CLOUD::RUN ? m_pStateMachineCom.lock()->Set_State<CState_Cloud_Run>() : m_pStateMachineCom.lock()->Set_State<CState_Cloud_Walk>();
		
		m_pGameInstance->Add_TickEvent(m_pActor.lock(), TICKEVENT_MAKE
		{
			if (m_bPlayable)
				return false;

			GET_SINGLE(CClient_Manager)->Update_TargetInfo(m_pActor, m_TargetInfo);

			if (m_TargetInfo.fDistance >= 5.f)
			{
				STATE_CLOUD::Reset_VirtualKey();
				return false;
			}
			else
			{
				CState* pCurState = m_pStateMachineCom.lock()->Get_CurState();
				if (typeid(*pCurState) == typeid(CState_Cloud_AI_AttackMode))
					STATE_CLOUD::eVirtualMoveKey == STATE_CLOUD::RUN ? m_pStateMachineCom.lock()->Set_State<CState_Cloud_Run>() : m_pStateMachineCom.lock()->Set_State<CState_Cloud_Walk>();

				m_pActor_TransformCom.lock()->Set_BaseDir(m_TargetInfo.vDirToTarget);
				return true;
			}
		});

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_TimeMax_1F
		= FUNCTION_NODE_MAKE
	{
		m_fTimeMax = 1.f;
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_TimeMax_1F_HALF
		= FUNCTION_NODE_MAKE
	{
		m_fTimeMax = 1.5f;
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_TimeMax_2F
		= FUNCTION_NODE_MAKE
	{
		m_fTimeMax = 2.f;
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_TimeMax_2F_HALF
		= FUNCTION_NODE_MAKE
	{
		m_fTimeMax = 2.5f;
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_MoveToTarget_Time
		= FUNCTION_NODE_MAKE
	{
		switch (STATE_CLOUD::eVirtualMoveKey)
		{
		case STATE_CLOUD::WALK:
			m_pStateMachineCom.lock()->Set_State<CState_Cloud_Walk>();
			break;
		case STATE_CLOUD::WALKR:
			m_pStateMachineCom.lock()->Set_State<CState_Cloud_WalkR>();
			break;
		case STATE_CLOUD::WALKL:
			m_pStateMachineCom.lock()->Set_State<CState_Cloud_WalkL>();
			break;
		case STATE_CLOUD::WALKB:
			m_pStateMachineCom.lock()->Set_State<CState_Cloud_WalkB>();
			break;
		case STATE_CLOUD::RUN:
			m_pStateMachineCom.lock()->Set_State<CState_Cloud_Run>();
			break;
		}

		m_pGameInstance->Add_TickEvent(m_pActor.lock(), TICKEVENT_MAKE
		{
			if (m_bPlayable)
				return false;

			GET_SINGLE(CClient_Manager)->Update_TargetInfo(m_pActor, m_TargetInfo);
				

			if (m_fTimeMax.Update(fTimeDelta))
			{
				//if (m_TargetInfo.pTarget.lock() && m_TargetInfo.pTarget.lock()->Get_ObjectType() != OBJTYPE::PLAYER)
				//	Set_VirtualActionKey(ASSERT_ATTACK1);
				//else
					Set_VirtualActionKey(STATE_CLOUD::VIRTUAL_ACTIONKEY_NONE);

				return false;
			}
			else
			{
				CState* pCurState = m_pStateMachineCom.lock()->Get_CurState();
				if (typeid(*pCurState) == typeid(CState_Cloud_AI_AttackMode))
				{
					switch (STATE_CLOUD::eVirtualMoveKey)
					{
					case STATE_CLOUD::WALK:
						m_pStateMachineCom.lock()->Set_State<CState_Cloud_Walk>();
						break;
					case STATE_CLOUD::WALKR:
						m_pStateMachineCom.lock()->Set_State<CState_Cloud_WalkR>();
						break;
					case STATE_CLOUD::WALKL:
						m_pStateMachineCom.lock()->Set_State<CState_Cloud_WalkL>();
						break;
					case STATE_CLOUD::WALKB:
						m_pStateMachineCom.lock()->Set_State<CState_Cloud_WalkB>();
						break;
					case STATE_CLOUD::RUN:
						m_pStateMachineCom.lock()->Set_State<CState_Cloud_Run>();
						break;
					}
				}
				m_pActor_TransformCom.lock()->Set_BaseDir(m_TargetInfo.vDirToTarget);
				return true;
			}
		});

		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_Action_AssertAttack_1
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::ASSERT_ATTACK1;
		
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Action_AssertAttack_2
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::ASSERT_ATTACK2;
		
		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Action_AssertAttack_3
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::ASSERT_ATTACK3;

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Action_AssertAttack_4
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::ASSERT_ATTACK4;

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Action_AssertAttack_5
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::ASSERT_ATTACK5;

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Action_BraveAttack_1
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::BRAVE_ATTACK1;

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Action_BraveAttack_2
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::BRAVE_ATTACK2;

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Action_BraveAttack_3
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::BRAVE_ATTACK3;

		return BT_STATUS::Success;
	};

	FUNCTION_NODE Call_Action_BraveAttack_4
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::BRAVE_ATTACK4;

		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_Action_AbilitySlash
		= FUNCTION_NODE_MAKE
	{
		if (pBlackboard->getInt("SkillType") == 0)
		{
			STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::ABILITY_ATTACK_SLASH;
			return BT_STATUS::Success;
		}
		else
			return BT_STATUS::Failure;

	};

	FUNCTION_NODE Call_Action_AbilityInfinity
		= FUNCTION_NODE_MAKE
	{
		if (pBlackboard->getInt("SkillType") == 1)
		{
			STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::ABILITY_ATTACK_SLASH;
			return BT_STATUS::Success;
		}
		else
			return BT_STATUS::Failure;

	};

	FUNCTION_NODE Call_Action_AbilityBlade
		= FUNCTION_NODE_MAKE
	{
		if (pBlackboard->getInt("SkillType") == 2)
		{
			STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::ABILITY_ATTACK_SLASH;
			return BT_STATUS::Success;
		}
		else
			return BT_STATUS::Failure;

	};




	FUNCTION_NODE Condition_Avoid
		= FUNCTION_NODE_MAKE
	{
		if (m_iDamageCount >= 5)
		{
			m_iDamageCount = 0;
			return BT_STATUS::Success;
		}
		else
			return BT_STATUS::Failure;
	};

	FUNCTION_NODE Call_Action_Avoid
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::AVOID;

		return BT_STATUS::Success;
	};


	FUNCTION_NODE Call_Action_Guard
		= FUNCTION_NODE_MAKE
	{
		STATE_CLOUD::eVirtualActionKey = STATE_CLOUD::GUARD;
		//Guard시간 + TickEvnet넣기
		m_fTimeMax = 4.f;

		m_pStateMachineCom.lock()->Set_State<CState_Cloud_Guard>();
		m_pGameInstance->Add_TickEvent(m_pActor.lock(), TICKEVENT_MAKE
		{

			if (m_bPlayable)
			{
				m_pStateMachineCom.lock()->Set_State<CState_Cloud_Idle>();
				return false;
			}

			GET_SINGLE(CClient_Manager)->Update_TargetInfo(m_pActor, m_TargetInfo);
			m_pActor_TransformCom.lock()->Set_Look(m_TargetInfo.vDirToTarget);

			if (m_fTimeMax.Update(fTimeDelta) || (m_TargetInfo.pTarget.lock() && DynPtrCast<IStatusInterface>(m_TargetInfo.pTarget.lock())->Get_StatusCom().lock()->Get_ActionPower() != 5))
			{				
				m_pStateMachineCom.lock()->Set_State<CState_Cloud_AI_AttackMode>();
				return false;
			}
			else 
				return true;

		});

		return BT_STATUS::Success;
	};



	m_pBehaviorTree = Builder()
			.composite<Selector>()
				.composite<Sequence>() //체력 체크 
					.leaf<FunctionNode>(Condition_Heal)
					.leaf<FunctionNode>(Call_Heal)
				.end() 
				.composite<Sequence>()//몬스터X + 플레이어와 거리 유지
					.leaf<FunctionNode>(Condition_FollowPlayer)		
					.leaf<FunctionNode>(Call_Dir_UP)
					.leaf<FunctionNode>(Call_Move_Walk)
					.decorator<Repeater>()
						.leaf<FunctionNode>(Call_TurnToTarget_WithAnim)
					.end()
					.leaf<FunctionNode>(Call_MoveToTarget_M5F)
				.end()
				.composite<Sequence>()
					.leaf<FunctionNode>(Condition_ActionPower_20)
					.leaf<FunctionNode>(Condition_AttackMode) //몬스터 유무
					.composite<Selector>()
					    .composite<Sequence>()
							.leaf<FunctionNode>(Condition_Avoid)
							.leaf<FunctionNode>(Call_Dir_DOWNLEFT)
							.decorator<Repeater>()
								.leaf<FunctionNode>(Call_TurnToTarget_WithAnim)
							.end()
							.leaf<FunctionNode>(Call_Action_Avoid)
						.end()
						.composite<Sequence>()
							.leaf<FunctionNode>(Condition_Forward_Distance)
							.leaf<FunctionNode>(Call_Dir_UP)
							.leaf<FunctionNode>(Call_Move_Walk)
							.decorator<Repeater>()
								.leaf<FunctionNode>(Call_TurnToTarget_WithAnim)
							.end()
							.leaf<FunctionNode>(Call_MoveToTarget_M5F)
						.end()
						.composite<Sequence>()
							.leaf<FunctionNode>(Condition_ActionPower_Minus10)
							.composite<StatefulSelector>()
								.composite<Sequence>()
									.leaf<FunctionNode>(Random_SkillType)
									.composite<Selector>()
										.leaf<FunctionNode>(Call_Action_AbilitySlash)
										.leaf<FunctionNode>(Call_Action_AbilityInfinity)
										.leaf<FunctionNode>(Call_Action_AbilityBlade)
									.end()
								.end()
								.leaf<FunctionNode>(Call_Action_AssertAttack_2)
								.leaf<FunctionNode>(Call_Action_AssertAttack_4)
							.end()
						.end()
						.composite<Sequence>()
							.leaf<FunctionNode>(Condition_ActionPower_0)
							.composite<StatefulSelector>()	
								.leaf<FunctionNode>(Call_Action_AssertAttack_3)
								.leaf<FunctionNode>(Call_Action_BraveAttack_2)
								.leaf<FunctionNode>(Call_Action_AssertAttack_4)
								.composite<Sequence>()
									.leaf<FunctionNode>(Call_Dir_UP)
									.leaf<FunctionNode>(Call_Move_WalkR)
									.leaf<FunctionNode>(Call_TimeMax_1F_HALF)
									.leaf<FunctionNode>(Call_MoveToTarget_Time)
								.end()
								.leaf<FunctionNode>(Call_Action_AssertAttack_5)
								.leaf<FunctionNode>(Call_Action_BraveAttack_4)
								.composite<Sequence>()
									.leaf<FunctionNode>(Call_Dir_UP)
									.leaf<FunctionNode>(Call_Move_WalkB)
									.leaf<FunctionNode>(Call_TimeMax_1F_HALF)
									.leaf<FunctionNode>(Call_MoveToTarget_Time)
								.end()
								.leaf<FunctionNode>(Call_Action_AssertAttack_4)
								.leaf<FunctionNode>(Call_Action_BraveAttack_3)
								.leaf<FunctionNode>(Call_Action_AssertAttack_2)
								.composite<Sequence>()
									.leaf<FunctionNode>(Call_Dir_UP)
									.leaf<FunctionNode>(Call_Move_WalkL)
									.leaf<FunctionNode>(Call_TimeMax_1F_HALF)
									.leaf<FunctionNode>(Call_MoveToTarget_Time)
								.end()
							.end()
						.end()
						.composite<Sequence>()							
							.leaf<FunctionNode>(Condition_ActionPower_5)
							.composite<Sequence>()
								.leaf<FunctionNode>(Condition_Backward_Distance)
								.leaf<FunctionNode>(Call_Dir_DOWN)
								.decorator<Repeater>()
									.leaf<FunctionNode>(Call_TurnToTarget_WithAnim)
								.end()
								.leaf<FunctionNode>(Call_LookToTarget_Inv)
								.decorator<Repeater>(2)
									.leaf<FunctionNode>(Call_Action_Avoid)
								.end()						
							.end()
							.leaf<FunctionNode>(Call_Action_Guard)
						.end()
						.composite<Sequence>()
							.leaf<FunctionNode>(Condition_ActionPower_10)
							.leaf<FunctionNode>(Call_Dir_DOWN)
							.leaf<FunctionNode>(Call_Move_Run)
							.decorator<Repeater>()
								.leaf<FunctionNode>(Call_TurnToTarget_WithAnim)
							.end()
							.leaf<FunctionNode>(Call_MoveToTarget_L5F)
						.end()
					.end()
				.end()
				.leaf<FunctionNode>(Call_Idle)
			.end()
		.build();	
}

