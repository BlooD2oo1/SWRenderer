#pragma once

#include "Common/Vector.h"

struct SBullet
{
	SVector3	m_vPos;
	SVector3	m_vDir;
	float		m_fSpeed;
	float		m_fTimer;
	float		m_fTime;
};

struct CShipControl
{
	CShipControl();
	~CShipControl();

	void Create();
	void Clear();

	void Accelerate( float fValue );
	void MouseMove( SVector2 vMouseDir );

	void SetShoot( bool bShoot );

	void Update( float fElapsedTimeMs, const SMatrix& matView000 );

	const SMatrix& GetMatrix() const { return m_matShip; }

	const std::vector< SBullet >& GetBullets() const { return m_aBullets; }

	SVector3	m_vPos;
	SVector3	m_vDir;
	SVector3	m_vUp;
	SVector3	m_vRight;

	SMatrix		m_matShip;

	float		m_fSpeedForward;
	SVector2	m_vMouseDir;

	bool		m_bShoot;
	float		m_fShootTimer;

	std::vector< SBullet >		m_aBullets;
};
