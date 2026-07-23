#include "Scene01.h"
#include "Common/Globals.h"
#include "Engine/Engine.h"
#include "winAudio.h"

CScene01::CScene01()
{
	m_pStars = nullptr;
	m_pBGStars = nullptr;
	m_pVBCircle = nullptr;
	m_pIBCircle = nullptr;
	Clear();
}

CScene01::~CScene01()
{
	Clear();
}

void CScene01::Clear()
{
	m_cShipControl.Clear();
	m_cShipMesh.Clear();

	SAFE_DELETE_ARRAY( m_pStars );
	m_iStarsCount = 0;
	SAFE_DELETE_ARRAY( m_pBGStars );
	m_iBGStarsCount = 0;
	SAFE_DELETE_ARRAY( m_pVBCircle );
	m_iVBCircleCount = 0;
	SAFE_DELETE_ARRAY( m_pIBCircle );
	m_iIBCircleCount = 0;
	SAFE_DELETE_ARRAY( m_pCirclePos );
	m_iCirclePosCount = 0;

	m_fTimeMultiplierW = 0.0f;
	m_fTimeMultiplier = 1.0f;
}

void CScene01::Create()
{
	Clear();

	m_cShipControl.Create();
	m_cShipMesh.Create();

	m_cCamera.SetAspect( (float)CGraphics::GetInstance().GetFrameBuffer().iWidth / (float)CGraphics::GetInstance().GetFrameBuffer().iHeight );
	m_cCameraShip.SetAspect( (float)CGraphics::GetInstance().GetFrameBuffer().iWidth / (float)CGraphics::GetInstance().GetFrameBuffer().iHeight );

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

	m_iBGStarsCount = 10000;
	m_pBGStars = new SVertexPC[m_iBGStarsCount];
	for ( uint32_t i = 0; i < m_iBGStarsCount; i++ )
	{
		float fU = ((float)rand()/(float)RAND_MAX);
		float fV = ((float)rand()/(float)RAND_MAX);

		fU *= PI2;

		if ( i%3!=0)
		{
			fV = 1.0f - powf( 1.0f - fV, 0.3f );
		}

		fV = powf( fV, 1.5f );

		fV = asinf( fV );		
		fV = i % 2 == 0 ? -fV : fV;

		float fNoiseScale = 2.0f;
		float fNoise = 1.0f;
		fNoise *= Noise2DPeriodic( fU/PI2*128.0f, fV/PI2*128.0f, 128 )*0.6f+0.4f;
		fNoise *= Noise2DPeriodic( fU/PI2*32.0f, fV/PI2*32.0f, 32 )*0.6f+0.4f;

		m_pBGStars[i].vPos.x = cosf(fU)*cosf(fV);
		m_pBGStars[i].vPos.y = sinf(fU)*cosf(fV);
		m_pBGStars[i].vPos.z = sinf(fV);
		m_pBGStars[i].vPos *= 10000.0f;

		float a = ((float)rand()/(float)RAND_MAX);
		a = powf( a, 40.0f );
		a = a * 0.8f + 0.2f;
		a *= fNoise;
		m_pBGStars[i].vColor = SVector4( ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, ((float)rand()/(float)RAND_MAX)*0.1f+0.7f, ((float)rand()/(float)RAND_MAX)*0.1f+0.6f, a*2.0f );
	}

	{
		m_iVBCircleCount = 8;
		m_pVBCircle = new SVertexPC[m_iVBCircleCount];
		for ( uint32_t i = 0; i < m_iVBCircleCount; i++ )
		{
			float fW = (float)i/(float)m_iVBCircleCount;
			m_pVBCircle[i].vPos.x = cosf( fW*PI2 );
			m_pVBCircle[i].vPos.y = sinf( fW*PI2 );
			m_pVBCircle[i].vPos.z = 0.0f;
			m_pVBCircle[i].vColor = SVector4( 0.9f, 0.9f, 0.3f, 1.0f );
		}

		m_iIBCircleCount = m_iVBCircleCount*2;
		m_pIBCircle = new uint32_t[m_iIBCircleCount];
		for ( uint32_t i = 0; i < m_iVBCircleCount; i++ )
		{
			m_pIBCircle[i*2+0] = i;
			m_pIBCircle[i*2+1] = (i+1)%m_iVBCircleCount;
		}
	}

	{
		m_iCirclePosCount = 100;
		m_pCirclePos = new SVector3[m_iCirclePosCount];
		for ( int i = 0; i < m_iCirclePosCount; i++ )
		{
			m_pCirclePos[i].x = ((float)rand()/(float)RAND_MAX)*2.0f-1.0f;
			m_pCirclePos[i].y = ((float)rand()/(float)RAND_MAX)*2.0f-1.0f;
			m_pCirclePos[i].z = ((float)rand()/(float)RAND_MAX)*2.0f-1.0f;
			m_pCirclePos[i] *= 10000.0f;
		}
	}
}

