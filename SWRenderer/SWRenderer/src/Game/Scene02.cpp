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

static float fAction = 0.0f;
static float fClimax = 0.0f;

void CScene02::Update()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();
	{
		// Update ship:

		m_sShipControl.m_vDirPrev = m_sShipControl.m_vDir;
		m_sShipControl.m_matShipPrev = m_sShipControl.m_matShip;

		m_sShipControl.m_fYawVel += m_sShipControl.m_fYawVelAcc * 0.00005f * fElapsedTimeMs;
		m_sShipControl.m_fYawVel = Clamp( m_sShipControl.m_fYawVel, -0.005f, 0.005f );
		float fYawW = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		m_sShipControl.m_fYawVel = Lerp( 0.0f, m_sShipControl.m_fYawVel, fYawW );

		m_sShipControl.m_fYaw += m_sShipControl.m_fYawVel * fElapsedTimeMs;
		m_sShipControl.m_fAcc += m_sShipControl.m_fAccVel * 0.00005f * fElapsedTimeMs;

		float fMovDecW = CalcSmoothUpdateWeight( 1.0001f, fElapsedTimeMs );
		m_sShipControl.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), m_sShipControl.m_vMov, fMovDecW );

		float fAccW = CalcSmoothUpdateWeight( 1.1f, fElapsedTimeMs );
		m_sShipControl.m_fAcc = Lerp( 0.0f, m_sShipControl.m_fAcc, fAccW );

		SVector3 vShipDir( cosf( m_sShipControl.m_fYaw ), sinf( m_sShipControl.m_fYaw ), 0.0f );

		m_sShipControl.m_vMov += vShipDir * m_sShipControl.m_fAcc * fElapsedTimeMs;
		m_sShipControl.m_vPos += m_sShipControl.m_vMov * fElapsedTimeMs;

		m_sShipControl.UpdateMatrices();

		if ( m_sShipControl.m_bShoot )
		{
			SVector3 vGunPos = SVector3( 0.0f, 0.0f, 0.0f );

			SVector3 vGunPosWorld;
			SVector3 vGunPosWorlPrev;

			SVector3 vGunDirWorld( m_sShipControl.m_vDir );
			SVector3 vGunDirWorlPrev( m_sShipControl.m_vDirPrev );

			SMatrix::TransformCoord( vGunPosWorld, vGunPos, m_sShipControl.m_matShip );
			SMatrix::TransformCoord( vGunPosWorlPrev, vGunPos, m_sShipControl.m_matShipPrev );

			const float fShootFreqHz = 60.0f;
			const uint64_t iShootPeriodNs = (uint64_t)(1.0f / fShootFreqHz * 1000.0f * 1000.0f * 1000.0f);

			for ( uint64_t iTNs = m_sShipControl.m_iLastBulletTimeStampNs+iShootPeriodNs; iTNs < CEngine::GetInstance().GetTimeStampNs(); iTNs += iShootPeriodNs )
			{		
				float fFrameW = (float)(iTNs - m_sShipControl.m_iLastBulletTimeStampNs) / (float)(CEngine::GetInstance().GetTimeStampNs() - m_sShipControl.m_iLastBulletTimeStampNs);

				m_sShipControl.m_iLastBulletTimeStampNs = iTNs;

				SShipControl::SBullet sBullet;
				sBullet.m_vPos = Lerp( vGunPosWorlPrev, vGunPosWorld, fFrameW );
				sBullet.m_vDir = Lerp( vGunDirWorlPrev, vGunDirWorld, fFrameW );

				sBullet.m_vDir.x += (rand() % 1000 - 500) * 0.00001f;
				sBullet.m_vDir.y += (rand() % 1000 - 500) * 0.00001f;
				sBullet.m_vDir.z += (rand() % 1000 - 500) * 0.00001f;

				SVector3::Normalize( sBullet.m_vDir, sBullet.m_vDir );
				sBullet.m_fSpeed = 0.1f;
				sBullet.m_fTime = 4000.0f;
				sBullet.m_fTimer = 0.0f;
				m_sShipControl.m_aBullets.push_back( sBullet );

				SAudioEvent sAudioEvent;
				sAudioEvent.type = SAudioEvent::GunShot;
				sAudioEvent.fVolume = 0.4f;
				sAudioEvent.iTimeStampNs = m_sShipControl.m_iLastBulletTimeStampNs;
				sAudioEvent.iLifeTimeNs = 1000 * 1000 * 150;
				sAudioEvent.iSampleCounter = 0;
				sAudioEvent.fPhase = 0.0f;			
				sAudioEvent.sGunShot.vPos = sBullet.m_vPos;
				CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );
			}		
		}

		for ( size_t iBulletInd = 0; iBulletInd < m_sShipControl.m_aBullets.size(); )
		{
			SShipControl::SBullet& sBullet = m_sShipControl.m_aBullets[iBulletInd];
			sBullet.m_fTimer += fElapsedTimeMs;
			if ( sBullet.m_fTimer > sBullet.m_fTime )
			{
				m_sShipControl.m_aBullets[iBulletInd] = m_sShipControl.m_aBullets.back();
				m_sShipControl.m_aBullets.pop_back();
				continue;
			}
			sBullet.m_vPos += sBullet.m_vDir * sBullet.m_fSpeed * fElapsedTimeMs;

			++iBulletInd;
		}
	}

	{
		// Update camera:
		float fWFast = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		float fWSlow = CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs );
		SVector3 vP( m_sShipControl.m_vPos );
		m_sCamera.m_vLookAt = vP;
		m_sCamera.m_vEye = vP;
		m_sCamera.m_vEye.z += 150.0f;
		m_sCamera.m_vLookAtSmooth = Lerp( m_sCamera.m_vLookAt, m_sCamera.m_vLookAtSmooth, fWFast );
		m_sCamera.m_vEyeSmooth = Lerp( m_sCamera.m_vEye, m_sCamera.m_vEyeSmooth, fWSlow );
		
		m_sCamera.UpdateMatrices();
	}

	CEngine::GetInstance().GetAudioFrameData().m_fShipSpeed = 0.0f;
	CEngine::GetInstance().GetAudioFrameData().m_vShipPos = m_sShipControl.m_vPos;
	CEngine::GetInstance().GetAudioFrameData().m_vCameraEye = m_sCamera.m_vEyeSmooth;
	CEngine::GetInstance().GetAudioFrameData().m_vCameraLookAt = m_sCamera.m_vLookAtSmooth;

	CEngine::GetInstance().GetAudioFrameData().m_fMusic_Action = fAction;
	CEngine::GetInstance().GetAudioFrameData().m_fMusic_Climax = fClimax;
}

