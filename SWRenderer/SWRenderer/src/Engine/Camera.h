#pragma once

#include "Common/Vector.h"

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
	
	const SVector3&	GetEye() const { return m_vEyeSmooth; }
	const SVector3&	GetLookAt() const { return m_vLookAtSmooth; }
	const SMatrix&	GetViewMatrix() { return m_matView; }
	const SMatrix&	GetProjectionMatrix() { return m_matProj; }
	const SMatrix&	GetViewProjectionMatrix() { return m_matViewProj; }
	const SMatrix&	GetViewProjectionMatrixPrev() { return m_matViewProjPrev; }
	const SMatrix&	GetViewProjectionMatrix000() { return m_matViewProj000; }
	const SMatrix&	GetViewProjectionMatrixPrev000() { return m_matViewProjPrev000; }

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

	SMatrix		m_matView000;
	SMatrix		m_matViewProj000;
	SMatrix		m_matViewProjPrev000;
};

class CCameraShip
{
public:
	CCameraShip();
	~CCameraShip();

	void SetAspect( float fAspect ) { m_fAspect = fAspect; }
	void SetNearFar( float fNear, float fFar ) { m_fNear = fNear; m_fFar = fFar; }

	void Update( float fElapsedTimeMs, const SMatrix& matShip );

	const SVector3&	GetEye() const { return m_vEyeSmooth1; }	
	const SVector3&	GetEyePrev() const { return m_vEyeSmooth1Prev; }
	const SVector3&	GetLookAt() const { return m_vLookAtSmooth1; }
	const SMatrix&	GetViewMatrix() { return m_matView; }
	const SMatrix&	GetProjectionMatrix() { return m_matProj; }
	const SMatrix&	GetViewProjectionMatrix() { return m_matViewProj; }
	const SMatrix&	GetViewProjectionMatrixPrev() { return m_matViewProjPrev; }
	const SMatrix&	GetViewProjectionMatrix000() { return m_matViewProj000; }
	const SMatrix&	GetViewProjectionMatrixPrev000() { return m_matViewProjPrev000; }

private:

	float		m_fFOVY;
	float		m_fAspect;
	float		m_fNear;
	float		m_fFar;

	SVector3	m_vEyeInShip;
	SVector3	m_vLookAtInShip;
	SVector3	m_vUpInShip;

	SVector3	m_vEyeSmooth0;
	SVector3	m_vEyeSmooth1;
	SVector3	m_vEyeSmooth1Prev;
	SVector3	m_vLookAtSmooth0;
	SVector3	m_vLookAtSmooth1;
	SVector3	m_vUpSmooth0;
	SVector3	m_vUpSmooth1;

	SMatrix		m_matView;
	SMatrix		m_matProj;
	SMatrix		m_matViewProj;
	
	SMatrix		m_matViewProjPrev;

	SMatrix		m_matView000;
	SMatrix		m_matViewProj000;
	SMatrix		m_matViewProjPrev000;
};
