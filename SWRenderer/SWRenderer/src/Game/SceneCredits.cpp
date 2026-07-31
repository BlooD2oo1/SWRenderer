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


	CGraphics::GetInstance().DrawText( 260, 200 - 8, "BlooD2oo1", BGRA8{ (uint8_t)100, 100, 100, 255 }, CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), 6, 6 );
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
