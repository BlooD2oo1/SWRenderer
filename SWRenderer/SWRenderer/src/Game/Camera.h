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

		// Left plane: Col3 + Col0
		m_aFrustumPlanes[0].x = m_matViewProj.m03 + m_matViewProj.m00;
		m_aFrustumPlanes[0].y = m_matViewProj.m13 + m_matViewProj.m10;
		m_aFrustumPlanes[0].z = m_matViewProj.m23 + m_matViewProj.m20;
		m_aFrustumPlanes[0].w = m_matViewProj.m33 + m_matViewProj.m30;

		// Right plane: Col3 - Col0
		m_aFrustumPlanes[1].x = m_matViewProj.m03 - m_matViewProj.m00;
		m_aFrustumPlanes[1].y = m_matViewProj.m13 - m_matViewProj.m10;
		m_aFrustumPlanes[1].z = m_matViewProj.m23 - m_matViewProj.m20;
		m_aFrustumPlanes[1].w = m_matViewProj.m33 - m_matViewProj.m30;

		// Bottom plane: Col3 + Col1
		m_aFrustumPlanes[2].x = m_matViewProj.m03 + m_matViewProj.m01;
		m_aFrustumPlanes[2].y = m_matViewProj.m13 + m_matViewProj.m11;
		m_aFrustumPlanes[2].z = m_matViewProj.m23 + m_matViewProj.m21;
		m_aFrustumPlanes[2].w = m_matViewProj.m33 + m_matViewProj.m31;

		// Top plane: Col3 - Col1
		m_aFrustumPlanes[3].x = m_matViewProj.m03 - m_matViewProj.m01;
		m_aFrustumPlanes[3].y = m_matViewProj.m13 - m_matViewProj.m11;
		m_aFrustumPlanes[3].z = m_matViewProj.m23 - m_matViewProj.m21;
		m_aFrustumPlanes[3].w = m_matViewProj.m33 - m_matViewProj.m31;

		// Near plane: Col2
		m_aFrustumPlanes[4].x = m_matViewProj.m02;
		m_aFrustumPlanes[4].y = m_matViewProj.m12;
		m_aFrustumPlanes[4].z = m_matViewProj.m22;
		m_aFrustumPlanes[4].w = m_matViewProj.m32;

		// Far plane: Col3 - Col2
		m_aFrustumPlanes[5].x = m_matViewProj.m03 - m_matViewProj.m02;
		m_aFrustumPlanes[5].y = m_matViewProj.m13 - m_matViewProj.m12;
		m_aFrustumPlanes[5].z = m_matViewProj.m23 - m_matViewProj.m22;
		m_aFrustumPlanes[5].w = m_matViewProj.m33 - m_matViewProj.m32;

		// Normalize plane normals
		for ( int i = 0; i < 6; ++i )
		{
			// Calculate plane normal length: sqrt(a^2 + b^2 + c^2)
			float fLength = sqrtf( m_aFrustumPlanes[i].x * m_aFrustumPlanes[i].x + m_aFrustumPlanes[i].y * m_aFrustumPlanes[i].y + m_aFrustumPlanes[i].z * m_aFrustumPlanes[i].z );

			if ( fLength > 0.0f )
			{
				float fInvLength = 1.0f / fLength;
				m_aFrustumPlanes[i].x *= fInvLength;
				m_aFrustumPlanes[i].y *= fInvLength;
				m_aFrustumPlanes[i].z *= fInvLength;
				m_aFrustumPlanes[i].w *= fInvLength;
			}
		}
	}

	bool FrustumSphereTest( const SVector3& vCenter, float fRadius ) const
	{
		// Perform sphere intersection test
		for ( int i = 0; i < 6; ++i )
		{
			// Calculate signed distance from plane to sphere center: Ax + By + Cz + D
			float fDistance =	m_aFrustumPlanes[i].x * vCenter.x + 
								m_aFrustumPlanes[i].y * vCenter.y + 
								m_aFrustumPlanes[i].z * vCenter.z + 
								m_aFrustumPlanes[i].w;

			// If the sphere center is further behind any plane than its radius, it's outside
			if ( fDistance < -fRadius )
			{
				return false; // Culled (Outside)
			}
		}

		return true; // Visible (Inside or Intersecting)
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

	SVector4	m_aFrustumPlanes[6];
};