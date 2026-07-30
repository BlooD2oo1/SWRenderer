#pragma once

#include "Common/Vector.h"
#include "Engine/Graphics.h"
#include "Game/Camera.h"
#include "Game/ShipControl.h"
#include "Game/ShipMesh.h"
#include "Game/AsteroidMesh.h"

class CSceneGame
{
public:
	CSceneGame();
	~CSceneGame();

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

	SCamera			m_sCamera;
	SShipControl	m_sShipControl;
	CShipMesh		m_cShipMesh;

	struct SAsteroid
	{
		SVector3	m_vPos;
		SQuaternion	m_qRot;
	};
	std::vector< SAsteroid >	m_aAsteroids;
	CAsteroidMesh	m_cAsteroidMesh;

	SVertexPC*		m_pStars;
	uint32_t		m_iStarsCount;
	
};
