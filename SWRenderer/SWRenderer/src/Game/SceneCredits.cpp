#include "SceneCredits.h"
#include "Engine/Engine.h"

CSceneCredits::CSceneCredits()
{

}

CSceneCredits::~CSceneCredits()
{
}

void CSceneCredits::Clear()
{
}

void CSceneCredits::Create()
{
}

void CSceneCredits::Update()
{
}

void CSceneCredits::Render()
{
	// todo text:
	CGraphics::GetInstance().DrawText( 10, 10, "TODO", BGRA8( (uint8_t)255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_KarenFat_13x16(), 13, 16 );
}

bool CSceneCredits::On_KeyDown( uint32_t key )
{
	switch ( key )
	{
	case KEY_UP:
	return true;

	case KEY_DOWN:
	return true;

	case KEY_ENTER:
	case KEY_SPACE:
	return true;

	case KEY_ESCAPE:
	CEngine::GetInstance().SetScene( EScene_MainMenu );
	return true;
	}

	return false;
}

bool CSceneCredits::On_KeyUp( uint32_t key )
{
	return false;
}
