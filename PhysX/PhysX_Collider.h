/*
* PhysX_Collider Ŭ�����Դϴ�.
* PxRigidDynamic �Ǵ� PxRigidStatic�� �����ϸ�, �浹ü ������ �մϴ�.
*
* �ɼ����δ� PxRigidDynamic - Kinematic, PxRigidStatic - Trigger �� �ֽ��ϴ�.
*
* Kinematic ������ ���� �������� �����ؾ��ϰ�, Overlap�� �����մϴ�. �ַ� �浹 �̺�Ʈ ������ �ʿ�������, ������ ���� ������� �ʴ� �����̴� �浹ü�� ����մϴ�.
* ��� ���÷δ� ���� ���� �ֽ��ϴ�.
*
* Trigger�� �ַ� �������� �ʰ�, ������ ���� ������� �ʴ� �浹ü�� ����մϴ�.
* ��� ���÷δ� �� �̺�Ʈ Trigger ���� �ֽ��ϴ�.
*/

#pragma once
#include "Component.h"

BEGIN(Engine)
class FMeshData;
class CCommonModelComp;
class CTransform;

class ENGINE_DLL CPhysX_Collider : public CComponent
{
public:
	enum ColliderType {COLL_ALL,COLL_MONSTER};
public:
	CPhysX_Collider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	CPhysX_Collider(const CPhysX_Collider& rhs);
	virtual ~CPhysX_Collider() = default;

public:
	virtual HRESULT		Initialize_Prototype();
	virtual HRESULT		Initialize(void* pArg);

public:
	/*�浹 �̺�Ʈ �Լ�*/
	virtual void		PhysX_OnCollision_Enter(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo);
	virtual void		PhysX_OnCollision_Stay(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo);
	virtual void		PhysX_OnCollision_Exit(CPhysX_Collider* pThisCol, CPhysX_Collider* pOtherCol, const PxContactPair& ContactInfo);

	virtual void		PhysX_Raycast_Stay(weak_ptr<CGameObject> pOther, _uint iOtherColLayer, _float4 vOrigin, _float4 vDirection, _float4 vColPosition);

public:
	_vector				Get_Position();
	_vector				Get_Quaternion();
	_vector				Get_Velocity();
	_float				Get_Mess();
	_vector				Get_AngularVelocity();
	_vector				Get_LinearVelocity();
	_matrix				Get_WorldMatrix();
	PHYSXCOLLIDER_DESC	Get_ColliderDesc() const { return m_PhysXColliderDesc; }

	HRESULT				Set_Position(_vector _vPos, _vector _vQuaternion, _bool bIsSimulation = true);
	HRESULT				Set_Position(_vector _vPos, _bool bIsSimulation = true);
	void				Set_MaxLinearVelocity(_vector vMaxVelocity) { m_vMaxVelocity = vMaxVelocity; }
	void				Set_ActorFlag(PxActorFlag::Enum eFlag, _bool bState);

public:
	_uint				Get_ColliderID() { return m_iColliderID; }
	PxRigidActor*		Get_RigidActor() { if (m_pRigidDynamic) return m_pRigidDynamic; else return m_pRigidStatic; return nullptr; }
	_bool				Get_Simulation() { return m_bSimulation; }
	void				Set_Simulation(_bool bSimulation);
	_float3				Get_Offset() { return m_vLocalOffset + m_vWorldOffset; }

	GETSET_EX2(_float3, m_vLocalOffset, LocalOffset, GET_C_REF, SET)
	GETSET_EX2(_float3, m_vWorldOffset, WorldOffset, GET_C_REF, SET)
	GETSET_EX2(ColliderType, m_eType, Type, GET, SET)

public:
	void				Delete_Collider();
	void				Create_Collider();

public:
	/*�� ���� �Լ�*/
	HRESULT				Add_Force(_vector _vForce);
	HRESULT				Clear_Force();
	HRESULT				Clear_Velocity();
	HRESULT				Add_LinearVelocityResistance(_vector vResistanceRate);

public:
	/*�浹ü ��ġ,ȸ�� ������Ʈ �Լ�*/
	void				Synchronize_Transform(weak_ptr<CTransform> pTransform, _fvector In_vLocalOffset = { 0.f, 0.f, 0.f }, _fvector In_vWorldOffset = { 0.f, 0.f, 0.f });
	void				Synchronize_Transform_Position(weak_ptr<CTransform> pTransform);
	void				Synchronize_Transform_Rotation(weak_ptr<CTransform> pTransform);
	_matrix				Synchronize_Matrix(_fmatrix In_WorldMatrix);

	void				Synchronize_Collider(weak_ptr<CTransform> pTransform, _fvector In_vLocalOffset = { 0.f, 0.f, 0.f }, _fvector In_vWorldOffset = { 0.f, 0.f, 0.f }, _bool bIsSimulation = true);
	void				Synchronize_Collider(_float4x4 In_WorldMatrix, _fvector In_vLocalOffset = { 0.f, 0.f, 0.f }, _fvector In_vWorldOffset = { 0.f, 0.f, 0.f }, _bool bIsSimulation = true);

public:
	void				PutToSleep();
	void				WakeUp();

public:
	void				Attach_Shape(PxShape* pShape);
	void				Detach_Shape(PxShape* pShape);

private:
	/*�浹ü ���� ���� �Լ�*/
	void				CreatePhysXActor(PHYSXCOLLIDER_DESC& PhysXColliderDesc);
	PxRigidActor*		Add_PhysXActorAtScene();

	void				Init_MeshCollider(FMeshData* pBone);
	void				Init_ModelCollider(CCommonModelComp* pModelData);

	void				Create_Geometry();
	void				Create_DynamicActor(PHYSXCOLLIDER_DESC& PhysXColliderDesc, PxTransform Transform);
	void				Create_StaticActor(PHYSXCOLLIDER_DESC& PhysXColliderDesc, PxTransform Transform);

public:
	virtual void		Write_Json(json& Out_Json);
	virtual void		Load_Json(const json& In_Json);

private:
	/*�浹ü ����*/
	PHYSXCOLLIDER_DESC		m_PhysXColliderDesc;
	PxRigidDynamic*			m_pRigidDynamic = nullptr;
	PxRigidStatic*			m_pRigidStatic = nullptr;

	vector<PxTriangleMesh*>	m_TriangleMesh;

	_bool					m_bPickState = false;
	_bool					m_bPickable = true;
	_bool					m_bYFixed = false;


	vector<PxGeometry*>		m_pGeometry;
	vector<PxShape*>		m_pShape;
	PxFilterData			m_FilterData;

private:
	static _uint			m_iNewColliderID;
	_uint					m_iColliderID;
	_bool					m_bSimulation = { true };

	_float3					m_vLocalOffset = { 0.f, 0.f, 0.f };
	_float3					m_vWorldOffset = { 0.f, 0.f, 0.f };

	_vector					m_vMaxVelocity;

	ColliderType			m_eType = { COLL_ALL };

public:
	static shared_ptr<CPhysX_Collider> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

	virtual shared_ptr<CComponent> Clone(void* pArg) override;
	virtual void Free() override;
};

END