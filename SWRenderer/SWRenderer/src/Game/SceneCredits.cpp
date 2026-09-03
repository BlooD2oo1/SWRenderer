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
	CGraphics::GetInstance().ClearFrameBuffer( BGRA8( 0 ) );

	int iCursorY = 24;
	int iFrameBufferWidth = CGraphics::GetInstance().GetFrameBuffer().iWidth;
	int iSpacing = -1;
	{
		const char* pText = "Game Design & Programming:";
		size_t iLen = strlen( pText );
		int iFontWidth = 9;
		int iFontHeight = 13;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Habbo_9x13(), iFontWidth, iFontHeight, iSpacing );
		iCursorY += iFontHeight + 4 + 2;
	}

	{
		const char* pText = "Tamas Melykuti";
		size_t iLen = strlen( pText );
		int iFontWidth = 6;
		int iFontHeight = 6;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), iFontWidth, iFontHeight, -1 );
		iCursorY += iFontHeight + 4 + 12;
	}

	{
		const char* pText = "Graphics, Sound & Music:";
		size_t iLen = strlen( pText );
		int iFontWidth = 9;
		int iFontHeight = 13;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Habbo_9x13(), iFontWidth, iFontHeight, -1 );
		iCursorY += iFontHeight + 4 + 2;
	}

	{
		const char* pText = "Tamas Melykuti";
		size_t iLen = strlen( pText );
		int iFontWidth = 6;
		int iFontHeight = 6;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), iFontWidth, iFontHeight, -1 );
		iCursorY += iFontHeight + 4 + 12;
	}

	{
		const char* pText = "Special Thanks:";
		size_t iLen = strlen( pText );
		int iFontWidth = 9;
		int iFontHeight = 13;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_Habbo_9x13(), iFontWidth, iFontHeight, -1 );
		iCursorY += iFontHeight + 4 + 2;
	}

	{
		const char* pText = "Dad";
		size_t iLen = strlen( pText );
		int iFontWidth = 6;
		int iFontHeight = 6;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), iFontWidth, iFontHeight, -1 );
		iCursorY += iFontHeight + 4;
	}

	{
		const char* pText = "Tomszi";
		size_t iLen = strlen( pText );
		int iFontWidth = 6;
		int iFontHeight = 6;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 80, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), iFontWidth, iFontHeight, -1 );
		iCursorY += iFontHeight + 4;
	}

	{
		const char* pText = "Robert Nagy";
		size_t iLen = strlen( pText );
		int iFontWidth = 6;
		int iFontHeight = 6;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), iFontWidth, iFontHeight, -1 );
		iCursorY += iFontHeight + 4;
	}

	{
		const char* pText = "Marcell Baranyai";
		size_t iLen = strlen( pText );
		int iFontWidth = 6;
		int iFontHeight = 6;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), iFontWidth, iFontHeight, -1 );
		iCursorY += iFontHeight + 4;
	}

	{
		const char* pText = "Janos Turanszki";
		size_t iLen = strlen( pText );
		int iFontWidth = 6;
		int iFontHeight = 6;
		CGraphics::GetInstance().DrawText( (iFrameBufferWidth - (int)iLen * (iFontWidth+iSpacing)) / 2, iCursorY, pText, BGRA8( 255, 255, 255, 255 ), SBlendFuncCopy(), CEngine::GetInstance().GetFontTex_TinyPixie2_6x6(), iFontWidth, iFontHeight, -1 );
		iCursorY += iFontHeight + 4;
	}
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
