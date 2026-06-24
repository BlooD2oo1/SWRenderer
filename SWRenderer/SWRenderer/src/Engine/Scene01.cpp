#include "Scene01.h"
#include "Common/Globals.h"
#include "Engine/Engine.h"

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
	m_fSpeedForward = 0.0f;
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

void SShip::Update( float fElapsedTimeMs )
{
	float fWDir = CalcSmoothUpdateWeight( 1.004f, fElapsedTimeMs );
	m_fSpeedUp *= fWDir;
	m_fSpeedRight *= fWDir;
	{
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
	}

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
}

void CScene01::Create()
{
	Clear();

	m_cCamera.SetAspect( (float)CGraphics::GetInstance().GetFrameBuffer().iWidth / (float)CGraphics::GetInstance().GetFrameBuffer().iHeight );
	m_cCameraShip.SetAspect( (float)CGraphics::GetInstance().GetFrameBuffer().iWidth / (float)CGraphics::GetInstance().GetFrameBuffer().iHeight );

	m_iStarsCount = 10000;
	m_pStars = new SParticle[m_iStarsCount];
	for ( int i = 0; i < m_iStarsCount; i++ )
	{
		m_pStars[i].vPos.x = ((float)rand()/(float)RAND_MAX);
		m_pStars[i].vPos.y = ((float)rand()/(float)RAND_MAX);
		m_pStars[i].vPos.z = ((float)rand()/(float)RAND_MAX);

		m_pStars[i].a = ((float)rand()/(float)RAND_MAX) * 0.55f + 0.45f;
		m_pStars[i].a *= m_pStars[i].a;
		m_pStars[i].a *= m_pStars[i].a;
		m_pStars[i].a = powf( m_pStars[i].a, 3.0f );
	}

	m_iBGStarsCount = 10000;
	m_pBGStars = new SParticle[m_iBGStarsCount];
	for ( int i = 0; i < m_iBGStarsCount; i++ )
	{
		float fU = ((float)rand()/(float)RAND_MAX)*PI2;
		float fV = ((float)rand()/(float)RAND_MAX);
		
		fV = 1.0f - powf( 1.0f - fV, 0.3f );
		fV = powf( fV, 1.5f );

		fV = asinf( fV );		
		fV = rand() % 2 == 0 ? -fV : fV;
		m_pBGStars[i].vPos.x = cosf(fU)*cosf(fV);
		m_pBGStars[i].vPos.y = sinf(fU)*cosf(fV);
		m_pBGStars[i].vPos.z = sinf(fV);
		m_pBGStars[i].vPos *= 10000.0f;

		m_pBGStars[i].a = ((float)rand()/(float)RAND_MAX) * 0.8f + 0.2f;
		m_pBGStars[i].a *= m_pBGStars[i].a;
		m_pBGStars[i].a *= m_pBGStars[i].a;
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

			m_pLineListSpaceShip =
				new SVertex[
					m_iLineListSpaceShipCount * 2];

			for(int i=0;
				i<m_iLineListSpaceShipCount;
				++i)
			{
				m_pLineListSpaceShip[i*2+0]
					.vPos =
					pts[edges[i].first];

				m_pLineListSpaceShip[i*2+1]
					.vPos =
					pts[edges[i].second];
			}
		}
	}
}

void CScene01::Update()
{
	if ( CEngine::GetInstance().GetMouseState().bLeftButton )
	{
		m_sShip.Accelerate( CEngine::GetInstance().GetElapsedTimeMs() * 0.01f );
	}

	if ( CEngine::GetInstance().GetMouseState().bRightButton )
	{
		m_sShip.Accelerate( CEngine::GetInstance().GetElapsedTimeMs() * -0.01f );
	}

	m_cCamera.Update( CEngine::GetInstance().GetElapsedTimeMs() );
	
	m_sShip.Update( CEngine::GetInstance().GetElapsedTimeMs() );

	SMatrix matShip;
	m_sShip.GetMatrix( matShip );
	m_cCameraShip.Update( CEngine::GetInstance().GetElapsedTimeMs(), matShip );
}

