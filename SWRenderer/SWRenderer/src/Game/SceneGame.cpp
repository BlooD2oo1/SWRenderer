#include "SceneGame.h"
#include "Common/PCXLoader.h"
#include "Engine/Engine.h"

CSceneGame::CSceneGame()
{
	Clear();
}

CSceneGame::~CSceneGame()
{
	Clear();
}

void CSceneGame::Clear()
{
	m_sCamera.Clear();
	m_sTexHUD.Clear();
	m_cStarfield.Clear();
	m_cGrid.Clear();

	m_cActors.Clear();
}

void CSceneGame::Create()
{
	Clear();

	PCX_LoadFromFile( "data/hud.pcx", m_sTexHUD );
	
	m_sViewportGameView.Create( SVector2( 0.0f, 0.0f ), SVector2( (float)CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD.m_iWidth, (float)CGraphics::GetInstance().GetFrameBuffer().iHeight ) );
	m_sCamera.m_fAspect = ( m_sViewportGameView.Get11().x - m_sViewportGameView.Get00().x ) / ( m_sViewportGameView.Get11().y - m_sViewportGameView.Get00().y );

	m_sViewportMiniMap.Create( SVector2( (float)( CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD.m_iWidth + 10 ), 134.0f ), SVector2( (float)( CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD.m_iWidth + 10 + 60 ), 134.0f + 60.0f ) );

	m_cStarfield.Create();
	m_cGrid.Create();

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

	m_cActors.Create();

	/*m_aEnemyShips.reserve( 10 );
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
	}*/
}

static float fAction = 0.0f;
static float fClimax = 0.0f;

void CSceneGame::Update()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();
	{
		m_cActors.Update();


	}

	{
		const SShip& sShipPlayer = m_cActors.GetShipPlayer();
		const SBoid& sBoidPlayer = m_cActors.GetBoidPlayer();
		// Update camera:
		float fWFast = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		float fWSlow = CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs );
		SVector3 vP( sBoidPlayer.m_vPos + sShipPlayer.m_vDir * 6.0f + sBoidPlayer.m_vMov*300.0f );
		//vP += m_sShipPlayer.m_vDir * 10.0f;
		m_sCamera.m_vLookAt = vP;
		m_sCamera.m_vEye = vP;
		m_sCamera.m_vEye.z += Lerp( 1200.0f, 100.0f, expf( -SVector3::Length( sBoidPlayer.m_vMov ) * 1.5f ) );
		//m_sCamera.m_vEye.z += 200.0f;
		m_sCamera.m_vLookAtSmooth = Lerp( m_sCamera.m_vLookAt, m_sCamera.m_vLookAtSmooth, fWFast );
		m_sCamera.m_vEyeSmooth = Lerp( m_sCamera.m_vEye, m_sCamera.m_vEyeSmooth, fWSlow );

		//SVector2 vDir2D( m_sShipPlayer.m_sShip.m_vDir.x, m_sShipPlayer.m_sShip.m_vDir.y );
		SVector2 vDir2D( sBoidPlayer.m_vMov.x, sBoidPlayer.m_vMov.y ); SVector2::Normalize( vDir2D, vDir2D );
		//SVector2::Slerp( vDir2D, vDir2D, SVector2( m_sCamera.m_vUp.x, m_sCamera.m_vUp.y ), CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );
		vDir2D = Lerp( vDir2D, SVector2( m_sCamera.m_vUp.x, m_sCamera.m_vUp.y ), CalcSmoothUpdateWeight( 1.0005f, fElapsedTimeMs ) );
		SVector2::Normalize( vDir2D, vDir2D );
		m_sCamera.m_vUp.x = vDir2D.x;
		m_sCamera.m_vUp.y = vDir2D.y;


		
		m_sCamera.UpdateMatrices();
	}	
	
	{
		const SShip& sShipPlayer = m_cActors.GetShipPlayer();
		const SBoid& sBoidPlayer = m_cActors.GetBoidPlayer();

		CEngine::GetInstance().GetAudioFrameData().m_fShipAcc = sShipPlayer.m_fAccForward + sShipPlayer.m_fAccRight;
		CEngine::GetInstance().GetAudioFrameData().m_fShipYawSpeed = sShipPlayer.m_fYawSpeed;
		CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed = SVector3::Length( sBoidPlayer.m_vMov ) / 0.1f;

		//LOG( "ShipAcc=%.8f, ShipYawSpeed=%.8f, ShipSpeed=%.8f\n", CEngine::GetInstance().GetAudioFrameData().m_fShipAcc, CEngine::GetInstance().GetAudioFrameData().m_fShipYawSpeed, CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed );
	
		CEngine::GetInstance().GetAudioFrameData().m_vShipPos = sBoidPlayer.m_vPos;
		CEngine::GetInstance().GetAudioFrameData().m_vCameraEye = m_sCamera.m_vEyeSmooth;
		CEngine::GetInstance().GetAudioFrameData().m_vCameraLookAt = m_sCamera.m_vLookAtSmooth;

		CEngine::GetInstance().GetAudioFrameData().m_fMusic_Action = fAction;
		CEngine::GetInstance().GetAudioFrameData().m_fMusic_Climax = fClimax;
	}
}

