#include "ShipControl.h"

#include "Common/Globals.h"
#include "Engine/Engine.h"

CActors::CActors()
{
	Clear();
}
CActors::~CActors()
{
	Clear();
}

void CActors::Clear()
{
	m_iPlayerShipInd = iIndInvalid;
	m_aShips.clear();
}

void CActors::Create()
{
	Clear();

	{
		m_iPlayerShipInd = AddShip();
		SShip& sShipPlayer = GetShip( m_iPlayerShipInd );
		sShipPlayer.m_sTurret.m_aTurretPositions.push_back( SVector3( 0.5f, 2.7f, 0.0f ) );
		sShipPlayer.m_sTurret.m_aTurretPositions.push_back( SVector3( 1.0f, 0.0f, 0.0f ) );
		sShipPlayer.m_sTurret.m_aTurretPositions.push_back( SVector3( 0.5f, -2.7f, 0.0f ) );
		sShipPlayer.m_sTurret.m_aTurretPositions.push_back( SVector3( 1.0f, 0.0f, 0.0f ) );
		sShipPlayer.m_sTurret.m_fShootFreqHz = 30.0f;
		sShipPlayer.m_sTurret.m_fBulletSpeed = 0.2f;
	}

	for ( int i = 0;i < 20; i++ )
	{
		uint32_t iShipInd = AddShip();
		SShip& sShipEnemy = GetShip( iShipInd );

		sShipEnemy.m_sTurret.m_aTurretPositions.push_back( SVector3( 0.3f, 0.0f, 0.0f ) );
		sShipEnemy.m_sTurret.m_fShootFreqHz = 10.0f;
		sShipEnemy.m_sTurret.m_fBulletSpeed = 0.1f;

		const float fScatterRadius = 40.0f;
		sShipEnemy.m_sBoid.m_vPos.x = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sShipEnemy.m_sBoid.m_vPos.y = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sShipEnemy.m_sBoid.m_vPos.z = 0.0f;


		sShipEnemy.m_fYaw = ((float)rand() / (float)RAND_MAX) * PI2;

		/*sShipEnemy.f0 = ((float)rand() / (float)RAND_MAX);
		sShipEnemy.f1 = ((float)rand() / (float)RAND_MAX);
		sShipEnemy.f2 = ((float)rand() / (float)RAND_MAX);
		sShipEnemy.f3 = ((float)rand() / (float)RAND_MAX);*/

		sShipEnemy.m_fPhase_DistanceToPlayer = ((float)rand() / (float)RAND_MAX) * PI2;

	}
}

void CActors::Update()
{
	_updateBoids();
	_updateShips();
}

void CActors::_updateBoids()
{
	for ( size_t i0 = 0; i0 < GetShipCount(); i0++ )
	{
		if ( i0 == m_iPlayerShipInd ) continue;

		SShip& sShip0 = GetShip( i0 );
		SBoid& sBoid0 = sShip0.m_sBoid;

		SVector2 vSeparation( 0.0f, 0.0f );
		SVector2 vAlignment( 0.0f, 0.0f );
		SVector2 vCohesion( 0.0f, 0.0f );

		int iNeighborCount = 0;
		SVector2 vAvgPos( 0.0f, 0.0f );
		SVector2 vAvgMov( 0.0f, 0.0f );

		for ( size_t i1 = 0; i1 < GetShipCount(); i1++ )
		{
			if ( i0 == i1 ) continue;
			SShip& sShip1 = GetShip( i1 );
			SBoid& sBoid1 = sShip1.m_sBoid;

			SVector2 vDist( sBoid1.m_vPos.x - sBoid0.m_vPos.x, sBoid1.m_vPos.y - sBoid0.m_vPos.y );
			float fDistSq = SVector2::LengthSq( vDist );
			if ( fDistSq < powf( 100.0f, 2 ) )
			{
				if ( fDistSq < powf( 50.0f, 2 ) )
				{
					SVector2 vDirAway( -vDist.x, -vDist.y );
					vSeparation += vDirAway * (1.0f / (fDistSq + 0.00001f));
				}

				iNeighborCount++;
				vAvgPos += SVector2( sBoid1.m_vPos.x, sBoid1.m_vPos.y );
				vAvgMov += SVector2( sBoid1.m_vMov.x, sBoid1.m_vMov.y );
			}
		}

		if ( iNeighborCount > 0 )
		{
			vAvgMov /= (float)iNeighborCount;
			vAlignment = vAvgMov - SVector2( sBoid0.m_vMov.x, sBoid0.m_vMov.y );

			vAvgPos /= (float)iNeighborCount;
			vCohesion = vAvgPos - SVector2( sBoid0.m_vPos.x, sBoid0.m_vPos.y );
		}

		SVector2 vBoidMov =	vSeparation * 600.0f +// * sEnemyShip0.f0 +
			vAlignment * 10.0f +
			vCohesion * 0.3f;

		sBoid0.m_vBoidMov = SVector3( vBoidMov.x, vBoidMov.y, 0.0f );
	}
}

