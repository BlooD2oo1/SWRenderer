#include "Scene01.h"
#include "Common/Globals.h"
#include "Engine/Engine.h"

CScene01::CScene01()
{
	m_pParticles = nullptr;
	m_pLineListSpaceShip = nullptr;
	Clear();
}

CScene01::~CScene01()
{
	Clear();
}

void CScene01::Clear()
{
	SAFE_DELETE_ARRAY( m_pParticles );
	m_iParticleCount = 0;
	SAFE_DELETE_ARRAY( m_pLineListSpaceShip );
	m_iLineListSpaceShipCount = 0;
	SMatrix::Identity( m_matWorldSpaceShip );
}

void CScene01::Create()
{
	Clear();

	m_cCamera.SetAspect( (float)CGraphics::GetInstance().GetFrameBuffer().iWidth / (float)CGraphics::GetInstance().GetFrameBuffer().iHeight );

	m_iParticleCount = 10000;
	m_pParticles = new SParticle[m_iParticleCount];
	for ( int i = 0; i < m_iParticleCount; i++ )
	{
		/*float fU = ((float)rand()/(float)RAND_MAX)*PI2;
		float fV = ((float)rand()/(float)RAND_MAX)*2.0f-1.0f;
		float fR = sqrtf( 1.0f - fV*fV );
		m_pParticles[i].vPos.x = cosf(fU)*fR;
		m_pParticles[i].vPos.y = sinf(fU)*fR;
		m_pParticles[i].vPos.z = fV;
		m_pParticles[i].vPos *= ( powf( (float)rand()/(float)RAND_MAX, 1.0f/3.0f ) ) * 3000.0f;*/

		m_pParticles[i].vPos.x = ((float)rand()/(float)RAND_MAX);
		m_pParticles[i].vPos.y = ((float)rand()/(float)RAND_MAX);
		m_pParticles[i].vPos.z = ((float)rand()/(float)RAND_MAX);

		m_pParticles[i].a = ((float)rand()/(float)RAND_MAX) * 0.55f + 0.45f;
		m_pParticles[i].a *= m_pParticles[i].a;
		m_pParticles[i].a *= m_pParticles[i].a;
		m_pParticles[i].a *= 0.3f;
	}

	{
		{
			std::vector<SVector3> pts;
			std::vector<std::pair<int,int>> edges;

			auto AddPoint =
				[&](float x, float y, float z) -> int
				{
					pts.emplace_back(x, y, z);
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
	m_cCamera.Update( CEngine::GetInstance().GetElapsedTimeMs() );
}

void CScene01::Render()
{
	// draw particles
	{
		float fStarBoxSize = 10000.0f;
		float fStarBoxSizeInv = 1.0f / fStarBoxSize;
		for ( int i = 0; i < m_iParticleCount; i++ )
		{
			SVector4 vPh0;
			SVector4 vPh1;
			{
				SVector3 v( m_pParticles[i].vPos * fStarBoxSize );
				v.x = v.x - floorf((v.x - m_cCamera.GetEye().x) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
				v.y = v.y - floorf((v.y - m_cCamera.GetEye().y) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
				v.z = v.z - floorf((v.z - m_cCamera.GetEye().z) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
				SVector4 vPhSrc( v, 1.0f );
				SMatrix::Mul( vPh0, vPhSrc, m_cCamera.GetViewProjectionMatrix() );			
				SMatrix::Mul( vPh1, vPhSrc, m_cCamera.GetViewProjectionMatrixPrev() );
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
						uint8_t alpha = (uint8_t)(m_pParticles[i].a * 255.0f);
						CGraphics::GetInstance().DrawLine( vP0, vP1, BGRA8{ 255, 200, 180, alpha } );
					}
					else
					{
						uint8_t alpha = (uint8_t)(m_pParticles[i].a * 255.0f);
						CGraphics::GetInstance().DrawPixel( (int)vP0.x, (int)vP0.y, BGRA8{ 255, 200, 180, alpha } );
					}
				}
			}
		}
	}

	{
		int iInstCount = 1;
		for ( int iInstIndX = 0; iInstIndX < iInstCount; iInstIndX++ )
		for ( int iInstIndY = 0; iInstIndY < iInstCount; iInstIndY++ )
		for ( int iInstIndZ = 0; iInstIndZ < iInstCount; iInstIndZ++ )
		{
			SVector3 vPos( (float)(iInstIndX-(float)(iInstCount-1)/2.0f)*10.0f, (float)(iInstIndY-(float)(iInstCount-1)/2.0f)*8.0f, (float)(iInstIndZ-(float)(iInstCount-1)/2.0f)*5.0f );
			SMatrix matWorld;
			SMatrix::Identity( matWorld );
			matWorld.m30 = vPos.x;
			matWorld.m31 = vPos.y;
			matWorld.m32 = vPos.z;
			SMatrix matWorldViewProj;
			SMatrix::Mul( matWorldViewProj, matWorld, m_cCamera.GetViewProjectionMatrix() );
			float fAlpha = 10.0f / SVector3::Length( vPos );
			fAlpha = Clamp( fAlpha, 0.0f, 1.0f );
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