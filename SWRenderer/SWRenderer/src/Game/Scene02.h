#pragma once

#include "Common/Vector.h"
#include "Engine/Camera.h"
#include "Engine/Graphics.h"
#include "Game/ShipControl.h"
#include "Game/ShipMesh.h"

struct SCamera
{
	void Clear()
	{
		m_fFOVY = 45.0f / 180.0f * PI;
		m_fAspect = 1.0f;
		m_fNear = 0.1f;
		m_fFar = 10000.0f;
		m_vEye = SVector3( 0.0f, 0.0f, 200.0f );
		m_vEyeSmooth = m_vEye;
		m_vLookAt = SVector3( 0.0f, 0.0f, 0.0f );
		m_vLookAtSmooth = m_vLookAt;
		m_vUp = SVector3( 0.0f, 1.0f, 0.0f );
		SMatrix::Identity( m_matView );
		SMatrix::Identity( m_matProj );
		SMatrix::Identity( m_matViewProj );
		SMatrix::Identity( m_matViewProjPrev );
	}

	void UpdateMatrices()
	{
		m_matViewProjPrev = m_matViewProj;
		SMatrix::BuildViewMatrix( m_matView, m_vEyeSmooth, m_vLookAtSmooth, m_vUp );
		SMatrix::BuildProjectionMatrix( m_matProj, m_fFOVY, m_fAspect, m_fNear, m_fFar );
		SMatrix::Mul( m_matViewProj, m_matView, m_matProj );
	}

	float		m_fFOVY;
	float		m_fAspect;
	float		m_fNear;
	float		m_fFar;

	SVector3	m_vEye;
	SVector3	m_vEyeSmooth;
	SVector3	m_vLookAt;
	SVector3	m_vLookAtSmooth;
	SVector3	m_vUp;

	SMatrix		m_matView;
	SMatrix		m_matProj;
	SMatrix		m_matViewProj;
	SMatrix		m_matViewProjPrev;
};

struct SShipControl
{
	void Clear()
	{
		m_fYaw = 0.0f;
		m_fYawVel = 0.0f;
		m_fYawVelAcc = 0.0f;
		m_fRoll = 0.0f;
		m_fRollVel = 0.0f;
		m_fAcc = 0.0f;
		m_fAccVel = 0.0f;
		m_vMov = SVector3( 0.0f, 0.0f, 0.0f );

		m_vPos = SVector3( 0.0f, 0.0f, 0.0f );
		m_vDir = SVector3( 1.0f, 0.0f, 0.0f );
		m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
		m_vRight = SVector3( 0.0f, 1.0f, 0.0f );
		SMatrix::Identity( m_matShip );
	}

	void UpdateMatrices()
	{
		//calc m_vDir, m_vUp, m_vRight
		SQuaternion qRot;
		SQuaternion::FromEulerXYZ( qRot, 0.0f, m_fRoll, m_fYaw );
		SQuaternion::ToMatrix( m_matShip, qRot );

		/*m_matShip.m00 = m_vDir.x;	m_matShip.m01 = m_vDir.y;	m_matShip.m02 = m_vDir.z;	m_matShip.m03 = 0.0f;
		m_matShip.m10 = m_vRight.x;	m_matShip.m11 = m_vRight.y;	m_matShip.m12 = m_vRight.z;	m_matShip.m13 = 0.0f;
		m_matShip.m20 = m_vUp.x;	m_matShip.m21 = m_vUp.y;	m_matShip.m22 = m_vUp.z;	m_matShip.m23 = 0.0f;*/
		m_matShip.m30 = m_vPos.x;	m_matShip.m31 = m_vPos.y;	m_matShip.m32 = m_vPos.z;	m_matShip.m33 = 1.0f;	
	}

	float		m_fYaw;
	float		m_fYawVel;
	float		m_fYawVelAcc;
	float		m_fRoll;
	float		m_fRollVel;
	float		m_fAcc;
	float		m_fAccVel;
	SVector3	m_vMov;


	SVector3	m_vPos;
	SVector3	m_vDir;
	SVector3	m_vUp;
	SVector3	m_vRight;
	SMatrix		m_matShip;
};

class CScene02
{
public:
	CScene02();
	~CScene02();

	void Clear();
	void Create();

	void Update();
	void Render();

	bool On_KeyDown( uint32_t key );
	bool On_KeyUp( uint32_t key );
	bool On_MouseMove( int deltax, int deltay );
	bool On_MouseButtonDown( uint32_t button );
	bool On_MouseButtonUp( uint32_t button );
	bool On_MouseWheel( int iDelta );

private:

	SCamera			m_sCamera;
	SShipControl	m_sShipControl;
	CShipMesh		m_cShipMesh;

	SVertexPC*		m_pStars;
	uint32_t		m_iStarsCount;
	
};
