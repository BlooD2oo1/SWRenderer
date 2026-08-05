#pragma once

#include <vector>
#include "Common/Vector.h"

struct SShip
{
	SShip()
	{
		Clear();
	}

	void Clear();
	void Update();

	float		m_fYaw;
	float		m_fRoll;
	SVector3	m_vPos;

	SVector3	m_vMov;
	SVector3	m_vMovPrev;

	SVector3	m_vDir;
	SVector3	m_vDirPrev;
	SVector3	m_vUp;
	SVector3	m_vRight;
	SMatrix		m_matShip;
	SMatrix		m_matShipPrev;
};

struct STurret
{
	STurret( SShip& sShip )
		: m_sShip( sShip )
	{
		Clear();
	}

	void Clear();

	void Update();

	std::vector< SVector3 >	m_aTurretPositions;
	int			m_iBulletCounter;

	struct SBullet
	{
		SVector3	m_vPos;
		SVector3	m_vPosPrev;
		SVector3	m_vMov;
		float		m_fTimer;
		float		m_fTime;
	};

	const SShip&				m_sShip;
	
	float						m_fShootFreqHz;
	float						m_fBulletSpeed;

	std::vector< SBullet >		m_aBullets;
	bool						m_bShoot;
	uint64_t					m_iLastBulletTimeStampNs;
};

struct SShipPlayer
{
	SShipPlayer()
		: m_sTurret( m_sShip )
	{
		Clear();
	}

	void Clear();

	void Update();

	SShip		m_sShip;
	STurret		m_sTurret;

	float		m_fYawSpeed;
	float		m_fYaw_ctrl;	
	float		m_fAccForward;
	float		m_fAccForward_ctrl;
	float		m_fAccRight;
	float		m_fAccRight_ctrl;
};

struct SShipEnemy
{
	SShipEnemy()
		: m_sTurret( m_sShip )
	{
		Clear();
	}

	void Clear();

	void Update();

	SShip		m_sShip;
	STurret		m_sTurret;

	SVector3	m_vBoidMov;
	float		m_fPhase_DistanceToPlayer;
	float		f0;
	float		f1;
	float		f2;
	float		f3;
};