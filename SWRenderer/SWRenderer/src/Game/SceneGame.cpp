#include "SceneGame.h"
#include "Common/PCXLoader.h"
#include "Engine/Engine.h"

CSceneGame::CSceneGame()
	: m_cActors( *this )
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
	m_sTexHUD_Top.Clear();
	m_sTexHUD_Left.Clear();
	m_sTexHUD_Bottom.Clear();
	m_sTexHUD_Right.Clear();
	m_sTexHUD_Tick.Clear();
	m_cStarfield.Clear();
	m_cGrid.Clear();
	m_cActors.Clear();
	m_cEffects.Clear();
}

void CSceneGame::Create()
{
	Clear();

	PCX_LoadFromFile( "data/hud_top.pcx", m_sTexHUD_Top );
	PCX_LoadFromFile( "data/hud_left.pcx", m_sTexHUD_Left );
	PCX_LoadFromFile( "data/hud_bottom.pcx", m_sTexHUD_Bottom );
	PCX_LoadFromFile( "data/hud_right.pcx", m_sTexHUD_Right );
	PCX_LoadFromFile( "data/hud_tick.pcx", m_sTexHUD_Tick );
	
	m_sViewportGameView.Create( SVector2( 1.0f, 1.0f ), SVector2( (float)CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD_Right.m_iWidth+2, (float)CGraphics::GetInstance().GetFrameBuffer().iHeight-1 ) );
	m_sCamera.m_fAspect = ( m_sViewportGameView.Get11().x - m_sViewportGameView.Get00().x ) / ( m_sViewportGameView.Get11().y - m_sViewportGameView.Get00().y );

	m_sViewportMiniMap.Create( SVector2( (float)( CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD_Right.m_iWidth + 9 ), 135.0f ), SVector2( (float)( CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD_Right.m_iWidth + 9 + 60 ), 135.0f + 60.0f ) );

	m_cStarfield.Create();
	m_cGrid.Create();
	m_cActors.Create();
	m_cEffects.Create();
}

static float fAction = 0.0f;
static float fClimax = 0.0f;

void CSceneGame::Update()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	m_cActors.Update();
	
	m_cEffects.Update();

	{
		const SShip& sShipPlayer = m_cActors.GetShipPlayer();
		// Update camera:
		float fWFast = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		float fWSlow = CalcSmoothUpdateWeight( 1.0005f, fElapsedTimeMs );
		SVector3 vP( sShipPlayer.m_vPos + sShipPlayer.m_vDir * 5.0f + sShipPlayer.m_vMov*200.0f );
		//vP += m_sShipPlayer.m_vDir * 10.0f;
		m_sCamera.m_vLookAt = vP;
		m_sCamera.m_vEye = vP;
		m_sCamera.m_vEye.z += Lerp( 1400.0f, 150.0f, expf( -SVector3::Length( sShipPlayer.m_vMov ) * 1.5f ) );
		//m_sCamera.m_vEye.z += 200.0f;
		m_sCamera.m_vLookAtSmooth = Lerp( m_sCamera.m_vLookAt, m_sCamera.m_vLookAtSmooth, fWFast );
		m_sCamera.m_vEyeSmooth = Lerp( m_sCamera.m_vEye, m_sCamera.m_vEyeSmooth, fWSlow );

		//SVector2 vDir2D( m_sShipPlayer.m_sShip.m_vDir.x, m_sShipPlayer.m_sShip.m_vDir.y );
		SVector2 vDir2D( sShipPlayer.m_vMov.x, sShipPlayer.m_vMov.y ); SVector2::Normalize( vDir2D, vDir2D );
		//SVector2::Slerp( vDir2D, vDir2D, SVector2( m_sCamera.m_vUp.x, m_sCamera.m_vUp.y ), CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );
		vDir2D = Lerp( vDir2D, SVector2( m_sCamera.m_vUp.x, m_sCamera.m_vUp.y ), CalcSmoothUpdateWeight( 1.0005f, fElapsedTimeMs ) );
		SVector2::Normalize( vDir2D, vDir2D );
		m_sCamera.m_vUp.x = vDir2D.x;
		m_sCamera.m_vUp.y = vDir2D.y;
		
		m_sCamera.UpdateMatrices();
	}	
	
	{
		const SShip& sShipPlayer = m_cActors.GetShipPlayer();

		CEngine::GetInstance().GetAudioFrameData().m_fShipAcc = sShipPlayer.m_fAccForward + sShipPlayer.m_fAccRight;
		CEngine::GetInstance().GetAudioFrameData().m_fShipYawSpeed = sShipPlayer.m_fYawSpeed;
		CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed = SVector3::Length( sShipPlayer.m_vMov ) / 0.1f;

		//LOG( "ShipAcc=%.8f, ShipYawSpeed=%.8f, ShipSpeed=%.8f\n", CEngine::GetInstance().GetAudioFrameData().m_fShipAcc, CEngine::GetInstance().GetAudioFrameData().m_fShipYawSpeed, CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed );
	
		CEngine::GetInstance().GetAudioFrameData().m_vShipPos = sShipPlayer.m_vPos;
		CEngine::GetInstance().GetAudioFrameData().m_vCameraEye = m_sCamera.m_vEyeSmooth;
		CEngine::GetInstance().GetAudioFrameData().m_vCameraLookAt = m_sCamera.m_vLookAtSmooth;

		CEngine::GetInstance().GetAudioFrameData().m_fMusic_Action = fAction;
		CEngine::GetInstance().GetAudioFrameData().m_fMusic_Climax = fClimax;
	}
}

