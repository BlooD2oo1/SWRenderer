#include "ShipControl.h"

#include "Common/Globals.h"
#include "Engine/Engine.h"

void SShip::Clear()
{
	m_fYaw = 0.0f;
	m_fRoll = 0.0f;
	m_vMov = SVector3( 0.0f, 0.0f, 0.0f );
	m_vMovPrev = SVector3( 0.0f, 0.0f, 0.0f );
	m_vPos = SVector3( 0.0f, 0.0f, 0.0f );
	m_vDir = SVector3( 1.0f, 0.0f, 0.0f );
	m_vDirPrev = m_vDir;
	m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
	m_vRight = SVector3( 0.0f, 1.0f, 0.0f );
	SMatrix::Identity( m_matShip );
	SMatrix::Identity( m_matShipPrev );
}

void SShip::Update()
{
	m_vMovPrev = m_vMov;
	m_vDirPrev = m_vDir;
	m_matShipPrev = m_matShip;

	SMatrix::BuildEulerXYZ( m_matShip, m_fRoll, 0.0f, m_fYaw );
	m_matShip.m30 = m_vPos.x;	m_matShip.m31 = m_vPos.y;	m_matShip.m32 = m_vPos.z;	m_matShip.m33 = 1.0f;

	m_vDir.x = m_matShip.m00;	m_vDir.y = m_matShip.m01;	m_vDir.z = m_matShip.m02;
	m_vUp.x = m_matShip.m20;	m_vUp.y = m_matShip.m21;	m_vUp.z = m_matShip.m22;
	SVector3::Cross( m_vRight, m_vDir, m_vUp );	

	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();
	m_vPos += m_vMov * fElapsedTimeMs;
}

void STurret::Clear()
{
	m_aTurretPositions.clear();
	m_iBulletCounter = 0;

	m_fShootFreqHz = 20.0f;
	m_fBulletSpeed = 0.2f;

	m_aBullets.clear();
	m_bShoot = false;
	m_iLastBulletTimeStampNs = 0;
}

