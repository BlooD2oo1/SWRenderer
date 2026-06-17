#pragma once

#include "Common/Globals.h"
#include "Graphics/Graphics.h"
#include "Engine/Scene01.h"

struct SMouseState
{
	int x = 0;
	int y = 0;
	bool bLeftButton = false;
	bool bRightButton = false;
	bool bMiddleButton = false;
};

class CEngine
{
public:
	__forceinline static void		CreateInstance() { SAFE_DELETE( m_pThis ); m_pThis = new CEngine(); }
	__forceinline static CEngine&	GetInstance() { return *m_pThis; }
	__forceinline static bool		HasInstance() { return m_pThis == nullptr ? false : true; }
	__forceinline static void		Destroy() { SAFE_DELETE( m_pThis ); }
private:
	static CEngine*	m_pThis;
	CEngine();
	~CEngine();

public:
	void Create( SFrameBuffer& sFrameBuffer );
	void Clear();
	void Update();
	void Render();

	bool On_KeyDown( uint32_t key );
	bool On_KeyUp( uint32_t key );
	bool On_MouseMove( int deltax, int deltay );
	bool On_MouseButtonDown( uint32_t button );
	bool On_MouseButtonUp( uint32_t button );

	SMouseState&	GetMouseState() { return m_sMouseState; }
	uint32_t		GetFrameCount() const { return m_iFrameCount; }

private:

	SMouseState		m_sMouseState;

	uint32_t		m_iFrameCount;

	CScene01		m_cScene01;
};