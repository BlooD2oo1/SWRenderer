#pragma once

#include "Common/Vector.h"
#include "Common/Math.h"

struct SCamera
{
	void Clear()
	{
		m_fFOVY = 45.0f / 180.0f * PI;
		m_fAspect = 1.0f;
		m_fNear = 0.1f;
		m_fFar = 10000.0f;
		m_vEye = SVector3( 0.0f, 0.0f, 10.0f );
		m_vEyeSmooth = m_vEye;
		m_vLookAt = SVector3( 0.0f, 0.0f, 0.0f );
		m_vLookAtSmooth = m_vLookAt;
		m_vUp = SVector3( 0.0f, 1.0f, 0.0f );
		SMatrix::Identity( m_matView );
		SMatrix::Identity( m_matProj );
		SMatrix::Identity( m_matViewProj );
		SMatrix::Identity( m_matViewProjPrev );
	}

	void UpdateMatrices()
	{
		m_matViewProjPrev = m_matViewProj;
		SMatrix::BuildLHViewMatrix( m_matView, m_vEyeSmooth, m_vLookAtSmooth, m_vUp );
		SMatrix::BuildLHProjectionMatrix( m_matProj, m_fFOVY, m_fAspect, m_fNear, m_fFar );
		SMatrix::Mul( m_matViewProj, m_matView, m_matProj );
	}

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