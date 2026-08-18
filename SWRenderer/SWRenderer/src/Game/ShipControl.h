#pragma once

#include <vector>
#include "Common/Vector.h"

#define SKA
#ifdef SKA
#include "Common/flat_hash_map.hpp"
#define spatial_hash_map ska::flat_hash_map
#else
#include <unordered_map>
#define spatial_hash_map std::unordered_map
#endif

static const uint32_t iIndInvalid = 0xFFFFFFFF;

struct SShip;
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
		float		m_fMass;
		float		m_fTimer;
		float		m_fTime;
	};

	float						m_fShootFreqHz;
	float						m_fBulletSpeed;
	SVector3					m_vColor;
	float						m_fDamage;

	std::vector< SBullet >		m_aBullets;
	bool						m_bShoot;
	uint64_t					m_iLastBulletTimeStampNs;	
};

struct SLaserGun
{
	SLaserGun()
	{
		Clear();
	}
	void Clear();
	
	SVector3					m_vGunPosition;
	SVector3					m_vColor;
	bool						m_bShoot;
};

struct SShip
{
	SShip();

	void Clear();

	void Update();

	STurret		m_sTurret;
	SLaserGun	m_sLaserGun;

	float		m_fYaw;
	float		m_fRoll;

	
	SVector3	m_vPos;	
	SVector3	m_vBoidMov;
	SVector3	m_vMov;
	SVector3	m_vMovPrev;
	SVector3	m_vDir;
	SVector3	m_vDirPrev;
	SVector3	m_vUp;
	SVector3	m_vRight;
	SMatrix		m_matShip;
	SMatrix		m_matShipPrev;	

	float		m_fSize;
	float		m_fMass;
	float		m_fHP;
	float		m_fDamageTimerMs;
	bool		m_bDead;
	float		m_fPhase_01;

	float		m_fYawSpeed;
	float		m_fYaw_ctrl;	
	float		m_fAccForward;
	float		m_fAccForward_ctrl;
	float		m_fAccRight;
	float		m_fAccRight_ctrl;
};

struct SAsteroid
{
	enum EModel
	{
		Model01,
		Model02,
		ModelBig,
	};

	SVector3	m_vPos;
	SQuaternion	m_qRot;
	//SVector3	m_vMov;
	float		m_fSize;
	//float		m_fMass;
	EModel		m_eModel;
};

class CSceneGame;
class CActors
{
public:
	CActors( CSceneGame& sSceneGame );
	~CActors();

	void Clear();
	void Create();

	void Update();
	void Render();

	uint32_t	AddShip();

	SShip&		GetShipPlayer() { return m_aShips[m_iPlayerShipInd]; }
	uint32_t	GetShipPlayerInd() { return m_iPlayerShipInd; }

	size_t		GetShipCount() const { return m_aShips.size(); }
	SShip&		GetShip( size_t i ) { return m_aShips[i]; }

	size_t		GetAsteroidCount() const { return m_aAsteroids.size(); }
	SAsteroid&	GetAsteroid( size_t i ) { return m_aAsteroids[i]; }

	// https://www.shadertoy.com/view/fcy3Wt
	void		GetField_ShipPlayer( SVector2& vField, const SVector2& p, const SShip& sShip );
	void		GetField_Asteroid( SVector2& vField, const SVector2& p, const SShip& sShip, const SAsteroid& sAsteroid );
	void		GetField_Asteroid2( SVector2& vField, const SVector2& p, const SShip& sShip, const SAsteroid& sAsteroid );

private:
	void _updateHashGrids();
	void _updateBoids();
	void _updateShips();
	//void _updateAsteroids();

	bool _onDamageShipByBullet( SShip& sShip, float fDamage, const SVector3& vMovBullet );
	bool _onDamageShipByCollision( SShip& sShip, float fDamage );

	inline void _getHash( int& iHashX, int& iHashY, const SVector2& vPos, float fMaxDist )
	{
		iHashX = (int)floorf( vPos.x / fMaxDist );
		iHashY = (int)floorf( vPos.y / fMaxDist );
	}
	inline uint32_t _getHash( const SVector2& vPos, float fMaxDist )
	{
		int iHashX;
		int iHashY;
		_getHash( iHashX, iHashY, vPos, fMaxDist );
		uint32_t iHash = ((uint32_t)iHashX << 16) | ((uint32_t)iHashY & 0xFFFF);
		return iHash;
	}
	uint32_t _getHash( int iHashX, int iHashY )
	{
		uint32_t iHash = ((uint32_t)iHashX << 16) | ((uint32_t)iHashY & 0xFFFF);
		return iHash;
	}

private:

	CSceneGame&					m_sSceneGame;

	uint32_t					m_iPlayerShipInd;
	std::vector< SShip >		m_aShips;
	std::vector< SAsteroid >	m_aAsteroids;

	const float					m_fHashGridShips_Size;
	spatial_hash_map< uint32_t, std::vector< uint32_t > > m_mapHashGridShips;
	const float					m_fHashGridAsteroids_Size;
	spatial_hash_map< uint32_t, std::vector< uint32_t > > m_mapHashGridAsteroids;

};
