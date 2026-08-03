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

	int iMenuHeight = 8;
	int iFirstHeight = 55;
	int iSpacingHeight = 8;

	m_pMenuItems[EMenuItem_StartGame].w = 160;
	m_pMenuItems[EMenuItem_StartGame].h = iMenuHeight;
	m_pMenuItems[EMenuItem_StartGame].x = 320 / 2 - m_pMenuItems[EMenuItem_StartGame].w / 2;
	m_pMenuItems[EMenuItem_StartGame].y = iFirstHeight + (int)EMenuItem_StartGame*iSpacingHeight;

	m_pMenuItems[EMenuItem_Logs].w = 160;
	m_pMenuItems[EMenuItem_Logs].h = iMenuHeight;
	m_pMenuItems[EMenuItem_Logs].x = 320 / 2 - m_pMenuItems[EMenuItem_Logs].w / 2;
	m_pMenuItems[EMenuItem_Logs].y = iFirstHeight + (int)EMenuItem_Logs * iSpacingHeight;

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
	CGraphics::GetInstance().DrawTexture( 0, 0, SBlendFuncCopy(), m_sTexBackground );

	char szMenuText[EMenuItem_Count][32] = { "START GAME", "LOGS", "CREDITS", "EXIT" };

	for ( int i = 0; i < EMenuItem_Count; i++ )
	{
		SMenu& sMenuItem = m_pMenuItems[i];
		BGRA8 sColor = (i == m_eSelectedMenuItem) ? BGRA8{ (uint8_t)180, 250, 255, 255 } : BGRA8{ (uint8_t)160, 130, 120, 255 };
		//CGraphics::GetInstance().DrawRect( sMenuItem.x, sMenuItem.y, sMenuItem.w, sMenuItem.h, sColor );
		const int iFontWidth = 6;
		const int iFontHeight = 6;
		const int iSpacing = 0;
		const int iTextLength = (int)strlen( szMenuText[i] );
		const int iTextWidth = iTextLength * iFontWidth + (iTextLength - 1) * iSpacing;
		CGraphics::GetInstance().DrawText( sMenuItem.x + (m_pMenuItems[i].w - iTextWidth) / 2, sMenuItem.y + 2, szMenuText[i], sColor, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), iFontWidth, iFontHeight, iSpacing );
		if ( i == m_eSelectedMenuItem )
		{
			//CGraphics::GetInstance().DrawRect( sMenuItem.x + (m_pMenuItems[i].w - iTextWidth) / 2 - 10, sMenuItem.y + 3, 5, 3, BGRA8{ (uint8_t)10, 10, 120, 255 } );
			//CGraphics::GetInstance().DrawRect( sMenuItem.x + (m_pMenuItems[i].w + iTextWidth) / 2 + 10 - 5, sMenuItem.y + 3, 5, 3, BGRA8{ (uint8_t)10, 10, 120, 255 } );
			SBlendFuncCopy sBlendFunc;
			CGraphics::GetInstance().DrawLineH( sMenuItem.x + (m_pMenuItems[i].w - iTextWidth) / 2 - 11, sMenuItem.y + 3, 3, BGRA8{ (uint8_t)10, 10, 120, 255 }, sBlendFunc );
			CGraphics::GetInstance().DrawLineH( sMenuItem.x + (m_pMenuItems[i].w - iTextWidth) / 2 - 10, sMenuItem.y + 4, 5, BGRA8{ (uint8_t)10, 10, 120, 255 }, sBlendFunc );
			CGraphics::GetInstance().DrawLineH( sMenuItem.x + (m_pMenuItems[i].w - iTextWidth) / 2 - 11, sMenuItem.y + 5, 3, BGRA8{ (uint8_t)10, 10, 120, 255 }, sBlendFunc );
			CGraphics::GetInstance().DrawLineH( sMenuItem.x + (m_pMenuItems[i].w + iTextWidth) / 2 + 11, sMenuItem.y + 3, -3, BGRA8{ (uint8_t)10, 10, 120, 255 }, sBlendFunc );
			CGraphics::GetInstance().DrawLineH( sMenuItem.x + (m_pMenuItems[i].w + iTextWidth) / 2 + 10, sMenuItem.y + 4, -5, BGRA8{ (uint8_t)10, 10, 120, 255 }, sBlendFunc );
			CGraphics::GetInstance().DrawLineH( sMenuItem.x + (m_pMenuItems[i].w + iTextWidth) / 2 + 11, sMenuItem.y + 5, -3, BGRA8{ (uint8_t)10, 10, 120, 255 }, sBlendFunc );
		}
	}

	CGraphics::GetInstance().DrawText( 280, 200 - 8, "BlooD2oo1", BGRA8{ (uint8_t)100, 100, 100, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6, -2 );
}

bool CSceneMainMenu::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
		case KEY_UP:
		if ( m_eSelectedMenuItem > 0 )
		{
			m_eSelectedMenuItem = (EMenuItem)((int)m_eSelectedMenuItem - 1);

			SAudioEvent sAudioEvent;
			sAudioEvent.type = SAudioEvent::MenuSelect;
			sAudioEvent.fVolume = 0.2f;
			sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
			sAudioEvent.iLifeTimeNs = 1000 * 1000 * 150;
			sAudioEvent.iSampleCounter = 0;
			sAudioEvent.fPhase = 0.0f;
			CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );
		}
		return true;

		case KEY_DOWN:
		if ( m_eSelectedMenuItem < EMenuItem_Count - 1 )
		{
			m_eSelectedMenuItem = (EMenuItem)((int)m_eSelectedMenuItem + 1);

			SAudioEvent sAudioEvent;
			sAudioEvent.type = SAudioEvent::MenuSelect;
			sAudioEvent.fVolume = 0.2f;
			sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
			sAudioEvent.iLifeTimeNs = 1000 * 1000 * 150;
			sAudioEvent.iSampleCounter = 0;
			sAudioEvent.fPhase = 0.0f;
			CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );
		}
		return true;

		case KEY_ENTER:
		case KEY_SPACE:
		{
			switch ( m_eSelectedMenuItem )
			{
			case EMenuItem_StartGame:
				CEngine::GetInstance().SetScene( EScene_Game );
				break;
			case EMenuItem_Logs:
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

				SAudioEvent sAudioEvent;
				sAudioEvent.type = SAudioEvent::MenuSelect;
				sAudioEvent.fVolume = 0.2f;
				sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
				sAudioEvent.iLifeTimeNs = 1000 * 1000 * 150;
				sAudioEvent.iSampleCounter = 0;
				sAudioEvent.fPhase = 0.0f;
				CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );
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
