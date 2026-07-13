#include "ShipControl.h"
#include "Engine/Engine.h"
#include "Engine/Audio.h"

CShipControl::CShipControl()
{
	Clear();
}

CShipControl::~CShipControl()
{
	Clear();
}

void CShipControl::Clear()
{
	m_vPos = SVector3( 0.0f, 0.0f, 0.0f );
	m_vDir = SVector3( 1.0f, 0.0f, 0.0f );
	m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
	m_vRight = SVector3( 0.0f, 1.0f, 0.0f );
	m_fSpeedForward = 2.0f;
	m_vMouseDir = SVector2( 0.0f, 0.0f );

	SMatrix::Identity( m_matShip );

	m_bShoot = false;
	m_fShootTimer = 0.0f;
	m_aBullets.clear();
}

void CShipControl::Create()
{
	Clear();
}

void CShipControl::Accelerate( float fValue )
{
	m_fSpeedForward += fValue*0.1f;
	m_fSpeedForward = std::max( 0.0f, m_fSpeedForward );
}

void CShipControl::MouseMove( SVector2 vMouseDir )
{
	m_vMouseDir += vMouseDir*0.0001f;

	float len = SVector2::Length( m_vMouseDir );
	if ( len > 0.0f )
	{
		m_vMouseDir /= len;

		static const float scale = 6.0f;
		len = ( 1.0f - expf( -len * scale ) ) / scale;

		m_vMouseDir *= len;
	}
}

void CShipControl::SetShoot( bool bShoot )
{
	m_bShoot = bShoot;
	m_fShootTimer = 0.0f;
}