void CSceneGame::Render()
{
	////////////////////////////////////////////////////////////////
	// HUD
	////////////////////////////////////////////////////////////////

	{
		CGraphics::GetInstance().DrawTexture( SBlendFuncCopy(), m_sTexHUD_Left, 0, 0 );
		CGraphics::GetInstance().DrawTexture( SBlendFuncCopy(), m_sTexHUD_Top, 3, 0 );
		CGraphics::GetInstance().DrawTexture( SBlendFuncCopy(), m_sTexHUD_Bottom, 3, 200-3 );

		int iHUDX = CGraphics::GetInstance().GetFrameBuffer().iWidth - m_sTexHUD_Right.m_iWidth;
		CGraphics::GetInstance().DrawTexture( SBlendFuncCopy(), m_sTexHUD_Right, iHUDX, 0 );

		char szText[256];
		//sprintf_s( szText, "HP     %4.1d", (int)m_cActors.GetShipPlayer().m_fHP );
		//CGraphics::GetInstance().DrawText( iHUDX + 7, 8,  szText, BGRA8{ (uint8_t)100, 70, 40, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Tiny_6x5(), 6, 5, -0 );
		CGraphics::GetInstance().DrawText( iHUDX + 5, 8,  "HP", BGRA8{ (uint8_t)40, 00, 100, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Tiny_6x5(), 6, 5, -2 );

		struct SBlendFuncAdditiveWithColorMultiplier
		{
			BGRA8 sColorMultiplier;
			inline void Execute(BGRA8& dest, BGRA8 src) const
			{
				uint32_t rOut = dest.r + ((src.r * src.a) >> 8);
				uint32_t gOut = dest.g + ((src.g * src.a) >> 8);
				uint32_t bOut = dest.b + ((src.b * src.a) >> 8);
				rOut = (uint32_t)(rOut * sColorMultiplier.r / 255);
				gOut = (uint32_t)(gOut * sColorMultiplier.g / 255);
				bOut = (uint32_t)(bOut * sColorMultiplier.b / 255);
				dest.r = (uint8_t)(rOut > 255 ? 255 : rOut);
				dest.g = (uint8_t)(gOut > 255 ? 255 : gOut);
				dest.b = (uint8_t)(bOut > 255 ? 255 : bOut);
			}
		} sBlendFuncAdditiveWithColorMultiplier;
		for ( int i = 0; i < 20; i++ )
		{
			sBlendFuncAdditiveWithColorMultiplier.sColorMultiplier = (i < (int)(m_cActors.GetShipPlayer().m_fHP*0.2f)) ? BGRA8{ (uint8_t)60, 0, 180, 255 } : BGRA8{ (uint8_t)20, 0, 30, 255 };
			CGraphics::GetInstance().DrawTexture( sBlendFuncAdditiveWithColorMultiplier, m_sTexHUD_Tick, iHUDX + 14 + i*3, 7 );
		}

		CGraphics::GetInstance().DrawText( iHUDX + 7, 14, "SHIELD  12%", BGRA8{ (uint8_t)100, 70, 40, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Tiny_6x5(), 6, 5, -0 );
		CGraphics::GetInstance().DrawText( iHUDX + 7, 20, "AMMO    174", BGRA8{ (uint8_t)100, 70, 40, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Tiny_6x5(), 6, 5, -0 );
		CGraphics::GetInstance().DrawText( iHUDX + 7, 26, "ROCKET  0", BGRA8{ (uint8_t)100, 70, 40, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Tiny_6x5(), 6, 5, -0 );

		
		float fSpeed = SVector3::Length( m_cActors.GetShipPlayer().m_vMov ) * 1000.0f;
		sprintf_s( szText, "SPEED %4.1d", (int)fSpeed );
		CGraphics::GetInstance().DrawText( iHUDX + 6, 42, szText, BGRA8{ (uint8_t)100, 70, 40, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Tiny_6x5(), 6, 5, -0 );

		CGraphics::GetInstance().DrawText( iHUDX + 14, 82, "RailGun", BGRA8{ (uint8_t)100, 70, 140, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Tiny_6x5(), 6, 5, -0 );
	}

	////////////////////////////////////////////////////////////////
	// Game View
	////////////////////////////////////////////////////////////////

	m_cStarfield.Render( m_sCamera, m_sViewportGameView );

	m_cGrid.RenderToScene( 100.0f, 6/2, m_sCamera.m_matViewProj, m_sViewportGameView, m_cActors.GetShipPlayer().m_vPos );

	m_cEffects.Render( m_sCamera, m_sViewportGameView );

	//m_cGrid.RenderCoordSys( m_sCamera.m_matViewProj, m_sViewportGameView, SVector3( 0.0f, 0.0f, 0.0f ), 10.0f );

//#define DRAW_FIELD
#ifdef DRAW_FIELD
	{
		struct SVertexShaderBasic
		{
			using AttribsType = SVertexPW::SAttribs;
			SMatrix matWorldViewProjViewPort;
			inline void Execute( SClipVertex<AttribsType>& out, const SVertexPW& in ) const
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
			inline BGRA8 Execute( const SVertexPW::SAttribs& in ) const
			{
				return BGRA8( 0.0f, 0.7f, 0.0f, in.fW );
			}
		};

		SVector2 vField( 0.0f, 0.0f );
	
		const float fSize = 5.0f;
		const int iGridSize = 50;
		for ( int x = -iGridSize; x < iGridSize; x++ )
		for ( int y = -iGridSize; y < iGridSize; y++ )
		{
			float fX = (float)x * fSize;
			float fY = (float)y * fSize;
			
			fX += (int)(m_cActors.GetShipPlayer().m_vPos.x/fSize)*fSize;
			fY += (int)(m_cActors.GetShipPlayer().m_vPos.y/fSize)*fSize;

			SVector2 vField( 0.0f, 0.0f );
			SVector2 p( fX, fY );
			m_cActors.GetField_ShipPlayer( vField, p, m_cActors.GetShipPlayer() );
			//m_cActors.GetField_Asteroid( vField, p,m_cActors.GetShipPlayer(), m_cActors.GetAsteroid(0) );

			SVertexPW vert0{ SVector3( fX, fY, 0.0f ), 1.0f };
			SVertexPW vert1{ SVector3( fX + vField.x * fSize*0.8f, fY + vField.y * fSize*0.8f, 0.0f ), 0.0f };

			CGraphics::GetInstance().DrawLine3D( vert0, vert1, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
		}
	}
#endif
	SMatrix matViewProjViewPort;
	SMatrix::Mul( matViewProjViewPort, m_sCamera.m_matViewProj, m_sViewportGameView.GetViewPortMatrix() );

	struct SVertexShaderBasic
	{
		using AttribsType = SVertexPC::SAttribs;
		SMatrix matWorldViewProjViewPort;
		SVector3 vColor0;
		SVector3 vColor1;
		SVector3 vColor2;
		float fAlpha;
		inline void Execute( SClipVertex<AttribsType>& out, const SVertexPC& in ) const
		{
			SVector4 vPhSrc( in.vPos, 1.0f );
			SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProjViewPort );
			out.sAttribs.vColor = SVector4( 0.0f, 0.0f, 0.0f, fAlpha );
			*(SVector3*)&out.sAttribs.vColor += *(SVector3*)&vColor0 * in.sAttribs.vColor.x;
			*(SVector3*)&out.sAttribs.vColor += *(SVector3*)&vColor1 * in.sAttribs.vColor.y;
			*(SVector3*)&out.sAttribs.vColor += *(SVector3*)&vColor2 * in.sAttribs.vColor.z;
		}
	} sVertexShaderBasic;	
	
	struct SPixelShaderBasic
	{
		inline BGRA8 Execute( const SVertexPC::SAttribs& in ) const
		{
			return BGRA8( in.vColor.x, in.vColor.y, in.vColor.z, in.vColor.w );
		}
	};

	// player ship
	{
		const SShip& sShipPlayer = m_cActors.GetShipPlayer();
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, sShipPlayer.m_matShip, matViewProjViewPort );
		
		sVertexShaderBasic.vColor0 = SVector3( 1.0f, 0.7f, 0.6f ) * 0.33f;
		sVertexShaderBasic.vColor1 = SVector3( 1.0f, 0.6f, 1.0f ) * 0.33f;
		sVertexShaderBasic.vColor2 = SVector3( 1.0f, 0.4f, 0.4f ) * 0.33f;
		sVertexShaderBasic.fAlpha = 0.8f;
		if ( sShipPlayer.m_sTurret.m_bShoot )
		{
			sVertexShaderBasic.vColor2 = SVector3( 0.6f, 0.5f, 1.0f ) * 0.33f;
		}
		if ( sShipPlayer.m_fDamageTimerMs > 0.0f )
		{
			sVertexShaderBasic.vColor0.x *= 0.2f;
			sVertexShaderBasic.vColor0.y *= 0.2f;
			sVertexShaderBasic.vColor0.z = 1.0f;
			sVertexShaderBasic.vColor1.x *= 0.2f;
			sVertexShaderBasic.vColor1.y *= 0.2f;
			sVertexShaderBasic.vColor1.z = 1.0f;
			sVertexShaderBasic.vColor2.x *= 0.2f;
			sVertexShaderBasic.vColor2.y *= 0.2f;
			sVertexShaderBasic.vColor2.z = 1.0f;
			sVertexShaderBasic.fAlpha = 0.3f;
		}

		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshShipPlayer().m_pVertices, CEngine::GetInstance().GetMeshShipPlayer().m_iVertexCount, CEngine::GetInstance().GetMeshShipPlayer().m_pIndices, CEngine::GetInstance().GetMeshShipPlayer().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
	}

	// destroyed ship
	{
		SMatrix matShip;
		SMatrix::BuildEulerXYZ( matShip, 0.5f, 0.5f, 2.1f );
		SMatrix::Scale( matShip, 80.0f );
		SMatrix::Translate( matShip, SVector3( 0.0f, 0.0f, -800.0f ) );
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, matShip, matViewProjViewPort );
		sVertexShaderBasic.vColor0 = SVector3( 1.0f, 0.7f, 0.6f ) * 0.33f;
		sVertexShaderBasic.vColor1 = SVector3( 1.0f, 0.6f, 0.5f ) * 0.33f;
		sVertexShaderBasic.vColor2 = SVector3( 1.0f, 0.4f, 0.4f ) * 0.33f;
		sVertexShaderBasic.fAlpha = 0.3f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshShipDestroyer().m_pVertices, CEngine::GetInstance().GetMeshShipDestroyer().m_iVertexCount, CEngine::GetInstance().GetMeshShipDestroyer().m_pIndices, CEngine::GetInstance().GetMeshShipDestroyer().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );

		SMatrix::BuildEulerXYZ( matShip, 0.1f, 3.5f, 2.1f );
		SMatrix::Scale( matShip, 110.0f );
		SMatrix::Translate( matShip, SVector3( 600.0f, 800.0f, -800.0f ) );
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, matShip, matViewProjViewPort );
		sVertexShaderBasic.vColor0 = SVector3( 1.0f, 0.7f, 0.6f ) * 0.33f*0.0f;
		sVertexShaderBasic.vColor1 = SVector3( 1.0f, 0.6f, 0.5f ) * 0.33f;
		sVertexShaderBasic.vColor2 = SVector3( 1.0f, 0.4f, 0.4f ) * 0.33f;
		sVertexShaderBasic.fAlpha = 0.3f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshAsteroidBig().m_pVertices, CEngine::GetInstance().GetMeshAsteroidBig().m_iVertexCount, CEngine::GetInstance().GetMeshAsteroidBig().m_pIndices, CEngine::GetInstance().GetMeshAsteroidBig().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );

		SMatrix::BuildEulerXYZ( matShip, 0.1f, 1.5f, 2.1f );
		SMatrix::Scale( matShip, 130.0f );
		SMatrix::Translate( matShip, SVector3( 900.0f, 1000.0f, -600.0f ) );
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, matShip, matViewProjViewPort );
		sVertexShaderBasic.vColor0 = SVector3( 1.0f, 0.7f, 0.6f ) * 0.33f*0.0f;
		sVertexShaderBasic.vColor1 = SVector3( 1.0f, 0.6f, 0.5f ) * 0.33f;
		sVertexShaderBasic.vColor2 = SVector3( 1.0f, 0.4f, 0.4f ) * 0.33f;
		sVertexShaderBasic.fAlpha = 0.3f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshAsteroidBig().m_pVertices, CEngine::GetInstance().GetMeshAsteroidBig().m_iVertexCount, CEngine::GetInstance().GetMeshAsteroidBig().m_pIndices, CEngine::GetInstance().GetMeshAsteroidBig().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );

		SMatrix::BuildEulerXYZ( matShip, 2.1f, 0.5f, 1.1f );
		SMatrix::Scale( matShip, 130.0f );
		SMatrix::Translate( matShip, SVector3( 1000.0f, 700.0f, -400.0f ) );
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, matShip, matViewProjViewPort );
		sVertexShaderBasic.vColor0 = SVector3( 1.0f, 0.7f, 0.6f ) * 0.33f*0.0f;
		sVertexShaderBasic.vColor1 = SVector3( 1.0f, 0.6f, 0.5f ) * 0.33f;
		sVertexShaderBasic.vColor2 = SVector3( 1.0f, 0.4f, 0.4f ) * 0.33f;
		sVertexShaderBasic.fAlpha = 0.3f;
		CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshAsteroidBig().m_pVertices, CEngine::GetInstance().GetMeshAsteroidBig().m_iVertexCount, CEngine::GetInstance().GetMeshAsteroidBig().m_pIndices, CEngine::GetInstance().GetMeshAsteroidBig().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
	}

	// enemy ships
	for ( size_t iShipInd = 0; iShipInd < m_cActors.GetShipCount(); iShipInd++ )
	{
		if ( iShipInd == m_cActors.GetShipPlayerInd() )
		{
			continue;
		}

		SShip& sShipEnemy = m_cActors.GetShip( iShipInd );

		if ( m_sCamera.FrustumSphereTest( sShipEnemy.m_vPos, 2.0f ) )
		{		
			SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, sShipEnemy.m_matShip, matViewProjViewPort );

			sVertexShaderBasic.vColor0 = SVector3( 1.0f, 0.5f, 0.0f ) * 0.33f;
			sVertexShaderBasic.vColor1 = SVector3( 0.0f, 0.9f, 1.0f ) * 0.33f;
			sVertexShaderBasic.vColor2 = SVector3( 0.0f, 0.8f, 1.0f ) * 0.33f;
			sVertexShaderBasic.fAlpha = 0.6f;

			if ( sShipEnemy.m_sTurret.m_bShoot )
			{
				sVertexShaderBasic.vColor2 = SVector3( 0.1f, 0.4f, 1.0f ) * 0.33f;
			}

			if ( sShipEnemy.m_fDamageTimerMs > 0.0f )
			{
				sVertexShaderBasic.vColor0.x *= 0.2f;
				sVertexShaderBasic.vColor0.y *= 0.2f;
				sVertexShaderBasic.vColor0.z = 1.0f;
				sVertexShaderBasic.vColor1.x *= 0.2f;
				sVertexShaderBasic.vColor1.y *= 0.2f;
				sVertexShaderBasic.vColor1.z = 1.0f;
				sVertexShaderBasic.vColor2.x *= 0.2f;
				sVertexShaderBasic.vColor2.y *= 0.2f;
				sVertexShaderBasic.vColor2.z = 1.0f;
				sVertexShaderBasic.fAlpha = 0.3f;
			}

			CGraphics::GetInstance().DrawLineList3D( CEngine::GetInstance().GetMeshShipScout().m_pVertices, CEngine::GetInstance().GetMeshShipScout().m_iVertexCount, CEngine::GetInstance().GetMeshShipScout().m_pIndices, CEngine::GetInstance().GetMeshShipScout().m_iIndexCount/2, m_sViewportGameView, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
		}
	}

	// asteroids
	{
		struct SVertexShaderAsteroid
		{
			using AttribsType = SVertexPC::SAttribs;
			SMatrix matWorld;
			SMatrix matViewProjViewPort;
			float fScale;
			SVector3 vPos;
			SVector4 vColor0;
			SVector4 vColor1;
			inline void Execute( SClipVertex<AttribsType>& out, const SVertexPC& in ) const
			{
				SVector4 vPhSrc( in.vPos, 1.0f );
				SMatrix::Mul( out.vPos, vPhSrc, matWorld );
				float fAlpha = out.vPos.z * 0.5f + 0.5f;
				out.vPos.x *= fScale;
				out.vPos.y *= fScale;
				out.vPos.z *= fScale;
				out.vPos.x += vPos.x;
				out.vPos.y += vPos.y;
				out.vPos.z += vPos.z;
				SMatrix::Mul( out.vPos, out.vPos, matViewProjViewPort );
				SVector4::Lerp( out.sAttribs.vColor, vColor0, vColor1, in.sAttribs.vColor.x );
				out.sAttribs.vColor.w *= fAlpha;
			}
		} sVertexShaderAsteroid;
		sVertexShaderAsteroid.matViewProjViewPort = matViewProjViewPort;
		sVertexShaderAsteroid.vColor0 = SVector4( 1.0f, 0.6f, 0.6f, 0.4f );
		sVertexShaderAsteroid.vColor1 = SVector4( 1.0f, 0.9f, 0.4f, 0.7f );
		for ( size_t iAsteroidInd = 0; iAsteroidInd < m_cActors.GetAsteroidCount(); iAsteroidInd++ )
		{
			const SAsteroid& sAsteroid = m_cActors.GetAsteroid(iAsteroidInd);
			if ( m_sCamera.FrustumSphereTest( sAsteroid.m_vPos, sAsteroid.m_fSize*2.0f ) )
			{
				SMatrix::Identity( sVertexShaderAsteroid.matWorld );
				SQuaternion::ToMatrix( sVertexShaderAsteroid.matWorld, sAsteroid.m_qRot );
			
				sVertexShaderAsteroid.vPos = sAsteroid.m_vPos;
				//sVertexShaderAsteroid.vPos = SVector3( 0.0f, 0.0f, 0.0f );
				sVertexShaderAsteroid.fScale = sAsteroid.m_fSize;			

				const SMesh* pMeshAsteroid = nullptr;
				switch ( sAsteroid.m_eModel )
				{
					case SAsteroid::Model01:				
					pMeshAsteroid = &CEngine::GetInstance().GetMeshAsteroid01();
					break;
					case SAsteroid::Model02:
					pMeshAsteroid = &CEngine::GetInstance().GetMeshAsteroid02();
					break;
					default:
					case SAsteroid::ModelBig:
					pMeshAsteroid = &CEngine::GetInstance().GetMeshAsteroidBig();
					break;
				}

				//for ( int i = 0; i < 10; i++ )
				CGraphics::GetInstance().DrawLineList3D( pMeshAsteroid->m_pVertices, pMeshAsteroid->m_iVertexCount, pMeshAsteroid->m_pIndices, pMeshAsteroid->m_iIndexCount/2, m_sViewportGameView, sVertexShaderAsteroid, SPixelShaderBasic(), SBlendFuncAdditive() );
			}
		}
	}
	
	//bullets
	{
		struct SPixelShaderBasic
		{
			BGRA8 sColor;
			inline BGRA8 Execute( const SVertexP::SAttribs& in ) const
			{
				return sColor;
			}
		} sPixelShaderBasic;

		SMatrix matViewProjViewport;
		SMatrix::Mul( matViewProjViewport, m_sCamera.m_matViewProj, m_sViewportGameView.GetViewPortMatrix() );
		SMatrix matViewProjViewportPrev;
		SMatrix::Mul( matViewProjViewportPrev, m_sCamera.m_matViewProjPrev, m_sViewportGameView.GetViewPortMatrix() );

		for ( size_t iShipInd = 0; iShipInd < m_cActors.GetShipCount(); iShipInd++ )
		{
			const SShip& sShip = m_cActors.GetShip( iShipInd );
			const STurret& sTurret = sShip.m_sTurret;

			for ( int iBulletInd = 0; iBulletInd < sShip.m_sTurret.m_aBullets.size(); iBulletInd++ )
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
						fAlpha *= 0.7f;
						fAlpha = 1.0f - fAlpha;
						SVector3 vColor = sTurret.m_vColor;

						sPixelShaderBasic.sColor = BGRA8( vColor.x, vColor.y, vColor.z, fAlpha );

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
	// MiniMap
	////////////////////////////////////////////////////////////////

	{
		float fMiniMapScale = 2000.0f;

		SMatrix matMiniMap;
		{
			SMatrix matView;
			SMatrix::Identity( matView );
			SMatrix::BuildLHViewMatrix( matView, SVector3( m_cActors.GetShipPlayer().m_vPos.x, m_cActors.GetShipPlayer().m_vPos.y, 1.0f ), SVector3( m_cActors.GetShipPlayer().m_vPos.x, m_cActors.GetShipPlayer().m_vPos.y, -1.0f ), m_cActors.GetShipPlayer().m_vDir );
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
			inline void Execute( SClipVertex<AttribsType>& out, const SVertexP& in ) const
			{
				SVector4 vPhSrc( in.vPos, 1.0f );
				SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProjViewPort );
			}
		} sVertexShaderBasic;
		sVertexShaderBasic.matWorldViewProjViewPort = matMiniMap;

		struct SPixelShaderBasic
		{
			BGRA8 sColor;
			inline BGRA8 Execute( const SVertexP::SAttribs& in ) const
			{
				return sColor;
			}
		} sPixelShaderBasic;

		{
			SVertexP sP;
			sP.vPos = m_cActors.GetShipPlayer().m_vPos;

			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x22, 0xff, 0x22, 0xff );
			CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
		}

		for ( size_t iShipInd = 0; iShipInd < m_cActors.GetShipCount(); iShipInd++ )
		{
			SShip& sShip = m_cActors.GetShip( iShipInd );
			SVertexP sP;
			sP.vPos = sShip.m_vPos;

			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x11, 0x00, 0xff, 0xbb );
			CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
		}

		for ( size_t iShipInd = 0; iShipInd < m_cActors.GetShipCount(); iShipInd++ )
		{
			const SShip& sShip = m_cActors.GetShip( iShipInd );

			SVertexP sP;
			
			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x11, 0x99, 0xff, 0x22 );
			for ( int iBulletInd = 0; iBulletInd < sShip.m_sTurret.m_aBullets.size(); iBulletInd++ )
			{
				const STurret::SBullet& sBullet = sShip.m_sTurret.m_aBullets[iBulletInd];
				sP.vPos = sBullet.m_vPos;

				CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
			}
		}

		for ( size_t iAsteroidInd = 0; iAsteroidInd < m_cActors.GetAsteroidCount(); iAsteroidInd++ )
		{
			const SAsteroid& sAsteroid = m_cActors.GetAsteroid(iAsteroidInd);
			SVertexP sP;
			sP.vPos = sAsteroid.m_vPos;
			sPixelShaderBasic.sColor = BGRA8( (uint8_t)0x88, 0x99, 0x55, 0x55 );
			CGraphics::GetInstance().DrawPoint3D( sP, m_sViewportMiniMap, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
		}

		m_cGrid.RenderToMiniMap( 50.0f*5.0f, 10/2, matMiniMap, m_sViewportMiniMap, m_cActors.GetShipPlayer().m_vPos );
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
		sShipPlayer.m_fAccForward_ctrl = -0.2f;

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
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		if ( !sShipPlayer.m_sTurret.m_bShoot )
		{
			sShipPlayer.m_sTurret.m_bShoot = true;
			sShipPlayer.m_sTurret.m_iLastBulletTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
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
		SShip& sShipPlayer = m_cActors.GetShipPlayer();
		sShipPlayer.m_sTurret.m_bShoot = false;
	}
	return true;
	}
	return false;
}
