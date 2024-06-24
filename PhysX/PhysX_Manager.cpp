#include "PhysX_Manager.h"
#include "GameInstance.h"
#include "PhysX_SimulationEventCallBack.h"
#include "PhysX_Collider.h"
#include "PhysX_Controller.h"

#ifdef _DEBUG
#include "DebugDraw.h"
#endif

IMPLEMENT_SINGLETON(CPhysX_Manager)

CPhysX_Manager::CPhysX_Manager()
{
}

HRESULT CPhysX_Manager::Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const _uint iNumColLayer)
{
	m_pDevice = pDevice;
	m_pContext = pContext;
	Safe_AddRef(m_pGameInstance = GI());

	m_CheckFilterGroup.reserve(iNumColLayer);
	for (_uint i = 0; i < iNumColLayer; ++i)
	{
		m_CheckFilterGroup.emplace_back(0);
	}

	// Foundation 생성
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);


	// Create PVD -> 다른 디버깅 방법으로 대체
	//char* strTransport = "127.0.0.1";
	//m_pPVD = PxCreatePvd(*m_pFoundation);
	//PxPvdTransport* Transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	//_bool	bPVDConnectionResult = m_pPVD->connect(*Transport, PxPvdInstrumentationFlag::eALL);
	//if (!bPVDConnectionResult)
	//{
	//	//assert(false);
	//}

	// PhysX 생성
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), true, m_pPVD);
		
	// Cuda 생성
	PxCudaContextManagerDesc tCudaDesc;
	tCudaDesc.graphicsDevice = m_pDevice.Get();
	
	m_pCudaContextManager = PxCreateCudaContextManager(*m_pFoundation, tCudaDesc, PxGetProfilerCallback());

	if (m_pCudaContextManager)
	{
		if (!m_pCudaContextManager->contextIsValid())
		{
			if (m_pCudaContextManager)
				m_pCudaContextManager->release();
			m_pCudaContextManager = nullptr;
		}
	}
	else
	{
		assert(false);
	}

	// Material 생성
	// 충돌체 정지 마찰계수, 운동 마찰 계수, 탄성력
	m_pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.f);

	Create_Scene();

	
#ifdef _DEBUG
	// 충돌체 렌더링용 IA 생성
	m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pContext.Get());
	m_pEffect = new BasicEffect(m_pDevice.Get());
	m_pEffect->SetVertexColorEnabled(true);

	const void* pShaderByteCode = { nullptr };
	size_t	iShaderCodeLength = { 0 };

	m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderCodeLength);

	if (FAILED(m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pShaderByteCode, iShaderCodeLength, &m_pInputLayout)))
		RETURN_EFAIL;

#endif

	return S_OK;
}

void CPhysX_Manager::Tick(_cref_time fTimeDelta)
{
	if (m_pCurScene)
	{
		m_pCurScene->simulate(1.0f / 60.0f);
		m_pCurScene->fetchResults(true);
	}
	Garbage_Collector();
}

