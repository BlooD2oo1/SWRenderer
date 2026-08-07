#include "SceneGame.h"
#include "Common/PCXLoader.h"
#include "Engine/Engine.h"

CSceneGame::CSceneGame()
{
	m_pStars = nullptr;
	Clear();
}

CSceneGame::~CSceneGame()
{
	Clear();
}

void CSceneGame::Clear()
{
	SAFE_DELETE_ARRAY( m_pStars );
	m_iStarsCount = 0;

	m_sCamera.Clear();
	m_sTexHUD.Clear();

	m_sShipPlayer.Clear();
	m_aEnemyShips.clear();	
}

void CSceneGame::Create()
{
	Clear();

	PCX_LoadFromFile( "data/hud.pcx", m_sTexHUD );
	
	m_sViewportGameView.Create( SVector2( 0.0f, 0.0f ), SVector2( (float)CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD.m_iWidth, (float)CGraphics::GetInstance().GetFrameBuffer().iHeight ) );
	m_sCamera.m_fAspect = ( m_sViewportGameView.Get11().x - m_sViewportGameView.Get00().x ) / ( m_sViewportGameView.Get11().y - m_sViewportGameView.Get00().y );

	m_sViewportMiniMap.Create( SVector2( (float)( CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD.m_iWidth + 10 ), 134.0f ), SVector2( (float)( CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD.m_iWidth + 10 + 60 ), 134.0f + 60.0f ) );

	m_iStarsCount = 1000;
	m_pStars = new SVertexPC[m_iStarsCount];
	for ( uint32_t i = 0; i < m_iStarsCount; i++ )
	{
		m_pStars[i].vPos.x = ((float)rand()/(float)RAND_MAX);
		m_pStars[i].vPos.y = ((float)rand()/(float)RAND_MAX);
		m_pStars[i].vPos.z = ((float)rand()/(float)RAND_MAX);

		float a = ((float)rand()/(float)RAND_MAX);
		a = powf( a, 40.0f );
		a = a * 0.8f + 0.2f;
		m_pStars[i].sAttribs.vColor = SVector4( ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, ((float)rand()/(float)RAND_MAX)*0.1f+0.7f, ((float)rand()/(float)RAND_MAX)*0.1f+0.6f, a*2.0f );
	}

	m_aAsteroids.reserve( 500 );
	for ( size_t i = 0; i < m_aAsteroids.capacity(); i++ )
	{
		SAsteroid sAsteroid;
		const float fScatterRadius = 2000.0f;
		sAsteroid.m_vPos.x = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sAsteroid.m_vPos.y = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sAsteroid.m_vPos.z = 0.0f;
		sAsteroid.m_fSize = ( ((float)rand() / (float)RAND_MAX) * 0.5f + 0.5f ) * 5.0f;

		// uniform distribution of quaternions:
		const float u1 = ((float)rand() / (float)RAND_MAX);
		const float u2 = ((float)rand() / (float)RAND_MAX);
		const float u3 = ((float)rand() / (float)RAND_MAX);
		sAsteroid.m_qRot.w = sqrtf( 1.0f - u1 ) * sinf( 2.0f * PI * u2 );
		sAsteroid.m_qRot.x = sqrtf( 1.0f - u1 ) * cosf( 2.0f * PI * u2 );
		sAsteroid.m_qRot.y = sqrtf( u1 ) * sinf( 2.0f * PI * u3 );
		sAsteroid.m_qRot.z = sqrtf( u1 ) * cosf( 2.0f * PI * u3 );

		m_aAsteroids.push_back( sAsteroid );
	}

	m_aEnemyShips.reserve( 20 );
	for ( size_t i = 0; i < m_aEnemyShips.capacity(); i++ )
	{
		m_aEnemyShips.emplace_back();
		SShipEnemy& sShipEnemy = m_aEnemyShips.back();

		sShipEnemy.m_sTurret.m_aTurretPositions.push_back( SVector3( 0.3f, 0.0f, 0.0f ) );

		const float fScatterRadius = 40.0f;
		sShipEnemy.m_sShip.m_vPos.x = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sShipEnemy.m_sShip.m_vPos.y = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sShipEnemy.m_sShip.m_vPos.z = 0.0f;

		sShipEnemy.m_sShip.m_fYaw = ((float)rand() / (float)RAND_MAX) * PI2;

		sShipEnemy.f0 = ((float)rand() / (float)RAND_MAX);
		sShipEnemy.f1 = ((float)rand() / (float)RAND_MAX);
		sShipEnemy.f2 = ((float)rand() / (float)RAND_MAX);
		sShipEnemy.f3 = ((float)rand() / (float)RAND_MAX);

		sShipEnemy.m_fPhase_DistanceToPlayer = ((float)rand() / (float)RAND_MAX) * PI2;

		sShipEnemy.m_sTurret.m_fShootFreqHz = 10.0f;
		sShipEnemy.m_sTurret.m_fBulletSpeed = 0.1f;
	}
}

static float fAction = 0.0f;
static float fClimax = 0.0f;

