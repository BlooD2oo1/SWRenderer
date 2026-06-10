#pragma once

#include "Graphics/Vector.h"

struct SVertex
{
	SVector3 vPos;
};

class CScene01
{
public:
	CScene01();
	~CScene01();

	void Clear();
	void Create();

	void Update();
	void Render();

private:

	struct SParticle
	{
		SVector3 vPos;
		float a;
	};
	SParticle*		m_pParticles;
	int				m_iParticleCount;

	SVertex*		m_pLineListSpaceShip;
	int				m_iLineListSpaceShipCount;
	SMatrix			m_matWorldSpaceShip;
};
