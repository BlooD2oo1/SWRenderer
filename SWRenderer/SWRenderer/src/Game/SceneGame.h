#pragma once

#include "Common/Vector.h"
#include "Engine/Graphics.h"
#include "Game/Camera.h"
#include "Game/ShipControl.h"

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

	SShipControl	m_sShipControl;

	std::vector< SShipControl >	m_aEnemyShips;

	struct SAsteroid
	{
		SVector3	m_vPos;
		float		m_fSize;
		SQuaternion	m_qRot;
	};
	std::vector< SAsteroid >	m_aAsteroids;

	SVertexPC*		m_pStars;
	uint32_t		m_iStarsCount;
	
};