HRESULT CPhysX_Manager::Create_Scene(PxVec3 Gravity)
{
	if(m_pCurScene)
		m_pCurScene->release();

	m_pSimulationEventCallBack = new PhysX_SimulationEventCallBack();

	//CpuDispatcher 생성
	if (m_pCpuDispatcher)
		m_pCpuDispatcher->release();

	m_pCpuDispatcher = PxDefaultCpuDispatcherCreate(2);
	if (!m_pCpuDispatcher)
		assert(false);

	// Scene 설정
	PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = Gravity;

	sceneDesc.cpuDispatcher = m_pCpuDispatcher;
	sceneDesc.filterShader = CollisionFilterShader;
	sceneDesc.simulationEventCallback = m_pSimulationEventCallBack;
	sceneDesc.cudaContextManager = m_pCudaContextManager;
	sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
	sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;	//Enable GPU dynamics - without this enabled, simulation (contact gen and solver) will run on the CPU.
	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;			//Enable PCM. PCM NP is supported on GPU. Legacy contact gen will fall back to CPU
	sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;	//Improve solver stability by enabling post-stabilization.
	//sceneDesc.flags|=PxSceneFlag::
	sceneDesc.gpuMaxNumPartitions = 8;						//Defines the maximum number of partitions used by the solver. Only power-of-2 values are valid. 
	//A value of 8 generally gives best balance between performance and stability.
	sceneDesc.kineKineFilteringMode = PxPairFilteringMode::eKEEP; //Kinematic - Kinematic 충돌 활성화
	sceneDesc.staticKineFilteringMode = PxPairFilteringMode::eKILL; //Static - Kinematic 충돌 비활성화
	//sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;

	m_pCurScene = m_pPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = m_pCurScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);		
	}

	if (m_pControllerManager)
		m_pControllerManager->release();

	m_pControllerManager = PxCreateControllerManager(*m_pCurScene);

	/*충돌체 시각화 정보 설정*/
	m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_EDGES, 2.0f);
	m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 2.0f);
	//m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eSDF, 2.0f);
	//m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_STATIC, 2.0f);
	//m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 2.0f);
	//m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eCONTACT_NORMAL, 2.0f);
	//m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eBODY_AXES, 2.0f);
	//m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eWORLD_AXES, 2.0f);
	//m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 2.0f);
	//m_pCurScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_DYNAMIC, 2.0f);
	
	return S_OK;
}

void CPhysX_Manager::Register_Collider(weak_ptr<CPhysX_Collider> pPhysXCollider,_bool bIsFirst)
{
	if(bIsFirst)
		m_pPhysXCollders.emplace(pPhysXCollider.lock()->Get_ColliderID(), pPhysXCollider);

	PxActor* pRigidActor = pPhysXCollider.lock()->Add_PhysXActorAtScene();
	m_pCurScene->addActor(*pRigidActor);
}

weak_ptr<CPhysX_Collider> CPhysX_Manager::Find_Collider(const _uint In_iPhysXColliderID)
{
	auto iter = m_pPhysXCollders.find(In_iPhysXColliderID);

	if (iter != m_pPhysXCollders.end())
	{
		return iter->second;
	}

	return weak_ptr<CPhysX_Collider>();
}

void CPhysX_Manager::Register_Controller(weak_ptr<CPhysX_Controller> pPhysXController, _bool bIsFirst)
{
	if (bIsFirst)
		m_pPhysXControllers.emplace(pPhysXController.lock()->Get_ControllerID(), pPhysXController);
}

void CPhysX_Manager::Remove_Collider(weak_ptr<CPhysX_Collider> pCollider, bool bClear)
{
	if (bClear)
	{
		for (auto iter = m_pPhysXCollders.begin(); iter != m_pPhysXCollders.end();)
		{

			if ((*iter).second.lock() == pCollider.lock())
			{
				iter = m_pPhysXCollders.erase(iter);
			}
			else iter++;
		}
	}

	m_pCurScene->removeActor(*pCollider.lock()->Get_RigidActor(), true);
}

void CPhysX_Manager::Remove_Controller(weak_ptr<CPhysX_Controller> pController, bool bClear)
{
	if (bClear)
	{
		for (auto iter = m_pPhysXControllers.begin(); iter != m_pPhysXControllers.end();)
		{

			if ((*iter).second.lock() == pController.lock())
			{
				iter = m_pPhysXControllers.erase(iter);
			}
			else iter++;
		}
	}

	m_pCurScene->removeActor(*pController.lock()->Get_Controller()->getActor(), true);
}

weak_ptr<CPhysX_Controller> CPhysX_Manager::Find_Controller(const _uint In_iPhysXControllerID)
{
	auto iter = m_pPhysXControllers.find(In_iPhysXControllerID);

	if (iter != m_pPhysXControllers.end())
	{
		return iter->second;
	}

	return weak_ptr<CPhysX_Controller>();
}