void GetField( SVector2& vField, const SVector2& p, const SVector2& p0, const SVector2& d0 )
{
	vField = SVector2( 0.0f, 0.0f );
	SVector2 v0( (p0+d0*2.0f) - p );
	SVector2 v1( (p0+d0*41.0f) - p );
	SVector2 v2( (p0+d0*42.0f) - p );
	float l0 = SVector2::Length( v0 );
	float l1 = SVector2::Length( v1 );
	float l2 = SVector2::Length( v2 );
	SVector2::Normalize( vField, v0*0.01f - v1/l1/l1*4.0f - v2/l2/l2*5.0f );
	float fRepel = Clamp( 1000.0f / l0 / l0 / l0, 0.0f, 0.03f );
	vField -= v0 * fRepel;
}

void CActors::_updateShips()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	SShip& sShipPlayer = GetShipPlayer();

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShip = GetShip( i );

		if ( i == m_iPlayerShipInd )
		{
			float fYawMultiplier = sShip.m_sTurret.m_bShoot ? 0.5f : 1.0f;

			sShip.m_fYawSpeed = SmoothConverge( sShip.m_fYawSpeed, sShip.m_fYaw_ctrl * fYawMultiplier, 1.01f, 1.01f, fElapsedTimeMs );
			sShip.m_fAccForward = SmoothConverge( sShip.m_fAccForward, sShip.m_fAccForward_ctrl, 1.01f, 1.01f, fElapsedTimeMs );
			sShip.m_fAccRight = SmoothConverge( sShip.m_fAccRight, sShip.m_fAccRight_ctrl, 1.001f, 1.01f, fElapsedTimeMs );

			sShip.m_fYaw += sShip.m_fYawSpeed * 0.004f * fElapsedTimeMs;
			SVector3 vShipDir( cosf( sShip.m_fYaw ), sinf( sShip.m_fYaw ), 0.0f );
			SVector3 vShipRight( -vShipDir.y, vShipDir.x, 0.0f );
			sShip.m_sBoid.m_vMov += vShipDir * sShip.m_fAccForward * 0.0001f * fElapsedTimeMs;
			sShip.m_sBoid.m_vMov += vShipRight * sShip.m_fAccRight * 0.0001f * fElapsedTimeMs;

			float fSpeedWeight = 1.00005f + SVector3::LengthSq( sShip.m_sBoid.m_vMov ) * 0.05f * fabsf( sShip.m_fAccForward );
			sShip.m_sBoid.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), sShip.m_sBoid.m_vMov, CalcSmoothUpdateWeight( fSpeedWeight, fElapsedTimeMs ) );

			//m_fRoll = m_fYaw*0.5f;
			sShip.m_fRoll = Lerp( -sShip.m_fYawSpeed, sShip.m_fRoll, CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );
			//m_fRoll = SmoothConverge( m_fRoll, -m_fYaw * 3.5f, 1.0002f, 1.0002f, fElapsedTimeMs );
		}
		else
		{
			SVector2 vField( 0.0f, 0.0f );
			GetField( vField, SVector2( sShip.m_sBoid.m_vPos.x, sShip.m_sBoid.m_vPos.y ), SVector2( sShipPlayer.m_sBoid.m_vPos.x, sShipPlayer.m_sBoid.m_vPos.y ), SVector2( sShipPlayer.m_vDir.x, sShipPlayer.m_vDir.y ) );

			SVector3 vEnemyToPlayerDir( sShipPlayer.m_sBoid.m_vPos - sShip.m_sBoid.m_vPos );
			const float fEnemyToPlayerDist = SVector3::Length( vEnemyToPlayerDir );
			SVector3::Normalize( vEnemyToPlayerDir, vEnemyToPlayerDir );
			const float fSin_DistanceToPlayer = powf( sinf( sShip.m_fPhase_DistanceToPlayer )*0.5f+0.5f, 0.5f );
			//const float fFollowAmount = Clamp( (fEnemyToPlayerDist-Lerp(20.0f, 110.0f, fSin_DistanceToPlayer))*0.02f, -0.4f, 1.0f );
			const float fFollowAmount = 1.5f;
			SVector3 vFollowMov = SVector3( vField.x, vField.y, 0.0f ) * fFollowAmount;

			SVector3 vMov = sShip.m_sBoid.m_vBoidMov * 0.001f + vFollowMov * 0.1f;

			sShip.m_sBoid.m_vMov = Lerp( vMov, sShip.m_sBoid.m_vMov, CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );

			sShip.m_sBoid.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), sShip.m_sBoid.m_vMov, CalcSmoothUpdateWeight( 1.000002f, fElapsedTimeMs ) );

			const float fYaw = atan2f( sShip.m_sBoid.m_vMov.y, sShip.m_sBoid.m_vMov.x );
			sShip.m_fYaw = LerpAngle( fYaw, sShip.m_fYaw, CalcSmoothUpdateWeight( 1.005f, fElapsedTimeMs ) );

			if ( fSin_DistanceToPlayer < 0.1f && SVector3::Dot( sShip.m_vDir, vEnemyToPlayerDir ) > 0.8f )
			{
				if ( !sShip.m_sTurret.m_bShoot )
				{
					sShip.m_sTurret.m_iLastBulletTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
				}
				sShip.m_sTurret.m_bShoot = true;
			}
			else
			{
				sShip.m_sTurret.m_bShoot = false;
			}

			sShip.m_fPhase_DistanceToPlayer += fElapsedTimeMs * 0.0003f;
			if ( sShip.m_fPhase_DistanceToPlayer > PI2 ) sShip.m_fPhase_DistanceToPlayer -= PI2;
		}
	}

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		if ( i == m_iPlayerShipInd ) continue;

		SShip& sShip = GetShip( i );
		for ( int iBulletInd = 0; iBulletInd < sShipPlayer.m_sTurret.m_aBullets.size(); iBulletInd++ )
		{
			STurret::SBullet& sBullet = sShipPlayer.m_sTurret.m_aBullets[iBulletInd];
			SVector2 vBulletPosPrev( sBullet.m_vPosPrev.x, sBullet.m_vPosPrev.y );
			SVector2 vBulletPos( sBullet.m_vPos.x, sBullet.m_vPos.y );
			float fT = 0.0f;
			if ( SegmentSphereTest( vBulletPosPrev, vBulletPos, SVector2( sShip.m_sBoid.m_vPos.x, sShip.m_sBoid.m_vPos.y ), 5.0f, fT ) )
			{
				sShip.m_sBoid.m_vMov += SVector3( sBullet.m_vMov.x, sBullet.m_vMov.y, 0.0f ) * 0.05f;
				sShip.m_fHP -= 12.0f;

				SVector2 vNormalDir( vBulletPos - SVector2( sShip.m_sBoid.m_vPos.x, sShip.m_sBoid.m_vPos.y ) );
				SVector2::Normalize( vNormalDir, vNormalDir );

				SVector2 vBulletMov( sBullet.m_vMov.x, sBullet.m_vMov.y );
				SVector2 vBulletMovReflected = vBulletMov - vNormalDir * SVector2::Dot( vBulletMov, vNormalDir ) * 2.0f;
				sBullet.m_vMov.x = vBulletMovReflected.x;
				sBullet.m_vMov.y = vBulletMovReflected.y;
				// drop out from the sphere, use fT to find the intersection point:
				sBullet.m_vPos.x = vBulletPosPrev.x + (vBulletPos.x - vBulletPosPrev.x) * fT + vNormalDir.x * 0.1f;
				sBullet.m_vPos.y = vBulletPosPrev.y + (vBulletPos.y - vBulletPosPrev.y) * fT + vNormalDir.y * 0.1f;

				sBullet.m_vMov *= 0.8f;

				SAudioEvent sAudioEvent;
				sAudioEvent.type = SAudioEvent::GunHit;
				sAudioEvent.fVolume = 0.15f;
				sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
				sAudioEvent.iLifeTimeNs = 1000 * 1000 * 200;
				sAudioEvent.iSampleCounter = 0;
				sAudioEvent.fPhase = 0.0f;	
				sAudioEvent.sClick.iButton = 1;
				sAudioEvent.sGun.vPos = sBullet.m_vPos;
				sAudioEvent.sGun.fPitch = 200.0f;
				CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );
			}				
		}

		if ( sShip.m_fHP <= 0.0f )
		{
			SAudioEvent sAudioEvent;
			sAudioEvent.type = SAudioEvent::GunHit;
			sAudioEvent.fVolume = 0.2f;
			sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
			sAudioEvent.iLifeTimeNs = 1000 * 1000 * 2000;
			sAudioEvent.iSampleCounter = 0;
			sAudioEvent.fPhase = 0.0f;	
			sAudioEvent.sClick.iButton = 1;
			sAudioEvent.sGun.vPos = sShip.m_sBoid.m_vPos;
			sAudioEvent.sGun.fPitch = 200.0f;
			CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );

			sShip.m_sBoid.m_vPos.x = ((float)rand() / (float)RAND_MAX) * 40.0f * 2.0f - 40.0f;
			sShip.m_sBoid.m_vPos.y = ((float)rand() / (float)RAND_MAX) * 40.0f * 2.0f - 40.0f;
			sShip.m_fHP = 100.0f;
		}
	}

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShip = GetShip( i );
		sShip.Update();
	}
}

