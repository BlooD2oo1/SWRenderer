#include "SceneGame.h"
#include "Common/Globals.h"
#include "Common/Log.h"
#include "Engine/Engine.h"
#include "winAudio.h"

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
	m_sShipControl.Clear();
	m_aEnemyShips.clear();
}

void CSceneGame::Create()
{
	Clear();

	m_sCamera.m_fAspect = (float)CGraphics::GetInstance().GetFrameBuffer().iWidth / (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;

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
		m_pStars[i].vColor = SVector4( ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, ((float)rand()/(float)RAND_MAX)*0.1f+0.7f, ((float)rand()/(float)RAND_MAX)*0.1f+0.6f, a*2.0f );
	}

	for ( int i = 0; i < 100; i++ )
	{
		SAsteroid sAsteroid;
		float fScatterRadius = 600.0f;
		sAsteroid.m_vPos.x = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sAsteroid.m_vPos.y = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sAsteroid.m_vPos.z = 0.0f;

		// uniform distribution of quaternions:
		float u1 = ((float)rand() / (float)RAND_MAX);
		float u2 = ((float)rand() / (float)RAND_MAX);
		float u3 = ((float)rand() / (float)RAND_MAX);
		sAsteroid.m_qRot.w = sqrtf( 1.0f - u1 ) * sinf( 2.0f * PI * u2 );
		sAsteroid.m_qRot.x = sqrtf( 1.0f - u1 ) * cosf( 2.0f * PI * u2 );
		sAsteroid.m_qRot.y = sqrtf( u1 ) * sinf( 2.0f * PI * u3 );
		sAsteroid.m_qRot.z = sqrtf( u1 ) * cosf( 2.0f * PI * u3 );

		m_aAsteroids.push_back( sAsteroid );
	}

	m_aEnemyShips.reserve( 10 );
	for ( int i = 0; i < m_aEnemyShips.capacity(); i++ )
	{
		m_aEnemyShips.emplace_back();
		SShipControl& sEnemyShip = m_aEnemyShips.back();
		float fScatterRadius = 40.0f;
		sEnemyShip.m_vPos.x = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sEnemyShip.m_vPos.y = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sEnemyShip.m_vPos.z = 0.0f;

		sEnemyShip.m_fYaw = ((float)rand() / (float)RAND_MAX) * 2.0f * PI;
	}
}

static float fAction = 0.0f;
static float fClimax = 0.0f;

void CSceneGame::Update()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();
	{
		// Update ship:
		m_sShipControl.UpdateControl();
		m_sShipControl.UpdateMatrices();
		m_sShipControl.UpdateShoot();
	}

	{
		// Update camera:
		float fWFast = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		float fWSlow = CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs );
		SVector3 vP( m_sShipControl.m_vPos + m_sShipControl.m_vDir * 5.0f + m_sShipControl.m_vMov*200.0f );
		//vP += m_sShipControl.m_vDir * 10.0f;
		m_sCamera.m_vLookAt = vP;
		m_sCamera.m_vEye = vP;
		m_sCamera.m_vEye.z += Lerp( 500.0f, 80.0f, expf( -SVector3::Length( m_sShipControl.m_vMov ) * 2.5f ) );
		//m_sCamera.m_vEye.z += 200.0f;
		m_sCamera.m_vLookAtSmooth = Lerp( m_sCamera.m_vLookAt, m_sCamera.m_vLookAtSmooth, fWFast );
		m_sCamera.m_vEyeSmooth = Lerp( m_sCamera.m_vEye, m_sCamera.m_vEyeSmooth, fWSlow );

		//m_sCamera.m_vUp = m_sShipControl.m_vDir;
		SVector2 vDir2D( m_sShipControl.m_vDir.x, m_sShipControl.m_vDir.y );
		//SVector2::Slerp( vDir2D, vDir2D, SVector2( m_sCamera.m_vUp.x, m_sCamera.m_vUp.y ), CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );
		vDir2D = Lerp( vDir2D, SVector2( m_sCamera.m_vUp.x, m_sCamera.m_vUp.y ), CalcSmoothUpdateWeight( 1.0006f, fElapsedTimeMs ) );
		SVector2::Normalize( vDir2D, vDir2D );
		m_sCamera.m_vUp.x = vDir2D.x;
		m_sCamera.m_vUp.y = vDir2D.y;


		
		m_sCamera.UpdateMatrices();
	}
	// log ShipAcc, ShipYaw, ShipSpeed:
	

	CEngine::GetInstance().GetAudioFrameData().m_fShipAcc = m_sShipControl.m_fAccForward + m_sShipControl.m_fAccLeft;
	CEngine::GetInstance().GetAudioFrameData().m_fShipYawSpeed = m_sShipControl.m_fYawSpeed;
	CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed = SVector3::Length( m_sShipControl.m_vMov ) / 0.1f;

	LOG( "ShipAcc=%.8f, ShipYawSpeed=%.8f, ShipSpeed=%.8f\n", CEngine::GetInstance().GetAudioFrameData().m_fShipAcc, CEngine::GetInstance().GetAudioFrameData().m_fShipYawSpeed, CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed );
	
	CEngine::GetInstance().GetAudioFrameData().m_vShipPos = m_sShipControl.m_vPos;
	CEngine::GetInstance().GetAudioFrameData().m_vCameraEye = m_sCamera.m_vEyeSmooth;
	CEngine::GetInstance().GetAudioFrameData().m_vCameraLookAt = m_sCamera.m_vLookAtSmooth;

	CEngine::GetInstance().GetAudioFrameData().m_fMusic_Action = fAction;
	CEngine::GetInstance().GetAudioFrameData().m_fMusic_Climax = fClimax;
}

