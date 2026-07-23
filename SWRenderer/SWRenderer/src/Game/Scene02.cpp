#include "Scene02.h"
#include "Common/Globals.h"
#include "Engine/Engine.h"
#include "winAudio.h"

CScene02::CScene02()
{
	m_pStars = nullptr;
	Clear();
}

CScene02::~CScene02()
{
	Clear();
}

void CScene02::Clear()
{
	SAFE_DELETE_ARRAY( m_pStars );
	m_iStarsCount = 0;

	m_sCamera.Clear();
	m_sShipControl.Clear();
	m_cShipMesh.Clear();
}

void CScene02::Create()
{
	Clear();
	m_cShipMesh.Create();

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
}

void CScene02::Update()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();
	{
		// Update ship:

		m_sShipControl.m_fYawVel += m_sShipControl.m_fYawVelAcc * 0.00005f * fElapsedTimeMs;
		m_sShipControl.m_fYawVel = Clamp( m_sShipControl.m_fYawVel, -0.005f, 0.005f );
		float fYawW = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		m_sShipControl.m_fYawVel = Lerp( 0.0f, m_sShipControl.m_fYawVel, fYawW );

		m_sShipControl.m_fYaw += m_sShipControl.m_fYawVel * fElapsedTimeMs;
		m_sShipControl.m_fAcc += m_sShipControl.m_fAccVel * 0.0000005f * fElapsedTimeMs;

		float fMovDecW = CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs );
		m_sShipControl.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), m_sShipControl.m_vMov, fMovDecW );

		float fAccW = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		m_sShipControl.m_fAcc = Lerp( 0.0f, m_sShipControl.m_fAcc, fAccW );

		SVector3 vShipDir( cosf( m_sShipControl.m_fYaw ), sinf( m_sShipControl.m_fYaw ), 0.0f );

		m_sShipControl.m_vMov += vShipDir * m_sShipControl.m_fAcc * fElapsedTimeMs;
		m_sShipControl.m_vPos += m_sShipControl.m_vMov * fElapsedTimeMs;

		m_sShipControl.UpdateMatrices();

	}

	{
		// Update camera:
		float fW = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		SVector3 vP( m_sShipControl.m_vPos );
		m_sCamera.m_vLookAt = vP;
		m_sCamera.m_vEye = vP;
		m_sCamera.m_vEye.z += 200.0f;
		m_sCamera.m_vLookAtSmooth = Lerp( m_sCamera.m_vLookAt, m_sCamera.m_vLookAtSmooth, fW );
		m_sCamera.m_vEyeSmooth = Lerp( m_sCamera.m_vEye, m_sCamera.m_vEyeSmooth, fW );
		
		m_sCamera.UpdateMatrices();
	}
}

void CScene02::Render()
{
	{
		float fAlpha = 1.0f;
		const int iSteps = 3;
		for ( int j =0; j < iSteps; j++ )
		{
			float fStarBoxSize = powf( (float)(j+1), 3.0f ) * 100.0f;
			float fStarBoxSizeInv = 1.0f / fStarBoxSize;
			for ( uint32_t i = 0; i < m_iStarsCount/((iSteps+1)-j); i++ )
			{
				SVertexPh sPh0;
				SVertexPh sPh1;
				{
					SVector4 vPhSrc( m_pStars[i].vPos * fStarBoxSize, 1.0f );
					vPhSrc.x = vPhSrc.x - floorf((vPhSrc.x - m_sCamera.m_vEye.x) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					vPhSrc.y = vPhSrc.y - floorf((vPhSrc.y - m_sCamera.m_vEye.y) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					vPhSrc.z = vPhSrc.z - floorf((vPhSrc.z - m_sCamera.m_vEye.z) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
					SMatrix::Mul( sPh0.vPos, vPhSrc, m_sCamera.m_matViewProj );
					SMatrix::Mul( sPh1.vPos, vPhSrc, m_sCamera.m_matViewProjPrev );
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
		SMatrix::Mul( sVertexShaderBasic.matWorldViewProj, m_sShipControl.m_matShip, m_sCamera.m_matViewProj );
		sVertexShaderBasic.fAlpha = 0.7f;
		CGraphics::GetInstance().DrawLineList3D( m_cShipMesh.GetLineList(), m_cShipMesh.GetLineListCount(), sVertexShaderBasic );
	}
}

bool CScene02::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
	// VK_UP
	case 0x26:
	{
		m_sShipControl.m_fAccVel = 1.0f;
	}
	break;
	// VK_DOWN
	case 0x28:
	{
		m_sShipControl.m_fAccVel = -1.0f;
	}
	break;
	// VK_LEFT
	case 0x25:
	{
		m_sShipControl.m_fYawVelAcc = 1.0f;
	}
	break;
	// VK_RIGHT
	case 0x27:
	{
		m_sShipControl.m_fYawVelAcc = -1.0f;
	}
	break;
	}
	return false;
}

bool CScene02::On_KeyUp( uint32_t key )
{
	switch ( key )
	{
		// VK_UP
	case 0x26:
	{
		m_sShipControl.m_fAccVel = 0.0f;
	}
	break;
	// VK_DOWN
	case 0x28:
	{
		m_sShipControl.m_fAccVel = 0.0f;
	}
	break;
	// VK_LEFT
	case 0x25:
	{
		m_sShipControl.m_fYawVelAcc = 0.0f;
	}
	break;
	// VK_RIGHT
	case 0x27:
	{
		m_sShipControl.m_fYawVelAcc = 0.0f;
	}
	break;
	}
	return false;
}

bool CScene02::On_MouseMove( int deltax, int deltay )
{
	if ( CEngine::GetInstance().GetMouseState().bLeftButton )
	{
	}
	else if ( CEngine::GetInstance().GetMouseState().bRightButton )
	{
	}

	return false;
}
bool CScene02::On_MouseButtonDown( uint32_t button )
{
	if ( button == 0 )
	{

	}
	return false;
}

bool CScene02::On_MouseButtonUp( uint32_t button )
{
	if ( button == 0 )
	{

	}

	return false;
}

bool CScene02::On_MouseWheel( int iDelta )
{

	return false;
}