#pragma once

#include "Common/Globals.h"
#include "Engine/Graphics.h"

struct SMenu
{
	int x;
	int y;
	int w;
	int h;
};

enum EMenuItem
{
	EMenuItem_StartGame,
	EMenuItem_Legend,
	EMenuItem_Credits,
	EMenuItem_Exit,
	EMenuItem_Count,
};

class CSceneMainMenu
{
public:
	CSceneMainMenu();
	~CSceneMainMenu();
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
	SMenu		m_pMenuItems[EMenuItem_Count];
	EMenuItem	m_eSelectedMenuItem;

	STextureIndexed m_sTexBackground;
};