void CPhysX_Manager::Register_PhysXFilterGroup(const _uint In_iLeftLayer, const _uint In_iRightLayer)
{
	_uint iRow = (_uint)In_iLeftLayer;
	_uint iCol = (_uint)In_iRightLayer; 

	_uint iMax = iCol;
	if (iRow > iCol)
	{
		iMax = iRow;
	}

	for (_uint i = (_uint)m_CheckFilterGroup.size(); i <= iMax; ++i)
	{
		m_CheckFilterGroup.emplace_back(0);
	}

	if (m_CheckFilterGroup[iRow] & (1 << iCol))
	{
		m_CheckFilterGroup[iRow] &= ~(1 << iCol); 
		m_CheckFilterGroup[iCol] &= ~(1 << iRow); 
	}
	else
	{
		m_CheckFilterGroup[iRow] |= (1 << iCol);
		m_CheckFilterGroup[iCol] |= (1 << iRow);
	}
}

_uint CPhysX_Manager::Get_PhysXFilterGroup(const _uint In_iIndex)
{
	return m_CheckFilterGroup[In_iIndex];
}

PxRigidDynamic* CPhysX_Manager::Create_DynamicActor(const PxTransform& t, const PxGeometry& geometry, PxMaterial* pMaterial)
{
	return PxCreateDynamic(*m_pPhysics, t, geometry, *pMaterial, 10000.f);
}

PxRigidDynamic* CPhysX_Manager::Create_DynamicActor(const PxTransform& t)
{
	return m_pPhysics->createRigidDynamic(t);
}

PxRigidStatic* CPhysX_Manager::Create_StaticActor(const PxTransform& t, const PxGeometry& geometry, PxMaterial* pMaterial)
{
	return PxCreateStatic(*m_pPhysics, t, geometry, *pMaterial);
}

PxRigidStatic* CPhysX_Manager::Create_StaticActor(const PxTransform& t)
{
	return  m_pPhysics->createRigidStatic(t);
}

void CPhysX_Manager::Create_Material(_float fStaticFriction, _float fDynamicFriction, _float fRestitution, PxMaterial** ppOut)
{
	*ppOut = m_pPhysics->createMaterial(fStaticFriction, fDynamicFriction, fRestitution);
}

void CPhysX_Manager::Create_Shape(const PxGeometry& Geometry, PxMaterial* pMaterial, const _bool isExculsive, const PxShapeFlags In_ShapeFlags, PxShape** ppOut)
{
	*ppOut = m_pPhysics->createShape(Geometry, *pMaterial, isExculsive, In_ShapeFlags);
}

void CPhysX_Manager::Create_Shape(const PxGeometry& Geometry, PxShape** ppOut)
{
	PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;

	*ppOut = m_pPhysics->createShape(Geometry,*m_pMaterial,false, shapeFlags);
}

