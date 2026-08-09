#pragma once

#include <vector>
#include "Common/Vector.h"
#include <unordered_map>

static const uint32_t iIndInvalid = 0xFFFFFFFF;

struct SShip;
struct SBoid;
struct STurret
{
	STurret()
	{
		Clear();
	}

	void Clear();

	void Update( const SShip& sShip );

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

	float						m_fShootFreqHz;
	float						m_fBulletSpeed;

	std::vector< SBullet >		m_aBullets;
	bool						m_bShoot;
	uint64_t					m_iLastBulletTimeStampNs;
};

struct SBoid
{
	SBoid()
	{
		Clear();
	}

	void Clear()
	{
		m_vPos = SVector3( 0.0f, 0.0f, 0.0f );
		m_vMov = SVector3( 0.0f, 0.0f, 0.0f );
		m_vMovPrev = SVector3( 0.0f, 0.0f, 0.0f );
		m_vBoidMov = SVector3( 0.0f, 0.0f, 0.0f );
	}

	SVector3	m_vPos;

	SVector3	m_vMov;
	SVector3	m_vMovPrev;

	SVector3	m_vBoidMov;
};

struct SShip
{
	SShip();

	void Clear();

	void Update();

	SBoid		m_sBoid;
	STurret		m_sTurret;

	float		m_fYaw;
	float		m_fRoll;

	SVector3	m_vDir;
	SVector3	m_vDirPrev;
	SVector3	m_vUp;
	SVector3	m_vRight;
	SMatrix		m_matShip;
	SMatrix		m_matShipPrev;	

	float		m_fHP;
	float		m_fPhase_DistanceToPlayer;

	float		m_fYawSpeed;
	float		m_fYaw_ctrl;	
	float		m_fAccForward;
	float		m_fAccForward_ctrl;
	float		m_fAccRight;
	float		m_fAccRight_ctrl;
};

class CActors
{
public:
	CActors();
	~CActors();

	void Clear();
	void Create();

	void Update();
	void Render();

	uint32_t AddShip();

	SShip& GetShipPlayer() { return m_aShips[m_iPlayerShipInd]; }

	size_t GetShipCount() const { return m_aShips.size(); }
	SShip& GetShip( size_t i ) { return m_aShips[i]; }

private:
	void _updateBoids();
	void _updateShips();

private:
	uint32_t				m_iPlayerShipInd;
	std::vector< SShip >	m_aShips;

	std::unordered_map< uint32_t, std::vector< uint32_t > > m_mapShipHashGrid;
};
