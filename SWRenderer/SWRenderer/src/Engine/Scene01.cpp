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
		SVector3 p[] =
		{
			// ==========================
			// Nose / front
			// ==========================
			SVector3( 0.0f,  2.8f,  0.0f), // 0 nose

			SVector3(-0.25f, 1.8f, -0.18f), // 1
			SVector3( 0.25f, 1.8f, -0.18f), // 2
			SVector3( 0.25f, 1.8f,  0.18f), // 3
			SVector3(-0.25f, 1.8f,  0.18f), // 4

			// ==========================
			// Mid fuselage
			// ==========================
			SVector3(-0.45f,  0.4f, -0.25f), // 5
			SVector3( 0.45f,  0.4f, -0.25f), // 6
			SVector3( 0.45f,  0.4f,  0.25f), // 7
			SVector3(-0.45f,  0.4f,  0.25f), // 8

			// ==========================
			// Rear fuselage
			// ==========================
			SVector3(-0.35f, -1.8f, -0.2f), // 9
			SVector3( 0.35f, -1.8f, -0.2f), // 10
			SVector3( 0.35f, -1.8f,  0.2f), // 11
			SVector3(-0.35f, -1.8f,  0.2f), // 12

			// ==========================
			// Main wings
			// ==========================
			SVector3(-2.2f, -0.2f, 0.0f), // 13 left tip
			SVector3( 2.2f, -0.2f, 0.0f), // 14 right tip

			SVector3(-1.3f, -1.1f, 0.0f), // 15
			SVector3( 1.3f, -1.1f, 0.0f), // 16

			SVector3(-1.4f,  0.8f, 0.0f), // 17
			SVector3( 1.4f,  0.8f, 0.0f), // 18

			// ==========================
			// Vertical fins (Z up!)
			// ==========================
			SVector3(-0.35f, -1.1f, 0.9f), // 19
			SVector3( 0.35f, -1.1f, 0.9f), // 20

			// ==========================
			// Cockpit ridge
			// ==========================
			SVector3(0.0f, 1.2f, 0.38f), // 21
			SVector3(0.0f, 0.3f, 0.48f), // 22
		};

		const int edges[][2] =
		{
			// =====================
			// Fuselage
			// =====================
			{0,1},{0,2},{0,3},{0,4},

			{1,2},{2,3},{3,4},{4,1},

			{1,5},{2,6},{3,7},{4,8},

			{5,6},{6,7},{7,8},{8,5},

			{5,9},{6,10},{7,11},{8,12},

			{9,10},{10,11},{11,12},{12,9},

			// =====================
			// Wings
			// =====================
			{5,13},
			{8,13},
			{13,15},
			{15,9},

			{5,17},
			{13,17},

			{6,14},
			{7,14},
			{14,16},
			{16,10},

			{6,18},
			{14,18},

			// rear wing support
			{15,16},

			// =====================
			// Vertical fins
			// =====================
			{9,19},
			{12,19},

			{10,20},
			{11,20},

			{19,20},

			// =====================
			// Cockpit
			// =====================
			{0,21},
			{21,22},
			{22,7},
			{22,8},
		};

		const int edgeCount =
			sizeof(edges) / sizeof(edges[0]);

		m_iLineListSpaceShipCount = edgeCount;

		delete[] m_pLineListSpaceShip;
		m_pLineListSpaceShip =
			new SVertex[edgeCount * 2];

		for (int e = 0; e < edgeCount; ++e)
		{
			const int i0 = edges[e][0];
			const int i1 = edges[e][1];

			m_pLineListSpaceShip[e * 2 + 0].vPos = p[i0];
			m_pLineListSpaceShip[e * 2 + 1].vPos = p[i1];
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
		float fTime = (float)CEngine::GetInstance().GetFrameCount() * 0.001f;
		SVector3 vEye( 250.0f*cosf( fTime ), 250.0f * sinf( fTime ), 100.0f );
		SVector3 vLookAt( 0.0f, 0.0f, 0.0f );
		SVector3 vUp( 0.0f, 0.0f, 1.0f );

		float fFOVY = 45.0f / 180.0f * PI;
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
			vP20.y = vP20.y*0.5f + 0.5f;
			vP20.x *= (float)CEngine::GetInstance().GetFrameBuffer().iWidth;
			vP20.y *= (float)CEngine::GetInstance().GetFrameBuffer().iHeight;

			vP21.x = vP21.x*0.5f + 0.5f;
			vP21.y = vP21.y*0.5f + 0.5f;
			vP21.x *= (float)CEngine::GetInstance().GetFrameBuffer().iWidth;			
			vP21.y *= (float)CEngine::GetInstance().GetFrameBuffer().iHeight;

			DrawLine( CEngine::GetInstance().GetFrameBuffer(), vP20, vP21, BGRA8{ 100, 50, 30, 255 } );
		}
	}
}