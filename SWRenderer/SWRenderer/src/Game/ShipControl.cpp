#include "ShipControl.h"

#include "Common/Globals.h"
#include "Engine/Engine.h"

void SShipControl::Clear()
{
	m_fYaw = 0.0f;
	m_fYawSpeed = 0.0f;
	m_fYaw_ctrl = 0.0f;
	m_fRoll = 0.0f;
	m_fAccForward = 0.0f;
	m_fAccForward_ctrl = 0.0f;
	m_fAccRight = 0.0f;
	m_fAccRight_ctrl = 0.0f;
	m_vMov = SVector3( 0.0f, 0.0f, 0.0f );
	m_vMovPrev = SVector3( 0.0f, 0.0f, 0.0f );

	m_vPos = SVector3( 0.0f, 0.0f, 0.0f );
	m_vDir = SVector3( 1.0f, 0.0f, 0.0f );
	m_vDirPrev = m_vDir;
	m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
	m_vRight = SVector3( 0.0f, 1.0f, 0.0f );
	SMatrix::Identity( m_matShip );
	SMatrix::Identity( m_matShipPrev );

	m_aTurretPositions.clear();
	m_iBulletCounter = 0;
	m_aBullets.clear();
	m_bShoot = false;
	m_iLastBulletTimeStampNs = 0;
}

void SShipControl::UpdateControl()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	m_vDirPrev = m_vDir;
	m_vMovPrev = m_vMov;
	m_matShipPrev = m_matShip;

	m_fYawSpeed = SmoothConverge( m_fYawSpeed, m_fYaw_ctrl, 1.01f, 1.01f, fElapsedTimeMs );
	m_fAccForward = SmoothConverge( m_fAccForward, m_fAccForward_ctrl, 1.01f, 1.001f, fElapsedTimeMs );
	m_fAccRight = SmoothConverge( m_fAccRight, m_fAccRight_ctrl, 1.001f, 1.01f, fElapsedTimeMs );

	m_fYaw += m_fYawSpeed * 0.004f * fElapsedTimeMs;
	SVector3 vShipDir( cosf( m_fYaw ), sinf( m_fYaw ), 0.0f );
	SVector3 vShipRight( -vShipDir.y, vShipDir.x, 0.0f );
	m_vMov += vShipDir * m_fAccForward * 0.00015f * fElapsedTimeMs;
	m_vMov += vShipRight * m_fAccRight * 0.00015f * fElapsedTimeMs;

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

	m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), m_vMov, CalcSmoothUpdateWeight( 1.0002f + SVector3::LengthSq( m_vMov ) * 0.05f, fElapsedTimeMs ) );

	//m_fRoll = m_fYaw*0.5f;
	m_fRoll = Lerp( -m_fYawSpeed, m_fRoll, CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );
	//m_fRoll = SmoothConverge( m_fRoll, -m_fYaw * 3.5f, 1.0002f, 1.0002f, fElapsedTimeMs );

	m_vPos += m_vMov * fElapsedTimeMs;
}

void SShipControl::UpdateMatrices()
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

void SShipControl::UpdateShoot()
{
	if ( m_bShoot )
	{
		SVector3 vGunDir( m_vDir );
		SVector3 vGunDirPrev( m_vDirPrev );

		SVector3 vGunMov( m_vMov );
		SVector3 vGunMovPrev( m_vMovPrev );

		const float fShootFreqHz = 20.0f;
		const uint64_t iShootPeriodNs = (uint64_t)(1.0f / fShootFreqHz * 1000.0f * 1000.0f * 1000.0f);

		uint64_t iTNs = m_iLastBulletTimeStampNs;
		for ( ; iTNs < CEngine::GetInstance().GetTimeStampNs(); iTNs += iShootPeriodNs )
		{
			SVector3 vGunPosWorld;
			SVector3 vGunPosWorldPrev;
			{
				SVector3 vGunPos = m_aTurretPositions[m_iBulletCounter%m_aTurretPositions.size()];
				SMatrix::TransformCoord( vGunPosWorld, vGunPos, m_matShip );
				SMatrix::TransformCoord( vGunPosWorldPrev, vGunPos, m_matShipPrev );
			}

			float fFrameW = (float)(iTNs - m_iLastBulletTimeStampNs) / (float)(CEngine::GetInstance().GetTimeStampNs() - m_iLastBulletTimeStampNs);

			SShipControl::SBullet sBullet;
			sBullet.m_vPos = Lerp( vGunPosWorldPrev, vGunPosWorld, fFrameW );

			SVector3 vBulletDir = Lerp( vGunDirPrev, vGunDir, fFrameW );
			sBullet.m_vMov = Lerp( vGunMovPrev, vGunMov, fFrameW ) + vBulletDir * 0.1f;

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
		SShipControl::SBullet& sBullet = m_aBullets[iBulletInd];
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
