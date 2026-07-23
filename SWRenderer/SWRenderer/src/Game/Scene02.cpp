#include "Scene02.h"
#include "Common/Globals.h"
#include "Engine/Engine.h"
#include "winAudio.h"

CScene02::CScene02()
{
	Clear();
}

CScene02::~CScene02()
{
	Clear();
}

void CScene02::Clear()
{
	m_sCamera.Clear();
	m_sShipControl.Clear();
	m_cShipMesh.Clear();
}

void CScene02::Create()
{
	Clear();
	m_cShipMesh.Create();

	m_sCamera.m_fAspect = (float)CGraphics::GetInstance().GetFrameBuffer().iWidth / (float)CGraphics::GetInstance().GetFrameBuffer().iHeight;
}

void CScene02::Update()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();
	{
		// Update ship:

		m_sShipControl.m_fYaw += m_sShipControl.m_fYawVel * fElapsedTimeMs;
		m_sShipControl.m_fSpeed += m_sShipControl.m_fSpeedVel * fElapsedTimeMs;

		SVector3 vMoveDir( cosf( m_sShipControl.m_fSpeedDirAngle ), sinf( m_sShipControl.m_fSpeedDirAngle ), 0.0f );
		vMoveDir *= m_sShipControl.m_fSpeed;
		m_sShipControl.m_vPos += vMoveDir * fElapsedTimeMs;

		m_sShipControl.UpdateMatrices();

	}

	{
		// Update camera:
		m_sCamera.UpdateMatrices();
	}
}

void CScene02::Render()
{
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
}

bool CScene02::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
	// VK_UP
	case 0x26:
	{
		m_sShipControl.m_fSpeedVel = 0.001f;
	}
	break;
	// VK_DOWN
	case 0x28:
	{
		m_sShipControl.m_fSpeedVel = -0.001f;
	}
	break;
	// VK_LEFT
	case 0x25:
	{
		m_sShipControl.m_fYawVel = -0.001f;
	}
	break;
	// VK_RIGHT
	case 0x27:
	{
		m_sShipControl.m_fYawVel = 0.001f;
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
		m_sShipControl.m_fSpeedVel = 0.0f;
	}
	break;
	// VK_DOWN
	case 0x28:
	{
		m_sShipControl.m_fSpeedVel = 0.0f;
	}
	break;
	// VK_LEFT
	case 0x25:
	{
		m_sShipControl.m_fYawVel = 0.0f;
	}
	break;
	// VK_RIGHT
	case 0x27:
	{
		m_sShipControl.m_fYawVel = 0.0f;
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