#pragma once

#include "Graphics/Vector.h"

class CCameraFree
{
public:
	CCameraFree();
	~CCameraFree();

	void SetAspect( float fAspect ) { m_fAspect = fAspect; }
	void SetNearFar( float fNear, float fFar ) { m_fNear = fNear; m_fFar = fFar; }
	void SetFOVY( float fFOVY ) { m_fFOVY = fFOVY; }

	void Rotate( float fIncAngleXY, float fIncAngleZ );
	void Pan( const SVector2& vMov );
	void Zoom( float fValue );

	void Update( float fElapsedTimeMs );
	
	const SMatrix& GetViewMatrix() { return m_matView; }
	const SMatrix& GetProjectionMatrix() { return m_matProj; }
	const SMatrix& GetViewProjectionMatrix() { return m_matViewProj; }
	const SMatrix& GetViewProjectionMatrixPrev() { return m_matViewProjPrev; }

private:
	void UpdateEye();

private:
	float		m_fAngleXY;
	float		m_fAngleZ;
	float		m_fDist;
	float		m_fFOVY;
	float		m_fAspect;
	float		m_fNear;
	float		m_fFar;
	SVector3	m_vEye;
	SVector3	m_vEyeSmooth;
	SVector3	m_vLookAt;
	SVector3	m_vLookAtSmooth;
	SVector3	m_vUp;
	
	SMatrix		m_matView;
	SMatrix		m_matProj;
	SMatrix		m_matViewProj;
	SMatrix		m_matViewProjPrev;
};