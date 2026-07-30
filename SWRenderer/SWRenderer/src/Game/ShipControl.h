#pragma once

#include "Common/Vector.h"

struct SShipControl
{
	void Clear()
	{
		m_fYaw = 0.0f;
		m_fYawSpeed = 0.0f;
		m_fYaw_ctrl = 0.0f;
		m_fRoll = 0.0f;
		m_fAccForward = 0.0f;
		m_fAccForward_ctrl = 0.0f;
		m_fAccLeft = 0.0f;
		m_fAccLeft_ctrl = 0.0f;
		m_vMov = SVector3( 0.0f, 0.0f, 0.0f );
		m_vMovPrev = SVector3( 0.0f, 0.0f, 0.0f );

		m_vPos = SVector3( 0.0f, 0.0f, 0.0f );
		m_vDir = SVector3( 1.0f, 0.0f, 0.0f );
		m_vDirPrev = m_vDir;
		m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
		m_vRight = SVector3( 0.0f, 1.0f, 0.0f );
		SMatrix::Identity( m_matShip );
		SMatrix::Identity( m_matShipPrev );

		m_aBullets.clear();
		m_bShoot = false;
		m_iLastBulletTimeStampNs = 0;
	}


	void UpdateMatrices()
	{
		SMatrix::BuildEulerXYZ( m_matShip, m_fRoll, 0.0f, m_fYaw );

		m_vDir.x = m_matShip.m00;	m_vDir.y = m_matShip.m01;	m_vDir.z = m_matShip.m02;
		m_vRight.x = m_matShip.m10;	m_vRight.y = m_matShip.m11;	m_vRight.z = m_matShip.m12;
		m_vUp.x = m_matShip.m20;	m_vUp.y = m_matShip.m21;	m_vUp.z = m_matShip.m22;

		/*
		m_matShip.m00 = m_vDir.x;	m_matShip.m01 = m_vDir.y;	m_matShip.m02 = m_vDir.z;	m_matShip.m03 = 0.0f;
		m_matShip.m10 = m_vRight.x;	m_matShip.m11 = m_vRight.y;	m_matShip.m12 = m_vRight.z;	m_matShip.m13 = 0.0f;
		m_matShip.m20 = m_vUp.x;	m_matShip.m21 = m_vUp.y;	m_matShip.m22 = m_vUp.z;	m_matShip.m23 = 0.0f;
		*/
		m_matShip.m30 = m_vPos.x;	m_matShip.m31 = m_vPos.y;	m_matShip.m32 = m_vPos.z;	m_matShip.m33 = 1.0f;	
	}

	float		m_fYaw;
	float		m_fYawSpeed;
	float		m_fYaw_ctrl;
	float		m_fRoll;
	float		m_fAccForward;
	float		m_fAccForward_ctrl;
	float		m_fAccLeft;
	float		m_fAccLeft_ctrl;
	SVector3	m_vMov;
	SVector3	m_vMovPrev;


	SVector3	m_vPos;
	SVector3	m_vDir;
	SVector3	m_vDirPrev;
	SVector3	m_vUp;
	SVector3	m_vRight;
	SMatrix		m_matShip;
	SMatrix		m_matShipPrev;

	struct SBullet
	{
		SVector3	m_vPos;
		SVector3	m_vPosPrev;
		SVector3	m_vMov;
		float		m_fTimer;
		float		m_fTime;
	};
	std::vector< SBullet >		m_aBullets;
	bool		m_bShoot;
	uint64_t	m_iLastBulletTimeStampNs;
};