#include "SceneMainMenu.h"
#include "Engine/Engine.h"

#include "Common/Threading.h"
#include "Common/PCXLoader.h"

CSceneMainMenu::CSceneMainMenu()
{
	
}

CSceneMainMenu::~CSceneMainMenu()
{
}

void CSceneMainMenu::Clear()
{
	m_sTexBackground.Clear();
}

void CSceneMainMenu::Create()
{
	PCX_LoadFromFile( "data\\mainscreen.pcx", m_sTexBackground );

	m_eSelectedMenuItem = EMenuItem_StartGame;

	int iMenuHeight = 10;
	int iFirstHeight = 30;
	int iSpacingHeight = 20;

	m_pMenuItems[EMenuItem_StartGame].w = 100;
	m_pMenuItems[EMenuItem_StartGame].h = iMenuHeight;
	m_pMenuItems[EMenuItem_StartGame].x = 320 / 2 - m_pMenuItems[EMenuItem_StartGame].w / 2;
	m_pMenuItems[EMenuItem_StartGame].y = iFirstHeight + (int)EMenuItem_StartGame*iSpacingHeight;

	m_pMenuItems[EMenuItem_Legend].w = 100;
	m_pMenuItems[EMenuItem_Legend].h = iMenuHeight;
	m_pMenuItems[EMenuItem_Legend].x = 320 / 2 - m_pMenuItems[EMenuItem_Legend].w / 2;
	m_pMenuItems[EMenuItem_Legend].y = iFirstHeight + (int)EMenuItem_Legend * iSpacingHeight;

	m_pMenuItems[EMenuItem_Credits].w = 100;
	m_pMenuItems[EMenuItem_Credits].h = iMenuHeight;
	m_pMenuItems[EMenuItem_Credits].x = 320 / 2 - m_pMenuItems[EMenuItem_Credits].w / 2;
	m_pMenuItems[EMenuItem_Credits].y = iFirstHeight + (int)EMenuItem_Credits * iSpacingHeight;

	m_pMenuItems[EMenuItem_Exit].w = 100;
	m_pMenuItems[EMenuItem_Exit].h = iMenuHeight;
	m_pMenuItems[EMenuItem_Exit].x = 320 / 2 - m_pMenuItems[EMenuItem_Exit].w / 2;
	m_pMenuItems[EMenuItem_Exit].y = iFirstHeight + (int)EMenuItem_Exit * iSpacingHeight;

}

void CSceneMainMenu::Update()
{
}

void CSceneMainMenu::Render()
{
	CGraphics::GetInstance().DrawTexture( 0, 0, m_sTexBackground );

	for ( int i = 0; i < EMenuItem_Count; i++ )
	{
		SMenu& sMenuItem = m_pMenuItems[i];
		BGRA8 sColor = (i == m_eSelectedMenuItem) ? BGRA8{ (uint8_t)0, 255, 255, 255 } : BGRA8{ (uint8_t)10, 20, 128, 255 };
		CGraphics::GetInstance().DrawRect( sMenuItem.x, sMenuItem.y, sMenuItem.w, sMenuItem.h, sColor );
	}
}

bool CSceneMainMenu::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
		case KEY_UP:
		m_eSelectedMenuItem = (EMenuItem)(((int)m_eSelectedMenuItem + EMenuItem_Count - 1) % EMenuItem_Count);
		return true;

		case KEY_DOWN:
		m_eSelectedMenuItem = (EMenuItem)( ( (int)m_eSelectedMenuItem + 1 ) % EMenuItem_Count );
		return true;

		case KEY_ENTER:
		case KEY_SPACE:
		{
			switch ( m_eSelectedMenuItem )
			{
			case EMenuItem_StartGame:
				CEngine::GetInstance().SetScene( EScene_Game );
				break;
			case EMenuItem_Legend:
				break;
			case EMenuItem_Credits:
				break;
			case EMenuItem_Exit:
				g_bRunning = false;
				break;
			}
		}
		return true;

		case KEY_ESCAPE:
			g_bRunning = false;
		return true;
	}

	return false;
}

bool CSceneMainMenu::On_KeyUp( uint32_t key )
{
	return false;
}

bool CSceneMainMenu::On_MouseMove( int deltax, int deltay )
{
	return false;
}
bool CSceneMainMenu::On_MouseButtonDown( uint32_t button )
{
	return false;
}

bool CSceneMainMenu::On_MouseButtonUp( uint32_t button )
{
	return false;
}

bool CSceneMainMenu::On_MouseWheel( int iDelta )
{
	return false;
}