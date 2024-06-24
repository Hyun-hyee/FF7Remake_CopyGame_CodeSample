/*
* PxSimulationEventCallback 커스텀을 위한 PhysX_SimulationEventCallBack 클래스입니다.
* 충돌 이벤트 발생시 PhysXCollider 컴포넌트에 충돌 정보를 전달하도록 구현했습니다.
*/

#pragma once

#include "Engine_Defines.h"

#define PHYSX_SIBULATIONCALLBACK_DEBUG 0

BEGIN(Engine)

class PhysX_SimulationEventCallBack :
    public PxSimulationEventCallback
{
public:
	PhysX_SimulationEventCallBack();
	~PhysX_SimulationEventCallBack();
public:
    virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override;
    virtual void onWake(PxActor** actors, PxU32 count) override;
    virtual void onSleep(PxActor** actors, PxU32 count) override;
    virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override;
    virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override;
    virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override;

private:
  class CGameInstance* m_pGameInstance = { nullptr };

public:
    void Release();
};

END