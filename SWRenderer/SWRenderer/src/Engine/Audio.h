#pragma once

#include <mutex>
#include <deque>
#include "Common/Globals.h"
#include "Common/Vector.h"
#include "Common/RingBuffer.h"

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

struct SAudioFrameData
{
	void Clear()
	{
		m_iTimeStampNs = 0;
		m_vCameraEye = SVector3( -1.0f, 0.0f, 0.0f );
		m_vCameraLookAt = SVector3( 0.0f, 0.0f, 0.0f );
		m_vShipPos = SVector3( 0.0f, 0.0f, 0.0f );
		m_fShipSpeed = 0.0f;
	}
	void Lerp( const SAudioFrameData& s0, const SAudioFrameData& s1, float fW )
	{
		double dt = (double)( s1.m_iTimeStampNs - s0.m_iTimeStampNs );
		m_iTimeStampNs = s0.m_iTimeStampNs + uint64_t(dt * (double)fW);
		m_vCameraEye = ::Lerp( s0.m_vCameraEye, s1.m_vCameraEye, fW );
		m_vCameraLookAt = ::Lerp( s0.m_vCameraLookAt, s1.m_vCameraLookAt, fW );
		m_vShipPos = ::Lerp( s0.m_vShipPos, s1.m_vShipPos, fW );
		m_fShipSpeed = ::Lerp( s0.m_fShipSpeed, s1.m_fShipSpeed, fW );
	}
	uint64_t		m_iTimeStampNs;
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
	uint64_t iTimeStampNs;
	uint64_t iLifeTimeNs;
	uint32_t iSampleCounter;
	float fPhase;

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
		
		struct
		{
			SVector3 vPos;

		} sGunShot;
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
	void				MainThread_PushAudioFrameData( const SAudioFrameData& sAudioFrameData );
	void				MainThread_PushAudioEvent( const SAudioEvent& sAudioEvent );

	void				AudioThread_Update( SAudioBuffer& sAudioBuffer );

private:

	uint64_t		m_iFrameInd;
	uint64_t		m_iTimeStampNs;
	uint64_t		m_iTimeStampPrevNs;

	CRingBuffer<SAudioFrameData, 32 >	m_ringAudioFrameData;
	std::deque<SAudioFrameData>			m_aAudioFrameData;
	CRingBuffer<SAudioEvent, 1024>		m_ringAudioEvents;
	std::vector<SAudioEvent>			m_aAudioEvents;
};
