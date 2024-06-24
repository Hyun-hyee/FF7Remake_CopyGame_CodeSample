#include "PhysX_SimulationEventCallBack.h"
#include "PhysX_Manager.h"
#include "PhysX_Collider.h"

#include "Engine_Defines.h"
#include "GameInstance.h"

PhysX_SimulationEventCallBack::PhysX_SimulationEventCallBack()
{
    m_pGameInstance = GET_SINGLE(CGameInstance);
}

PhysX_SimulationEventCallBack::~PhysX_SimulationEventCallBack()
{
	Safe_Release(m_pGameInstance);
}

void PhysX_SimulationEventCallBack::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
{
#if PHYSX_SIBULATIONCALLBACK_DEBUG
    cout << "onConstraintBreak" << endl;
#endif
}

void PhysX_SimulationEventCallBack::onWake(PxActor** actors, PxU32 count)
{
#if PHYSX_SIBULATIONCALLBACK_DEBUG
    cout << "onWake" << endl;
#endif
}

void PhysX_SimulationEventCallBack::onSleep(PxActor** actors, PxU32 count)
{
#if PHYSX_SIBULATIONCALLBACK_DEBUG
    cout << "onSleep" << endl;
#endif
}

void PhysX_SimulationEventCallBack::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
    PX_UNUSED((pairHeader));

    for (PxU32 i = 0; i < nbPairs; ++i) {
        const PxContactPair& cp = pairs[i];

        if (pairHeader.flags & (PxContactPairHeaderFlag::eREMOVED_ACTOR_0 | PxContactPairHeaderFlag::eREMOVED_ACTOR_1))
            return;

        PxRigidActor* Actor0 = cp.shapes[0]->getActor();
        PxRigidActor* Actor1 = cp.shapes[1]->getActor();

        if (Actor0->userData == nullptr || Actor1->userData == nullptr)
            return;

        CPhysX_Collider* colliderA = (CPhysX_Collider*)Actor0->userData;
        CPhysX_Collider* colliderB = (CPhysX_Collider*)Actor1->userData;
          
        if (!colliderA->Get_Simulation() || !colliderB->Get_Simulation())
            return;

        if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
#if PHYSX_SIBULATIONCALLBACK_DEBUG
            std::cout << "Contact found!" << std::endl;		
#endif
            colliderA->PhysX_OnCollision_Enter(colliderA, colliderB, cp);
			colliderB->PhysX_OnCollision_Enter(colliderB, colliderA, cp);
        }
        else if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST) {
#if PHYSX_SIBULATIONCALLBACK_DEBUG
            std::cout << "Contact Lost!" << std::endl;
#endif          
            colliderA->PhysX_OnCollision_Exit(colliderA, colliderB, cp);
            colliderB->PhysX_OnCollision_Exit(colliderB, colliderA, cp); 
        }

        if (cp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {
#if PHYSX_SIBULATIONCALLBACK_DEBUG
            std::cout << "Contact Stay!" << std::endl;
#endif
            colliderA->PhysX_OnCollision_Stay(colliderA, colliderB, cp);
            colliderB->PhysX_OnCollision_Stay(colliderB, colliderA, cp);
        }
    }
}

void PhysX_SimulationEventCallBack::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
    for (PxU32 i = 0; i < count; ++i) {
        const PxTriggerPair& cp = pairs[i];
 
        if (cp.otherActor->userData == nullptr || cp.triggerActor->userData == nullptr)
            return;

        CPhysX_Collider* colliderOther = (CPhysX_Collider*)cp.otherActor->userData;
        CPhysX_Collider* colliderTrigger = (CPhysX_Collider*)cp.triggerActor->userData;

        PxContactPair TempInfo;
        if (cp.status & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
#if PHYSX_SIBULATIONCALLBACK_DEBUG
            std::cout << "Trigger found!" << std::endl;
#endif
			colliderTrigger->PhysX_OnCollision_Enter(colliderTrigger, colliderOther, TempInfo);
        }
        else if (cp.status & PxPairFlag::eNOTIFY_TOUCH_LOST) {
#if PHYSX_SIBULATIONCALLBACK_DEBUG
            std::cout << "Trigger Lost!" << std::endl;
#endif          
            colliderTrigger->PhysX_OnCollision_Exit(colliderTrigger, colliderOther, TempInfo);
        }
    }
}

void PhysX_SimulationEventCallBack::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
{
    cout << "onAdvance" << endl;
}