void CSceneGame::Update()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();
	{
		// Update ship:
		m_sShipPlayer.Update();

		// Update enemy ships:
		//boids:
		for ( size_t i0 = 0; i0 < m_aEnemyShips.size(); i0++ )
		{
			SShipEnemy& sEnemyShip0 = m_aEnemyShips[i0];

			SVector2 vEnemyToPlayer( m_sShipPlayer.m_sShip.m_vPos.x - sEnemyShip0.m_sShip.m_vPos.x, m_sShipPlayer.m_sShip.m_vPos.y - sEnemyShip0.m_sShip.m_vPos.y );
			float fEnemyToPlayerDist = SVector2::Length( vEnemyToPlayer );

			SVector2 vSeparation( 0.0f, 0.0f );
			SVector2 vAlignment( 0.0f, 0.0f );
			SVector2 vCohesion( 0.0f, 0.0f );
			
			int iNeighborCount = 0;
			SVector2 vAvgPos( 0.0f, 0.0f );
			SVector2 vAvgMov( 0.0f, 0.0f );

			for ( size_t i1 = 0; i1 < m_aEnemyShips.size(); i1++ )
			{
				if ( i0 == i1 ) continue;
				SShipEnemy& sEnemyShip1 = m_aEnemyShips[i1];

				SVector2 vDist( sEnemyShip1.m_sShip.m_vPos.x - sEnemyShip0.m_sShip.m_vPos.x, sEnemyShip1.m_sShip.m_vPos.y - sEnemyShip0.m_sShip.m_vPos.y );
				float fDistSq = SVector2::LengthSq( vDist );
				if ( fDistSq < powf( 200.0f, 2 ) )
				{
					if ( fDistSq < powf( 100.0f, 2 ) )
					{
						SVector2 vDirAway( -vDist.x, -vDist.y );
						vSeparation += vDirAway * (1.0f / (fDistSq + 0.00001f));
					}

					iNeighborCount++;
					vAvgPos += SVector2( sEnemyShip1.m_sShip.m_vPos.x, sEnemyShip1.m_sShip.m_vPos.y );
					vAvgMov += SVector2( sEnemyShip1.m_sShip.m_vMov.x, sEnemyShip1.m_sShip.m_vMov.y );
				}
			}

			if ( iNeighborCount > 0 )
			{
				vAvgMov /= (float)iNeighborCount;
				vAlignment = vAvgMov - SVector2( sEnemyShip0.m_sShip.m_vMov.x, sEnemyShip0.m_sShip.m_vMov.y );

				vAvgPos /= (float)iNeighborCount;
				vCohesion = vAvgPos - SVector2( sEnemyShip0.m_sShip.m_vPos.x, sEnemyShip0.m_sShip.m_vPos.y );
			}

			SVector2 vBoidMov =	vSeparation * 600.0f +// * sEnemyShip0.f0 +
								vAlignment * 10.0f +
								vCohesion * 0.3f;

			sEnemyShip0.m_vBoidMov = SVector3( vBoidMov.x, vBoidMov.y, 0.0f );
		}

		for ( int i = 0; i < m_aEnemyShips.size(); i++ )
		{
			SShipEnemy& sEnemyShip = m_aEnemyShips[i];

			SVector3 vEnemyToPlayerDir( m_sShipPlayer.m_sShip.m_vPos - sEnemyShip.m_sShip.m_vPos /*- m_sShipPlayer.m_sShip.m_vDir*10.0f*/ );
			const float fEnemyToPlayerDist = SVector3::Length( vEnemyToPlayerDir );
			SVector3::Normalize( vEnemyToPlayerDir, vEnemyToPlayerDir );
			const float fSin_DistanceToPlayer = powf( sinf( sEnemyShip.m_fPhase_DistanceToPlayer )*0.5f+0.5f, 0.5f );
			const float fFollowAmount = Clamp( (fEnemyToPlayerDist-Lerp(20.0f, 110.0f, fSin_DistanceToPlayer))*0.02f, -0.4f, 1.0f );
			SVector3 vFollowMov = vEnemyToPlayerDir * fFollowAmount;
			
			sEnemyShip.m_sShip.m_vMov += sEnemyShip.m_vBoidMov * 0.001f;
			sEnemyShip.m_sShip.m_vMov += vFollowMov * 0.1f;
			
			sEnemyShip.m_sShip.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), sEnemyShip.m_sShip.m_vMov, CalcSmoothUpdateWeight( 1.02f, fElapsedTimeMs ) );
			//float fSpeedWeight = 1.02f + SVector3::LengthSq( sEnemyShip.m_sShip.m_vMov ) * 0.1f;
			//sEnemyShip.m_sShip.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), sEnemyShip.m_sShip.m_vMov, CalcSmoothUpdateWeight( fSpeedWeight, fElapsedTimeMs ) );

			float l = SVector3::Length( sEnemyShip.m_sShip.m_vMov );
			if ( l > 0.0001f )
			{
				sEnemyShip.m_sShip.m_vMov = sEnemyShip.m_sShip.m_vMov * (1.0f / l) * Clamp( l, 0.0f, 0.15f );
			}

			// yaw from mov:
			const float fYaw = atan2f( sEnemyShip.m_sShip.m_vMov.y, sEnemyShip.m_sShip.m_vMov.x );
			sEnemyShip.m_sShip.m_fYaw = Lerp( fYaw, sEnemyShip.m_sShip.m_fYaw, CalcSmoothUpdateWeight( 1.005f, fElapsedTimeMs ) );

			if ( fSin_DistanceToPlayer < 0.1f && SVector3::Dot( sEnemyShip.m_sShip.m_vDir, vEnemyToPlayerDir ) > 0.8f )
			{
				if ( !sEnemyShip.m_sTurret.m_bShoot )
				{
					sEnemyShip.m_sTurret.m_iLastBulletTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
				}
				sEnemyShip.m_sTurret.m_bShoot = true;
			}
			else
			{
				sEnemyShip.m_sTurret.m_bShoot = false;
			}

			sEnemyShip.m_fPhase_DistanceToPlayer += fElapsedTimeMs * 0.0003f;
			if ( sEnemyShip.m_fPhase_DistanceToPlayer > PI2 ) sEnemyShip.m_fPhase_DistanceToPlayer -= PI2;

			for ( int iBulletInd = 0; iBulletInd < m_sShipPlayer.m_sTurret.m_aBullets.size(); iBulletInd++ )
			{
				STurret::SBullet& sBullet = m_sShipPlayer.m_sTurret.m_aBullets[iBulletInd];
				SVector2 vBulletPosPrev( sBullet.m_vPosPrev.x, sBullet.m_vPosPrev.y );
				SVector2 vBulletPos( sBullet.m_vPos.x, sBullet.m_vPos.y );
				float fT = 0.0f;
				if ( SegmentSphereTest( vBulletPosPrev, vBulletPos, SVector2( sEnemyShip.m_sShip.m_vPos.x, sEnemyShip.m_sShip.m_vPos.y ), 2.0f, fT ) )
				{
					sBullet.m_vMov *= 0.3f;
					sEnemyShip.m_sShip.m_vMov += SVector3( sBullet.m_vMov.x, sBullet.m_vMov.y, 0.0f ) * 0.5f;
					sEnemyShip.m_fHP -= 40.0f;

					SAudioEvent sAudioEvent;
					sAudioEvent.type = SAudioEvent::ClickDown;
					sAudioEvent.fVolume = 0.2f;
					sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
					sAudioEvent.iLifeTimeNs = 1000 * 1000 * 1000;
					sAudioEvent.iSampleCounter = 0;
					sAudioEvent.fPhase = 0.0f;	
					sAudioEvent.sClick.iButton = 1;
					CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );
				}				
			}

			if ( sEnemyShip.m_fHP <= 0.0f )
			{
				sEnemyShip.m_sShip.m_vPos.x = ((float)rand() / (float)RAND_MAX) * 40.0f * 2.0f - 40.0f;
				sEnemyShip.m_sShip.m_vPos.y = ((float)rand() / (float)RAND_MAX) * 40.0f * 2.0f - 40.0f;
				sEnemyShip.m_fHP = 100.0f;
			}

			sEnemyShip.Update();
		}
	}

	{
		// Update camera:
		float fWFast = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		float fWSlow = CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs );
		SVector3 vP( m_sShipPlayer.m_sShip.m_vPos + m_sShipPlayer.m_sShip.m_vDir * 6.0f + m_sShipPlayer.m_sShip.m_vMov*300.0f );
		//vP += m_sShipPlayer.m_vDir * 10.0f;
		m_sCamera.m_vLookAt = vP;
		m_sCamera.m_vEye = vP;
		m_sCamera.m_vEye.z += Lerp( 1200.0f, 80.0f, expf( -SVector3::Length( m_sShipPlayer.m_sShip.m_vMov ) * 2.5f ) );
		//m_sCamera.m_vEye.z += 200.0f;
		m_sCamera.m_vLookAtSmooth = Lerp( m_sCamera.m_vLookAt, m_sCamera.m_vLookAtSmooth, fWFast );
		m_sCamera.m_vEyeSmooth = Lerp( m_sCamera.m_vEye, m_sCamera.m_vEyeSmooth, fWSlow );

		//SVector2 vDir2D( m_sShipPlayer.m_sShip.m_vDir.x, m_sShipPlayer.m_sShip.m_vDir.y );
		SVector2 vDir2D( m_sShipPlayer.m_sShip.m_vMov.x, m_sShipPlayer.m_sShip.m_vMov.y ); SVector2::Normalize( vDir2D, vDir2D );
		//SVector2::Slerp( vDir2D, vDir2D, SVector2( m_sCamera.m_vUp.x, m_sCamera.m_vUp.y ), CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );
		vDir2D = Lerp( vDir2D, SVector2( m_sCamera.m_vUp.x, m_sCamera.m_vUp.y ), CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );
		SVector2::Normalize( vDir2D, vDir2D );
		m_sCamera.m_vUp.x = vDir2D.x;
		m_sCamera.m_vUp.y = vDir2D.y;


		
		m_sCamera.UpdateMatrices();
	}
	// log ShipAcc, ShipYaw, ShipSpeed:
	

	CEngine::GetInstance().GetAudioFrameData().m_fShipAcc = m_sShipPlayer.m_fAccForward + m_sShipPlayer.m_fAccRight;
	CEngine::GetInstance().GetAudioFrameData().m_fShipYawSpeed = m_sShipPlayer.m_fYawSpeed;
	CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed = SVector3::Length( m_sShipPlayer.m_sShip.m_vMov ) / 0.1f;

	//LOG( "ShipAcc=%.8f, ShipYawSpeed=%.8f, ShipSpeed=%.8f\n", CEngine::GetInstance().GetAudioFrameData().m_fShipAcc, CEngine::GetInstance().GetAudioFrameData().m_fShipYawSpeed, CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed );
	
	CEngine::GetInstance().GetAudioFrameData().m_vShipPos = m_sShipPlayer.m_sShip.m_vPos;
	CEngine::GetInstance().GetAudioFrameData().m_vCameraEye = m_sCamera.m_vEyeSmooth;
	CEngine::GetInstance().GetAudioFrameData().m_vCameraLookAt = m_sCamera.m_vLookAtSmooth;

	CEngine::GetInstance().GetAudioFrameData().m_fMusic_Action = fAction;
	CEngine::GetInstance().GetAudioFrameData().m_fMusic_Climax = fClimax;
}