void STurret::Update()
{
	if ( m_bShoot )
	{
		SVector3 vGunDir( m_sShip.m_vDir );
		SVector3 vGunDirPrev( m_sShip.m_vDirPrev );

		SVector3 vGunMov( m_sShip.m_vMov );
		SVector3 vGunMovPrev( m_sShip.m_vMovPrev );

		const float fShootFreqHz = m_fShootFreqHz;
		const uint64_t iShootPeriodNs = (uint64_t)(1.0f / fShootFreqHz * 1000.0f * 1000.0f * 1000.0f);

		uint64_t iTNs = m_iLastBulletTimeStampNs;
		for ( ; iTNs < CEngine::GetInstance().GetTimeStampNs(); iTNs += iShootPeriodNs )
		{
			SVector3 vGunPosWorld;
			SVector3 vGunPosWorldPrev;
			{
				SVector3 vGunPos = m_aTurretPositions[m_iBulletCounter%m_aTurretPositions.size()];
				SMatrix::TransformCoord( vGunPosWorld, vGunPos, m_sShip.m_matShip );
				SMatrix::TransformCoord( vGunPosWorldPrev, vGunPos, m_sShip.m_matShipPrev );
			}

			float fFrameW = (float)(iTNs - m_iLastBulletTimeStampNs) / (float)(CEngine::GetInstance().GetTimeStampNs() - m_iLastBulletTimeStampNs);

			SBullet sBullet;
			sBullet.m_vPos = Lerp( vGunPosWorldPrev, vGunPosWorld, fFrameW );

			SVector3 vBulletDir = Lerp( vGunDirPrev, vGunDir, fFrameW );
			sBullet.m_vMov = Lerp( vGunMovPrev, vGunMov, fFrameW ) + vBulletDir * m_fBulletSpeed;

			/*sBullet.m_vDir.x += (rand() % 1000 - 500) * 0.00001f;
			sBullet.m_vDir.y += (rand() % 1000 - 500) * 0.00001f;
			sBullet.m_vDir.z += (rand() % 1000 - 500) * 0.00001f;*/

			sBullet.m_fTime = 3000.0f;
			sBullet.m_fTimer = 0.0f;
			m_aBullets.push_back( sBullet );			

			SAudioEvent sAudioEvent;
			sAudioEvent.type = SAudioEvent::GunShot;
			sAudioEvent.fVolume = 0.1f;
			sAudioEvent.iTimeStampNs = iTNs;
			sAudioEvent.iLifeTimeNs = 1000 * 1000 * 1500;
			sAudioEvent.iSampleCounter = 0;
			sAudioEvent.fPhase = 0.0f;			
			sAudioEvent.sGunShot.vPos = sBullet.m_vPos;
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

void SShipPlayer::Clear()
{
	m_fYawSpeed = 0.0f;
	m_fYaw_ctrl = 0.0f;
	m_fAccForward = 0.0f;
	m_fAccForward_ctrl = 0.0f;
	m_fAccRight = 0.0f;
	m_fAccRight_ctrl = 0.0f;

	m_sShip.Clear();
	m_sTurret.Clear();

	m_sTurret.m_aTurretPositions.push_back( SVector3( 0.5f, 2.7f, 0.0f ) );
	m_sTurret.m_aTurretPositions.push_back( SVector3( 1.0f, 0.0f, 0.0f ) );
	m_sTurret.m_aTurretPositions.push_back( SVector3( 0.5f, -2.7f, 0.0f ) );
	m_sTurret.m_aTurretPositions.push_back( SVector3( 1.0f, 0.0f, 0.0f ) );
}

void SShipPlayer::Update()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	float fYawMultiplier = m_sTurret.m_bShoot ? 0.5f : 1.0f;

	m_fYawSpeed = SmoothConverge( m_fYawSpeed, m_fYaw_ctrl * fYawMultiplier, 1.01f, 1.01f, fElapsedTimeMs );
	m_fAccForward = SmoothConverge( m_fAccForward, m_fAccForward_ctrl, 1.01f, 1.01f, fElapsedTimeMs );
	m_fAccRight = SmoothConverge( m_fAccRight, m_fAccRight_ctrl, 1.001f, 1.01f, fElapsedTimeMs );

	m_sShip.m_fYaw += m_fYawSpeed * 0.004f * fElapsedTimeMs;
	SVector3 vShipDir( cosf( m_sShip.m_fYaw ), sinf( m_sShip.m_fYaw ), 0.0f );
	SVector3 vShipRight( -vShipDir.y, vShipDir.x, 0.0f );
	m_sShip.m_vMov += vShipDir * m_fAccForward * 0.0001f * fElapsedTimeMs;
	m_sShip.m_vMov += vShipRight * m_fAccRight * 0.0001f * fElapsedTimeMs;

	// m_vMov felbontasa m_vDir es m_vRight iranyara, hogy a ship ne tudjon "csuszni" a levegoben
	/*SVector3 vMovDir( m_vDir );
	vMovDir = vMovDir * SVector3::Dot( m_vMov, vMovDir );
	SVector3 vMovRight( -m_vDir.y, m_vDir.x, 0.0f );
	vMovRight = vMovRight * SVector3::Dot( m_vMov, vMovRight );
	vMovDir = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), vMovDir, CalcSmoothUpdateWeight( 1.0002f, fElapsedTimeMs ) );
	vMovRight = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), vMovRight, CalcSmoothUpdateWeight( 1.0005f, fElapsedTimeMs ) );
	m_vMov = vMovDir + vMovRight;*/

	//m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), m_vMov, CalcSmoothUpdateWeight( 1.0002f, fElapsedTimeMs ) );
	// minel gyorsabban mozgunk annal nagyobb legyen a legellenallas:

	float fSpeedWeight = 1.00005f + SVector3::LengthSq( m_sShip.m_vMov ) * 0.05f * fabsf( m_fAccForward );
	m_sShip.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), m_sShip.m_vMov, CalcSmoothUpdateWeight( fSpeedWeight, fElapsedTimeMs ) );

	//m_fRoll = m_fYaw*0.5f;
	m_sShip.m_fRoll = Lerp( -m_fYawSpeed, m_sShip.m_fRoll, CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );
	//m_fRoll = SmoothConverge( m_fRoll, -m_fYaw * 3.5f, 1.0002f, 1.0002f, fElapsedTimeMs );

	m_sShip.Update();
	m_sTurret.Update();
}

void SShipEnemy::Clear()
{
	m_vBoidMov = SVector3( 0.0f, 0.0f, 0.0f );
	m_fPhase_DistanceToPlayer = 0.0f;
	m_fHP = 100.0f;
}

void SShipEnemy::Update()
{
	m_sShip.Update();
	m_sTurret.Update();
}