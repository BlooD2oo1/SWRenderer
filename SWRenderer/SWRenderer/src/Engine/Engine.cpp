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