void CScene02::Render()
{
	{
		float fAlpha = 1.0f;
		const int iSteps = 3;
		for ( int j =0; j < iSteps; j++ )
		{
			float fStarBoxSize = powf( (float)(j+1), 3.0f ) * 1000.0f;
			float fStarBoxSizeInv = 1.0f / fStarBoxSize;
			for ( uint32_t i = 0; i < m_iStarsCount; i++ )
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

	for ( int iBulletInd = 0; iBulletInd < m_sShipControl.m_aBullets.size(); iBulletInd++ )
	{
		const SShipControl::SBullet& sBullet = m_sShipControl.m_aBullets[iBulletInd];
		SVertexPh sPh0;
		SVertexPh sPh1;
		{
			SVector4 vPhSrc( sBullet.m_vPos, 1.0f );
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


	SVector4 vColor = SVector4( 0.1f, 0.0f, 0.4f, 0.5f );
	if ( vColor.w > 1.0f/255.0f )
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

		float fSpacing = 30.0f;
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
	// VK_SPACE
	case 0x20:
	{
		m_sShipControl.m_bShoot = true;
		m_sShipControl.m_iLastBulletTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
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
		fAction += 0.1f;
		fAction = Clamp( fAction, 0.0f, 1.0f );
		m_sShipControl.m_fAccVel = 0.0f;
	}
	break;
	// VK_DOWN
	case 0x28:
	{
		fAction -= 0.1f;
		fAction = Clamp( fAction, 0.0f, 1.0f );
		m_sShipControl.m_fAccVel = 0.0f;
	}
	break;
	// VK_LEFT
	case 0x25:
	{
		fClimax += 0.1f;
		fClimax = Clamp( fClimax, 0.0f, 1.0f );
		m_sShipControl.m_fYawVelAcc = 0.0f;
	}
	break;
	// VK_RIGHT
	case 0x27:
	{
		fClimax -= 0.1f;
		fClimax = Clamp( fClimax, 0.0f, 1.0f );
		m_sShipControl.m_fYawVelAcc = 0.0f;
	}
	break;
	// VK_SPACE
	case 0x20:
	{
		m_sShipControl.m_bShoot = false;
		m_sShipControl.m_iLastBulletTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
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