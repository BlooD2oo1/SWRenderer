#pragma once

#include <vector>
#include "Common/Vector.h"

struct SShip;
struct SBoid;
struct STurret
{
	STurret()
	{
		Clear();
	}

	void Clear();

	void Update( const SShip& sShip, const SBoid& sBoid );

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

	void Update( SBoid& sBoid );

	static const uint32_t m_iIndInvalid = 0xFFFFFFFF;
	uint32_t	m_iBoidInd;
	uint32_t	m_iTurretInd;

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

	SShip& AddEnemyShip();

	SShip& GetShipPlayer() { return m_sShipPlayer; }
	SBoid& GetBoidPlayer() { return m_aBoids[m_sShipPlayer.m_iBoidInd]; }
	STurret& GetTurretPlayer() { return m_aTurrets[m_sShipPlayer.m_iTurretInd]; }

	size_t GetShipEnemyCount() const { return m_aShipEnemies.size(); }
	SShip& GetShipEnemy( size_t i ) { return m_aShipEnemies[i]; }
	SBoid& GetBoidEnemy( size_t i ) { return m_aBoids[m_aShipEnemies[i].m_iBoidInd]; }

	size_t GetTurretCount() const { return m_aTurrets.size(); }
	STurret& GetTurret( size_t i ) { return m_aTurrets[i]; }

private:
	void _updatePlayer();
	void _updateBoids();
	void _updateEnemies();
	void _updateShips();

private:
	SShip					m_sShipPlayer;
	std::vector< SShip >	m_aShipEnemies;
	std::vector< SBoid >	m_aBoids;
	std::vector< STurret >	m_aTurrets;
};
