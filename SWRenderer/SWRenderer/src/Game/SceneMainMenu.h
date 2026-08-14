#pragma once

#include "Common/Globals.h"
#include "Engine/Graphics.h"
#include <vector>
#include <string>

enum EMenuType
{
	Menu_StartGame,
	Menu_Logs,
	Menu_Credits,
	Menu_Exit,

	Menu_Level,

	Menu_nullptr,
};


struct SMenuItemDesc
{
	void Clear()
	{
		pParentMenu = nullptr;
		eMenuType = Menu_nullptr;
		sText = "";
		bHidden = true;		
		aSubMenus.clear();
	}

	SMenuItemDesc*	pParentMenu;
	EMenuType		eMenuType;
	std::string		sText;	
	bool			bHidden;

	static void SetMenuParents( SMenuItemDesc* pMenuItem )
	{
		pMenuItem->pParentMenu = pMenuItem;
		pMenuItem->SetMenuParentsRecursive();
	}
	void SetMenuParentsRecursive()
	{
		for ( size_t i = 0; i < aSubMenus.size(); i++ )
		{
			SMenuItemDesc* pSubMenu = &aSubMenus[i];
			pSubMenu->pParentMenu = this;
			pSubMenu->SetMenuParentsRecursive();
		}
	}

	std::vector<SMenuItemDesc> aSubMenus;	
	
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

private:

	SMenuItemDesc		m_sMainMenu;
	SMenuItemDesc*		m_pSelectedMenu;

	STextureIndexed		m_sTexBackground;
};