void CShipControl::Update( float fElapsedTimeMs, const SMatrix& matView000 )
{
	SVector3 vPosPrev = m_vPos;
	SVector3 vDirPrev = m_vDir;
	SMatrix matShipPrev = m_matShip;

	{
		SVector3 vUpView;
		SMatrix::TransformNormal( vUpView, m_vUp, matView000 );
		SVector2 vUpView2DNorm( -vUpView.x, vUpView.y );
		SVector2::Normalize( vUpView2DNorm, vUpView2DNorm );

		SVector3 vRightView;
		SMatrix::TransformNormal( vRightView, m_vRight, matView000 );
		SVector2 vRightView2DNorm( -vRightView.x, vRightView.y );
		SVector2::Normalize( vRightView2DNorm, vRightView2DNorm );

		//SVector2 vMouseDir2DNorm;		
		//SVector2::Normalize( vMouseDir2DNorm, m_vMouseDir );

		float fUp = -SVector2::Dot( vUpView2DNorm, m_vMouseDir ) * fElapsedTimeMs;
		float fRoll = -SVector2::Dot( vRightView2DNorm, m_vMouseDir ) * fElapsedTimeMs;


		m_fSpeedForward *= expf( -fUp*fUp*0.1f );
		{
			float fW = CalcSmoothUpdateWeight( 1.005f, fElapsedTimeMs );
			m_fSpeedForward = Lerp( 2.0f, m_fSpeedForward, fW );
		}

		{
			SQuaternion q;
			SQuaternion::FromAxisAngle( q, m_vRight, fUp*0.1f );
			SMatrix matRot;
			SQuaternion::ToMatrix( matRot, q );

			SVector4 vTemp;
			SVector4 vDir4( m_vDir, 1.0f );		
			SMatrix::Mul( vTemp, vDir4, matRot );
			m_vDir.x = vTemp.x;
			m_vDir.y = vTemp.y;
			m_vDir.z = vTemp.z;

			SVector4 vUp4( m_vUp, 1.0f );
			SMatrix::Mul( vTemp, vUp4, matRot );
			m_vUp.x = vTemp.x;
			m_vUp.y = vTemp.y;
			m_vUp.z = vTemp.z;

			SVector4 vRight4( m_vRight, 1.0f );
			SMatrix::Mul( vTemp, vRight4, matRot );
			m_vRight.x = vTemp.x;
			m_vRight.y = vTemp.y;
			m_vRight.z = vTemp.z;

			SVector3::Cross( m_vRight, m_vDir, m_vUp );
			SVector3::Cross( m_vUp, m_vRight, m_vDir );
			SVector3::Normalize( m_vDir, m_vDir );
			SVector3::Normalize( m_vUp, m_vUp );
			SVector3::Normalize( m_vRight, m_vRight );
		}

		{
			SQuaternion q;
			SQuaternion::FromAxisAngle( q, m_vDir, fRoll*0.3f );
			SMatrix matRot;
			SQuaternion::ToMatrix( matRot, q );

			SVector4 vTemp;
			SVector4 vDir4( m_vDir, 1.0f );		
			SMatrix::Mul( vTemp, vDir4, matRot );
			m_vDir.x = vTemp.x;
			m_vDir.y = vTemp.y;
			m_vDir.z = vTemp.z;

			SVector4 vUp4( m_vUp, 1.0f );
			SMatrix::Mul( vTemp, vUp4, matRot );
			m_vUp.x = vTemp.x;
			m_vUp.y = vTemp.y;
			m_vUp.z = vTemp.z;

			SVector4 vRight4( m_vRight, 1.0f );
			SMatrix::Mul( vTemp, vRight4, matRot );
			m_vRight.x = vTemp.x;
			m_vRight.y = vTemp.y;
			m_vRight.z = vTemp.z;

			SVector3::Cross( m_vRight, m_vDir, m_vUp );
			SVector3::Cross( m_vUp, m_vRight, m_vDir );
			SVector3::Normalize( m_vDir, m_vDir );
			SVector3::Normalize( m_vUp, m_vUp );
			SVector3::Normalize( m_vRight, m_vRight );
		}

		/*SMatrix matView000Inv;
		SMatrix::Transpose( matView000Inv, matView000 );
		SVector3 vMouseDirWorld;
		SMatrix::TransformNormal( vMouseDirWorld, vMouseDir, matView000Inv );*/

	}


	m_vPos += m_vDir * m_fSpeedForward * fElapsedTimeMs;

	float fWDir = CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs );
	m_vMouseDir *= fWDir;

	{
		m_matShip.m00 = m_vDir.x;	m_matShip.m01 = m_vDir.y;	m_matShip.m02 = m_vDir.z;	m_matShip.m03 = 0.0f;
		m_matShip.m10 = m_vRight.x;	m_matShip.m11 = m_vRight.y;	m_matShip.m12 = m_vRight.z;	m_matShip.m13 = 0.0f;
		m_matShip.m20 = m_vUp.x;	m_matShip.m21 = m_vUp.y;	m_matShip.m22 = m_vUp.z;	m_matShip.m23 = 0.0f;
		m_matShip.m30 = m_vPos.x;	m_matShip.m31 = m_vPos.y;	m_matShip.m32 = m_vPos.z;	m_matShip.m33 = 1.0f;	
	}

	if ( m_bShoot )
	{
		SVector3 vGunPos = SVector3( 1.0f, 0.0f, 0.1f );

		SVector3 vGunPosWorld;
		SVector3 vGunPosWorlPrev;

		SVector3 vGunDirWorld( m_vDir );
		SVector3 vGunDirWorlPrev( vDirPrev );

		SMatrix::TransformCoord( vGunPosWorld, vGunPos, m_matShip );
		SMatrix::TransformCoord( vGunPosWorlPrev, vGunPos, matShipPrev );

		const float fShootTimerPrev = m_fShootTimer;
		m_fShootTimer += fElapsedTimeMs;
		const float fShootFreqHz = 20.0f;
		float fShootPeriodMs = 1000.0f / fShootFreqHz;
		for ( float t = (float)(int)( fShootTimerPrev / fShootPeriodMs + 1 ) * fShootPeriodMs; t < m_fShootTimer; t += fShootPeriodMs )
		{		

			float fFrameW = (t - fShootTimerPrev) / (m_fShootTimer - fShootTimerPrev);

			SBullet sBullet;
			sBullet.m_vPos = Lerp( vGunPosWorlPrev, vGunPosWorld, fFrameW );
			sBullet.m_vDir = Lerp( vGunDirWorlPrev, vGunDirWorld, fFrameW );

			sBullet.m_vDir.x += (rand() % 1000 - 500) * 0.0001f;
			sBullet.m_vDir.y += (rand() % 1000 - 500) * 0.0001f;
			sBullet.m_vDir.z += (rand() % 1000 - 500) * 0.0001f;

			SVector3::Normalize( sBullet.m_vDir, sBullet.m_vDir );
			sBullet.m_fSpeed = (m_fSpeedForward + 0.000f) * fElapsedTimeMs;
			sBullet.m_fTime = 4000.0f;
			sBullet.m_fTimer = 0.0f;
			m_aBullets.push_back( sBullet );

			SAudioEvent sAudioEvent;
			sAudioEvent.type = SAudioEvent::GunShot;
			sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
			sAudioEvent.fVolume = 0.3f;
			sAudioEvent.sGunShot.vPos = sBullet.m_vPos;
			CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );
		}		
	}

	for ( int iBulletInd = 0; iBulletInd < m_aBullets.size(); ++iBulletInd )
	{
		SBullet& sBullet = m_aBullets[iBulletInd];
		sBullet.m_vPos += sBullet.m_vDir * sBullet.m_fSpeed * fElapsedTimeMs;
		sBullet.m_fTimer += fElapsedTimeMs;
		if ( sBullet.m_fTimer > sBullet.m_fTime )
		{
			m_aBullets[iBulletInd] = m_aBullets.back();
			m_aBullets.pop_back();
			--iBulletInd;
		}
	}
}

