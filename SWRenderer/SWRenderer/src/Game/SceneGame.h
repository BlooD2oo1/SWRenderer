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

	CEffects& GetEffects() { return m_cEffects; }

private:

	SCamera			m_sCamera;
	
	SViewPort		m_sViewportGameView;
	
	SViewPort		m_sViewportMiniMap;
	
	//STextureIndexed	m_sTexHUD_Top;
	//STextureIndexed	m_sTexHUD_Left;
	//STextureIndexed	m_sTexHUD_Bottom;
	//STextureIndexed	m_sTexHUD_Right;
	STextureIndexed	m_sTexHUD_Screen;
	STextureIndexed	m_sTexHUD_Menu;
	STextureIndexed	m_sTexHUD_Tick;

	CStarfield		m_cStarfield;
	
	CGrid			m_cGrid;

	CActors			m_cActors;
	
	CEffects		m_cEffects;

};
