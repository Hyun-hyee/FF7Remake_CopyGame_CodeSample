/*
* PhysX_Manager Ŭ�����Դϴ�.
* PhysX_Collider, PhysX_Controller�� ����(���,����,Sleep ��)�ϰ�,PxScene�� �����մϴ�.
* PxScene�� simulate�� ������Ʈ�ϸ�, ���� PhysX�� ���õ� ����� �����մϴ�.* 
*/

#pragma once
#include "Base.h"

BEGIN(Engine)

class CPhysX_Collider;
class CPhysX_Controller;

class ENGINE_DLL CPhysX_Manager final : public CBase
{
	DECLARE_SINGLETON(CPhysX_Manager)

public:
	CPhysX_Manager();
	virtual ~CPhysX_Manager() = default;

public:
	HRESULT	Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const _uint iNumColLayer);
	void	Tick(_cref_time fTimeDelta);

public:
	/*PxScene ����*/
	HRESULT								Create_Scene( PxVec3 Gravity = PxVec3(0.0f, -9.81f * 1.f, 0.0f));

	/*CPhysX_Collider,CPhysX_Controller ������Ʈ ����,����*/
	void								Register_Collider(weak_ptr<CPhysX_Collider> pPhysXCollider,_bool bIsFirst = true);
	void								Register_Controller(weak_ptr<CPhysX_Controller> pPhysXController, _bool bIsFirst = true);

	void								Remove_Collider(weak_ptr<CPhysX_Collider> pCollider, bool bClear = true);
	void	 							Remove_Controller(weak_ptr<CPhysX_Controller> pController, bool bClear = true);

	weak_ptr<CPhysX_Collider>			Find_Collider(const _uint In_iPhysXColliderID);
	weak_ptr<CPhysX_Controller>			Find_Controller(const _uint In_iPhysXControllerID);

	/*�浹 ���̾� ����*/
	void	Register_PhysXFilterGroup(const _uint In_iLeftLayer, const _uint In_iRightLayer);
	_uint	Get_PhysXFilterGroup(const _uint In_iIndex);

public:
	/*PxRigidActor ����, ����*/
	PxRigidDynamic* Create_DynamicActor(const PxTransform& t, const PxGeometry& geometry, PxMaterial* pMaterial = nullptr);
	PxRigidDynamic* Create_DynamicActor(const PxTransform& t);
	PxRigidStatic* Create_StaticActor(const PxTransform& t, const PxGeometry& geometry, PxMaterial* pMaterial = nullptr);
	PxRigidStatic* Create_StaticActor(const PxTransform& t);
	void			Create_Material(_float fStaticFriction, _float fDynamicFriction, _float fRestitution, PxMaterial** ppOut);
	void			Create_Shape(const PxGeometry& Geometry, PxMaterial* pMaterial, const _bool isExculsive, const PxShapeFlags In_ShapeFlags, PxShape** ppOut);
	void			Create_Shape(const PxGeometry& Geometry,PxShape** ppOut);
	void			Create_MeshFromTriangles(const PxTriangleMeshDesc& In_MeshDesc, PxTriangleMesh** ppOut);
	void			Create_Controller(const PxCapsuleControllerDesc& In_ControllerDesc, PxController** ppOut);
	void			Create_Controller(const PxBoxControllerDesc& In_ControllerDesc, PxController** ppOut);

public:
	/*Raycast �浹 ���� �Լ�*/
	/*�浹�� GameObject ��� ��ü�� PhysX_Raycast_Stay �Լ��� �浹 ���� ����*/
	/*�Լ� ����� ���� ��ü�� hitCall�� ��� �����ͷ� �浹 ���� ��� ����*/
	_bool RaycastSingle(weak_ptr<CGameObject> pOwner,_uint iColLayer, _float4 vOrigin, _float4 vDirection, _float fMaxDistance, PxRaycastHit& hitCall);
	_bool RaycastMulti(weak_ptr<CGameObject> pOwner, _uint iColLayer, _float4 vOrigin, _float4 vDirection, _float fMaxDistance, PxRaycastHit* hitBuffer);

private:
	/*����� ��ü�� Collider,Controller ����*/
	void			Garbage_Collector();

