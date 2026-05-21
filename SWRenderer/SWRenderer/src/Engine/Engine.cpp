#include "Common/Globals.h"
#include "Engine.h"

CEngine* CEngine::m_pThis = nullptr;

CEngine::CEngine()
	: m_sFrameBuffer( nullptr, 0, 0 )
{
	m_pParticles = nullptr;
	m_iParticleCount = 0;
}

CEngine::~CEngine()
{
	m_sFrameBuffer = SFrameBuffer( nullptr, 0, 0 );
	Clear();
}

void CEngine::Clear()
{
	SAFE_DELETE_ARRAY( m_pParticles );
	m_iParticleCount = 0;
}

void CEngine::Create( SFrameBuffer& sFrameBuffer )
{
	Clear();
	m_sFrameBuffer = sFrameBuffer;
	
	m_iParticleCount = 10000;
	m_pParticles = new SParticle[m_iParticleCount];
	for ( int i = 0; i < m_iParticleCount; i++ )
	{
		m_pParticles[i].x = (float)( rand()%m_sFrameBuffer.iWidth );
		m_pParticles[i].y = (float)( rand()%m_sFrameBuffer.iWidth );
		m_pParticles[i].xm = ((float)( rand()%1000 )/1000.0f)-0.5f;
		m_pParticles[i].ym = ((float)( rand()%1000 )/1000.0f)-0.5f;
		m_pParticles[i].a = (float)( rand()%1024 ) / 1024.0f;
	}
}

uint32_t BlendAdditive( uint32_t dest, uint32_t src )
{
	uint8_t rDest = (dest >> 16) & 0xFF;
	uint8_t gDest = (dest >> 8) & 0xFF;
	uint8_t bDest = dest & 0xFF;
	uint8_t rSrc = (src >> 16) & 0xFF;
	uint8_t gSrc = (src >> 8) & 0xFF;
	uint8_t bSrc = src & 0xFF;
	uint8_t rOut = std::min( rDest + rSrc, 255 );
	uint8_t gOut = std::min( gDest + gSrc, 255 );
	uint8_t bOut = std::min( bDest + bSrc, 255 );
	return (bOut) | (gOut << 8) | (rOut << 16);
}

void CEngine::Update()
{
    for (int y = 0; y < m_sFrameBuffer.iHeight; y++)
    {
        for (int x = 0; x < m_sFrameBuffer.iWidth; x++)
        {
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

			m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x] = (b) | (g << 8) | (r << 16);
        }
    }

	//rotate particles arount the center of the screen
	for ( int i = 0; i < m_iParticleCount; i++ )
	{
		float dx = m_pParticles[i].x - (float)m_sMouseState.x;
		float dy = m_pParticles[i].y - (float)m_sMouseState.y;
		float dist = sqrtf( dx * dx + dy * dy );
		dist += 0.001f;
		m_pParticles[i].xm -= (dx / dist) * 0.001f;
		m_pParticles[i].ym -= (dy / dist) * 0.001f;

		m_pParticles[i].xm *= 0.9999f;
		m_pParticles[i].ym *= 0.9999f;

		m_pParticles[i].x += m_pParticles[i].xm;
		m_pParticles[i].y += m_pParticles[i].ym;		
	}

	for ( int i = 0; i < m_iParticleCount; i++ )
	{
		int x = (int)m_pParticles[i].x;
		int y = (int)m_pParticles[i].y;
		if ( x >= 0 && x < m_sFrameBuffer.iWidth-1 &&
			 y >= 0 && y < m_sFrameBuffer.iHeight-1 )
		{
			float fxmod = m_pParticles[i].x - (float)x;
			float fymod = m_pParticles[i].y - (float)y;
			float fxmodinv = 1.0f - fxmod;
			float fymodinv = 1.0f - fymod;
			uint8_t i00 = (uint8_t)( sqrtf( fxmodinv*fymodinv ) * m_pParticles[i].a * 255.0f );
			uint8_t i01 = (uint8_t)( sqrtf( fxmodinv*fymod ) * m_pParticles[i].a * 255.0f );
			uint8_t i10 = (uint8_t)( sqrtf( fxmod * fymodinv ) * m_pParticles[i].a * 255.0f);
			uint8_t i11 = (uint8_t)( sqrtf( fxmod * fymod ) * m_pParticles[i].a * 255.0f);

			//m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x] = i00 | i00 << 8 | i00 << 16;
			//m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x + 1] = i10 | i10 << 8 | i10 << 16;
			//m_sFrameBuffer.pData[(y + 1) * m_sFrameBuffer.iWidth + x] = i01 | i01 << 8 | i01 << 16;
			//m_sFrameBuffer.pData[(y + 1) * m_sFrameBuffer.iWidth + x + 1] = i11 | i11 << 8 | i11 << 16;
			
			m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x] = BlendAdditive( m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x], i00 | i00>>1 << 8 | i00>>2 << 16 );
			m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x + 1] = BlendAdditive( m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x + 1], i10 | i10>>1 << 8 | i10>>2 << 16 );
			m_sFrameBuffer.pData[(y + 1) * m_sFrameBuffer.iWidth + x] = BlendAdditive( m_sFrameBuffer.pData[(y + 1) * m_sFrameBuffer.iWidth + x], i01 | i01>>1 << 8 | i01>>2 << 16 );
			m_sFrameBuffer.pData[(y + 1) * m_sFrameBuffer.iWidth + x + 1] = BlendAdditive( m_sFrameBuffer.pData[(y + 1) * m_sFrameBuffer.iWidth + x + 1], i11 | i11>>1 << 8 | i11>>2 << 16 );
		}
	}

	if ( m_sMouseState.x >= 0 && m_sMouseState.x < m_sFrameBuffer.iWidth &&
		 m_sMouseState.y >= 0 && m_sMouseState.y < m_sFrameBuffer.iHeight )
	{
		m_sFrameBuffer.pData[m_sMouseState.y * m_sFrameBuffer.iWidth + m_sMouseState.x] = 0xFFFF0000;
	}
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