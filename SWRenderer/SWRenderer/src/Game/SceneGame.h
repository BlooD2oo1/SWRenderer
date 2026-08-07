#pragma once

#include "Common/Vector.h"
#include "Engine/Graphics.h"
#include "Game/Camera.h"
#include "Game/ShipControl.h"
#include "Game/Starfield.h"
#include "Game/Grid.h"

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

private:

	SCamera			m_sCamera;
	SViewPort		m_sViewportGameView;
	SViewPort		m_sViewportMiniMap;
	STextureIndexed	m_sTexHUD;

	SShipPlayer	m_sShipPlayer;

	std::vector< SShipEnemy >	m_aEnemyShips;

	struct SAsteroid
	{
		SVector3	m_vPos;
		float		m_fSize;
		SQuaternion	m_qRot;
	};
	std::vector< SAsteroid >	m_aAsteroids;

	CStarfield		m_cStarfield;
	CGrid			m_cGrid;
	
};
