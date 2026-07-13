#pragma once

#include "Common/Globals.h"
#include "Engine/Graphics.h"
#include "Engine/Audio.h"
#include "Game/Scene01.h"

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
	void Update();	
	void Render();

	bool On_KeyDown( uint32_t key );
	bool On_KeyUp( uint32_t key );
	bool On_MouseMove( int deltax, int deltay );
	bool On_MouseButtonDown( uint32_t button );
	bool On_MouseButtonUp( uint32_t button );
	bool On_MouseWheel( int iDelta );

	SMouseState&		GetMouseState() { return m_sMouseState; }
	uint64_t			GetFrameInd() const { return m_iFrameInd; }
	uint64_t			GetTimeStampNs() const { return m_iTimeStampNs; }
	uint64_t			GetTimeStampPrevNs() const { return m_iTimeStampPrevNs; }
	float				GetElapsedTimeMs() const { return m_fElapsedTimeMs; }
	SAudioFrameData&	GetAudioFrameData() { return m_sAudioFrameData; }

private:

	uint64_t		m_iFrameInd;
	uint64_t		m_iTimeStampNs;
	uint64_t		m_iTimeStampPrevNs;
	float			m_fElapsedTimeMs;

	SMouseState		m_sMouseState;

	SAudioFrameData	m_sAudioFrameData;

	CScene01		m_cScene01;
};