void CSceneGame::Render()
{
	{
		SVector2 vPlayerPos2D( m_sShipControl.m_vPos.x, m_sShipControl.m_vPos.y );

		for ( size_t i = 0; i < m_aEnemyShips.size(); i++ )
		{
			SShipControl& sEnemyShip = m_aEnemyShips[i];

			SVector2 vPlayerPos2D( m_sShipControl.m_vPos.x, m_sShipControl.m_vPos.y );
			SVector2 vEnemyPos2D( sEnemyShip.m_vPos.x, sEnemyShip.m_vPos.y );
			SVector2 vEnemyDir2D( sEnemyShip.m_vDir.x, sEnemyShip.m_vDir.y );
			SVector2::Normalize( vEnemyDir2D, vEnemyDir2D );
			SVector2 vEnemyToPlayer2D( vPlayerPos2D - vEnemyPos2D );
			SVector2 vEnemyToPlayer2DNorm;
			SVector2::Normalize( vEnemyToPlayer2DNorm, vEnemyToPlayer2D );
			SVector2 vPlayerMovDir2D( m_sShipControl.m_vMov.x, m_sShipControl.m_vMov.y );
			SVector2::Normalize( vPlayerMovDir2D, vPlayerMovDir2D );
			float fPlayerSpeed = SVector2::Length( SVector2( m_sShipControl.m_vMov.x, m_sShipControl.m_vMov.y ) );
			
			float fTimeToReachPlayer = 0.0f;
			{
				float fEnemyToPlayerDistance = SVector2::Length( vEnemyToPlayer2D );
				//float fEnemyToPlayerDistance = fabsf( SVector2::Cross( vEnemyToPlayer2D, vEnemyDir2D ) );
				float fEnemySpeed = SVector2::Length( SVector2( sEnemyShip.m_vMov.x, sEnemyShip.m_vMov.y ) );			
				if ( fEnemySpeed > 0.0f )
				{
					fTimeToReachPlayer = fEnemyToPlayerDistance / fEnemySpeed;
					fTimeToReachPlayer = std::min( fTimeToReachPlayer, 50000.0f );
				}
			}

			float fEstimatedPlayerDistance = fPlayerSpeed * fTimeToReachPlayer;
			SVector2 vEstimatedPlayerPos2D = vPlayerPos2D + vPlayerMovDir2D * fEstimatedPlayerDistance;

			{
				struct SVertexShaderBasic
				{
					SMatrix matWorldViewProj;
					void Process( SVertexP::SVertexh& out, const SVertexP& in ) const
					{
						SVector4 vPhSrc0( in.vPos, 1.0f );
						SMatrix::Mul( out.vPos, vPhSrc0, matWorldViewProj );
					}
				} sVertexShaderBasic;
				sVertexShaderBasic.matWorldViewProj = m_sCamera.m_matViewProj;

				struct SPixelShaderBasic
				{
					BGRA8 Process( const SVertexP::SVertexh& in ) const
					{
						return BGRA8( 0xff00ffff );
					}
				} sPixelShaderBasic;

				SVertexP sLine[2];
				sLine[0].vPos = SVector3( sEnemyShip.m_vPos.x, sEnemyShip.m_vPos.y, 0.0f );
				sLine[1].vPos = SVector3( vEstimatedPlayerPos2D.x, vEstimatedPlayerPos2D.y, 0.0f );
				CGraphics::GetInstance().DrawLine3D<SVertexP, SVertexShaderBasic, SVertexP::SVertexh, SPixelShaderBasic>( sLine[0], sLine[1], sVertexShaderBasic, sPixelShaderBasic );
			}

			SVector2 vEnemyToEstimatedPlayer2D( vEstimatedPlayerPos2D - vEnemyPos2D );
			SVector2 vEnemyToEstimatedPlayer2DNorm;
			SVector2::Normalize( vEnemyToEstimatedPlayer2DNorm, vEnemyToEstimatedPlayer2D );
			float fEnemyToEstimatedPlayerForwardDistance = SVector2::Dot( vEnemyToEstimatedPlayer2D, vEnemyDir2D );

			sEnemyShip.m_fAccForward_ctrl = fEnemyToEstimatedPlayerForwardDistance / 10.0f;
			sEnemyShip.m_fAccForward_ctrl = Clamp( sEnemyShip.m_fAccForward_ctrl, 0.0f, 1.0f ) * 2.0f;

			float fSide = SVector2::Cross( vEnemyDir2D, vEnemyToEstimatedPlayer2D );

			sEnemyShip.m_fYaw_ctrl = fSide * 0.2f;

			//sEnemyShip.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), sEnemyShip.m_vMov, CalcSmoothUpdateWeight( 1.05f, CEngine::GetInstance().GetElapsedTimeMs() ) );

			m_aEnemyShips[i].UpdateControl();
			m_aEnemyShips[i].UpdateMatrices();
			m_aEnemyShips[i].UpdateShoot();
		}
	}

/*
	{
		float fAlpha = 1.0f;
		const int iSteps = 2;
		for ( int j =0; j < iSteps; j++ )
		{
			float fStarBoxSize = powf( (float)(j+1), 3.0f ) * 200.0f;
			float fStarBoxSizeInv = 1.0f / fStarBoxSize;
			for ( uint32_t i = 0; i < m_iStarsCount; i++ )
			{
				SVertexP::SVertexh sPh0;
				SVertexP::SVertexh sPh1;
				{
					SVector4 vPhSrc( m_pStars[i].vPos * fStarBoxSize, 1.0f );
					vPhSrc.x = vPhSrc.x - floorf((vPhSrc.x - m_sCamera.m_vEyeSmooth.x) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					vPhSrc.y = vPhSrc.y - floorf((vPhSrc.y - m_sCamera.m_vEyeSmooth.y) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					vPhSrc.z = vPhSrc.z - floorf((vPhSrc.z - m_sCamera.m_vEyeSmooth.z) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					SMatrix::Mul( sPh0.vPos, vPhSrc, m_sCamera.m_matViewProj );
					SMatrix::Mul( sPh1.vPos, vPhSrc, m_sCamera.m_matViewProjPrev );
				}

				if ( CGraphics::GetInstance().ClipLineDepth<SVertexP::SVertexh>( sPh0, sPh1 ) )
				{
					if ( CGraphics::GetInstance().ClipLineXY<SVertexP::SVertexh>( sPh0, sPh1 ) )
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
						vL.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth * 0.5f;
						vL.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight * 0.5f;
						float fL = SVector2::Length( vL );

						sPh0.vPos.x = sPh0.vPos.x * 0.5f + 0.5f;
						sPh0.vPos.y = -(sPh0.vPos.y) * 0.5f + 0.5f;
						sPh0.vPos.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth;
						sPh0.vPos.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;

						sPh1.vPos.x = sPh1.vPos.x * 0.5f + 0.5f;
						sPh1.vPos.y = -(sPh1.vPos.y) * 0.5f + 0.5f;
						sPh1.vPos.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth;
						sPh1.vPos.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;

						if ( fL > 1.5f )
						{
							CGraphics::GetInstance().DrawLine( SVector2( sPh0.vPos.x, sPh0.vPos.y ), SVector2( sPh1.vPos.x, sPh1.vPos.y ), BGRA8( m_pStars[i].vColor.x*fAlpha, m_pStars[i].vColor.y, m_pStars[i].vColor.z*fAlpha, m_pStars[i].vColor.w/(fL*0.2f+1.0f) ) );
						}
						else
						{
							CGraphics::GetInstance().DrawPixel( (int)sPh0.vPos.x, (int)sPh0.vPos.y, BGRA8(m_pStars[i].vColor.x*fAlpha, m_pStars[i].vColor.y, m_pStars[i].vColor.z*fAlpha, m_pStars[i].vColor.w ) );
						}
					}
				}
			}
		}
	}*/

	struct SVertexShaderBasic
	{
		SMatrix matWorldViewProj;
		float fAlpha;
		void Process( SVertexPC::SVertexh& out, const SVertexPC& in ) const
		{
			SVector4 vPhSrc0( in.vPos, 1.0f );
			SMatrix::Mul( out.vPos, vPhSrc0, matWorldViewProj );
			out.vColor = in.vColor;
			out.vColor.w *= fAlpha;
		}
	} sVertexShaderBasic;

	struct SPixelShaderBasic
	{
		BGRA8 Process( const SVertexPC::SVertexh& in ) const
		{
			return BGRA8( in.vColor.x, in.vColor.y, in.vColor.z, in.vColor.w );
		}
	} sPixelShaderBasic;

	{
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProj, m_sShipControl.m_matShip, m_sCamera.m_matViewProj );
		sVertexShaderBasic.fAlpha = 0.7f;
		CGraphics::GetInstance().DrawLineList3D<SVertexPC, SVertexShaderBasic, SVertexPC::SVertexh, SPixelShaderBasic>( CEngine::GetInstance().GetShipMesh().GetLineList(), CEngine::GetInstance().GetShipMesh().GetLineListCount(), sVertexShaderBasic, sPixelShaderBasic );
	}

	for ( size_t iEnemyShipInd = 0; iEnemyShipInd < m_aEnemyShips.size(); iEnemyShipInd++ )
	{
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProj, m_aEnemyShips[iEnemyShipInd].m_matShip, m_sCamera.m_matViewProj );
		sVertexShaderBasic.fAlpha = 0.7f;
		CGraphics::GetInstance().DrawLineList3D<SVertexPC, SVertexShaderBasic, SVertexPC::SVertexh, SPixelShaderBasic>( CEngine::GetInstance().GetEnemyShipMesh().GetLineList(), CEngine::GetInstance().GetEnemyShipMesh().GetLineListCount(), sVertexShaderBasic, sPixelShaderBasic );
	}

	for ( size_t iAsteroidInd = 0; iAsteroidInd < m_aAsteroids.size(); iAsteroidInd++ )
	{
		SMatrix matAsteroid;
		SMatrix::Identity( matAsteroid );
		SQuaternion::ToMatrix( matAsteroid, m_aAsteroids[iAsteroidInd].m_qRot );
		matAsteroid.m30 = m_aAsteroids[iAsteroidInd].m_vPos.x;
		matAsteroid.m31 = m_aAsteroids[iAsteroidInd].m_vPos.y;
		matAsteroid.m32 = m_aAsteroids[iAsteroidInd].m_vPos.z;
		SMatrix::Scale( matAsteroid, 3.0f );
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProj, matAsteroid, m_sCamera.m_matViewProj );
		sVertexShaderBasic.fAlpha = 0.7f;
		CGraphics::GetInstance().DrawLineList3D<SVertexPC, SVertexShaderBasic, SVertexPC::SVertexh, SPixelShaderBasic>( CEngine::GetInstance().GetAsteroidMesh().GetLineList(), CEngine::GetInstance().GetAsteroidMesh().GetLineListCount(), sVertexShaderBasic, sPixelShaderBasic );
	}
	/*
	for ( int iBulletInd = 0; iBulletInd < m_sShipControl.m_aBullets.size(); iBulletInd++ )
	{
		const SShipControl::SBullet& sBullet = m_sShipControl.m_aBullets[iBulletInd];
		SVertexP::SVertexh sPh0;
		SVertexP::SVertexh sPh1;
		{
			SVector4 vPhSrc0( sBullet.m_vPos, 1.0f );
			SVector4 vPhSrc1( sBullet.m_vPosPrev, 1.0f );
			SMatrix::Mul( sPh0.vPos, vPhSrc0, m_sCamera.m_matViewProj );
			SMatrix::Mul( sPh1.vPos, vPhSrc1, m_sCamera.m_matViewProjPrev );
		}

		if ( CGraphics::GetInstance().ClipLineDepth<SVertexP::SVertexh>( sPh0, sPh1 ) )
		{
			if ( CGraphics::GetInstance().ClipLineXY<SVertexP::SVertexh>( sPh0, sPh1 ) )
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
				vL.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth * 0.5f;
				vL.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight * 0.5f;
				float fL = SVector2::Length( vL );

				sPh0.vPos.x = sPh0.vPos.x * 0.5f + 0.5f;
				sPh0.vPos.y = -(sPh0.vPos.y) * 0.5f + 0.5f;
				sPh0.vPos.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth;
				sPh0.vPos.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;

				sPh1.vPos.x = sPh1.vPos.x * 0.5f + 0.5f;
				sPh1.vPos.y = -(sPh1.vPos.y) * 0.5f + 0.5f;
				sPh1.vPos.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth;
				sPh1.vPos.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;

				float fAlpha = 1.0f - sBullet.m_fTimer / sBullet.m_fTime;
				if ( fL > 1.5f )
				{
					CGraphics::GetInstance().DrawLine( SVector2( sPh0.vPos.x, sPh0.vPos.y ), SVector2( sPh1.vPos.x, sPh1.vPos.y ), BGRA8( 0.0f, fAlpha, 1.0f, fAlpha ) );
				}
				else
				{
					CGraphics::GetInstance().DrawPixel( (int)sPh0.vPos.x, (int)sPh0.vPos.y, BGRA8( 0.0f, fAlpha, 1.0f, fAlpha ) );
				}
			}
		}
	}


	SVector4 vColor = SVector4( 0.3f, 0.2f, 0.1f, 0.5f );
	if ( vColor.w > 1.0f/255.0f )
	{
		struct SVertexShaderGrid
		{
			SMatrix matWorldViewProj;
			void Process( SVertexPC::SVertexh& out, const SVertexPC& in ) const
			{
				SVector4 vPhSrc0( in.vPos, 1.0f );
				SMatrix::Mul( out.vPos, vPhSrc0, matWorldViewProj );
				out.vColor = in.vColor;
			}
		} sVertexShaderGrid;

		float fSpacing = 60.0f;
		int iHalfGridSize = 10/2;

		SMatrix matScale;
		SMatrix::Identity(matScale);
		SMatrix::Scale( matScale, fSpacing );

		{
			SVector3 vCenter = m_sShipControl.m_vPos / fSpacing;
			SVector3 vCenterQ;
			vCenterQ.x = vCenter.x;
			vCenterQ.y = floorf( vCenter.y );
			vCenterQ.z = floorf( vCenter.z );

			SMatrix matWorld( matScale );
			matWorld.m30 = vCenterQ.x*fSpacing;
			matWorld.m31 = vCenterQ.y*fSpacing;
			matWorld.m32 = vCenterQ.z*fSpacing;
			SMatrix::Mul( matWorld, matWorld, m_sCamera.m_matViewProj );
			sVertexShaderGrid.matWorldViewProj = matWorld;

			float fi = vCenter.y - floorf(vCenter.y);
			float fj = vCenter.z - floorf(vCenter.z);

			for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
			{
				//for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
				int j = 0;
				{
					SVector3 vOffset( 0.0f, (float)i, (float)j );

					SVertexPC sVertex0;
					SVertexPC sVertex1;
					SVertexPC sVertex2;

					sVertex0.vPos = SVector3( vOffset );
					sVertex0.vColor = vColor;

					sVertex1.vPos = sVertex0.vPos;
					sVertex1.vColor = vColor;

					sVertex2.vPos = sVertex0.vPos;
					sVertex2.vColor = vColor;

					sVertex0.vPos.x -= (float)iHalfGridSize;
					sVertex2.vPos.x += (float)iHalfGridSize;

					float di = (float)i - fi;
					float dj = (float)j - fj;
					float d = di*di+dj*dj;
					float t = d / (iHalfGridSize*iHalfGridSize);
					t = Clamp( t, 0.0f, 1.0f );

					float fAlpha = 1.0f-t;

					sVertex0.vColor.w *= 0.0f;
					sVertex1.vColor.w *= fAlpha;
					sVertex2.vColor.w *= 0.0f;

					CGraphics::GetInstance().DrawLine3D( sVertex0, sVertex1, sVertexShaderGrid );
					CGraphics::GetInstance().DrawLine3D( sVertex1, sVertex2, sVertexShaderGrid );
				}
			}
		}

		{
			SVector3 vCenter = m_sShipControl.m_vPos / fSpacing;
			SVector3 vCenterQ;
			vCenterQ.x = floorf( vCenter.x );
			vCenterQ.y = vCenter.y;
			vCenterQ.z = floorf( vCenter.z );

			SMatrix matWorld( matScale );
			matWorld.m30 = vCenterQ.x*fSpacing;
			matWorld.m31 = vCenterQ.y*fSpacing;
			matWorld.m32 = vCenterQ.z*fSpacing;
			SMatrix::Mul( matWorld, matWorld, m_sCamera.m_matViewProj );
			sVertexShaderGrid.matWorldViewProj = matWorld;

			float fi = vCenter.x - floorf(vCenter.x);
			float fj = vCenter.z - floorf(vCenter.z);

			for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
			{
				//for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
				int j = 0;
				{
					SVector3 vOffset( (float)i, 0.0f, (float)j );

					SVertexPC sVertex0;
					SVertexPC sVertex1;
					SVertexPC sVertex2;

					sVertex0.vPos = SVector3( vOffset );
					sVertex0.vColor = vColor;

					sVertex1.vPos = sVertex0.vPos;
					sVertex1.vColor = vColor;

					sVertex2.vPos = sVertex0.vPos;
					sVertex2.vColor = vColor;

					sVertex0.vPos.y -= (float)iHalfGridSize;
					sVertex2.vPos.y += (float)iHalfGridSize;

					float di = (float)i - fi;
					float dj = (float)j - fj;
					float d = di*di+dj*dj;
					float t = d / (iHalfGridSize*iHalfGridSize);
					t = Clamp( t, 0.0f, 1.0f );

					float fAlpha = 1.0f-t;

					sVertex0.vColor.w *= 0.0f;
					sVertex1.vColor.w *= fAlpha;
					sVertex2.vColor.w *= 0.0f;

					CGraphics::GetInstance().DrawLine3D( sVertex0, sVertex1, sVertexShaderGrid );
					CGraphics::GetInstance().DrawLine3D( sVertex1, sVertex2, sVertexShaderGrid );
				}
			}
		}
	}*/
}

