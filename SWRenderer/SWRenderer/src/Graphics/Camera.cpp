#include "Camera.h"
#include "Graphics/Math.h"

CCameraFree::CCameraFree()
{
	m_fAngleXY = -PI05;
	m_fAngleZ = 0.3f;
	m_fDist = 10.0f;
	m_fFOVY = 70.0f / 180.0f * PI;
	m_vLookAt = SVector3( 0.0f, 0.0f, 0.0f );
	m_vLookAtSmooth = SVector3( 0.0f, 0.0f, 0.0f );
	m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
	m_fAspect = 1.0f;
	m_fNear = 0.1f;
	m_fFar = 100000.0f;

	UpdateEye();
	m_vEyeSmooth = m_vEye;

	SMatrix::Identity( m_matView );
	SMatrix::Identity( m_matProj );
	SMatrix::Identity( m_matViewProj );
	SMatrix::Identity( m_matViewProjPrev );

	SMatrix::Identity( m_matView000 );
	SMatrix::Identity( m_matViewProj000 );
	SMatrix::Identity( m_matViewProjPrev000 );
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
	m_matViewProjPrev000 = m_matViewProj000;

	float fW = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
	m_vEyeSmooth = Lerp( m_vEye, m_vEyeSmooth, fW );
	m_vLookAtSmooth = Lerp( m_vLookAt, m_vLookAtSmooth, fW );
	
	SMatrix::BuildViewMatrix( m_matView, m_vEyeSmooth, m_vLookAtSmooth, m_vUp );
	SMatrix::BuildProjectionMatrix( m_matProj, m_fFOVY, m_fAspect, m_fNear, m_fFar );
	SMatrix::Mul( m_matViewProj, m_matView, m_matProj );

	m_matView000 = m_matView;
	m_matView000.m30 = 0.0f;
	m_matView000.m31 = 0.0f;
	m_matView000.m32 = 0.0f;
	SMatrix::Mul( m_matViewProj000, m_matView000, m_matProj );
}

CCameraShip::CCameraShip()
{
	m_fFOVY = 110.0f / 180.0f * PI;
	m_fAspect = 1.0f;
	m_fNear = 0.1f;
	m_fFar = 10000.0f;

	m_vEyeInShip = SVector3( -10.0f, 0.0f, 0.0f );
	m_vLookAtInShip = SVector3( 100.0f, 0.0f, 0.0f );
	m_vUpInShip = SVector3( 0.0f, 0.0f, 1.0f );

	m_vLookAtSmooth0 = SVector3( 0.0f, 0.0f, 0.0f );
	m_vLookAtSmooth1 = SVector3( 0.0f, 0.0f, 0.0f );
	m_vEyeSmooth0 = SVector3( -1.0f, 0.0f, 0.0f );
	m_vEyeSmooth1 = SVector3( -1.0f, 0.0f, 0.0f );
	m_vEyeSmooth1Prev = SVector3( -1.0f, 0.0f, 0.0f );
	m_vUpSmooth0 = SVector3( 0.0f, 0.0f, 1.0f );
	m_vUpSmooth1 = SVector3( 0.0f, 0.0f, 1.0f );

	SMatrix::Identity( m_matView );
	SMatrix::Identity( m_matProj );
	SMatrix::Identity( m_matViewProj );
	SMatrix::Identity( m_matViewProjPrev );

	SMatrix::Identity( m_matView000 );
	SMatrix::Identity( m_matViewProj000 );
	SMatrix::Identity( m_matViewProjPrev000 );
}

CCameraShip::~CCameraShip()
{
}

void CCameraShip::Update( float fElapsedTimeMs, const SMatrix& matShip )
{
	m_matViewProjPrev = m_matViewProj;
	m_matViewProjPrev000 = m_matViewProj000;
	m_vEyeSmooth1Prev = m_vEyeSmooth1;


	SVector3 vEye;
	SVector3 vLookAt;
	SVector3 vUp;

	SMatrix::TransformNormal( vEye, m_vEyeInShip, matShip );
	SMatrix::TransformNormal( vLookAt, m_vLookAtInShip, matShip );
	SMatrix::TransformNormal( vUp, m_vUpInShip, matShip );

	{
		float fW01 = CalcSmoothUpdateWeight( 1.003f, fElapsedTimeMs );
		float fW02 = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		float fW03 = CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs );
		m_vEyeSmooth0 = Lerp( vEye, m_vEyeSmooth0, fW01 );
		m_vLookAtSmooth0 = Lerp( vLookAt, m_vLookAtSmooth0, fW02 );
		m_vUpSmooth0 = Lerp( vUp, m_vUpSmooth0, fW03 );
		//m_vUpSmooth0 = SVector3::Slerp( m_vUpSmooth0, vUp, m_vUpSmooth0, fW02 );
		SVector3::Normalize( m_vUpSmooth0, m_vUpSmooth0 );
	}

	vEye = m_vEyeSmooth0 + SVector3( matShip.m30, matShip.m31, matShip.m32 );
	vLookAt = m_vLookAtSmooth0 + SVector3( matShip.m30, matShip.m31, matShip.m32 );
	vUp = m_vUpSmooth0;
	{
		float fW01 = CalcSmoothUpdateWeight( 1.1f, fElapsedTimeMs );
		float fW02 = CalcSmoothUpdateWeight( 1.4f, fElapsedTimeMs );
		float fW03 = CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs );
		m_vEyeSmooth1 = Lerp( vEye, m_vEyeSmooth1, fW01 );
		m_vLookAtSmooth1 = Lerp( vLookAt, m_vLookAtSmooth1, fW02 );
		m_vUpSmooth1 = Lerp( vUp, m_vUpSmooth1, fW03 );
		//m_vUpSmooth1 = SVector3::Slerp( m_vUpSmooth1, vUp, m_vUpSmooth1, fW02 );
		SVector3::Normalize( m_vUpSmooth1, m_vUpSmooth1 );
	}

	SMatrix::BuildViewMatrix( m_matView, m_vEyeSmooth1, m_vLookAtSmooth1, m_vUpSmooth1 );
	SMatrix::BuildProjectionMatrix( m_matProj, m_fFOVY, m_fAspect, m_fNear, m_fFar );
	SMatrix::Mul( m_matViewProj, m_matView, m_matProj );

	m_matView000 = m_matView;
	m_matView000.m30 = 0.0f;
	m_matView000.m31 = 0.0f;
	m_matView000.m32 = 0.0f;
	SMatrix::Mul( m_matViewProj000, m_matView000, m_matProj );
}