void CPhysX_Manager::Create_MeshFromTriangles(const PxTriangleMeshDesc& In_MeshDesc, PxTriangleMesh** ppOut)
{
	//Use BVH33 midphase
	PxCookingParams params = PxCookingParams(PxTolerancesScale());	

	params.suppressTriangleMeshRemapTable = true;
	params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH);
	params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE);
	params.midphaseDesc.mBVH34Desc.numPrimsPerLeaf = 2;

	PxDefaultMemoryOutputStream outBuffer;
	
	// 최적화 분류 필요함
	// 작은 메쉬들 값 조절 필요 
	params.meshWeldTolerance = (0.0f);
	params.meshAreaMinLimit = (0.0f);
	params.meshEdgeLengthMaxLimit = (500.0f);

	PxCookTriangleMesh(params, In_MeshDesc, outBuffer);
	/*{ //단계적으로 값 적용해서 최적화 시도 -> 한계있음
		params.meshWeldTolerance = (0.1f);
		params.meshAreaMinLimit = (0.1f);
		params.meshEdgeLengthMaxLimit = (300.0f);

		if (!PxCookTriangleMesh(params, In_MeshDesc, outBuffer))
		{
			params.meshWeldTolerance = (0.1f);
			params.meshAreaMinLimit = (0.1f);
			params.meshEdgeLengthMaxLimit = (300.0f);
			
			if (!PxCookTriangleMesh(params, In_MeshDesc, outBuffer))
			{
				params.meshWeldTolerance = (0.01f);
				params.meshAreaMinLimit = (0.01f);
				params.meshEdgeLengthMaxLimit = (400.0f);

				if (!PxCookTriangleMesh(params, In_MeshDesc, outBuffer))
				{
					params.meshWeldTolerance = (0.001f);
					params.meshAreaMinLimit = (0.001f);
					params.meshEdgeLengthMaxLimit = (500.0f);

					if (!PxCookTriangleMesh(params, In_MeshDesc, outBuffer))
					{
						params.meshWeldTolerance = (0.00001f);
						params.meshAreaMinLimit = (0.00001f);
						params.meshEdgeLengthMaxLimit = (500.0f);

						if (!PxCookTriangleMesh(params, In_MeshDesc, outBuffer))
						{
							params.meshWeldTolerance = (0.f);
							params.meshAreaMinLimit = (0.f);
							params.meshEdgeLengthMaxLimit = (500.0f);
						}
					}
				}
			}
		}
	}*/

	PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
	*ppOut = m_pPhysics->createTriangleMesh(stream);	
}

void CPhysX_Manager::Create_Controller(const PxCapsuleControllerDesc& In_ControllerDesc, PxController** ppOut)
{
	*ppOut = m_pControllerManager->createController(In_ControllerDesc);
}


void CPhysX_Manager::Create_Controller(const PxBoxControllerDesc& In_ControllerDesc, PxController** ppOut)
{
	*ppOut = m_pControllerManager->createController(In_ControllerDesc);
}

_bool CPhysX_Manager::RaycastSingle(weak_ptr<CGameObject> pOwner, _uint iColLayer, _float4 vOrigin, _float4 vDirection, _float fMaxDistance, PxRaycastHit& hitCall)
{
	PxSceneQueryFilterData FilterData = PxSceneQueryFilterData();
	FilterData.flags |= PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER;

	_bool bResult = PxSceneQueryExt::raycastSingle(*m_pCurScene, Convert_PxVec3(vOrigin), Convert_PxVec3(vDirection), fMaxDistance, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eMTD, hitCall, FilterData, &CPhysX_RaycastFilterCallback(iColLayer));

	//맞은 actor에게 정보 전달
	if (bResult)
	{
		CPhysX_Collider* pCollider = (CPhysX_Collider*)(hitCall.actor->userData);
		pCollider->PhysX_Raycast_Stay(pOwner,iColLayer, vOrigin, vDirection, Convert_Vector(hitCall.position));
	}

	return bResult;
}

_bool CPhysX_Manager::RaycastMulti(weak_ptr<CGameObject> pOwner, _uint iColLayer, _float4 vOrigin, _float4 vDirection, _float fMaxDistance, PxRaycastHit* hitBuffer)
{
	PxSceneQueryFilterData FilterData = PxSceneQueryFilterData();
	FilterData.flags |= PxQueryFlag::ePREFILTER | PxQueryFlag::ePOSTFILTER;
	_bool bResult = {};

	PxU32 nbHit = PxSceneQueryExt::raycastMultiple(*m_pCurScene, Convert_PxVec3(vOrigin), Convert_PxVec3(vDirection), fMaxDistance, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eMTD, hitBuffer, sizeof(*hitBuffer), bResult, FilterData, &CPhysX_RaycastFilterCallback(iColLayer));

	//맞은 actor에게 정보 전달
	if (bResult)
	{
		for (PxU32 i = 0; i < nbHit; ++i)
		{
			CPhysX_Collider* pCollider = (CPhysX_Collider*)(hitBuffer[i].actor->userData);
			pCollider->PhysX_Raycast_Stay(pOwner,iColLayer, vOrigin, vDirection, Convert_Vector(hitBuffer[i].position));
		}
	}
	

	 return bResult;
}

