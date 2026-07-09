#include "Scene01.h"
#include "Common/Globals.h"
#include "Engine/Engine.h"
#include "winAudio.h"

SShip::SShip()
{
	Init();
}

void SShip::Init()
{
	m_vPos = SVector3( 0.0f, 0.0f, 0.0f );
	m_vDir = SVector3( 1.0f, 0.0f, 0.0f );
	m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
	m_vRight = SVector3( 0.0f, 1.0f, 0.0f );
	m_fSpeedForward = 2.0f;
	m_fSpeedUp =  0.0f;
	m_fSpeedRight = 0.0f;
}

void SShip::Accelerate( float fValue )
{
	m_fSpeedForward += fValue*0.1f;
	m_fSpeedForward = std::max( 0.0f, m_fSpeedForward );
}

void SShip::MoveUp( float fValue )
{
	m_fSpeedUp += fValue*0.0001f;
}

void SShip::MoveRight( float fValue )
{
	m_fSpeedRight += fValue*0.0001f;
}

void SShip::Update( float fElapsedTimeMs, const SMatrix& matView000 )
{
	{
		const SVector2 vMouseDir2D( m_fSpeedRight, m_fSpeedUp );

		SVector3 vUpView;
		SMatrix::TransformNormal( vUpView, m_vUp, matView000 );
		SVector2 vUpView2DNorm( -vUpView.x, vUpView.y );
		SVector2::Normalize( vUpView2DNorm, vUpView2DNorm );

		SVector3 vRightView;
		SMatrix::TransformNormal( vRightView, m_vRight, matView000 );
		SVector2 vRightView2DNorm( -vRightView.x, vRightView.y );
		SVector2::Normalize( vRightView2DNorm, vRightView2DNorm );
		
		SVector2 vMouseDir2DNorm;		
		SVector2::Normalize( vMouseDir2DNorm, vMouseDir2D );

		float fUp = -SVector2::Dot( vUpView2DNorm, vMouseDir2D );
		float fRoll = -SVector2::Dot( vRightView2DNorm, vMouseDir2D );

		
		m_fSpeedForward *= expf( -fUp*fUp*1.0f );
		{
			float fW = CalcSmoothUpdateWeight( 1.005f, fElapsedTimeMs );
			m_fSpeedForward = Lerp( 2.0f, m_fSpeedForward, fW );
		}

		{
			SQuaternion q;
			SQuaternion::FromAxisAngle( q, m_vRight, fUp*0.1f*fElapsedTimeMs );
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
			SQuaternion::FromAxisAngle( q, m_vDir, fRoll*0.3f*fElapsedTimeMs );
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

	/*{
		SQuaternion q;
		SQuaternion::FromAxisAngle( q, m_vRight, m_fSpeedUp );
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
		SQuaternion::FromAxisAngle( q, m_vDir, m_fSpeedRight );
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
	}*/

	/*{
		SQuaternion q;
		SQuaternion::FromAxisAngle( q, m_vUp, -m_fSpeedRight );
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
	}*/

	/*{
		SQuaternion q;
		SQuaternion::FromAxisAngle( q, m_vRight, abs( m_fSpeedRight ) );
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
	}*/

	m_vPos += m_vDir * m_fSpeedForward * fElapsedTimeMs;

	float fWDir = CalcSmoothUpdateWeight( 1.005f, fElapsedTimeMs );
	m_fSpeedUp *= fWDir;
	m_fSpeedRight *= fWDir;
}

void SShip::GetMatrix( SMatrix& sOut )
{
	sOut.m00 = m_vDir.x;	sOut.m01 = m_vDir.y;	sOut.m02 = m_vDir.z;	sOut.m03 = 0.0f;
	sOut.m10 = m_vRight.x;	sOut.m11 = m_vRight.y;	sOut.m12 = m_vRight.z;	sOut.m13 = 0.0f;
	sOut.m20 = m_vUp.x;		sOut.m21 = m_vUp.y;		sOut.m22 = m_vUp.z;		sOut.m23 = 0.0f;
	sOut.m30 = m_vPos.x;	sOut.m31 = m_vPos.y;	sOut.m32 = m_vPos.z;	sOut.m33 = 1.0f;
}

CScene01::CScene01()
{
	m_pStars = nullptr;
	m_pBGStars = nullptr;
	m_pLineListSpaceShip = nullptr;
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
	SAFE_DELETE_ARRAY( m_pStars );
	m_iStarsCount = 0;
	SAFE_DELETE_ARRAY( m_pBGStars );
	m_iBGStarsCount = 0;
	SAFE_DELETE_ARRAY( m_pLineListSpaceShip );
	m_iLineListSpaceShipCount = 0;
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

	{
		{
			std::vector<SVector3> pts;
			std::vector<std::pair<int,int>> edges;

			auto AddPoint =
				[&](float x, float y, float z) -> int
				{
					pts.emplace_back(y, -x, z);
					return (int)pts.size() - 1;
				};

			auto AddEdge =
				[&](int a, int b)
				{
					edges.emplace_back(a, b);
				};

			// =====================================================
			// FUSELAGE
			// =====================================================

			const int nose =
				AddPoint(0.0f, 4.8f, 0.0f);

			const int n0 = AddPoint(-0.15f, 3.6f, -0.12f);
			const int n1 = AddPoint( 0.15f, 3.6f, -0.12f);
			const int n2 = AddPoint( 0.15f, 3.6f,  0.12f);
			const int n3 = AddPoint(-0.15f, 3.6f,  0.12f);

			const int m0 = AddPoint(-0.42f, 1.4f, -0.22f);
			const int m1 = AddPoint( 0.42f, 1.4f, -0.22f);
			const int m2 = AddPoint( 0.42f, 1.4f,  0.22f);
			const int m3 = AddPoint(-0.42f, 1.4f,  0.22f);

			const int r0 = AddPoint(-0.35f, -1.3f, -0.20f);
			const int r1 = AddPoint( 0.35f, -1.3f, -0.20f);
			const int r2 = AddPoint( 0.35f, -1.3f,  0.20f);
			const int r3 = AddPoint(-0.35f, -1.3f,  0.20f);

			// nose connections
			AddEdge(nose,n0);
			AddEdge(nose,n1);
			AddEdge(nose,n2);
			AddEdge(nose,n3);

			// front ring
			AddEdge(n0,n1);
			AddEdge(n1,n2);
			AddEdge(n2,n3);
			AddEdge(n3,n0);

			// mid
			AddEdge(n0,m0);
			AddEdge(n1,m1);
			AddEdge(n2,m2);
			AddEdge(n3,m3);

			AddEdge(m0,m1);
			AddEdge(m1,m2);
			AddEdge(m2,m3);
			AddEdge(m3,m0);

			// rear
			AddEdge(m0,r0);
			AddEdge(m1,r1);
			AddEdge(m2,r2);
			AddEdge(m3,r3);

			AddEdge(r0,r1);
			AddEdge(r1,r2);
			AddEdge(r2,r3);
			AddEdge(r3,r0);

			// diagonals for detail
			AddEdge(m0,m2);
			AddEdge(m1,m3);
			AddEdge(r0,r2);
			AddEdge(r1,r3);

			// =====================================================
			// COCKPIT
			// =====================================================

			int c0 = AddPoint(0.0f, 2.6f, 0.45f);
			int c1 = AddPoint(0.0f, 1.8f, 0.58f);
			int c2 = AddPoint(0.0f, 1.0f, 0.42f);

			AddEdge(nose,c0);
			AddEdge(c0,c1);
			AddEdge(c1,c2);
			AddEdge(c2,m2);
			AddEdge(c2,m3);

			// =====================================================
			// WINGS
			// =====================================================

			auto BuildWing =
				[&](float side, float zOffset)
				{
					float sx = side;

					int rootA = AddPoint(
						0.55f*sx, 0.8f, zOffset);

					int rootB = AddPoint(
						0.55f*sx, -0.8f, zOffset);

					int tipFront = AddPoint(
						2.8f*sx, 0.9f, zOffset);

					int tipRear = AddPoint(
						2.2f*sx, -1.2f, zOffset);

					int midOuter = AddPoint(
						2.0f*sx, -0.2f, zOffset);

					// outer frame
					AddEdge(rootA,tipFront);
					AddEdge(tipFront,midOuter);
					AddEdge(midOuter,tipRear);
					AddEdge(tipRear,rootB);
					AddEdge(rootA,rootB);

					// internal ribs
					AddEdge(rootA,midOuter);
					AddEdge(rootB,midOuter);
					AddEdge(rootA,tipRear);
					AddEdge(rootB,tipFront);

					// attach to body
					AddEdge(m0,rootA);
					AddEdge(r0,rootB);

					AddEdge(m1,rootA);
					AddEdge(r1,rootB);

					// laser cannon
					int gun0 = AddPoint(
						2.8f * sx,
						1.0f,
						zOffset);

					int gun1 = AddPoint(
						2.8f * sx,
						2.0f,
						zOffset);

					AddEdge(tipFront, gun0);
					AddEdge(gun0, gun1);

					// engine pod
					const float ex = 1.35f*sx;
					const float ey = -0.7f;
					const float ez = zOffset;

					const float radius = 0.16f;

					int prev = -1;
					int first = -1;

					for(int i=0;i<8;i++)
					{
						float a =
							float(i) / 8.0f *
							6.2831853f;

						// XZ kor, Y iranyba nez
						int p = AddPoint(
							ex + cosf(a) * radius,
							ey,
							ez + sinf(a) * radius);

						if(i == 0)
							first = p;

						if(prev != -1)
							AddEdge(prev, p);

						prev = p;
					}

					AddEdge(prev,first);

					// engine mount
					AddEdge(rootB,first);
					AddEdge(midOuter,first);
				};

			// top wings
			BuildWing(-1.6f, 0.12f);
			BuildWing( 1.6f, 0.12f);

			// bottom wings
			BuildWing(-1.0f,-0.62f);
			BuildWing( 1.0f,-0.62f);

			// =====================================================
			// Allocate
			// =====================================================

			m_iLineListSpaceShipCount =
				(int)edges.size();

			delete[] m_pLineListSpaceShip;

			m_pLineListSpaceShip = new SVertexPC[m_iLineListSpaceShipCount * 2];

			for(uint32_t i=0; i<m_iLineListSpaceShipCount; ++i)
			{
				m_pLineListSpaceShip[i*2+0]
					.vPos =
					pts[edges[i].first];

				m_pLineListSpaceShip[i*2+1]
					.vPos =
					pts[edges[i].second];

				m_pLineListSpaceShip[i * 2 + 0].vColor = SVector4( 0.4f, 0.4f, 0.4f, 1.0f );
				m_pLineListSpaceShip[i * 2 + 1].vColor = SVector4( 0.0f, 0.3f, 1.0f, 0.4f );
			}
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
	
	m_sShip.Update( fElapsedTimeMs, m_cCameraShip.GetViewMatrix() );

	SMatrix matShip;
	m_sShip.GetMatrix( matShip );
	m_cCameraShip.Update( fElapsedTimeMs, matShip );

	CAudio::GetInstance().MainThread_GetAudioFrameData()->m_fShipSpeed = m_sShip.m_fSpeedForward * m_fTimeMultiplier;
	CAudio::GetInstance().MainThread_GetAudioFrameData()->m_vShipPos = m_sShip.m_vPos;
	CAudio::GetInstance().MainThread_GetAudioFrameData()->m_vCameraEye = cCamera.GetEye();
	CAudio::GetInstance().MainThread_GetAudioFrameData()->m_vCameraLookAt = cCamera.GetLookAt();
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
			for ( int i = 0; i < m_iStarsCount/(float)((iSteps+1)-j); i++ )
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
		SMatrix matWorld;
		m_sShip.GetMatrix( matWorld );

		int iInstCount = 1;
		for ( int iInstIndX = 0; iInstIndX < iInstCount; iInstIndX++ )
		for ( int iInstIndY = 0; iInstIndY < iInstCount; iInstIndY++ )
		for ( int iInstIndZ = 0; iInstIndZ < iInstCount; iInstIndZ++ )
		{
			SMatrix::Mul( sVertexShaderBasic.matWorldViewProj, matWorld, cCamera.GetViewProjectionMatrix() );
			sVertexShaderBasic.fAlpha = 0.7f;
			CGraphics::GetInstance().DrawLineList3D( m_pLineListSpaceShip, m_iLineListSpaceShipCount, sVertexShaderBasic );
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
				SVector3 vCenter = m_sShip.m_vPos / fSpacing;
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
				SVector3 vCenter = m_sShip.m_vPos / fSpacing;
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
	m_sShip.MoveRight( (float)deltax );
	m_sShip.MoveUp( (float)deltay );

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
	return false;
}

bool CScene01::On_MouseButtonUp( uint32_t button )
{
	return false;
}

bool CScene01::On_MouseWheel( int iDelta )
{
	m_cCamera.Zoom( (float)iDelta * 0.001f );
	return false;
}