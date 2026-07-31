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
	PCX_LoadFromFile( "data/mainscreen.pcx", m_sTexBackground );

	m_eSelectedMenuItem = EMenuItem_StartGame;

	int iMenuHeight = 20;
	int iFirstHeight = 60;
	int iSpacingHeight = 20;

	m_pMenuItems[EMenuItem_StartGame].w = 160;
	m_pMenuItems[EMenuItem_StartGame].h = iMenuHeight;
	m_pMenuItems[EMenuItem_StartGame].x = 320 / 2 - m_pMenuItems[EMenuItem_StartGame].w / 2;
	m_pMenuItems[EMenuItem_StartGame].y = iFirstHeight + (int)EMenuItem_StartGame*iSpacingHeight;

	m_pMenuItems[EMenuItem_Legend].w = 160;
	m_pMenuItems[EMenuItem_Legend].h = iMenuHeight;
	m_pMenuItems[EMenuItem_Legend].x = 320 / 2 - m_pMenuItems[EMenuItem_Legend].w / 2;
	m_pMenuItems[EMenuItem_Legend].y = iFirstHeight + (int)EMenuItem_Legend * iSpacingHeight;

	m_pMenuItems[EMenuItem_Credits].w = 160;
	m_pMenuItems[EMenuItem_Credits].h = iMenuHeight;
	m_pMenuItems[EMenuItem_Credits].x = 320 / 2 - m_pMenuItems[EMenuItem_Credits].w / 2;
	m_pMenuItems[EMenuItem_Credits].y = iFirstHeight + (int)EMenuItem_Credits * iSpacingHeight;

	m_pMenuItems[EMenuItem_Exit].w = 160;
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
		BGRA8 sColor = (i == m_eSelectedMenuItem) ? BGRA8{ (uint8_t)180, 255, 255, 64 } : BGRA8{ (uint8_t)150, 50, 30, 64 };
		//CGraphics::GetInstance().DrawRect( sMenuItem.x, sMenuItem.y, sMenuItem.w, sMenuItem.h, sColor );
		CGraphics::GetInstance().DrawText( sMenuItem.x + 5, sMenuItem.y + 2, 
			(i == EMenuItem_StartGame) ?	"START GAME" :
			(i == EMenuItem_Legend) ?		"LEGEND" :
			(i == EMenuItem_Credits) ?		"CREDITS" :
			(i == EMenuItem_Exit) ?			"EXIT" : "", 
			sColor, CEngine::GetInstance().GetFontTex_KarenFat_13x16(), 13, 16 );
	}

	CGraphics::GetInstance().DrawText( 246, 200 - 8, "BlooD2oo1", BGRA8{ (uint8_t)100, 100, 100, 255 }, CEngine::GetInstance().GetFontTex_TinyPixie2_8x6(), 8, 6 );
}

bool CSceneMainMenu::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
		case KEY_UP:
		if ( m_eSelectedMenuItem > 0 )
			m_eSelectedMenuItem = (EMenuItem)((int)m_eSelectedMenuItem - 1);
		return true;

		case KEY_DOWN:
		if ( m_eSelectedMenuItem < EMenuItem_Count - 1 )
			m_eSelectedMenuItem = (EMenuItem)((int)m_eSelectedMenuItem + 1);
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
				CEngine::GetInstance().SetScene( EScene_Credits );
				break;
			case EMenuItem_Exit:
				g_bRunning = false;
				break;
			}
		}
		return true;

		case KEY_ESCAPE:
			if ( m_eSelectedMenuItem != EMenuItem_Exit )
			{
				m_eSelectedMenuItem = EMenuItem_Exit;
			}
			else
			{
				g_bRunning = false;
			}
		return true;
	}

	return false;
}

bool CSceneMainMenu::On_KeyUp( uint32_t key )
{
	return false;
}
