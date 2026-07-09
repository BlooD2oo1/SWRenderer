#pragma once

#include "Common/Globals.h"
#include "Graphics/Graphics.h"
#include "Graphics/Audio.h"
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
	void UpdateAudioThread( SAudioBuffer& sAudioBuffer );
	void Update( float fElapsedTimeMs );	
	void Render();

	bool On_KeyDown( uint32_t key );
	bool On_KeyUp( uint32_t key );
	bool On_MouseMove( int deltax, int deltay );
	bool On_MouseButtonDown( uint32_t button );
	bool On_MouseButtonUp( uint32_t button );
	bool On_MouseWheel( int iDelta );

	SMouseState&	GetMouseState() { return m_sMouseState; }
	uint64_t		GetFrameInd() const { return m_iFrameInd; }
	float			GetElapsedTimeMs() const { return m_fElapsedTimeMs; }

private:

	SMouseState		m_sMouseState;

	uint64_t		m_iFrameInd;
	float			m_fElapsedTimeMs;

	CScene01		m_cScene01;
};