void CSceneGame::Render()
{
	{
		struct SPixelShaderBasic
		{
			BGRA8 sColor;
			BGRA8 Execute( const SVertexP::SAttribs& in ) const
			{
				return sColor;
			}
		} sPixelShaderBasic;

		SMatrix matViewProjViewport;
		SMatrix::Mul( matViewProjViewport, m_sCamera.m_matViewProj, m_sViewportGameView.GetViewPortMatrix() );
		SMatrix matViewProjViewportPrev;
		SMatrix::Mul( matViewProjViewportPrev, m_sCamera.m_matViewProjPrev, m_sViewportGameView.GetViewPortMatrix() );

		float fAlpha = 1.0f;
		const int iSteps = 2;
		for ( int j =0; j < iSteps; j++ )
		{
			float fStarBoxSize = powf( (float)(j+1), 3.0f ) * 200.0f;
			float fStarBoxSizeInv = 1.0f / fStarBoxSize;
			for ( uint32_t i = 0; i < m_iStarsCount; i++ )
			{
				SClipVertex<SVertexP::SAttribs> sPh0;
				SClipVertex<SVertexP::SAttribs> sPh1;
				{
					SVector4 vPhSrc( m_pStars[i].vPos * fStarBoxSize, 1.0f );
					vPhSrc.x = vPhSrc.x - floorf((vPhSrc.x - m_sCamera.m_vEyeSmooth.x) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					vPhSrc.y = vPhSrc.y - floorf((vPhSrc.y - m_sCamera.m_vEyeSmooth.y) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					vPhSrc.z = vPhSrc.z - floorf((vPhSrc.z - m_sCamera.m_vEyeSmooth.z) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					SMatrix::Mul( sPh0.vPos, vPhSrc, matViewProjViewport );
					SMatrix::Mul( sPh1.vPos, vPhSrc, matViewProjViewportPrev );

				}

				if ( CGraphics::GetInstance().ClipLineZ( sPh0, sPh1 ) )
				{
					if ( CGraphics::GetInstance().ClipLineXY( sPh0, sPh1, m_sViewportGameView ) )
					{
						{
							float fWRec0 = 1.0f / sPh0.vPos.w;
							sPh0.vPos.x = sPh0.vPos.x * fWRec0;
							sPh0.vPos.y = sPh0.vPos.y * fWRec0;

							float fWRec1 = 1.0f / sPh1.vPos.w;
							sPh1.vPos.x = sPh1.vPos.x * fWRec1;
							sPh1.vPos.y = sPh1.vPos.y * fWRec1;
						}

						SVector2 vL( sPh0.vPos.x - sPh1.vPos.x, sPh0.vPos.y - sPh1.vPos.y );
						float fL = SVector2::Length( vL );

						if ( fL > 1.5f )
						{
							sPixelShaderBasic.sColor = BGRA8( m_pStars[i].sAttribs.vColor.x * fAlpha, m_pStars[i].sAttribs.vColor.y, m_pStars[i].sAttribs.vColor.z * fAlpha, m_pStars[i].sAttribs.vColor.w / (fL * 0.2f + 1.0f) );
							CGraphics::GetInstance().RasterizeLineFlat( SVector2( sPh0.vPos.x, sPh0.vPos.y ), SVector2( sPh1.vPos.x, sPh1.vPos.y ), sPh0.sAttribs, sPixelShaderBasic, SBlendFuncAdditive() );
						}
						else
						{
							BGRA8 sColor = BGRA8( m_pStars[i].sAttribs.vColor.x * fAlpha, m_pStars[i].sAttribs.vColor.y, m_pStars[i].sAttribs.vColor.z * fAlpha, m_pStars[i].sAttribs.vColor.w );
							CGraphics::GetInstance().RasterizePixel( (int)sPh0.vPos.x, (int)sPh0.vPos.y, sColor, SBlendFuncAdditive() );
						}
					}
				}
			}
		}
	}
	

	struct SVertexShaderBasic
	{
		using AttribsType = SVertexPC::SAttribs;
		SMatrix matWorldViewProjViewPort;
		float fAlpha;
		void Execute( SClipVertex<AttribsType>& out, const SVertexPC& in ) const
		{
			SVector4 vPhSrc( in.vPos, 1.0f );
			SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProjViewPort );
			out.sAttribs.vColor = in.sAttribs.vColor;
			out.sAttribs.vColor.w *= fAlpha;
		}
	} sVertexShaderBasic;
	SMatrix matViewProjViewPort;
	SMatrix::Mul( matViewProjViewPort, m_sCamera.m_matViewProj, m_sViewportGameView.GetViewPortMatrix() );

	struct SPixelShaderBasic
	{
		BGRA8 Execute( const SVertexPC::SAttribs& in ) const
		{
			return BGRA8( in.vColor.x, in.vColor.y, in.vColor.z, in.vColor.w );
		}
	};

	//render coord sys at origo:
	{
		sVertexShaderBasic.matWorldViewProjViewPort = matViewProjViewPort;                                                            
		sVertexShaderBasic.fAlpha = 1.0f;
		SVertexPC sLine[2];

		sLine[0].vPos = SVector3( 0.0f, 0.0f, 0.0f );
		sLine[1].vPos = SVector3( 1.0f, 0.0f, 0.0f );
		sLine[0].sAttribs.vColor = SVector4( 0.0f, 0.0f, 1.0f, 1.0f );
		sLine[1].sAttribs.vColor = SVector4( 0.0f, 0.0f, 1.0f, 0.5f );
		CGraphics::GetInstance().DrawLine3D( sLine[0], sLine[1], m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );

		sLine[0].vPos = SVector3( 0.0f, 0.0f, 0.0f );
		sLine[1].vPos = SVector3( 0.0f, 1.0f, 0.0f );
		sLine[0].sAttribs.vColor = SVector4( 0.0f, 1.0f, 0.0f, 1.0f );
		sLine[1].sAttribs.vColor = SVector4( 0.0f, 1.0f, 0.0f, 0.5f );
		CGraphics::GetInstance().DrawLine3D( sLine[0], sLine[1], m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );

		sLine[0].vPos = SVector3( 0.0f, 0.0f, 0.0f );
		sLine[1].vPos = SVector3( 0.0f, 0.0f, 1.0f );
		sLine[0].sAttribs.vColor = SVector4( 1.0f, 0.0f, 0.0f, 1.0f );
		sLine[1].sAttribs.vColor = SVector4( 1.0f, 0.0f, 0.0f, 0.5f );
		CGraphics::GetInstance().DrawLine3D( sLine[0], sLine[1], m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );

	}

	{
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, m_sShipPlayer.m_sShip.m_matShip, matViewProjViewPort );
		
		sVertexShaderBasic.fAlpha = 0.7f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetShipMeshPlayer().m_pVertices, CEngine::GetInstance().GetShipMeshPlayer().m_pIndices, CEngine::GetInstance().GetShipMeshPlayer().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
	}

	{
		SMatrix matShip;
		SMatrix::BuildEulerXYZ( matShip, 0.5f, 0.5f, 2.1f );
		SMatrix::Scale( matShip, 80.0f );

		matShip.m32 = -800.0f;
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, matShip, matViewProjViewPort );

		sVertexShaderBasic.fAlpha = 0.2f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetShipMeshDestroyer().m_pVertices, CEngine::GetInstance().GetShipMeshDestroyer().m_pIndices, CEngine::GetInstance().GetShipMeshDestroyer().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
	}

	for ( size_t iEnemyShipInd = 0; iEnemyShipInd < m_aEnemyShips.size(); iEnemyShipInd++ )
	{
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, m_aEnemyShips[iEnemyShipInd].m_sShip.m_matShip, matViewProjViewPort );
		sVertexShaderBasic.fAlpha = 0.7f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetShipMeshScout().m_pVertices, CEngine::GetInstance().GetShipMeshScout().m_pIndices, CEngine::GetInstance().GetShipMeshScout().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
	}

	for ( size_t iAsteroidInd = 0; iAsteroidInd < m_aAsteroids.size(); iAsteroidInd++ )
	{
		const SAsteroid& sAsteroid = m_aAsteroids[iAsteroidInd];
		if ( m_sCamera.FrustumSphereTest( sAsteroid.m_vPos, sAsteroid.m_fSize*2.0f ) )
		{
			SMatrix matAsteroid;
			SMatrix::Identity( matAsteroid );
			//SQuaternion::ToMatrix( matAsteroid, sAsteroid.m_qRot );
			matAsteroid.m30 = sAsteroid.m_vPos.x;
			matAsteroid.m31 = sAsteroid.m_vPos.y;
			matAsteroid.m32 = sAsteroid.m_vPos.z;
			SMatrix::Scale( matAsteroid, sAsteroid.m_fSize );
			SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, matAsteroid, matViewProjViewPort );
			sVertexShaderBasic.fAlpha = 0.7f;
			CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetAsteroidMesh().m_pVertices, CEngine::GetInstance().GetAsteroidMesh().m_pIndices, CEngine::GetInstance().GetAsteroidMesh().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
		}
	}
	
	
	{
		struct SPixelShaderBasic
		{
			BGRA8 sColor;
			BGRA8 Execute( const SVertexP::SAttribs& in ) const
			{
				return sColor;
			}
		} sPixelShaderBasic;

		SMatrix matViewProjViewport;
		SMatrix::Mul( matViewProjViewport, m_sCamera.m_matViewProj, m_sViewportGameView.GetViewPortMatrix() );
		SMatrix matViewProjViewportPrev;
		SMatrix::Mul( matViewProjViewportPrev, m_sCamera.m_matViewProjPrev, m_sViewportGameView.GetViewPortMatrix() );

		for ( int iShipInd = -1; iShipInd < (int)m_aEnemyShips.size(); iShipInd++ )
		{
			const STurret& sShip = (iShipInd == -1) ? m_sShipPlayer.m_sTurret : m_aEnemyShips[iShipInd].m_sTurret;

			for ( int iBulletInd = 0; iBulletInd < sShip.m_aBullets.size(); iBulletInd++ )
			{
				const STurret::SBullet& sBullet = sShip.m_aBullets[iBulletInd];
				SClipVertex<SVertexP::SAttribs> sPh0;
				SClipVertex<SVertexP::SAttribs> sPh1;

				{
					SVector4 vPhSrc0( sBullet.m_vPos, 1.0f );
					SVector4 vPhSrc1( sBullet.m_vPosPrev, 1.0f );
					SMatrix::Mul( sPh0.vPos, vPhSrc0, matViewProjViewport );
					SMatrix::Mul( sPh1.vPos, vPhSrc1, matViewProjViewportPrev );
				}

				if ( CGraphics::GetInstance().ClipLineZ( sPh0, sPh1 ) )
				{
					if ( CGraphics::GetInstance().ClipLineXY( sPh0, sPh1, m_sViewportGameView ) )
					{
						{
							float fWRec0 = 1.0f / sPh0.vPos.w;
							sPh0.vPos.x = sPh0.vPos.x * fWRec0;
							sPh0.vPos.y = sPh0.vPos.y * fWRec0;

							float fWRec1 = 1.0f / sPh1.vPos.w;
							sPh1.vPos.x = sPh1.vPos.x * fWRec1;
							sPh1.vPos.y = sPh1.vPos.y * fWRec1;
						}

						SVector2 vL( sPh0.vPos.x - sPh1.vPos.x, sPh0.vPos.y - sPh1.vPos.y );
						float fL = SVector2::Length( vL );

						float fAlpha = sBullet.m_fTimer / sBullet.m_fTime;
						fAlpha *= fAlpha * fAlpha;
						fAlpha *= fAlpha * fAlpha;
						fAlpha = 1.0f - fAlpha;
						sPixelShaderBasic.sColor = BGRA8( 0.0f, fAlpha, 1.0f, 1.0f );

						if ( fL > 1.5f )
						{
							CGraphics::GetInstance().RasterizeLineFlat( SVector2( sPh0.vPos.x, sPh0.vPos.y ), SVector2( sPh1.vPos.x, sPh1.vPos.y ), sPh0.sAttribs, sPixelShaderBasic, SBlendFuncAdditive() );
						}
						else
						{
							CGraphics::GetInstance().RasterizePixel( (int)sPh0.vPos.x, (int)sPh0.vPos.y, sPixelShaderBasic.sColor, SBlendFuncAdditive() );
						}
					}
				}
			}
		}
	}
	
	
	{
		SVector4 vColor = SVector4( 0.3f, 0.2f, 0.1f, 0.7f );
		struct SVertexShaderGrid
		{
			using AttribsType = SVertexPCW::SAttribs;
			SMatrix matWorldViewProj;
			void Execute( SClipVertex<AttribsType>& out, const SVertexPCW& in ) const
			{
				SVector4 vPhSrc( in.vPos, 1.0f );
				SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProj );
				out.sAttribs.vColor = in.sAttribs.vColor;
				out.sAttribs.fW = in.sAttribs.fW;
			}
		} sVertexShaderGrid;

		struct SPixelShaderGrid
		{
			BGRA8 Execute( const SVertexPCW::SAttribs& in ) const
			{
				return BGRA8( in.vColor.x, in.vColor.y, in.vColor.z, ( ( ((int)(in.fW*10.0f)) % 5 ) != 2 ) ? in.vColor.w : 0.0f );
			}
		};

		float fSpacing = 50.0f;
		int iHalfGridSize = 16/2;

		SMatrix matScale;
		SMatrix::Identity(matScale);
		SMatrix::Scale( matScale, fSpacing );

		{
			SVector3 vCenter = m_sShipPlayer.m_sShip.m_vPos / fSpacing;
			SVector3 vCenterQ;
			vCenterQ.x = vCenter.x;
			vCenterQ.y = floorf( vCenter.y );
			vCenterQ.z = floorf( vCenter.z );

			SMatrix matWorld( matScale );
			matWorld.m30 = vCenterQ.x*fSpacing;
			matWorld.m31 = vCenterQ.y*fSpacing;
			matWorld.m32 = vCenterQ.z*fSpacing;
			SMatrix::Mul( matWorld, matWorld, m_sCamera.m_matViewProj );
			SMatrix::Mul( matWorld, matWorld, m_sViewportGameView.GetViewPortMatrix() );
			sVertexShaderGrid.matWorldViewProj = matWorld;

			float fi = vCenter.y - floorf(vCenter.y);
			float fj = vCenter.z - floorf(vCenter.z);
			float fk = vCenter.x - floorf(vCenter.x);

			for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
			{
				//for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
				int j = 0;
				{
					SVector3 vOffset( 0.0f, (float)i, (float)j );

					SVertexPCW sVertex0;
					SVertexPCW sVertex1;
					SVertexPCW sVertex2;

					sVertex0.vPos = SVector3( vOffset );
					sVertex0.sAttribs.vColor = vColor;
					sVertex0.sAttribs.fW = fk;

					sVertex1.vPos = sVertex0.vPos;
					sVertex1.sAttribs.vColor = vColor;
					sVertex1.sAttribs.fW = (float)iHalfGridSize + fk;
					sVertex2.vPos = sVertex0.vPos;
					sVertex2.sAttribs.vColor = vColor;
					sVertex2.sAttribs.fW = (float)(iHalfGridSize*2) + fk;

					sVertex0.vPos.x -= (float)iHalfGridSize;
					sVertex2.vPos.x += (float)iHalfGridSize;

					float di = (float)i - fi;
					float dj = (float)j - fj;
					float d = di*di+dj*dj;
					float t = d / (iHalfGridSize*iHalfGridSize);
					t = Clamp( t, 0.0f, 1.0f );

					float fAlpha = 1.0f-t;

					sVertex0.sAttribs.vColor.w *= 0.0f;
					sVertex1.sAttribs.vColor.w *= fAlpha;
					sVertex2.sAttribs.vColor.w *= 0.0f;

					CGraphics::GetInstance().DrawLine3D( sVertex0, sVertex1, m_sViewportGameView, sVertexShaderGrid, SPixelShaderGrid(), SBlendFuncAdditive() );
					CGraphics::GetInstance().DrawLine3D( sVertex1, sVertex2, m_sViewportGameView, sVertexShaderGrid, SPixelShaderGrid(), SBlendFuncAdditive() );
				}
			}
		}

		{
			SVector3 vCenter = m_sShipPlayer.m_sShip.m_vPos / fSpacing;
			SVector3 vCenterQ;
			vCenterQ.x = floorf( vCenter.x );
			vCenterQ.y = vCenter.y;
			vCenterQ.z = floorf( vCenter.z );

			SMatrix matWorld( matScale );
			matWorld.m30 = vCenterQ.x*fSpacing;
			matWorld.m31 = vCenterQ.y*fSpacing;
			matWorld.m32 = vCenterQ.z*fSpacing;
			SMatrix::Mul( matWorld, matWorld, m_sCamera.m_matViewProj );
			SMatrix::Mul( matWorld, matWorld, m_sViewportGameView.GetViewPortMatrix() );
			sVertexShaderGrid.matWorldViewProj = matWorld;

			float fi = vCenter.x - floorf(vCenter.x);
			float fj = vCenter.z - floorf(vCenter.z);
			float fk = vCenter.y - floorf(vCenter.y);

			for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
			{
				//for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
				int j = 0;
				{
					SVector3 vOffset( (float)i, 0.0f, (float)j );

					SVertexPCW sVertex0;
					SVertexPCW sVertex1;
					SVertexPCW sVertex2;

					sVertex0.vPos = SVector3( vOffset );
					sVertex0.sAttribs.vColor = vColor;
					sVertex0.sAttribs.fW = fk;

					sVertex1.vPos = sVertex0.vPos;
					sVertex1.sAttribs.vColor = vColor;
					sVertex1.sAttribs.fW = (float)iHalfGridSize + fk;

					sVertex2.vPos = sVertex0.vPos;
					sVertex2.sAttribs.vColor = vColor;
					sVertex2.sAttribs.fW = (float)(iHalfGridSize*2) + fk;

					sVertex0.vPos.y -= (float)iHalfGridSize;
					sVertex2.vPos.y += (float)iHalfGridSize;

					float di = (float)i - fi;
					float dj = (float)j - fj;
					float d = di*di+dj*dj;
					float t = d / (iHalfGridSize*iHalfGridSize);
					t = Clamp( t, 0.0f, 1.0f );

					float fAlpha = 1.0f-t;

					sVertex0.sAttribs.vColor.w *= 0.0f;
					sVertex1.sAttribs.vColor.w *= fAlpha;
					sVertex2.sAttribs.vColor.w *= 0.0f;

					CGraphics::GetInstance().DrawLine3D( sVertex0, sVertex1, m_sViewportGameView, sVertexShaderGrid, SPixelShaderGrid(), SBlendFuncAdditive() );
					CGraphics::GetInstance().DrawLine3D( sVertex1, sVertex2, m_sViewportGameView, sVertexShaderGrid, SPixelShaderGrid(), SBlendFuncAdditive() );
				}
			}
		}
	}


	{
		int iHUDX = CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD.m_iWidth;
		CGraphics::GetInstance().DrawTexture( SBlendFuncCopy(), m_sTexHUD, iHUDX, 0 );

		CGraphics::GetInstance().DrawText( iHUDX + 7, 7,  "FUEL    83%", BGRA8{ (uint8_t)0, 70, 200, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );
		CGraphics::GetInstance().DrawText( iHUDX + 7, 16, "SHIELD  12%", BGRA8{ (uint8_t)0, 70, 200, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );
		CGraphics::GetInstance().DrawText( iHUDX + 7, 25, "AMMO    174", BGRA8{ (uint8_t)0, 70, 200, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );

		char szText[256];
		float fSpeed = SVector3::Length( m_sShipPlayer.m_sShip.m_vMov ) * 1000.0f;
		sprintf_s( szText, "SPEED %4.1d", (int)fSpeed );
		CGraphics::GetInstance().DrawText( iHUDX + 7, 42, szText, BGRA8{ (uint8_t)100, 100, 100, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );

		CGraphics::GetInstance().DrawText( iHUDX + 14, 82, "RailGun", BGRA8{ (uint8_t)0, 0, 200, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );
	}

	{
		float fMiniMapScale = 2000.0f;

		SMatrix matMiniMap;
		{
			SMatrix matView;
			SMatrix::Identity( matView );
			SMatrix::BuildLHViewMatrix( matView, SVector3( m_sShipPlayer.m_sShip.m_vPos.x, m_sShipPlayer.m_sShip.m_vPos.y, 1.0f ), SVector3( m_sShipPlayer.m_sShip.m_vPos.x, m_sShipPlayer.m_sShip.m_vPos.y, -1.0f ), m_sShipPlayer.m_sShip.m_vDir );
			/*SMatrix matScale;
			SMatrix::Identity( matScale );
			SMatrix::Scale( matScale, fMiniMapScale );
			SMatrix::Mul( matView, matView, matScale );
			matView.m01 *= -1.0f;
			matView.m11 *= -1.0f;
			matView.m21 *= -1.0f;
			matView.m31 *= -1.0f;*/

			SMatrix matProj;
			SMatrix::BuildLHOrthoMatrix( matProj, fMiniMapScale, fMiniMapScale, -1.0f, 1.0f );
		
			SMatrix::Mul( matMiniMap, matView, matProj );
			SMatrix::Mul( matMiniMap, matMiniMap, m_sViewportMiniMap.GetViewPortMatrix() );
		}

		struct SVertexShaderBasic
		{
			using AttribsType = SVertexP::SAttribs;
			SMatrix matWorldViewProjViewPort;
			void Execute( SClipVertex<AttribsType>& out, const SVertexP& in ) const
			{
				SVector4 vPhSrc( in.vPos, 1.0f );
				SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProjViewPort );
			}
		} sVertexShaderBasic;
		sVertexShaderBasic.matWorldViewProjViewPort = matMiniMap;

		struct SPixelShaderBasic
		{
			BGRA8 sColor;
			BGRA8 Execute( const SVertexP::SAttribs& in ) const
			{
				return sColor;
			}
		} sPixelShaderBasic;

		{
			SVertexP sP;
			sP.vPos = m_sShipPlayer.m_sShip.m_vPos;

			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x22, 0xff, 0x22, 0xff );
			CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
		}

		for ( int iShipInd = 0; iShipInd < m_aEnemyShips.size(); iShipInd++ )
		{
			const SShipEnemy& sEnemyShip = m_aEnemyShips[iShipInd];
			SVertexP sP;
			sP.vPos = sEnemyShip.m_sShip.m_vPos;

			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x11, 0x00, 0xff, 0xbb );
			CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
		}

		for ( int iShipInd = -1; iShipInd < (int)m_aEnemyShips.size(); iShipInd++ )
		{
			const STurret& sShip = (iShipInd == -1) ? m_sShipPlayer.m_sTurret : m_aEnemyShips[iShipInd].m_sTurret;

			SVertexP sP;
			
			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x11, 0x99, 0xff, 0x22 );
			for ( int iBulletInd = 0; iBulletInd < sShip.m_aBullets.size(); iBulletInd++ )
			{
				const STurret::SBullet& sBullet = sShip.m_aBullets[iBulletInd];
				sP.vPos = sBullet.m_vPos;

				CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
			}
		}

		for ( int iAsteroidInd = 0; iAsteroidInd < (int)m_aAsteroids.size(); iAsteroidInd++ )
		{
			const SAsteroid& sAsteroid = m_aAsteroids[iAsteroidInd];
			SVertexP sP;
			sP.vPos = sAsteroid.m_vPos;
			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x99, 0x99, 0x99, 0x55 );
			CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
		}

		{
			SVector4 vColor = SVector4( 0.3f, 0.2f, 0.1f, 0.7f );
			struct SVertexShaderGrid
			{
				using AttribsType = SVertexP::SAttribs;
				SMatrix matWorldViewProj;
				void Execute( SClipVertex<AttribsType>& out, const SVertexP& in ) const
				{
					SVector4 vPhSrc( in.vPos, 1.0f );
					SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProj );
				}
			} sVertexShaderGrid;

			struct SPixelShaderGrid
			{
				BGRA8 sColor;
				BGRA8 Execute( const SVertexP::SAttribs& in ) const
				{
					return sColor;
				}
			}sPixelShaderGrid;
			sPixelShaderGrid.sColor = BGRA8( vColor.x, vColor.y, vColor.z, vColor.w );

			float fSpacing = 60.0f*5.0f;
			int iHalfGridSize = 10/2;

			SMatrix matScale;
			SMatrix::Identity(matScale);
			SMatrix::Scale( matScale, fSpacing );

			{
				SVector3 vCenter = m_sShipPlayer.m_sShip.m_vPos / fSpacing;
				SVector3 vCenterQ;
				vCenterQ.x = vCenter.x;
				vCenterQ.y = floorf( vCenter.y );
				vCenterQ.z = floorf( vCenter.z );

				SMatrix matWorld( matScale );
				matWorld.m30 = vCenterQ.x*fSpacing;
				matWorld.m31 = vCenterQ.y*fSpacing;
				matWorld.m32 = vCenterQ.z*fSpacing;
				SMatrix::Mul( matWorld, matWorld, matMiniMap );
				sVertexShaderGrid.matWorldViewProj = matWorld;

				float fi = vCenter.y - floorf(vCenter.y);
				float fj = vCenter.z - floorf(vCenter.z);
				float fk = vCenter.x - floorf(vCenter.x);

				for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
				{
					//for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
					int j = 0;
					{
						SVector3 vOffset( 0.0f, (float)i, (float)j );

						SVertexP sVertex0;
						SVertexP sVertex2;

						sVertex0.vPos = SVector3( vOffset );
						sVertex2.vPos = sVertex0.vPos;
						sVertex0.vPos.x -= (float)iHalfGridSize;
						sVertex2.vPos.x += (float)iHalfGridSize;

						CGraphics::GetInstance().DrawLine3D( sVertex0, sVertex2, m_sViewportMiniMap, sVertexShaderGrid, sPixelShaderGrid, SBlendFuncAdditive() );
					}
				}
			}

			{
				SVector3 vCenter = m_sShipPlayer.m_sShip.m_vPos / fSpacing;
				SVector3 vCenterQ;
				vCenterQ.x = floorf( vCenter.x );
				vCenterQ.y = vCenter.y;
				vCenterQ.z = floorf( vCenter.z );

				SMatrix matWorld( matScale );
				matWorld.m30 = vCenterQ.x*fSpacing;
				matWorld.m31 = vCenterQ.y*fSpacing;
				matWorld.m32 = vCenterQ.z*fSpacing;
				SMatrix::Mul( matWorld, matWorld, matMiniMap );
				sVertexShaderGrid.matWorldViewProj = matWorld;

				float fi = vCenter.x - floorf(vCenter.x);
				float fj = vCenter.z - floorf(vCenter.z);
				float fk = vCenter.y - floorf(vCenter.y);

				for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
				{
					//for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
					int j = 0;
					{
						SVector3 vOffset( (float)i, 0.0f, (float)j );

						SVertexP sVertex0;
						SVertexP sVertex2;

						sVertex0.vPos = SVector3( vOffset );
						sVertex2.vPos = sVertex0.vPos;
						sVertex0.vPos.y -= (float)iHalfGridSize;
						sVertex2.vPos.y += (float)iHalfGridSize;

						CGraphics::GetInstance().DrawLine3D( sVertex0, sVertex2, m_sViewportMiniMap, sVertexShaderGrid, sPixelShaderGrid, SBlendFuncAdditive() );
					}
				}
			}
		}

	}
}

