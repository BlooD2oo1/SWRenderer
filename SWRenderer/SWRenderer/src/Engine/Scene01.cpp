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
}

void CScene01::Create()
{
	Clear();
	m_iParticleCount = 10;
	m_pParticles = new SParticle[m_iParticleCount];
	for ( int i = 0; i < m_iParticleCount; i++ )
	{
		m_pParticles[i].vPos.x = (float)( rand()%CEngine::GetInstance().GetFrameBuffer().iWidth );
		m_pParticles[i].vPos.y = (float)( rand()%CEngine::GetInstance().GetFrameBuffer().iHeight );
		m_pParticles[i].vMov.x = (((float)( rand()%1000 )/1000.0f)-0.5f)*0.2f;
		m_pParticles[i].vMov.y = (((float)( rand()%1000 )/1000.0f)-0.5f)*0.2f;
		m_pParticles[i].a = (float)( rand()%1024 ) / 1024.0f;
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
	float force = 0.0001f;
	float drag = 0.999f;
	if ( CEngine::GetInstance().GetMouseState().bLeftButton )
	{
		force = 0.001f;
		drag = 0.8f;
	}
	for ( int i = 0; i < m_iParticleCount; i++ )
	{
		float dx = m_pParticles[i].vPos.x - (float)CEngine::GetInstance().GetMouseState().x;
		float dy = m_pParticles[i].vPos.y - (float)CEngine::GetInstance().GetMouseState().y;
		float dist = sqrtf( dx * dx + dy * dy );
		dist += 0.000001f;

		m_pParticles[i].vMov.x -= (dx / dist) * force;
		m_pParticles[i].vMov.y -= (dy / dist) * force;

		m_pParticles[i].vMov.x *= drag;
		m_pParticles[i].vMov.y *= drag;

		m_pParticles[i].vPos.x += m_pParticles[i].vMov.x;
		m_pParticles[i].vPos.y += m_pParticles[i].vMov.y;		
	}
}

void CScene01::Render()
{
	// draw particles
	/*for ( int i = 0; i < m_iParticleCount; i++ )
	{
		uint8_t alpha = (uint8_t)(m_pParticles[i].a * 255.0f);
		int x = (int)m_pParticles[i].vPos.x;
		int y = (int)m_pParticles[i].vPos.y;		
		DrawPixel( CEngine::GetInstance().GetFrameBuffer(), x, y, BGRA8{ 128, 64, 32, alpha } );
		//DrawPixelAA( CEngine::GetInstance().GetFrameBuffer(), m_pParticles[i].vPos, RGBA8{ alpha, alpha, alpha, 255 } );
	}

	const uint16_t iLineCount = 16;
	for ( int i = 0; i < iLineCount; i++ )
	{
		float fW = (float)i/(float)iLineCount;
		SVector2 v0( (float)(CEngine::GetInstance().GetFrameBuffer().iWidth>>1), (float)(CEngine::GetInstance().GetFrameBuffer().iHeight>>1) );
		SVector2 v1( v0 );

		float a = (float)CEngine::GetInstance().GetFrameCount()*0.0001f;
		a = CEngine::GetInstance().GetMouseState().x*0.002f;
		float x = cosf( a + (float)fW*PI2 );
		float y = sinf( a + (float)fW*PI2 );
		v0.x += x * 18.0f;
		v0.y += y * 18.0f;
		v1.x += x * 60.0f;
		v1.y += y * 60.0f;

		//DrawPixel( CEngine::GetInstance().GetFrameBuffer(), (int)(v0.x), (int)(v0.y), RGBA8{ 20, 100, 22, 255 } );
		//DrawPixel( CEngine::GetInstance().GetFrameBuffer(), (int)(v1.x), (int)(v1.y), RGBA8{ 20, 100, 22, 255 } );
		//DrawPixelAA( CEngine::GetInstance().GetFrameBuffer(), v0, RGBA8{ 200, 10, 127, 255 } );
		//DrawPixelAA( CEngine::GetInstance().GetFrameBuffer(), v1, RGBA8{ 200, 100, 127, 255 } );

		DrawLine( CEngine::GetInstance().GetFrameBuffer(), v0, v1, BGRA8{ 64, 43, 0, 0 } );

		DrawLine( CEngine::GetInstance().GetFrameBuffer(), v1, SVector2( (float)CEngine::GetInstance().GetMouseState().x, (float)CEngine::GetInstance().GetMouseState().y ), BGRA8{ 32, 0, 64, 0 } );
	}*/

	//DrawLine( CEngine::GetInstance().GetFrameBuffer(), SVector2( 100.0, 100.0 ), SVector2( (float)m_sMouseState.x, (float)m_sMouseState.y ), RGBA8{ 200, 0, 0, 255 } );

	//DrawPixel( CEngine::GetInstance().GetFrameBuffer(), m_sMouseState.x, m_sMouseState.y, RGBA8{ 255, 0, 0, 255 } );

	{
		float fTime = (float)CEngine::GetInstance().GetFrameCount() * 0.002f;
		SVector3 vEye( 200.0f*cosf( fTime ), 200.0f * sinf( fTime ), 50.0f );
		SVector3 vLookAt( 0.0f, 0.0f, 0.0f );
		SVector3 vUp( 0.0f, 0.0f, 1.0f );

		float fFOVY = 90.0f / 180.0f * PI;
		float fAspect = (float)CEngine::GetInstance().GetFrameBuffer().iWidth / (float)CEngine::GetInstance().GetFrameBuffer().iHeight;
		float fNear = 0.01f;
		float fFar = 1000.0f;

		SMatrix matView;
		SMatrix matProj;

		SMatrix::BuildViewMatrix( matView, vEye, vLookAt, vUp );
		SMatrix::BuildProjectionMatrix( matProj, fFOVY, fAspect, fNear, fFar );

		SMatrix matViewProj;

		SMatrix::Mul( matViewProj, matView, matProj );

		for ( int i = 0; i < m_iLineListSpaceShipCount; i++ )
		{
			SVector4 vPhSrc0( m_pLineListSpaceShip[i*2+0].vPos.x, m_pLineListSpaceShip[i*2+0].vPos.y, m_pLineListSpaceShip[i*2+0].vPos.z, 1.0f );
			SVector4 vPhSrc1( m_pLineListSpaceShip[i*2+1].vPos.x, m_pLineListSpaceShip[i*2+1].vPos.y, m_pLineListSpaceShip[i*2+1].vPos.z, 1.0f );
			SVector4 vPh0;
			SVector4 vPh1;

			SMatrix::Mul( vPh0, vPhSrc0, matViewProj );
			SMatrix::Mul( vPh1, vPhSrc1, matViewProj );

			SVector3 vP0( vPh0.x, vPh0.y, vPh0.z );
			SVector3 vP1( vPh1.x, vPh1.y, vPh1.z );

			SVector3::Mul( vP0, vP0, 1.0f / vPh0.w );
			SVector3::Mul( vP1, vP1, 1.0f / vPh1.w );

			SVector2 vP20( vP0.x, vP0.y );
			SVector2 vP21( vP1.x, vP1.y );

			vP20.x = vP20.x*0.5f + 0.5f;
			vP20.y = -vP20.y*0.5f + 0.5f;
			vP20.x *= (float)CEngine::GetInstance().GetFrameBuffer().iWidth;
			vP20.y *= (float)CEngine::GetInstance().GetFrameBuffer().iHeight;

			vP21.x = vP21.x*0.5f + 0.5f;
			vP21.y = -vP21.y*0.5f + 0.5f;
			vP21.x *= (float)CEngine::GetInstance().GetFrameBuffer().iWidth;			
			vP21.y *= (float)CEngine::GetInstance().GetFrameBuffer().iHeight;

			DrawLine( CEngine::GetInstance().GetFrameBuffer(), vP20, vP21, BGRA8{ 100, 50, 30, 255 } );
		}
	}
}