#define cCamera m_cCameraShip

void CScene01::Update()
{
	if ( CEngine::GetInstance().GetMouseState().bRightButton )
	{
		float fW = CalcSmoothUpdateWeight( 1.02f, CEngine::GetInstance().GetElapsedTimeMs() );
		m_fTimeMultiplierW = Lerp( 0.0f, m_fTimeMultiplierW, fW );
	}
	else
	{
		float fW = CalcSmoothUpdateWeight( 1.002f, CEngine::GetInstance().GetElapsedTimeMs() );
		m_fTimeMultiplierW = Lerp( 1.0f, m_fTimeMultiplierW, fW );
	}

	m_fTimeMultiplier = Lerp( 0.1f, 1.0f, m_fTimeMultiplierW );

	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs() * m_fTimeMultiplier;

	/*else if ( CEngine::GetInstance().GetMouseState().bLeftButton )
	{
		m_sShip.Accelerate( fElapsedTimeMs * 0.01f );
	}
	else if ( CEngine::GetInstance().GetMouseState().bRightButton )
	{
		m_sShip.Accelerate( fElapsedTimeMs * -0.05f );
	}*/

	m_cCamera.Update( fElapsedTimeMs );
	
	m_cShipControl.Update( fElapsedTimeMs, m_fTimeMultiplier, m_cCameraShip.GetViewMatrix() );

	const SMatrix& matShip = m_cShipControl.GetMatrix();
	m_cCameraShip.Update( fElapsedTimeMs, matShip );

	CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed = m_cShipControl.m_fSpeedForward * m_fTimeMultiplier;
	CEngine::GetInstance().GetAudioFrameData().m_vShipPos = m_cShipControl.m_vPos;
	CEngine::GetInstance().GetAudioFrameData().m_vCameraEye = cCamera.GetEye();
	CEngine::GetInstance().GetAudioFrameData().m_vCameraLookAt = cCamera.GetLookAt();
}

