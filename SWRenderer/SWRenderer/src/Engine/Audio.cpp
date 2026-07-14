#include "Audio.h"
#include "Common/Globals.h"

CAudio*	CAudio::m_pThis = nullptr;

CAudio::CAudio()
{
	m_iFrameInd = 0;
	m_iTimeStampNs = 0;
	m_iTimeStampPrevNs = 0;
}

CAudio::~CAudio()
{
}

void CAudio::MainThread_PushAudioFrameData( const SAudioFrameData& sAudioFrameData )
{
	m_ringAudioFrameData.Push( sAudioFrameData );
}

void CAudio::MainThread_PushAudioEvent( const SAudioEvent& sAudioEvent )
{
	m_ringAudioEvents.Push( sAudioEvent );
}

std::vector< float > aFreq[2];
std::vector< float > aPhase[2];

std::vector< float > aFreq2[2];
std::vector< float > aPhase2[2];

int iC = 5;
bool b = true;

void CAudio::AudioThread_Update( SAudioBuffer& sAudioBuffer )
{
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

	//////////////////////////////////////////////////////////////////////////

	m_iFrameInd++;
	if ( m_iTimeStampNs == 0 )
	{
		m_iTimeStampNs = GetGlobalTimeStampNs();
		m_iTimeStampPrevNs = m_iTimeStampNs-1000;
	}
	else
	{
		m_iTimeStampPrevNs = m_iTimeStampNs;
		m_iTimeStampNs = GetGlobalTimeStampNs();
	}

	const uint64_t iDelayNs = (uint64_t)( 1000.0 / 30.0 * 1000.0 * 1000.0 );
	
	const uint64_t iRealTimeNs = m_iTimeStampNs - iDelayNs;

	//////////////////////////////////////////////////////////////////////////

	{
		SAudioFrameData sTemp;
		while ( m_ringAudioFrameData.Pop( sTemp ) )
		{
			m_aAudioFrameData.push_back( sTemp );
		}

		while ( m_aAudioFrameData.size() > 20 )
		{
			m_aAudioFrameData.pop_front();
		}
	}

	{
		SAudioEvent sTemp;
		while ( m_ringAudioEvents.Pop( sTemp ) )
		{
			m_aAudioEvents.push_back( sTemp );
		}
	}

	SAudioFrameData sAudioFrameData;
	sAudioFrameData.Clear();
	if ( !m_aAudioFrameData.empty() )
	{
		for ( int i = 0; i < m_aAudioFrameData.size()-1; i++ )
		{
			SAudioFrameData& sAudioFrameData0 = m_aAudioFrameData[i];
			SAudioFrameData& sAudioFrameData1 = m_aAudioFrameData[i + 1];
			if ( sAudioFrameData0.m_iTimeStampNs <= iRealTimeNs && iRealTimeNs < sAudioFrameData1.m_iTimeStampNs )
			{
				float fW = (float)(iRealTimeNs - sAudioFrameData0.m_iTimeStampNs) / (float)(sAudioFrameData1.m_iTimeStampNs - sAudioFrameData0.m_iTimeStampNs);
				sAudioFrameData.Lerp( sAudioFrameData0, sAudioFrameData1, fW );
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	for (uint32_t iFrameInd = 0; iFrameInd < sAudioBuffer.iNumFrames; iFrameInd++)
	{
		for ( int iChInd = 0; iChInd < 2; iChInd++ )
		{
			float fOut = 0.0f;
			for ( int i = 0; i < iC; i++ )
			{
				float fW = (float)i / (float)iC;
				float& fPhase = aPhase[iChInd][i];
				float fFreq = aFreq[iChInd][i]*sAudioFrameData.m_fShipSpeed*60.0f;

				float& fPhase2 = aPhase2[iChInd][i];
				float fFreq2 = aFreq2[iChInd][i]*sAudioFrameData.m_fShipSpeed*60.0f;

				fOut  += powf( sinf(fPhase * PI2), 3.0f ) * powf( sinf( fPhase2 * PI2 ), 5.0f ) * (1.0f - fW*0.8f );

				fPhase += fFreq / (float)sAudioBuffer.iSampleRate;
				fPhase2 += fFreq2 / (float)sAudioBuffer.iSampleRate;

				// Wrap around the phase to maintain precision
				if (fPhase >= 1.0f) fPhase -= 1.0f;
				if (fPhase2 >= 1.0f) fPhase2 -= 1.0f;
			}
			fOut /= (float)iC;
			sAudioBuffer.pData[iFrameInd * 2 + iChInd] = fOut  * 0.4f;

			uint64_t iFrameIndTimeNs = (uint64_t)iFrameInd * 1000 * 1000 * 1000 / sAudioBuffer.iSampleRate;
			uint64_t iRealTimeFrameNs = iRealTimeNs + iFrameIndTimeNs;
			for ( int iEvent = 0; iEvent < m_aAudioEvents.size(); )
			{
				SAudioEvent& sAudioEvent = m_aAudioEvents[iEvent];
				if ( sAudioEvent.iTimeStampNs > iRealTimeFrameNs )
				{
					iEvent++;
					continue;										
				}
				if ( sAudioEvent.iTimeStampNs + sAudioEvent.iLifeTimeNs < iRealTimeNs )
				{
					if ( iEvent < m_aAudioEvents.size() - 1 )
					{
						m_aAudioEvents[iEvent] = m_aAudioEvents.back();
						m_aAudioEvents.pop_back();
					}
					else
					{
						m_aAudioEvents.pop_back();
						break;
					}
				}
				else
				{
					iEvent++;
				}

				

				//double fTimeNs = (double)(iRealTimeFrameNs - sAudioEvent.iTimeStampNs);
				//float fTimeSec = (float)( fTimeNs * 1e-9 );
				//float fTimeW = (float)( fTimeNs / (double)sAudioEvent.iLifeTimeNs );
				//float fExpMaster = 1.0f - abs( powf( fTimeW, 0.1f ) - 0.5f ) * 2.0f;

				//calculate fExpmaster from sAudioEvent.iSampleCounter:
				const double dLifeTimeSamples = (double)sAudioEvent.iLifeTimeNs * sAudioBuffer.iSampleRate / 1e9;
				float fTimeW = (float)((double)sAudioEvent.iSampleCounter / dLifeTimeSamples);
				float fExpMaster = 1.0f - fabsf( powf( fTimeW, 0.3f ) - 0.5f ) * 2.0f;

				sAudioBuffer.pData[iFrameInd * 2 + iChInd] += (sAudioEvent.fVolume * 0.5f) * sinf( sAudioEvent.fPhase * PI2 ) * fExpMaster;

				if ( iChInd == 1 )
				{
					sAudioEvent.iSampleCounter++;

					if ( sAudioEvent.type == SAudioEvent::ClickDown )
					{
						sAudioEvent.fPhase += Lerp( 1000.0f, 440.0f, fTimeW ) / (float)sAudioBuffer.iSampleRate;
					}
					if ( sAudioEvent.type == SAudioEvent::ClickUp )
					{
						sAudioEvent.fPhase += Lerp( 700.0f, 440.0f, fTimeW ) / (float)sAudioBuffer.iSampleRate;
					}
					if ( sAudioEvent.type == SAudioEvent::GunShot )
					{
						sAudioEvent.fPhase += Lerp( 44.0f, 1000.0f, fTimeW ) / (float)sAudioBuffer.iSampleRate;
					}

					
					if ( sAudioEvent.fPhase >= 1.0f ) sAudioEvent.fPhase -= 1.0f;
				}
			}
		}
	}
}