private:
	ComPtr<ID3D11Device>			m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };
	class CGameInstance*			m_pGameInstance = { nullptr };

private:
	/*PhysX ��ü*/
	PxPhysics* m_pPhysics = nullptr;
	
	/*PhysX �����ϱ� ���� �ʿ��Ѱ͵�*/
	PxDefaultAllocator		m_Allocator;
	PxDefaultErrorCallback	m_ErrorCallback;
	PxFoundation*			m_pFoundation;
	PxCudaContextManager*	m_pCudaContextManager = nullptr;


	// CPU ���ҽ��� ȿ�������� ������ �� �ֵ��� �ϱ� ���� ���
	PxDefaultCpuDispatcher* m_pCpuDispatcher = nullptr;

	// MeshCooking�� �ϱ� ���� ���� -> ����ȭ ������ ��Ȱ��ȭ
	//PxCookingParams* m_pCooking = nullptr;

	/*Controller ���� �Ŵ���*/
	PxControllerManager* m_pControllerManager = nullptr;

	/*������� PxScene*/
	PxScene* m_pCurScene = nullptr;

	// �浹ü ������, Dynamic ������, ź������ �����Ͽ� ���
	PxMaterial* m_pMaterial;

	//Visual Debugger -> ���� �浹ü �ð�ȭ�ϴ� ������� ��ü
	PxPvd* m_pPVD;
	PxOmniPvd* m_pOmniPVD;

private:
	map<_uint, weak_ptr<CPhysX_Collider>>	m_pPhysXCollders;
	map<_uint, weak_ptr<CPhysX_Controller>>	m_pPhysXControllers;

	vector<_uint> m_CheckFilterGroup;
	class PhysX_SimulationEventCallBack* m_pSimulationEventCallBack = nullptr;
public:
	GETSET_EX2(PxMaterial*, m_pMaterial,Material,GET,SET)

#ifdef _DEBUG
/*�浹ü ������*/
private:
	PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
	BasicEffect* m_pEffect = { nullptr };
	ID3D11InputLayout* m_pInputLayout = { nullptr };
public:
	HRESULT 		Render_PhysXDebugger();
#endif

private:
	void Free();
};


/*�浹 ���͸� ����*/

PxFilterFlags CollisionFilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	if (filterData0.word2 != 1 || filterData1.word2 != 1)
	{
		return PxFilterFlag::eKILL;
	}

	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		pairFlags |= PxPairFlag::eCONTACT_DEFAULT;
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND; //Collision Enter
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST; //Collision Exit
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_PERSISTS;//Collision Stay
		pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;// �浹 ���� ����(��ġ, ���� ����, ���� ��) ����
		pairFlags |= PxPairFlag::eTRIGGER_DEFAULT;
		//pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
		return PxFilterFlag::eDEFAULT;
	}
	else
	{
		return PxFilterFlag::eKILL;
	}

	return PxFilterFlag::eDEFAULT;
}

class CPhysX_RaycastFilterCallback : public PxQueryFilterCallback
{
public:
	CPhysX_RaycastFilterCallback(_uint iFilterData) 
	{
		m_iFilterData = iFilterData; 
		m_word0 = (1 << m_iFilterData);
		m_word1 = GET_SINGLE(CPhysX_Manager)->Get_PhysXFilterGroup(m_iFilterData);
	}
	virtual ~CPhysX_RaycastFilterCallback() = default;

	virtual PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)
	{
		PxFilterData OtherFilter = shape->getSimulationFilterData();

		if ((m_word0 & OtherFilter.word1) && (OtherFilter.word0 & m_word1))
		{
			return PxQueryHitType::eBLOCK;
		}

		return PxQueryHitType::eNONE;
	}

	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor)
	{
		PxFilterData OtherFilter = shape->getSimulationFilterData();

		if ((m_word0 & OtherFilter.word1) && (OtherFilter.word0 & m_word1))
		{
			return PxQueryHitType::eBLOCK;
		}
		return PxQueryHitType::eNONE;
	}

	_uint m_iFilterData = 0;
	_uint m_word0 = 0;
	_uint m_word1 = 0;
};

END