#include "Effects.h"

#include "Engine/Engine.h"

CEffects::CEffects()
{
	Clear();
}
CEffects::~CEffects()
{
	Clear();
}

void CEffects::Clear()
{
	m_aShipExplosions.clear();
}

void CEffects::Create()
{
	Clear();
}

void CEffects::Update()
{
	for ( size_t i = 0; i < m_aShipExplosions.size(); )
	{
		SEffect_ShipExplosion& sEffect = m_aShipExplosions[i];
		if ( sEffect.Update() )
		{
			m_aShipExplosions[i] = m_aShipExplosions.back();
			m_aShipExplosions.pop_back();
			continue;
		}
		i++;
	}
}

void CEffects::Render( const SCamera& sCamera, const SViewPort& sViewport )
{
	for ( size_t i = 0; i < m_aShipExplosions.size(); i++ )
	{
		SEffect_ShipExplosion& sEffect = m_aShipExplosions[i];
		sEffect.Render( sCamera, sViewport );
	}
}

////////////////////////////////////////////////////////////////

void SEffect_ShipExplosion::Create()
{
	m_aParticles.clear();

	for ( size_t i = 0; i < 32; i++ )
	{
		SParticle sParticle;
		sParticle.m_vPos = m_vPos;
		float u = ((float)rand() / (float)RAND_MAX);
		float v = ((float)rand() / (float)RAND_MAX);

		float theta = 2.0f * 3.14159265359f * u;
		float phi = acosf(2.0f * v - 1.0f);

		sParticle.m_vMov.x = sinf(phi) * cosf(theta);
		sParticle.m_vMov.y = sinf(phi) * sinf(theta);
		sParticle.m_vMov.z = cosf(phi);
		sParticle.m_vMov *= 1.0f + powf( ((float)rand() / (float)RAND_MAX), 4 ) * 10.0f;
		sParticle.m_vMov *= 0.02f;
		sParticle.m_vMov += Lerp( m_vMovShip, m_vMovBullet*0.3f, ((float)rand() / (float)RAND_MAX) );

		sParticle.m_fLifeTimeMs = ((float)rand() / (float)RAND_MAX) * 4000.0f + 300.0f;
		sParticle.m_fAgeMs = 0.0f;
		m_aParticles.push_back( sParticle );
	}
}

bool SEffect_ShipExplosion::Update()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	for ( size_t i = 0; i < m_aParticles.size(); )
	{
		SParticle& sParticle = m_aParticles[i];
		sParticle.m_vPos += sParticle.m_vMov * fElapsedTimeMs;
		sParticle.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), sParticle.m_vMov, CalcSmoothUpdateWeight( 1.005f, fElapsedTimeMs ) );
		sParticle.m_fAgeMs += fElapsedTimeMs;
		if ( sParticle.m_fAgeMs >= sParticle.m_fLifeTimeMs )
		{
			m_aParticles[i] = m_aParticles.back();
			m_aParticles.pop_back();
			continue;
		}
		i++;
	}

	return m_aParticles.empty();
}


void SEffect_ShipExplosion::Render( const SCamera& sCamera, const SViewPort& sViewport )
{
	struct SVertexShaderBasic
	{
		using AttribsType = SVertexP::SAttribs;
		SMatrix matWorldViewProjViewPort;
		inline void Execute( SClipVertex<AttribsType>& out, const SVertexP& in ) const
		{
			SVector4 vPhSrc( in.vPos, 1.0f );
			SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProjViewPort );
		}
	} sVertexShaderBasic;
	SMatrix::Mul( sVertexShaderBasic.matWorldViewProjViewPort, sCamera.m_matViewProj, sViewport.GetViewPortMatrix() );

	struct SPixelShaderBasic
	{
		BGRA8 sColor;
		inline BGRA8 Execute( const SVertexP::SAttribs& in ) const
		{
			return sColor;
		}
	} sPixelShaderBasic;

	for ( size_t i = 0; i < m_aParticles.size(); i++ )
	{
		SParticle& sParticle = m_aParticles[i];

		SVertexP sP;
		sP.vPos = sParticle.m_vPos;

		float fAlpha = 1.0f - (sParticle.m_fAgeMs / sParticle.m_fLifeTimeMs);

		sPixelShaderBasic.sColor = BGRA8( 0.2f, 0.8f, 1.0f, fAlpha );
		CGraphics::GetInstance().DrawPoint3D( sP, sViewport, sVertexShaderBasic, sPixelShaderBasic, SBlendFuncAdditive() );
	}
}