#pragma once

#include "Common/Globals.h"

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

extern float gAudioFreq[2];

class CAudio
{
public:
	static void Update( SAudioBuffer& sAudioBuffer );
};