void CActors::Render()
{
}

uint32_t CActors::AddShip()
{
	m_aShips.emplace_back();
	return (uint32_t)(m_aShips.size() - 1);
}

////////////////////////////////////////////////////////////////

SShip::SShip()
{
	Clear();
}

void SShip::Clear()
{
	// uint32_t max:
	m_sBoid.Clear();
	m_sTurret.Clear();

	m_fYaw = 0.0f;
	m_fRoll = 0.0f;

	m_vDir = SVector3( 1.0f, 0.0f, 0.0f );
	m_vDirPrev = m_vDir;
	m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
	m_vRight = SVector3( 0.0f, 1.0f, 0.0f );
	SMatrix::Identity( m_matShip );
	m_matShipPrev = m_matShip;

	m_fHP = 100.0f;
	m_fPhase_DistanceToPlayer = 0.0f;

	m_fYawSpeed = 0.0f;
	m_fYaw_ctrl = 0.0f;	
	m_fAccForward = 0.0f;
	m_fAccForward_ctrl = 0.0f;
	m_fAccRight = 0.0f;
	m_fAccRight_ctrl = 0.0f;
}

void SShip::Update()
{
	m_sBoid.m_vMovPrev = m_sBoid.m_vMov;
	m_vDirPrev = m_vDir;
	m_matShipPrev = m_matShip;
	SMatrix::BuildEulerXYZ( m_matShip, m_fRoll, 0.0f, m_fYaw );
	m_matShip.m30 = m_sBoid.m_vPos.x;	m_matShip.m31 = m_sBoid.m_vPos.y;	m_matShip.m32 = m_sBoid.m_vPos.z;	m_matShip.m33 = 1.0f;

	m_vDir.x = m_matShip.m00;	m_vDir.y = m_matShip.m01;	m_vDir.z = m_matShip.m02;
	m_vUp.x = m_matShip.m20;	m_vUp.y = m_matShip.m21;	m_vUp.z = m_matShip.m22;
	SVector3::Cross( m_vRight, m_vDir, m_vUp );	
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();
	m_sBoid.m_vPos += m_sBoid.m_vMov * fElapsedTimeMs;

	m_sTurret.Update( *this );
}

