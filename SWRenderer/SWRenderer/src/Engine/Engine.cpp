#include "Engine.h"

CEngine* CEngine::m_pThis = nullptr;

CEngine::CEngine()
	: m_sFrameBuffer( nullptr, 0, 0 )
{
	m_pParticles = nullptr;
	Clear();
}

CEngine::~CEngine()
{
	m_sFrameBuffer = SFrameBuffer( nullptr, 0, 0 );
	Clear();
}

void CEngine::Clear()
{
	m_iFrameCount = 0;
	SAFE_DELETE_ARRAY( m_pParticles );
	m_iParticleCount = 0;
}

void CEngine::Create( SFrameBuffer& sFrameBuffer )
{
	Clear();
	m_sFrameBuffer = sFrameBuffer;
	
	m_iParticleCount = 0;
	m_pParticles = new SParticle[m_iParticleCount];
	for ( int i = 0; i < m_iParticleCount; i++ )
	{
		m_pParticles[i].vPos.x = (float)( rand()%m_sFrameBuffer.iWidth );
		m_pParticles[i].vPos.y = (float)( rand()%m_sFrameBuffer.iHeight );
		m_pParticles[i].vMov.x = (((float)( rand()%1000 )/1000.0f)-0.5f)*0.2f;
		m_pParticles[i].vMov.y = (((float)( rand()%1000 )/1000.0f)-0.5f)*0.2f;
		m_pParticles[i].a = (float)( rand()%1024 ) / 1024.0f;
	}
}

void CEngine::Update()
{
	float force = 0.0001f;
	float drag = 0.999f;
	if ( m_sMouseState.bLeftButton )
	{
		force = 0.001f;
		drag = 0.8f;
	}
	for ( int i = 0; i < m_iParticleCount; i++ )
	{
		float dx = m_pParticles[i].vPos.x - (float)m_sMouseState.x;
		float dy = m_pParticles[i].vPos.y - (float)m_sMouseState.y;
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

void CEngine::Render()
{
	// clear framebuffer
	memset( m_sFrameBuffer.pData, 0, m_sFrameBuffer.iWidth * m_sFrameBuffer.iHeight * sizeof( uint32_t ) );

	// draw particles
	for ( int i = 0; i < m_iParticleCount; i++ )
	{
		uint8_t alpha = (uint8_t)(m_pParticles[i].a * 255.0f);
		//int x = (int)m_pParticles[i].vPos.x;
		//int y = (int)m_pParticles[i].vPos.y;		
		//DrawPixel( m_sFrameBuffer, x, y, RGBA8{ alpha, alpha, alpha, 255 } );
		DrawPixelAA( m_sFrameBuffer, m_pParticles[i].vPos, RGBA8{ alpha, alpha, alpha, 255 } );
	}

	const uint16_t iLineCount = 8;
	for ( int i = 0; i < iLineCount; i++ )
	{
		float fW = (float)i/(float)iLineCount;
		SVector2 v0( (float)(m_sFrameBuffer.iWidth>>1), (float)(m_sFrameBuffer.iHeight>>1) );
		SVector2 v1( v0 );

		float a = (float)m_iFrameCount*0.0001f;
		a = m_sMouseState.x*0.002f;
		float x = cosf( a + (float)fW*PI2 );
		float y = sinf( a + (float)fW*PI2 );
		v0.x += x * -18.0f;
		v0.y += y * -18.0f;
		v1.x += x * 120.0f;
		v1.y += y * 120.0f;

		DrawPixel( m_sFrameBuffer, (int)(v0.x), (int)(v0.y), RGBA8{ 20, 100, 22, 255 } );
		DrawPixel( m_sFrameBuffer, (int)(v1.x), (int)(v1.y), RGBA8{ 20, 100, 22, 255 } );
		//DrawPixelAA( m_sFrameBuffer, v0, RGBA8{ 200, 10, 127, 255 } );
		//DrawPixelAA( m_sFrameBuffer, v1, RGBA8{ 200, 100, 127, 255 } );

		DrawLine( m_sFrameBuffer, v0, v1, RGBA8{ 200, 0, 127, 255 } );
	}

	DrawLine( m_sFrameBuffer, SVector2( 10.0, 10.0 ), SVector2( (float)m_sMouseState.x, (float)m_sMouseState.y ), RGBA8{ 200, 0, 127, 255 } );

	// draw mouse cursor
	/*if ( m_sMouseState.x >= 0 && m_sMouseState.x < m_sFrameBuffer.iWidth &&
		m_sMouseState.y >= 0 && m_sMouseState.y < m_sFrameBuffer.iHeight )
	{
		m_sFrameBuffer.pData[m_sMouseState.y * m_sFrameBuffer.iWidth + m_sMouseState.x] = 0xFFFF0000;
	}*/
	DrawPixel( m_sFrameBuffer, m_sMouseState.x, m_sMouseState.y, RGBA8{ 255, 0, 0, 255 } );

	m_iFrameCount++;
}

bool CEngine::On_KeyDown( uint32_t key )
{
    return false;
}

bool CEngine::On_KeyUp( uint32_t key )
{
    return false;
}

bool CEngine::On_MouseMove( int x, int y )
{
	m_sMouseState.x = x;
	m_sMouseState.y = y;
    return false;
}
bool CEngine::On_MouseButtonDown( uint32_t button )
{
	switch ( button )
	{
	case 0:
	m_sMouseState.bLeftButton = true;
	break;
    case 1:
	m_sMouseState.bRightButton = true;
	break;
	case 2:
	m_sMouseState.bMiddleButton = true;
	break;
	}

    return false;
}

bool CEngine::On_MouseButtonUp( uint32_t button )
{
	switch ( button )
	{
	case 0:
	m_sMouseState.bLeftButton = false;
	break;
    case 1:
	m_sMouseState.bRightButton = false;
	break;
	case 2:
	m_sMouseState.bMiddleButton = false;
	break;
	}

    return false;
}