void CSceneGame::Render()
{
	////////////////////////////////////////////////////////////////
	// Game View
	////////////////////////////////////////////////////////////////

	m_cStarfield.Render( m_sCamera, m_sViewportGameView );

	const SShip& sShipPlayer = m_cActors.GetShipPlayer();
	const SBoid& sBoidPlayer = m_cActors.GetBoidPlayer();

	m_cGrid.RenderToScene( 50.0f, 16/2, m_sCamera.m_matViewProj, m_sViewportGameView, sBoidPlayer.m_vPos );

	m_cGrid.RenderCoordSys( m_sCamera.m_matViewProj, m_sViewportGameView, SVector3( 0.0f, 0.0f, 0.0f ), 10.0f );

	/*{
		struct SVertexShaderBasic
		{
			using AttribsType = SVertexPW::SAttribs;
			SMatrix matWorldViewProjViewPort;
			void Execute( SClipVertex<AttribsType>& out, const SVertexPW& in ) const
			{
				SVector4 vPhSrc( in.vPos, 1.0f );
				SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProjViewPort );
				out.sAttribs.fW = in.sAttribs.fW;
			}
		} sVertexShaderBasic;
		SMatrix matViewProjViewPort;
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, m_sCamera.m_matViewProj, m_sViewportGameView.GetViewPortMatrix() );
		struct SPixelShaderBasic
		{
			BGRA8 Execute( const SVertexPW::SAttribs& in ) const
			{
				return BGRA8( 0.0f, 0.7f, 0.0f, in.fW );
			}
		};

		SVector2 vField( 0.0f, 0.0f );
	
		for ( int x = -100; x < 100; x++ )
		for ( int y = -100; y < 100; y++ )
		{
			float fX = (float)x * 10.0f;
			float fY = (float)y * 10.0f;

			SVector2 p( fX, fY );
			SVector2 vField( 0.0f, 0.0f );
			GetField( vField, p, SVector2( sShipPlayer.m_sShip.m_vPos.x, m_sShipPlayer.m_sShip.m_vPos.y ), SVector2( m_sShipPlayer.m_sShip.m_vDir.x, m_sShipPlayer.m_sShip.m_vDir.y ) );

			SVertexPW vert0{ SVector3( fX, fY, 0.0f ), 1.0f };
			SVertexPW vert1{ SVector3( fX + vField.x * 10.0f, fY + vField.y * 10.0f, 0.0f ), 0.0f };

			//CGraphics::GetInstance().DrawLine3D( vert0, vert1, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
		}
	}*/

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

	{
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, sShipPlayer.m_matShip, matViewProjViewPort );
		
		sVertexShaderBasic.fAlpha = 0.7f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshShipPlayer().m_pVertices, CEngine::GetInstance().GetMeshShipPlayer().m_pIndices, CEngine::GetInstance().GetMeshShipPlayer().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
	}

	{
		SMatrix matShip;
		SMatrix::BuildEulerXYZ( matShip, 0.5f, 0.5f, 2.1f );
		SMatrix::Scale( matShip, 80.0f );

		matShip.m32 = -800.0f;
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, matShip, matViewProjViewPort );

		sVertexShaderBasic.fAlpha = 0.2f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshShipDestroyer().m_pVertices, CEngine::GetInstance().GetMeshShipDestroyer().m_pIndices, CEngine::GetInstance().GetMeshShipDestroyer().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
	}

	for ( size_t iEnemyShipInd = 0; iEnemyShipInd < m_cActors.GetShipEnemyCount(); iEnemyShipInd++ )
	{
		SShip& sShipEnemy = m_cActors.GetShipEnemy( iEnemyShipInd );
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, sShipEnemy.m_matShip, matViewProjViewPort );
		sVertexShaderBasic.fAlpha = 0.5f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshShipScout().m_pVertices, CEngine::GetInstance().GetMeshShipScout().m_pIndices, CEngine::GetInstance().GetMeshShipScout().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
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
			CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshAsteroid().m_pVertices, CEngine::GetInstance().GetMeshAsteroid().m_pIndices, CEngine::GetInstance().GetMeshAsteroid().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
		}
	}
	
	//bullets
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

		for ( size_t iTurretInd = 0; iTurretInd < m_cActors.GetTurretCount(); iTurretInd++ )
		{
			const STurret& sTurret = m_cActors.GetTurret( iTurretInd );

			for ( int iBulletInd = 0; iBulletInd < sTurret.m_aBullets.size(); iBulletInd++ )
			{
				const STurret::SBullet& sBullet = sTurret.m_aBullets[iBulletInd];
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
						sPixelShaderBasic.sColor = BGRA8( 0.0f, fAlpha*0.6f, 1.0f, 0.8f );

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

	////////////////////////////////////////////////////////////////
	// HUD
	////////////////////////////////////////////////////////////////

	{
		int iHUDX = CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD.m_iWidth;
		CGraphics::GetInstance().DrawTexture( SBlendFuncCopy(), m_sTexHUD, iHUDX, 0 );

		CGraphics::GetInstance().DrawText( iHUDX + 7, 7,  "FUEL    83%", BGRA8{ (uint8_t)0, 70, 200, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );
		CGraphics::GetInstance().DrawText( iHUDX + 7, 16, "SHIELD  12%", BGRA8{ (uint8_t)0, 70, 200, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );
		CGraphics::GetInstance().DrawText( iHUDX + 7, 25, "AMMO    174", BGRA8{ (uint8_t)0, 70, 200, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );

		char szText[256];
		float fSpeed = SVector3::Length( sBoidPlayer.m_vMov ) * 1000.0f;
		sprintf_s( szText, "SPEED %4.1d", (int)fSpeed );
		CGraphics::GetInstance().DrawText( iHUDX + 7, 42, szText, BGRA8{ (uint8_t)100, 100, 100, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );

		CGraphics::GetInstance().DrawText( iHUDX + 14, 82, "RailGun", BGRA8{ (uint8_t)0, 0, 200, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -0 );
	}

	////////////////////////////////////////////////////////////////
	// MiniMap
	////////////////////////////////////////////////////////////////

	{
		float fMiniMapScale = 2000.0f;

		SMatrix matMiniMap;
		{
			SMatrix matView;
			SMatrix::Identity( matView );
			SMatrix::BuildLHViewMatrix( matView, SVector3( sBoidPlayer.m_vPos.x, sBoidPlayer.m_vPos.y, 1.0f ), SVector3( sBoidPlayer.m_vPos.x, sBoidPlayer.m_vPos.y, -1.0f ), sShipPlayer.m_vDir );
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
			sP.vPos = sBoidPlayer.m_vPos;

			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x22, 0xff, 0x22, 0xff );
			CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
		}

		for ( size_t iEnemyShipInd = 0; iEnemyShipInd < m_cActors.GetShipEnemyCount(); iEnemyShipInd++ )
		{
			SShip& sShipEnemy = m_cActors.GetShipEnemy( iEnemyShipInd );
			SBoid& sBoidEnemy = m_cActors.GetBoidEnemy( iEnemyShipInd );
			SVertexP sP;
			sP.vPos = sBoidEnemy.m_vPos;

			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x11, 0x00, 0xff, 0xbb );
			CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
		}

		for ( size_t iTurretInd = 0; iTurretInd < m_cActors.GetTurretCount(); iTurretInd++ )
		{
			const STurret& sTurret = m_cActors.GetTurret( iTurretInd );

			SVertexP sP;
			
			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x11, 0x99, 0xff, 0x22 );
			for ( int iBulletInd = 0; iBulletInd < sTurret.m_aBullets.size(); iBulletInd++ )
			{
				const STurret::SBullet& sBullet = sTurret.m_aBullets[iBulletInd];
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

		m_cGrid.RenderToMiniMap( 50.0f*5.0f, 10/2, matMiniMap, m_sViewportMiniMap, sBoidPlayer.m_vPos );
	}
}

bool CSceneGame::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
	case KEY_UP:
	{
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		sShipPlayer.m_fAccForward_ctrl = 1.0f;

		fAction += 0.1f;
		fAction = Clamp( fAction, 0.0f, 1.0f );
	}
	return true;
	case KEY_DOWN:
	{
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		sShipPlayer.m_fAccForward_ctrl = -1.0f;

		fAction -= 0.1f;
		fAction = Clamp( fAction, 0.0f, 1.0f );
	}
	return true;
	case KEY_LEFT:
	{
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		sShipPlayer.m_fYaw_ctrl = -1.0f;
		//m_sShipPlayer.m_fAccRight_ctrl = -1.0f;

		fClimax += 0.1f;
		fClimax = Clamp( fClimax, 0.0f, 1.0f );
	}
	return true;
	case KEY_RIGHT:
	{
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		sShipPlayer.m_fYaw_ctrl = 1.0f;
		//m_sShipPlayer.m_fAccRight_ctrl = 1.0f;

		fClimax -= 0.1f;
		fClimax = Clamp( fClimax, 0.0f, 1.0f );
	}
	return true;
	case KEY_SPACE:
	{
		STurret& sTurret = m_cActors.GetTurretPlayer();
		if ( !sTurret.m_bShoot )
		{
			sTurret.m_bShoot = true;
			sTurret.m_iLastBulletTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
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
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		sShipPlayer.m_fAccForward_ctrl = 0.0f;
	}
	return true;
	case KEY_DOWN:
	{
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		sShipPlayer.m_fAccForward_ctrl = 0.0f;
	}
	return true;
	case KEY_LEFT:
	{
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		sShipPlayer.m_fYaw_ctrl = 0.0f;
		//m_sShipPlayer.m_fAccLeft_ctrl = 0.0f;
	}
	return true;
	case KEY_RIGHT:
	{
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		sShipPlayer.m_fYaw_ctrl = 0.0f;
		//m_sShipPlayer.m_fAccLeft_ctrl = 0.0f;
	}
	return true;
	case KEY_SPACE:
	{
		STurret& sTurret = m_cActors.GetTurretPlayer();
		sTurret.m_bShoot = false;
	}
	return true;
	}
	return false;
}