void CScene01::Render()
{
	{
		float fAlpha = m_fTimeMultiplierW*0.5f+0.5f;
		const int iSteps = 3;
		for ( int j =0; j < iSteps; j++ )
		{
			float fStarBoxSize = powf( (float)(j+1), 3.0f ) * 1000.0f;
			float fStarBoxSizeInv = 1.0f / fStarBoxSize;
			for ( uint32_t i = 0; i < m_iStarsCount/((iSteps+1)-j); i++ )
			{
				SVertexPh sPh0;
				SVertexPh sPh1;
				{
					SVector4 vPhSrc( m_pStars[i].vPos * fStarBoxSize, 1.0f );
					vPhSrc.x = vPhSrc.x - floorf((vPhSrc.x - cCamera.GetEye().x) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					vPhSrc.y = vPhSrc.y - floorf((vPhSrc.y - cCamera.GetEye().y) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					vPhSrc.z = vPhSrc.z - floorf((vPhSrc.z - cCamera.GetEye().z) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					SMatrix::Mul( sPh0.vPos, vPhSrc, cCamera.GetViewProjectionMatrix() );
					SMatrix::Mul( sPh1.vPos, vPhSrc, cCamera.GetViewProjectionMatrixPrev() );
				}

				if ( CGraphics::GetInstance().ClipLineDepth<SVertexPh>( sPh0, sPh1 ) )
				{
					if ( CGraphics::GetInstance().ClipLineXY<SVertexPh>( sPh0, sPh1 ) )
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
	}

	{
		float fAlpha = m_fTimeMultiplierW*0.5f+0.5f;
		for ( uint32_t i = 0; i < m_iBGStarsCount; i++ )
		{
			SVertexPh sPh0;
			SVertexPh sPh1;
			{
				SVector4 vPhSrc( m_pBGStars[i].vPos, 1.0f );
				SMatrix::Mul( sPh0.vPos, vPhSrc, cCamera.GetViewProjectionMatrix000() );
				SMatrix::Mul( sPh1.vPos, vPhSrc, cCamera.GetViewProjectionMatrixPrev000() );
			}

			if ( CGraphics::GetInstance().ClipLineDepth<SVertexPh>( sPh0, sPh1 ) )
			{
				if ( CGraphics::GetInstance().ClipLineXY<SVertexPh>( sPh0, sPh1 ) )
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

					SVector2 v( sPh0.vPos.x - sPh1.vPos.x, sPh0.vPos.y - sPh1.vPos.y );
					float fLSq = SVector2::LengthSq( v );
					if ( fLSq > 1.5f )
					{
						CGraphics::GetInstance().DrawLine( SVector2( sPh0.vPos.x, sPh0.vPos.y ), SVector2( sPh1.vPos.x, sPh1.vPos.y ), BGRA8( m_pBGStars[i].vColor.x*fAlpha, m_pBGStars[i].vColor.y, m_pBGStars[i].vColor.z*fAlpha, m_pBGStars[i].vColor.w/(fL*0.2f+1.0f)*fAlpha ) );
					}
					else
					{
						CGraphics::GetInstance().DrawPixel( (int)sPh0.vPos.x, (int)sPh0.vPos.y, BGRA8(m_pBGStars[i].vColor.x*fAlpha, m_pBGStars[i].vColor.y, m_pBGStars[i].vColor.z*fAlpha, m_pBGStars[i].vColor.w*fAlpha ) );
					}
				}
			}
		}
	}

	struct SVertexShaderBasic
	{
		SMatrix matWorldViewProj;
		float fAlpha;
		void Process( SVertexPhC& out, const SVertexPC& in ) const
		{
			SVector4 vPhSrc0( in.vPos, 1.0f );
			SMatrix::Mul( out.vPos, vPhSrc0, matWorldViewProj );
			out.vColor = in.vColor;
			out.vColor.w *= fAlpha;
		}
	} sVertexShaderBasic;

	{
		const SMatrix& matWorld = m_cShipControl.GetMatrix();

		int iInstCount = 1;
		for ( int iInstIndX = 0; iInstIndX < iInstCount; iInstIndX++ )
		for ( int iInstIndY = 0; iInstIndY < iInstCount; iInstIndY++ )
		for ( int iInstIndZ = 0; iInstIndZ < iInstCount; iInstIndZ++ )
		{
			SMatrix::Mul( sVertexShaderBasic.matWorldViewProj, matWorld, cCamera.GetViewProjectionMatrix() );
			sVertexShaderBasic.fAlpha = 0.7f;
			CGraphics::GetInstance().DrawLineList3D( m_cShipMesh.GetLineList(), m_cShipMesh.GetLineListCount(), sVertexShaderBasic );
		}

		for ( int iBulletInd = 0; iBulletInd < m_cShipControl.GetBullets().size(); iBulletInd++ )
		{
			const SBullet& sBullet = m_cShipControl.GetBullets()[iBulletInd];
			SVertexPh sPh0;
			SVertexPh sPh1;
			{
				SVector4 vPhSrc( sBullet.m_vPos, 1.0f );
				SMatrix::Mul( sPh0.vPos, vPhSrc, cCamera.GetViewProjectionMatrix() );
				SMatrix::Mul( sPh1.vPos, vPhSrc, cCamera.GetViewProjectionMatrixPrev() );
			}

			if ( CGraphics::GetInstance().ClipLineDepth<SVertexPh>( sPh0, sPh1 ) )
			{
				if ( CGraphics::GetInstance().ClipLineXY<SVertexPh>( sPh0, sPh1 ) )
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
	}

	for ( int i = 0; i < m_iCirclePosCount; i++ )
	{
		SMatrix matWorld;
		SMatrix::Identity( matWorld );
		SMatrix::BuildViewMatrix( matWorld, m_pCirclePos[i], cCamera.GetEye() + ( cCamera.GetLookAt() - cCamera.GetEye() )*100.0f, SVector3( 0.0f, 0.0f, 1.0f ) );
		SMatrix::Inverse( matWorld, matWorld );
		SMatrix::Scale( matWorld, 100.0f );

		SMatrix::Mul( sVertexShaderBasic.matWorldViewProj, matWorld, cCamera.GetViewProjectionMatrix() );
		sVertexShaderBasic.fAlpha = 0.7f;
		CGraphics::GetInstance().DrawLineList3D( m_pVBCircle, m_pIBCircle, m_iIBCircleCount / 2, sVertexShaderBasic );
	}

	{
		struct SVertexShaderGrid
		{
			SMatrix matWorldViewProj;
			void Process( SVertexPhC& out, const SVertexPC& in ) const
			{
				SVector4 vPhSrc0( in.vPos, 1.0f );
				SMatrix::Mul( out.vPos, vPhSrc0, matWorldViewProj );
				out.vColor = in.vColor;
			}
		} sVertexShaderGrid;

		

		SVector4 vColor = SVector4( 0.1f, 0.0f, 0.4f, (1.0f-m_fTimeMultiplierW)*0.5f );
		if ( vColor.w > 1.0f/255.0f )
		{
			float fSpacing = 1000.0f;
			int iHalfGridSize = 10/2;

			SMatrix matScale;
			SMatrix::Identity(matScale);
			SMatrix::Scale( matScale, fSpacing );
		
			{
				SVector3 vCenter = m_cShipControl.m_vPos / fSpacing;
				SVector3 vCenterQ;
				vCenterQ.x = vCenter.x;
				vCenterQ.y = floorf( vCenter.y );
				vCenterQ.z = floorf( vCenter.z );

				SMatrix matWorld( matScale );
				matWorld.m30 = vCenterQ.x*fSpacing;
				matWorld.m31 = vCenterQ.y*fSpacing;
				matWorld.m32 = vCenterQ.z*fSpacing;
				SMatrix::Mul( matWorld, matWorld, cCamera.GetViewProjectionMatrix() );
				sVertexShaderGrid.matWorldViewProj = matWorld;

				float fi = vCenter.y - floorf(vCenter.y);
				float fj = vCenter.z - floorf(vCenter.z);

				for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
				{
					for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
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
						/*t -= 0.5f;
						t = abs( t );
						t *= 2.0f;*/
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
				SVector3 vCenter = m_cShipControl.m_vPos / fSpacing;
				SVector3 vCenterQ;
				vCenterQ.x = floorf( vCenter.x );
				vCenterQ.y = vCenter.y;
				vCenterQ.z = floorf( vCenter.z );

				SMatrix matWorld( matScale );
				matWorld.m30 = vCenterQ.x*fSpacing;
				matWorld.m31 = vCenterQ.y*fSpacing;
				matWorld.m32 = vCenterQ.z*fSpacing;
				SMatrix::Mul( matWorld, matWorld, cCamera.GetViewProjectionMatrix() );
				sVertexShaderGrid.matWorldViewProj = matWorld;

				float fi = vCenter.x - floorf(vCenter.x);
				float fj = vCenter.z - floorf(vCenter.z);

				for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
				{
					for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
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
						/*t -= 0.5f;
						t = abs( t );
						t *= 2.0f;*/
						float fAlpha = 1.0f-t;

						sVertex0.vColor.w *= 0.0f;
						sVertex1.vColor.w *= fAlpha;
						sVertex2.vColor.w *= 0.0f;

						CGraphics::GetInstance().DrawLine3D( sVertex0, sVertex1, sVertexShaderGrid );
						CGraphics::GetInstance().DrawLine3D( sVertex1, sVertex2, sVertexShaderGrid );
					}
				}
			}
		}
	}
}

bool CScene01::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
	case 0x26:
	{
		//m_sShip.Accelerate( 1.0f );
	}
	break;
	case 0x28:
	{
		//m_sShip.Accelerate( -5.0f );
	}
	break;
	}
	return false;
}

bool CScene01::On_KeyUp( uint32_t key )
{
	return false;
}

bool CScene01::On_MouseMove( int deltax, int deltay )
{
	m_cShipControl.MouseMove( SVector2( (float)deltax, (float)deltay ) );

	if ( CEngine::GetInstance().GetMouseState().bLeftButton )
	{
		m_cCamera.Rotate( (float)deltax * 0.005f, (float)deltay * 0.005f );
	}
	else if ( CEngine::GetInstance().GetMouseState().bRightButton )
	{
		m_cCamera.Pan( SVector2( (float)deltax * 0.005f, (float)deltay * 0.005f ) );
	}

	return false;
}
bool CScene01::On_MouseButtonDown( uint32_t button )
{
	if ( button == 0 )
	{
		m_cShipControl.SetShoot( true );
	}
	return false;
}

bool CScene01::On_MouseButtonUp( uint32_t button )
{
	if ( button == 0 )
	{
		m_cShipControl.SetShoot( false );
	}

	return false;
}

bool CScene01::On_MouseWheel( int iDelta )
{
	m_cCamera.Zoom( (float)iDelta * 0.001f );
	return false;
}