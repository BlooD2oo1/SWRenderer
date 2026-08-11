#pragma once

#include "Engine/Graphics.h"
#include <vector>
#include "Game/Camera.h"

struct SEffect_ShipExplosion
{
	SEffect_ShipExplosion()
	{
	}
	~SEffect_ShipExplosion()
	{
	}

	void Create();

	bool Update();
	void Render( const SCamera& sCamera, const SViewPort& sViewport );

	SVector3	m_vPos;
	SVector3	m_vMovShip;
	SVector3	m_vMovBullet;

protected:	
	struct SParticle
	{
		SVector3	m_vPos;
		SVector3	m_vMov;
		float		m_fLifeTimeMs;
		float		m_fAgeMs;
	};

	std::vector<SParticle>	m_aParticles;

};

class CEffects
{
public:
	CEffects();
	~CEffects();

	void Clear();
	void Create();

	void Update();
	void Render( const SCamera& sCamera, const SViewPort& sViewport );

	SEffect_ShipExplosion& CreateShipExplosion()
	{
		m_aShipExplosions.emplace_back();
		return m_aShipExplosions.back();
	}

private:
	std::vector<SEffect_ShipExplosion>	m_aShipExplosions;
};