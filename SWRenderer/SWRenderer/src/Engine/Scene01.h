#pragma once

#include "Graphics/Vector.h"
#include "Graphics/Camera.h"

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

	bool On_KeyDown( uint32_t key );
	bool On_KeyUp( uint32_t key );
	bool On_MouseMove( int deltax, int deltay );
	bool On_MouseButtonDown( uint32_t button );
	bool On_MouseButtonUp( uint32_t button );
	bool On_MouseWheel( int iDelta );

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

	CCameraFree		m_cCamera;
};
