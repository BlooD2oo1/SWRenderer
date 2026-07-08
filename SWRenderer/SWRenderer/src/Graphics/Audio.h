#pragma once

#include "Common/Globals.h"
#include "Graphics/Vector.h"
#include <mutex>
#include "Common/EventQue.h"

struct SAudioBuffer
{
	SAudioBuffer()
		: pData( nullptr ), iNumFrames( 0 ), iSampleRate( 0 )
	{
	}
	SAudioBuffer( float* pData, uint32_t iNumFrames, uint32_t iSampleRate )
		: pData( pData ), iNumFrames( iNumFrames ), iSampleRate( iSampleRate )
	{
	}
	float*		pData = nullptr;
	uint32_t	iNumFrames = 0;
	uint32_t	iSampleRate = 0;
};

struct SAudioData
{
	void Clear()
	{
		m_iFrameInd = 0;
		m_vCameraEye = SVector3( -1.0f, 0.0f, 0.0f );
		m_vCameraLookAt = SVector3( 0.0f, 0.0f, 0.0f );
		m_vShipPos = SVector3( 0.0f, 0.0f, 0.0f );
		m_fShipSpeed = 0.0f;
	}
	uint64_t		m_iFrameInd;
	SVector3		m_vCameraEye;
	SVector3		m_vCameraLookAt;
	SVector3		m_vShipPos;
	float			m_fShipSpeed;
};

struct SAudioEvent
{
	SAudioEvent() {}

	enum EAudioEventType
	{
		ClickDown,
		ClickUp,
		MenuHover,
		MenuSelect,
		GunShot,
	} type;

	float fVolume;

	union
	{
		struct
		{
			uint32_t iButton;
		} sClick;
		
		struct
		{
			uint32_t iMenu;
		} sMenu;
	};
};

class CAudio
{
public:
	__forceinline static void		CreateInstance() { SAFE_DELETE( m_pThis ); m_pThis = new CAudio(); }
	__forceinline static CAudio&	GetInstance() { return *m_pThis; }
	__forceinline static bool		HasInstance() { return m_pThis == nullptr ? false : true; }
	__forceinline static void		Destroy() { SAFE_DELETE( m_pThis ); }
private:
	static CAudio*	m_pThis;
	CAudio();
	~CAudio();

public:
	void		MainThread_PushAudioEvent( const SAudioEvent& sAudioEvent );
	SAudioData* MainThread_GetAudioData();
	void		MainThread_AudioDataDone();
	void		AudioThread_Update( SAudioBuffer& sAudioBuffer );

private:
	std::mutex			m_mutexAudioData;
	static const int	m_iAudioDataCount = 3;
	SAudioData			m_pAudioData[m_iAudioDataCount];
	int					m_iAudioDataInd_Process;
	int					m_iAudioDataInd_Upload;
	int					m_iAudioDataInd_Free;

	CEventQueue<SAudioEvent, 1024> m_AudioQueue;
};
