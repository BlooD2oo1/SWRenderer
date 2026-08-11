#pragma once

#include "Common/Vector.h"
#include "Engine/Graphics.h"
#include "Game/Camera.h"
#include "Game/ShipControl.h"
#include "Game/Starfield.h"
#include "Game/Grid.h"
#include "Game/Effects.h"

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

	CStarfield		m_cStarfield;
	
	CGrid			m_cGrid;

	CActors			m_cActors;
	
	CEffects		m_cEffects;

};
