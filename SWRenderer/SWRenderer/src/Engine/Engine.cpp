#include "Engine.h"

CEngine* CEngine::m_pThis = nullptr;

CEngine::CEngine()
	: m_sFrameBuffer( nullptr, 0, 0 )
{
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
	m_cScene01.Clear();
}

void CEngine::Create( SFrameBuffer& sFrameBuffer )
{
	Clear();
	m_sFrameBuffer = sFrameBuffer;
	m_cScene01.Create();
}

void CEngine::Update()
{
	m_cScene01.Update();
}

void CEngine::Render()
{
	// clear framebuffer
	memset( m_sFrameBuffer.pData, 0, m_sFrameBuffer.iWidth * m_sFrameBuffer.iHeight * sizeof( uint32_t ) );

	m_cScene01.Render();
	
	/*const uint16_t iLineCount = 16;
	for ( int i = 0; i < iLineCount; i++ )
	{
		float fW = (float)i/(float)iLineCount;
		SVector2 v0( (float)(GetFrameBuffer().iWidth>>1), (float)(GetFrameBuffer().iHeight>>1) );
		SVector2 v1( v0 );

		float a = (float)GetFrameCount()*0.0001f;
		a = GetMouseState().x*0.002f;
		float x = cosf( a + (float)fW*PI2 );
		float y = sinf( a + (float)fW*PI2 );
		v0.x += x * 18.0f;
		v0.y += y * 18.0f;
		v1.x += x * 60.0f;
		v1.y += y * 60.0f;

		//DrawPixel( GetFrameBuffer(), (int)(v0.x), (int)(v0.y), RGBA8{ 20, 100, 22, 255 } );
		//DrawPixel( GetFrameBuffer(), (int)(v1.x), (int)(v1.y), RGBA8{ 20, 100, 22, 255 } );
		//DrawPixelAA( GetFrameBuffer(), v0, RGBA8{ 200, 10, 127, 255 } );
		//DrawPixelAA( GetFrameBuffer(), v1, RGBA8{ 200, 100, 127, 255 } );

		DrawLine( GetFrameBuffer(), v0, v1, BGRA8{ 64, 43, 0, 255 } );

		DrawLine( GetFrameBuffer(), v1, SVector2( (float)GetMouseState().x, (float)GetMouseState().y ), BGRA8{ 32, 0, 64, 255 } );
	}*/

	DrawPixel( GetFrameBuffer(), GetMouseState().x, GetMouseState().y, BGRA8{ 255, 255, 255, 255 } );

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

bool CEngine::On_MouseMove( int deltax, int deltay )
{
	m_sMouseState.x += deltax;
	m_sMouseState.y += deltay;
	m_sMouseState.x = Clamp( m_sMouseState.x, 0, m_sFrameBuffer.iWidth-1 );
	m_sMouseState.y = Clamp( m_sMouseState.y, 0, m_sFrameBuffer.iHeight-1 );
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