bool CSceneGame::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
	case KEY_UP:
	{
		m_sShipPlayer.m_fAccForward_ctrl = 1.0f;

		fAction += 0.1f;
		fAction = Clamp( fAction, 0.0f, 1.0f );
	}
	return true;
	case KEY_DOWN:
	{
		m_sShipPlayer.m_fAccForward_ctrl = -1.0f;

		fAction -= 0.1f;
		fAction = Clamp( fAction, 0.0f, 1.0f );
	}
	return true;
	case KEY_LEFT:
	{
		m_sShipPlayer.m_fYaw_ctrl = -1.0f;
		//m_sShipPlayer.m_fAccRight_ctrl = -1.0f;

		fClimax += 0.1f;
		fClimax = Clamp( fClimax, 0.0f, 1.0f );
	}
	return true;
	case KEY_RIGHT:
	{
		m_sShipPlayer.m_fYaw_ctrl = 1.0f;
		//m_sShipPlayer.m_fAccRight_ctrl = 1.0f;

		fClimax -= 0.1f;
		fClimax = Clamp( fClimax, 0.0f, 1.0f );
	}
	return true;
	case KEY_SPACE:
	{
		if ( !m_sShipPlayer.m_sTurret.m_bShoot )
		{
			m_sShipPlayer.m_sTurret.m_bShoot = true;
			m_sShipPlayer.m_sTurret.m_iLastBulletTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
		}		
	}
	return true;

	case KEY_ESCAPE:
	{
		CEngine::GetInstance().SetScene( EScene_MainMenu );
	}
	return true;
	}
	return false;
}

bool CSceneGame::On_KeyUp( uint32_t key )
{
	switch ( key )
	{
	case KEY_UP:
	{
		m_sShipPlayer.m_fAccForward_ctrl = 0.0f;
	}
	return true;
	case KEY_DOWN:
	{
		m_sShipPlayer.m_fAccForward_ctrl = 0.0f;
	}
	return true;
	case KEY_LEFT:
	{
		m_sShipPlayer.m_fYaw_ctrl = 0.0f;
		//m_sShipPlayer.m_fAccLeft_ctrl = 0.0f;
	}
	return true;
	case KEY_RIGHT:
	{
		m_sShipPlayer.m_fYaw_ctrl = 0.0f;
		//m_sShipPlayer.m_fAccLeft_ctrl = 0.0f;
	}
	return true;
	case KEY_SPACE:
	{
		m_sShipPlayer.m_sTurret.m_bShoot = false;
	}
	return true;
	}
	return false;
}
