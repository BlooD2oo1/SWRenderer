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

	m_sMainMenu.Clear();
	m_sMainMenu.pParentMenu = &m_sMainMenu;
	m_pSelectedMenu = &m_sMainMenu;
}

void CSceneMainMenu::Create()
{
	PCX_LoadFromFile( "data/mainscreen.pcx", m_sTexBackground );

	{
		m_sMainMenu.aSubMenus.emplace_back();
		SMenuItemDesc& sMenu_NewGame = m_sMainMenu.aSubMenus.back();
		sMenu_NewGame.eMenuType = Menu_StartGame;
		sMenu_NewGame.sText = "NEW GAME";
		sMenu_NewGame.bHidden = false;
		

		{
			sMenu_NewGame.aSubMenus.emplace_back();
			SMenuItemDesc& sMenu_Level0 = sMenu_NewGame.aSubMenus.back();
			sMenu_Level0.eMenuType = Menu_Level;
			sMenu_Level0.sText = "LEVEL 0";
			sMenu_Level0.bHidden = false;
			

			sMenu_NewGame.aSubMenus.emplace_back();
			SMenuItemDesc& sMenu_Level1 = sMenu_NewGame.aSubMenus.back();
			sMenu_Level1.eMenuType = Menu_Level;
			sMenu_Level1.sText = "LEVEL 1";
			sMenu_Level1.bHidden = false;
			

			sMenu_NewGame.aSubMenus.emplace_back();
			SMenuItemDesc& sMenu_Level2 = sMenu_NewGame.aSubMenus.back();
			sMenu_Level2.eMenuType = Menu_Level;
			sMenu_Level2.sText = "LEVEL 2";
			sMenu_Level2.bHidden = false;
		}

		m_sMainMenu.aSubMenus.emplace_back();
		SMenuItemDesc& sMenu_Logs = m_sMainMenu.aSubMenus.back();
		sMenu_Logs.eMenuType = Menu_Logs;
		sMenu_Logs.sText = "LOGS";
		sMenu_Logs.bHidden = false;

		m_sMainMenu.aSubMenus.emplace_back();
		SMenuItemDesc& sMenu_Credits = m_sMainMenu.aSubMenus.back();
		sMenu_Credits.eMenuType = Menu_Credits;
		sMenu_Credits.sText = "CREDITS";
		sMenu_Credits.bHidden = false;

		m_sMainMenu.aSubMenus.emplace_back();
		SMenuItemDesc& sMenu_Exit = m_sMainMenu.aSubMenus.back();
		sMenu_Exit.eMenuType = Menu_Exit;
		sMenu_Exit.sText = "EXIT";
		sMenu_Exit.bHidden = false;		
	}

	SMenuItemDesc::SetMenuParents( &m_sMainMenu );

	m_pSelectedMenu = &m_sMainMenu.aSubMenus[0];	
}

void CSceneMainMenu::Update()
{
}

void CSceneMainMenu::Render()
{
	CGraphics::GetInstance().DrawTexture( SBlendFuncCopy(), m_sTexBackground );


	SMenuItemDesc* pParentMenu = m_pSelectedMenu->pParentMenu;
	for ( size_t i = 0; i < pParentMenu->aSubMenus.size(); i++ )
	{
		SMenuItemDesc& sMenuItem = pParentMenu->aSubMenus[i];
		BGRA8 sColor = (&sMenuItem == m_pSelectedMenu) ? BGRA8{ (uint8_t)180, 250, 255, 255 } : BGRA8{ (uint8_t)160, 130, 120, 255 };
		const int iFontWidth = 6;
		const int iFontHeight = 6;
		const int iSpacing = 0;
		const int iTextLength = (int)sMenuItem.sText.length();
		const int iTextWidth = iTextLength * iFontWidth + (iTextLength - 1) * iSpacing;
		CGraphics::GetInstance().DrawText( 320 / 2 - iTextWidth / 2, 55 + (int)i * 8, sMenuItem.sText.c_str(), sColor, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), iFontWidth, iFontHeight, iSpacing );
	}

	CGraphics::GetInstance().DrawText( 280, 200 - 7, "BlooD2oo1", BGRA8{ (uint8_t)100, 100, 100, 255 }, SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Tiny_6x5(), 6, 5, -2 );
}

int FindMenuInd( const SMenuItemDesc& sMenu, const SMenuItemDesc* pSelectedMenu )
{
	for ( size_t i = 0; i < sMenu.aSubMenus.size(); i++ )
	{
		if ( &sMenu.aSubMenus[i] == pSelectedMenu )
		{
			return (int)i;
		}
	}
	return -1;
}

void PlayMenuSound()
{
	SAudioEvent sAudioEvent;
	sAudioEvent.type = SAudioEvent::MenuSelect;
	sAudioEvent.fVolume = 0.2f;
	sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
	sAudioEvent.iLifeTimeNs = 1000 * 1000 * 450;
	sAudioEvent.iSampleCounter = 0;
	sAudioEvent.fPhase = 0.0f;
	CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );
}

bool CSceneMainMenu::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
		case KEY_UP:
		{
			int iCurrInd = FindMenuInd( *m_pSelectedMenu->pParentMenu, m_pSelectedMenu );
			if ( iCurrInd > 0 )
			{
				m_pSelectedMenu = &m_pSelectedMenu->pParentMenu->aSubMenus[iCurrInd - 1];
				PlayMenuSound();
			}				
		}
		return true;

		case KEY_DOWN:
		{
			int iCurrInd = FindMenuInd( *m_pSelectedMenu->pParentMenu, m_pSelectedMenu );
			if ( iCurrInd >= 0 && iCurrInd < (int)m_pSelectedMenu->pParentMenu->aSubMenus.size() - 1 )
			{
				m_pSelectedMenu = &m_pSelectedMenu->pParentMenu->aSubMenus[iCurrInd + 1];
				PlayMenuSound();
			}
		}
		return true;

		case KEY_ENTER:
		case KEY_SPACE:
		{
			switch ( m_pSelectedMenu->eMenuType )
			{
			case Menu_StartGame:
				if ( !m_pSelectedMenu->aSubMenus.empty() )
				{
					m_pSelectedMenu = &m_pSelectedMenu->aSubMenus[0];
					PlayMenuSound();
				}
				break;
			case Menu_Level:
				CEngine::GetInstance().SetScene( EScene_Game );
				break;
			case Menu_Logs:
				break;
			case Menu_Credits:
				CEngine::GetInstance().SetScene( EScene_Credits );
				break;
			case Menu_Exit:
				g_bRunning = false;
				break;
			}
		}
		return true;

		case KEY_ESCAPE:
			if ( m_pSelectedMenu->eMenuType == Menu_Exit )
			{
				g_bRunning = false;
			}
			if ( m_pSelectedMenu->pParentMenu == &m_sMainMenu )
			{
				m_pSelectedMenu = &m_sMainMenu.aSubMenus[m_sMainMenu.aSubMenus.size() - 1];
				PlayMenuSound();
			}
			else
			{
				m_pSelectedMenu = m_pSelectedMenu->pParentMenu;
				PlayMenuSound();
			}

		return true;
	}

	return false;
}

bool CSceneMainMenu::On_KeyUp( uint32_t key )
{
	return false;
}
