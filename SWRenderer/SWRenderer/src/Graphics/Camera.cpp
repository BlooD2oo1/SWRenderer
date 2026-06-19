#include "Camera.h"
#include "Graphics/Math.h"

CCameraFree::CCameraFree()
{
	m_fAngleXY = PI;
	m_fAngleZ = 0.3f;
	m_fDist = 10.0f;
	m_fFOVY = 70.0f / 180.0f * PI;
	m_vEye = SVector3( -1.0f, 0.0f, 0.0f );
	m_vEyeSmooth = SVector3( -1.0f, 0.0f, 0.0f );
	m_vLookAt = SVector3( 0.0f, 0.0f, 0.0f );
	m_vLookAtSmooth = SVector3( 0.0f, 0.0f, 0.0f );
	m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
	m_fAspect = 1.0f;
	m_fNear = 0.1f;
	m_fFar = 100000.0f;

	SMatrix::Identity( m_matView );
	SMatrix::Identity( m_matProj );
	SMatrix::Identity( m_matViewProj );
	SMatrix::Identity( m_matViewProjPrev );
}

CCameraFree::~CCameraFree()
{
}

void CCameraFree::Rotate( float fIncAngleXY, float fIncAngleZ )
{
	m_fAngleXY += fIncAngleXY;
	m_fAngleZ += fIncAngleZ;
	m_fAngleZ = Clamp( m_fAngleZ, -PI05*0.999f, PI05*0.999f );
	m_fAngleXY = fmodf( m_fAngleXY, PI2 );
}

void CCameraFree::Pan( const SVector2& vMov )
{
	UpdateEye();

	SVector3 vDir = m_vLookAt - m_vEye;

	SVector3 vLeft;
	SVector3::Cross( vLeft, m_vUp, vDir );
	SVector3::Normalize( vLeft, vLeft );
	
	SVector3 vUp;
	SVector3::Cross( vUp, vDir, vLeft );
	SVector3::Normalize( vUp, vUp );

	vLeft *= vMov.x * m_fDist;
	vUp *= vMov.y * m_fDist;

	m_vLookAt += vLeft;
	m_vLookAt += vUp;
}

void CCameraFree::Zoom( float fValue )
{
	m_fDist *= expf( -fValue );
}

void CCameraFree::UpdateEye()
{
	m_vEye = SVector3(
		m_fDist * sinf( m_fAngleXY ) * cosf( m_fAngleZ ),
		m_fDist * cosf( m_fAngleXY ) * cosf( m_fAngleZ ),
		m_fDist * sinf( m_fAngleZ )
	);

	m_vEye += m_vLookAt;
}

void CCameraFree::Update( float fElapsedTimeMs )
{
	UpdateEye();

	m_matViewProjPrev = m_matViewProj;

	float fW = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
	m_vEyeSmooth = Lerp( m_vEye, m_vEyeSmooth, fW );
	m_vLookAtSmooth = Lerp( m_vLookAt, m_vLookAtSmooth, fW );
	
	SMatrix::BuildViewMatrix( m_matView, m_vEyeSmooth, m_vLookAtSmooth, m_vUp );
	SMatrix::BuildProjectionMatrix( m_matProj, m_fFOVY, m_fAspect, m_fNear, m_fFar );
	SMatrix::Mul( m_matViewProj, m_matView, m_matProj );
}