////////////////////////////////////////////////////////////////

void STurret::Clear()
{
	m_aTurretPositions.clear();
	m_iBulletCounter = 0;

	m_fShootFreqHz = 1.0f;
	m_fBulletSpeed = 0.1f;

	m_aBullets.clear();
	m_bShoot = false;
	m_iLastBulletTimeStampNs = 0;
}

void STurret::Update( const SShip& sShip )
{
	if ( m_bShoot )
	{
		SVector3 vGunDir( sShip.m_vDir );
		SVector3 vGunDirPrev( sShip.m_vDirPrev );

		SVector3 vGunMov( sShip.m_sBoid.m_vMov );
		SVector3 vGunMovPrev( sShip.m_sBoid.m_vMovPrev );

		const float fShootFreqHz = m_fShootFreqHz;
		const uint64_t iShootPeriodNs = (uint64_t)(1.0f / fShootFreqHz * 1000.0f * 1000.0f * 1000.0f);

		uint64_t iTNs = m_iLastBulletTimeStampNs;
		for ( ; iTNs < CEngine::GetInstance().GetTimeStampNs(); iTNs += iShootPeriodNs )
		{
			if ( m_aTurretPositions.size() == 0 )
			{
				break;
			}			

			SVector3 vGunPosWorld;
			SVector3 vGunPosWorldPrev;
			{
				SVector3 vGunPos = m_aTurretPositions[m_iBulletCounter%m_aTurretPositions.size()];
				SMatrix::TransformCoord( vGunPosWorld, vGunPos, sShip.m_matShip );
				SMatrix::TransformCoord( vGunPosWorldPrev, vGunPos, sShip.m_matShipPrev );
			}

			float fFrameW = (float)(iTNs - m_iLastBulletTimeStampNs) / (float)(CEngine::GetInstance().GetTimeStampNs() - m_iLastBulletTimeStampNs);

			SBullet sBullet;
			sBullet.m_vPos = Lerp( vGunPosWorldPrev, vGunPosWorld, fFrameW );

			SVector3 vBulletDir = Lerp( vGunDirPrev, vGunDir, fFrameW );
			sBullet.m_vMov = Lerp( vGunMovPrev, vGunMov, fFrameW ) + vBulletDir * m_fBulletSpeed;

			/*sBullet.m_vDir.x += (rand() % 1000 - 500) * 0.00001f;
			sBullet.m_vDir.y += (rand() % 1000 - 500) * 0.00001f;
			sBullet.m_vDir.z += (rand() % 1000 - 500) * 0.00001f;*/

			sBullet.m_fTime = 1000.0f;
			sBullet.m_fTimer = 0.0f;
			m_aBullets.push_back( sBullet );			

			SAudioEvent sAudioEvent;
			sAudioEvent.type = SAudioEvent::GunShot;
			sAudioEvent.fVolume = 0.06f;
			sAudioEvent.iTimeStampNs = iTNs;
			sAudioEvent.iLifeTimeNs = 1000 * 1000 * 1500;
			sAudioEvent.iSampleCounter = 0;
			sAudioEvent.fPhase = 0.0f;			
			sAudioEvent.sGun.vPos = sBullet.m_vPos;
			sAudioEvent.sGun.fPitch = 400.0f + (rand() % 1000 - 500) * 0.1f;
			CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );

			m_iBulletCounter++;
		}

		m_iLastBulletTimeStampNs = iTNs;
	}

	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	for ( size_t iBulletInd = 0; iBulletInd < m_aBullets.size(); )
	{
 		SBullet& sBullet = m_aBullets[iBulletInd];
		sBullet.m_fTimer += fElapsedTimeMs;
		if ( sBullet.m_fTimer > sBullet.m_fTime )
		{
			m_aBullets[iBulletInd] = m_aBullets.back();
			m_aBullets.pop_back();
			continue;
		}
		sBullet.m_vPosPrev = sBullet.m_vPos;
		sBullet.m_vPos += sBullet.m_vMov * fElapsedTimeMs;

		++iBulletInd;
	}
}
