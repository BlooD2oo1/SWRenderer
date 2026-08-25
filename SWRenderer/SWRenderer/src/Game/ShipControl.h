#pragma once

#include <vector>
#include "Common/Vector.h"
#include "Common/DenseMap.h"
#include "Common/SpatialHashGrid.h"

using ShipID = uint32_t;
const ShipID iShipIDInvalid = 0xffffffff;

enum EControlType
{
	Player,
	AI,
};

struct SBullet
{
	ShipID			m_iShipID;
	EControlType	m_eShipControlType;

	SVector3	m_vPos;
	SVector3	m_vPosPrev;
	SVector3	m_vMov;
	float		m_fMass;
	float		m_fTimer;
	float		m_fTime;

	SVector3	m_vColor;
};

struct SShip;
struct STurret
{
	STurret()
	{
		Clear();
	}

	void Clear();

	int			m_iBulletCounter;

	float						m_fShootFreqHz;
	float						m_fBulletSpeed;
	SVector3					m_vColor;
	float						m_fDamage;

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

struct SShipDesc
{
	float		fDragExponent;
	float		fDragCoeff;
	float		fBoidMul_Separation;
	float		fBoidMul_Alignment;
	float		fBoidMul_Cohesion;
	float		fMovSmooth;
	float		fMovMul_Boid;
	float		fMovMul_AsteroidDeflect;
	float		fMovMul_AsteroidDropOut;
	float		fMovMul_UserCtrl;
	float		fMovMul_Follow;
	float		fSpeedMin;
	float		fAccelMax;
	float		fAngularAccelMax;
	float		fSize;
	float		fMass;

	std::vector< SVector3 >	m_aTurretPositions;
};

struct SShip
{
	SShip();

	void Clear();

	ShipID			m_iID;		//used by DenseMap
	EControlType	m_eControlType;

	enum EShipType
	{
		Interceptor,
		Scout,
		Destroyer,
		ShipType_Count,
	} m_eShipType;

	STurret		m_sTurret;
	SLaserGun	m_sLaserGun;

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

	SVector3	m_vMov_Curr;
	SVector3	m_vMov_CurrPrev;

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
	float		m_fSize;
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

	ShipID		AddShip();
	SShip&		GetShipPlayer() { return m_mShips.GetByID(m_iPlayerShipID); }
	ShipID		GetShipIDPlayer() { return m_iPlayerShipID; }
	size_t		GetShipCount() const { return m_mShips.GetCount(); }
	SShip&		GetShip( size_t i ) { return m_mShips.GetByInd(i); }
	SShip&		GetShipByID( ShipID iID ) { return m_mShips.GetByID( iID ); }

	size_t		GetBulletCount() const { return m_aBullets.size(); }
	SBullet&	GetBullet( size_t i ) { return m_aBullets[i]; }


	size_t		GetAsteroidCount() const { return m_aAsteroids.size(); }
	SAsteroid&	GetAsteroid( size_t i ) { return m_aAsteroids[i]; }

	// https://www.shadertoy.com/view/fcy3Wt
	void		GetField_ShipPlayer( SVector2& vField, const SVector2& p, const SShip& sShip );
	void		GetField_Asteroid( SVector2& vField, const SVector2& p, const SShip& sShip, const SAsteroid& sAsteroid );
	void		GetField_Asteroid2( SVector2& vField, const SVector2& p, const SShip& sShip, const SAsteroid& sAsteroid );

	inline const SShipDesc&	GetShipDesc( SShip::EShipType eShipType ) const { return m_pShipDescs[eShipType]; }

private:
	void _updateHashGrids();
	void _updateShips();

	bool _onDamageShipByBullet( SShip& sShip, float fDamage, const SVector3& vMovBullet );
	bool _onDamageShipByCollision( SShip& sShip, float fDamage );

private:

	CSceneGame&					m_sSceneGame;

	SShipDesc					m_pShipDescs[SShip::ShipType_Count];

	ShipID						m_iPlayerShipID;
	DenseMap< SShip, ShipID >	m_mShips;
	std::vector< SAsteroid >	m_aAsteroids;
	std::vector< SBullet >		m_aBullets;

	CSpatialHashGrid< size_t >	m_cHashGridShips;
	CSpatialHashGrid< size_t >	m_cHashGridAsteroids;
	CSpatialHashGrid< size_t >	m_cHashGridBullets;
};
