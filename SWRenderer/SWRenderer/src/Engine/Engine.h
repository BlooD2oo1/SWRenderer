#pragma once

#include "Common/Globals.h"
#include "Engine/Graphics.h"
#include "Engine/Audio.h"
#include "Engine/Mesh.h"
#include "Game/SceneMainMenu.h"
#include "Game/SceneCredits.h"
#include "Game/SceneGame.h"

enum ESceneType
{
	EScene_MainMenu,
	EScene_Credits,
	EScene_Game,
};

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

	void SetScene( ESceneType eSceneType );

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

	const SMesh&		GetShipMeshPlayer() const { return m_sMeshShipPlayer; }
	const SMesh&		GetShipMeshScout() const { return m_sMeshShipScout; }
	const SMesh&		GetShipMeshDestroyer() const { return m_sMeshShipDestroyer; }
	const SMesh&		GetAsteroidMesh() const { return m_sMeshAsteroid; }

	const STextureIndexed& GetFontTex_KarenFat_13x16() const { return m_sTexFont_KarenFat_13x16; }
	const STextureIndexed& GetFontTex_TinyPixie2_6x6() const { return m_sTexFont_TinyPixie2_6x6; }
	const STextureIndexed& GetFontTex_Habbo_9x13() const { return m_sTexFont_Habbo_9x13; }

private:

	uint64_t		m_iFrameInd;
	uint64_t		m_iTimeStampNs;
	uint64_t		m_iTimeStampPrevNs;
	float			m_fElapsedTimeMs;

	SMouseState		m_sMouseState;

	SAudioFrameData	m_sAudioFrameData;

	ESceneType		m_eCurrentScene;
	CSceneMainMenu	m_cSceneMainMenu;
	CSceneCredits	m_cSceneCredits;
	CSceneGame		m_cSceneGame;

	SMesh				m_sMeshShipPlayer;
	SMesh				m_sMeshShipScout;
	SMesh				m_sMeshShipDestroyer;
	SMesh				m_sMeshAsteroid;

	STextureIndexed	m_sTexFont_KarenFat_13x16;
	STextureIndexed	m_sTexFont_TinyPixie2_6x6;
	STextureIndexed	m_sTexFont_Habbo_9x13;

};