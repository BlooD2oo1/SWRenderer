#pragma once

#include "Common/Globals.h"
#include "Graphics.h"

struct SMouseState
{
	int x = 0;
	int y = 0;
	bool bLeftButton = false;
	bool bRightButton = false;
	bool bMiddleButton = false;
};

struct SParticle
{
	float x;
	float y;
	float xm;
	float ym;
	float a;
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
	bool On_MouseMove( int x, int y );
	bool On_MouseButtonDown( uint32_t button );
	bool On_MouseButtonUp( uint32_t button );

private:
	SFrameBuffer m_sFrameBuffer;

	SMouseState m_sMouseState;

	SParticle* m_pParticles;
	int			m_iParticleCount;
};