void CScene01::Render()
{
	// draw stars
	const int iSteps = 3;
	for ( int j =0; j < iSteps; j++ )
	{
		float fStarBoxSize = powf( (float)(j+1), 3.0f ) * 1000.0f;
		float fStarBoxSizeInv = 1.0f / fStarBoxSize;
		for ( int i = 0; i < m_iStarsCount/(float)((iSteps+1)-j); i++ )
		{
			SVector4 vPh0;
			SVector4 vPh1;
			{
				SVector4 vPhSrc( m_pStars[i].vPos * fStarBoxSize, 1.0f );
				vPhSrc.x = vPhSrc.x - floorf((vPhSrc.x - m_cCameraShip.GetEye().x) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
				vPhSrc.y = vPhSrc.y - floorf((vPhSrc.y - m_cCameraShip.GetEye().y) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
				vPhSrc.z = vPhSrc.z - floorf((vPhSrc.z - m_cCameraShip.GetEye().z) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
				SMatrix::Mul( vPh0, vPhSrc, m_cCameraShip.GetViewProjectionMatrix() );
				SMatrix::Mul( vPh1, vPhSrc, m_cCameraShip.GetViewProjectionMatrixPrev() );
			}

			if ( CGraphics::GetInstance().ClipLineDepth( vPh0, vPh1 ) )
			{
				{
					float fWRec0 = 1.0f / vPh0.w;
					vPh0.x = vPh0.x * fWRec0;
					vPh0.y = vPh0.y * fWRec0;
					//vPh0.z = vPh0.z * fWRec0;
					vPh0.w = 1.0f;

					float fWRec1 = 1.0f / vPh1.w;
					vPh1.x = vPh1.x * fWRec1;
					vPh1.y = vPh1.y * fWRec1;
					//vPh1.z = vPh1.z * fWRec1;
					vPh1.w = 1.0f;
				}

				if ( CGraphics::GetInstance().ClipLineXY( vPh0, vPh1 ) )
				{
					SVector2 vP0( (vPh0.x)*0.5f + 0.5f, -(vPh0.y)*0.5f + 0.5f );			
					vP0.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth;
					vP0.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;
				
					SVector2 vP1( (vPh1.x)*0.5f + 0.5f, -(vPh1.y)*0.5f + 0.5f );
					vP1.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth;
					vP1.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;

					SVector2 v( vP1 - vP0 );
					float fLSq = SVector2::LengthSq( v );
					if ( fLSq > 1.0f )
					{
						//uint8_t alpha = (uint8_t)(m_pParticles[i].a/fL * 255.0f);
						uint8_t alpha = (uint8_t)(m_pStars[i].a*0.6f * 255.0f);
						CGraphics::GetInstance().DrawLine( vP0, vP1, BGRA8{ 255, 200, 180, alpha } );
					}
					else
					{
						uint8_t alpha = (uint8_t)(m_pStars[i].a*0.6f * 255.0f);
						CGraphics::GetInstance().DrawPixel( (int)vP0.x, (int)vP0.y, BGRA8{ 255, 200, 180, alpha } );
					}
				}
			}
		}
	}

	{
		for ( int i = 0; i < m_iBGStarsCount; i++ )
		{
			SVector4 vPh0;
			SVector4 vPh1;
			{
				SVector4 vPhSrc( m_pBGStars[i].vPos, 1.0f );
				SMatrix::Mul( vPh0, vPhSrc, m_cCameraShip.GetViewProjectionMatrix000() );
				SMatrix::Mul( vPh1, vPhSrc, m_cCameraShip.GetViewProjectionMatrixPrev000() );
			}

			if ( CGraphics::GetInstance().ClipLineDepth( vPh0, vPh1 ) )
			{
				{
					float fWRec0 = 1.0f / vPh0.w;
					vPh0.x = vPh0.x * fWRec0;
					vPh0.y = vPh0.y * fWRec0;
					//vPh0.z = vPh0.z * fWRec0;
					vPh0.w = 1.0f;

					float fWRec1 = 1.0f / vPh1.w;
					vPh1.x = vPh1.x * fWRec1;
					vPh1.y = vPh1.y * fWRec1;
					//vPh1.z = vPh1.z * fWRec1;
					vPh1.w = 1.0f;
				}

				if ( CGraphics::GetInstance().ClipLineXY( vPh0, vPh1 ) )
				{
					SVector2 vP0( (vPh0.x)*0.5f + 0.5f, -(vPh0.y)*0.5f + 0.5f );			
					vP0.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth;
					vP0.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;

					SVector2 vP1( (vPh1.x)*0.5f + 0.5f, -(vPh1.y)*0.5f + 0.5f );
					vP1.x *= (float)CGraphics::GetInstance().GetFrameBuffer().iWidth;
					vP1.y *= (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;

					SVector2 v( vP1 - vP0 );
					float fLSq = SVector2::LengthSq( v );
					if ( fLSq > 1.0f )
					{
						//uint8_t alpha = (uint8_t)(m_pParticles[i].a/fL * 255.0f);
						uint8_t alpha = (uint8_t)(m_pBGStars[i].a*0.4f * 255.0f);
						CGraphics::GetInstance().DrawLine( vP0, vP1, BGRA8{ 255, 200, 180, alpha } );
					}
					else
					{
						uint8_t alpha = (uint8_t)(m_pBGStars[i].a*0.4f * 255.0f);
						CGraphics::GetInstance().DrawPixel( (int)vP0.x, (int)vP0.y, BGRA8{ 255, 200, 180, alpha } );
					}
				}
			}
		}
	}

	{
		/*SMatrix matWorld;
		matWorld.m00 = m_sShip.m_vDir.x;	matWorld.m01 = m_sShip.m_vRight.x;	matWorld.m02 = m_sShip.m_vUp.x;		matWorld.m03 = 0.0f;
		matWorld.m10 = m_sShip.m_vDir.y;	matWorld.m11 = m_sShip.m_vRight.y;	matWorld.m12 = m_sShip.m_vUp.y;		matWorld.m13 = 0.0f;
		matWorld.m20 = m_sShip.m_vDir.z;	matWorld.m21 = m_sShip.m_vRight.z;	matWorld.m22 = m_sShip.m_vUp.z;		matWorld.m23 = 0.0f;
		matWorld.m30 = m_sShip.m_vPos.x;	matWorld.m31 = m_sShip.m_vPos.y;	matWorld.m32 = m_sShip.m_vPos.z;	matWorld.m33 = 1.0f;*/

		SMatrix matWorld;
		m_sShip.GetMatrix( matWorld );

		int iInstCount = 1;
		for ( int iInstIndX = 0; iInstIndX < iInstCount; iInstIndX++ )
		for ( int iInstIndY = 0; iInstIndY < iInstCount; iInstIndY++ )
		for ( int iInstIndZ = 0; iInstIndZ < iInstCount; iInstIndZ++ )
		{
			//SVector3 vPos( (float)(iInstIndX-(float)(iInstCount-1)/2.0f)*10.0f, (float)(iInstIndY-(float)(iInstCount-1)/2.0f)*8.0f, (float)(iInstIndZ-(float)(iInstCount-1)/2.0f)*5.0f );
			//SMatrix matWorld;
			//SMatrix::Identity( matWorld );
			//matWorld.m30 = vPos.x;
			//matWorld.m31 = vPos.y;
			//matWorld.m32 = vPos.z;
			SMatrix matWorldViewProj;
			SMatrix::Mul( matWorldViewProj, matWorld, m_cCameraShip.GetViewProjectionMatrix() );
			float fAlpha = 1.0f;
			//float fAlpha = 10.0f / SVector3::Length( m_sShip.m_vPos );
			//fAlpha = Clamp( fAlpha, 0.0f, 1.0f );
			uint8_t iAlpha = (uint8_t)(fAlpha*255.0f);
			for ( int i = 0; i < m_iLineListSpaceShipCount; i++ )
			{
				CGraphics::GetInstance().DrawLine3D( m_pLineListSpaceShip[i*2+0].vPos, m_pLineListSpaceShip[i*2+1].vPos, matWorldViewProj, BGRA8{ 30, 50, 100, iAlpha } );
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
		m_sShip.Accelerate( 1.0f );
	}
	break;
	case 0x28:
	{
		m_sShip.Accelerate( -1.0f );
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
	if ( CEngine::GetInstance().GetMouseState().bLeftButton )
	{
		m_cCamera.Rotate( (float)deltax * 0.005f, (float)deltay * 0.005f );
	}
	else if ( CEngine::GetInstance().GetMouseState().bRightButton )
	{
		m_cCamera.Pan( SVector2( (float)deltax * 0.005f, (float)deltay * 0.005f ) );
	}
	else
	{
		m_sShip.MoveRight( (float)deltax );
		m_sShip.MoveUp( (float)deltay );		
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