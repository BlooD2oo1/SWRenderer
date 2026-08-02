#pragma once

#include <vector>
#include "Common/Vector.h"

struct SShipControl
{
public:
	void Clear();

	void UpdateControl();
	void UpdateMatrices();
	void UpdateShoot();

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