bool CSceneGame::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
	case KEY_UP:
	{
		m_sShipControl.m_fAccForward_ctrl = 1.0f;

		fAction += 0.1f;
		fAction = Clamp( fAction, 0.0f, 1.0f );
	}
	return true;
	case KEY_DOWN:
	{
		m_sShipControl.m_fAccForward_ctrl = -1.0f;

		fAction -= 0.1f;
		fAction = Clamp( fAction, 0.0f, 1.0f );
	}
	return true;
	case KEY_LEFT:
	{
		m_sShipControl.m_fYaw_ctrl = 1.0f;
		//m_sShipControl.m_fAccLeft_ctrl = 1.0f;

		fClimax += 0.1f;
		fClimax = Clamp( fClimax, 0.0f, 1.0f );
	}
	return true;
	case KEY_RIGHT:
	{
		m_sShipControl.m_fYaw_ctrl = -1.0f;
		//m_sShipControl.m_fAccLeft_ctrl = -1.0f;

		fClimax -= 0.1f;
		fClimax = Clamp( fClimax, 0.0f, 1.0f );
	}
	return true;
	case KEY_SPACE:
	{
		if ( !m_sShipControl.m_bShoot )
		{
			m_sShipControl.m_bShoot = true;
			m_sShipControl.m_iLastBulletTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
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
		m_sShipControl.m_fAccForward_ctrl = 0.0f;
	}
	return true;
	case KEY_DOWN:
	{
		m_sShipControl.m_fAccForward_ctrl = 0.0f;
	}
	return true;
	case KEY_LEFT:
	{
		m_sShipControl.m_fYaw_ctrl = 0.0f;
		//m_sShipControl.m_fAccLeft_ctrl = 0.0f;
	}
	return true;
	case KEY_RIGHT:
	{
		m_sShipControl.m_fYaw_ctrl = 0.0f;
		//m_sShipControl.m_fAccLeft_ctrl = 0.0f;
	}
	return true;
	case KEY_SPACE:
	{
		m_sShipControl.m_bShoot = false;
	}
	return true;
	}
	return false;
}
