#include "Audio.h"
#include "Common/Globals.h"

CAudio*	CAudio::m_pThis = nullptr;

CAudio::CAudio()
{
	for ( int i = 0; i < m_iAudioFrameDataCount; i++ )
	{
		m_pAudioFrameData[i].Clear();
	}
	m_iAudioFrameDataInd_Process = 0;
	m_iAudioFrameDataInd_Upload = 1;
	m_iAudioFrameDataInd_Free = 2;
}

CAudio::~CAudio()
{
}

void CAudio::MainThread_PushAudioEvent( const SAudioEvent& sAudioEvent )
{
	m_AudioQueue.Push( sAudioEvent );
}

SAudioFrameData* CAudio::MainThread_GetAudioFrameData()
{
	return &m_pAudioFrameData[m_iAudioFrameDataInd_Upload];
}

void CAudio::MainThread_AudioFrameDataDone()
{
	std::unique_lock<std::mutex> lock(m_mutexAudioFrameData);
	std::swap( m_iAudioFrameDataInd_Free, m_iAudioFrameDataInd_Upload );
}

std::vector< float > aFreq[2];
std::vector< float > aPhase[2];

std::vector< float > aFreq2[2];
std::vector< float > aPhase2[2];

int iC = 5;
bool b = true;

void CAudio::AudioThread_Update( SAudioBuffer& sAudioBuffer )
{
	SAudioFrameData* pAudioFrameData = nullptr;
	{
		std::unique_lock<std::mutex> lock(m_mutexAudioFrameData);
		if ( m_pAudioFrameData[m_iAudioFrameDataInd_Free].m_iFrameInd > m_pAudioFrameData[m_iAudioFrameDataInd_Process].m_iFrameInd )
		{
			std::swap( m_iAudioFrameDataInd_Free, m_iAudioFrameDataInd_Process );
		}
		pAudioFrameData = &m_pAudioFrameData[m_iAudioFrameDataInd_Process];
	}

	if (b)
	{
		b = false;
		for ( int iChInd = 0; iChInd < 2; iChInd++ )
		{
			for ( int i = 0; i < iC; i++ )
			{
				float fW = (float)i / (float)iC;
				aFreq[iChInd].push_back( 0.8f + fW*fW*14.0f*((float)rand()/(float)RAND_MAX) );
				aPhase[iChInd].push_back( 0.0f );

				aFreq2[iChInd].push_back( 0.01f + fW*0.1f*((float)rand()/(float)RAND_MAX) );
				aPhase2[iChInd].push_back( 0.0f );
			}
		}
	}

	std::vector< SAudioEvent > aAudioEvents;
	aAudioEvents.resize( m_AudioQueue.Capacity() );
	int i = 0;
	while (m_AudioQueue.Pop(aAudioEvents[i]))
	{
		i++;
	}

	for ( int iChInd = 0; iChInd < 2; iChInd++ )
	{
		for (uint32_t iTime = 0; iTime < sAudioBuffer.iNumFrames; iTime++)
		{
			float fOut = 0.0f;
			for ( int i = 0; i < iC; i++ )
			{
				float fW = (float)i / (float)iC;
				float& fPhase = aPhase[iChInd][i];
				float fFreq = aFreq[iChInd][i]*pAudioFrameData->m_fShipSpeed*60.0f;

				float& fPhase2 = aPhase2[iChInd][i];
				float fFreq2 = aFreq2[iChInd][i]*pAudioFrameData->m_fShipSpeed*60.0f;

				fOut  += powf( sinf(fPhase * PI2), 3.0f ) * powf( sinf( fPhase2 * PI2 ), 5.0f ) * (1.0f - fW*0.8f );

				fPhase += fFreq / sAudioBuffer.iSampleRate;
				fPhase2 += fFreq2 / sAudioBuffer.iSampleRate;

				// Wrap around the phase to maintain precision
				if (fPhase >= 1.0f) fPhase -= 1.0f;
				if (fPhase2 >= 1.0f) fPhase2 -= 1.0f;
			}
			fOut /= (float)iC;
			sAudioBuffer.pData[iTime * 2 + iChInd] = fOut  * 0.4f;

			for ( int iEvent = 0; iEvent < i; iEvent++ )
			{
				SAudioEvent& sAudioEvent = aAudioEvents[iEvent];
				if ( sAudioEvent.type == SAudioEvent::ClickDown )
				{
					float fExp = 1.0f - abs( (float)iTime / (float)sAudioBuffer.iNumFrames - 0.5f ) * 2.0f;
					fExp *= fExp;
					sAudioBuffer.pData[iTime * 2 + iChInd] += (sAudioEvent.fVolume * 0.5f) * sinf( (float)iTime * 0.1f ) * fExp;
				}
				if ( sAudioEvent.type == SAudioEvent::ClickUp )
				{
					float fExp = 1.0f - abs( (float)iTime / (float)sAudioBuffer.iNumFrames - 0.5f ) * 2.0f;
					fExp *= fExp;
					sAudioBuffer.pData[iTime * 2 + iChInd] += (sAudioEvent.fVolume * 0.3f) * sinf( (float)iTime * 0.13f ) * fExp;
				}
			}
		}
	}
}