void CPhysX_Manager::Garbage_Collector()
{
	// 사망한 객체 정리
	for (auto iter = m_pPhysXCollders.begin(); iter != m_pPhysXCollders.end();)
	{
		
		if (!(*iter).second.lock() || (*iter).second.lock()->Get_Owner().lock()->IsState(OBJSTATE::WillRemoved))
		{
			if((*iter).second.lock())
				m_pCurScene->removeActor(*(*iter).second.lock()->Get_RigidActor(),true);
			iter = m_pPhysXCollders.erase(iter);
		}
		else
		{			
			++iter;
		}		
	}

	for (auto iter = m_pPhysXControllers.begin(); iter != m_pPhysXControllers.end();)
	{
		if (!(*iter).second.lock() || (*iter).second.lock()->Get_Owner().lock()->IsState(OBJSTATE::WillRemoved))
		{
			if ((*iter).second.lock())
				m_pCurScene->removeActor(*(*iter).second.lock()->Get_Controller()->getActor(), true);
			iter = m_pPhysXControllers.erase(iter);
		}
		else
		{			
			++iter;
		}
	}
}


#ifdef _DEBUG
/*PhysX에서 충돌체 정점 정보 받아서 직접 렌더링*/
HRESULT CPhysX_Manager::Render_PhysXDebugger()
{
	m_pContext->GSSetShader(nullptr, nullptr, 0);

	m_pBatch->Begin();

	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(m_pGameInstance->Get_TransformMatrix(CPipeLine::TS_VIEW));
	m_pEffect->SetProjection(m_pGameInstance->Get_TransformMatrix(CPipeLine::TS_PROJ));

	m_pContext->IASetInputLayout(m_pInputLayout);

	m_pEffect->Apply(m_pContext.Get());

	const PxRenderBuffer& rb = m_pCurScene->getRenderBuffer();

	for (PxU32 i = 0; i < rb.getNbTriangles(); i++)
	{
		const PxDebugTriangle& triangle = rb.getTriangles()[i];
		// render the point
		DX::DrawTriangle(m_pBatch, Convert_Vector(triangle.pos0), Convert_Vector(triangle.pos1), Convert_Vector(triangle.pos2), XMVectorSet(1.f, 0.f, 1.f, 1.f));
	}

	for (PxU32 i = 0; i < rb.getNbLines(); i++)
	{
		const PxDebugLine& line = rb.getLines()[i];
		// render the point
		VertexPositionColor verts[2];
		XMStoreFloat3(&verts[0].position, Convert_Vector(line.pos0));
		XMStoreFloat3(&verts[1].position, Convert_Vector(line.pos1));

		XMStoreFloat4(&verts[0].color, XMVectorSet(1.f, 0.f, 1.f, 1.f));
		XMStoreFloat4(&verts[1].color, XMVectorSet(1.f, 0.f, 1.f, 1.f));
		m_pBatch->DrawLine(verts[0], verts[1]);
	}

	m_pBatch->End();
	return S_OK;
}
#endif 

void CPhysX_Manager::Free()
{
	if (m_pControllerManager)
		m_pControllerManager->release();

	if(m_pCurScene)
		m_pCurScene->release();

	if (m_pCpuDispatcher)
		m_pCpuDispatcher->release();

	if (m_pPhysics)
		m_pPhysics->release();

	if (m_pPVD)
	{
		PxPvdTransport* transport = m_pPVD->getTransport();
		m_pPVD->release();
		m_pPVD = nullptr;
		transport->release();
	}
	if (m_pCudaContextManager)
		m_pCudaContextManager->release();

	if (m_pFoundation)
		m_pFoundation->release();

#ifdef _DEBUG
	Safe_Delete(m_pEffect);
	Safe_Delete(m_pBatch);
	Safe_Release(m_pInputLayout);
#endif

	Safe_Delete(m_pSimulationEventCallBack);
	Safe_Release(m_pGameInstance);
}



                          
