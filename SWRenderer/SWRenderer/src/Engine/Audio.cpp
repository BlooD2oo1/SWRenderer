#include "Audio.h"
#include "Common/Globals.h"

static float WaveShaper_Sin( float x, float fParam )
{
	return sinf( x * fParam );
}

static float WaveShaper_Power( float x, float fParam )
{
	return copysignf( powf( fabsf( x ), fParam ), x );
}

static float WaveShaper_Tan( float x, float fParam )
{
	return tanhf( x * fParam );
}

static float WaveShaper_Atan( float x, float fParam )
{
	return atanf( x * fParam ) / atanf( fParam );
}

static float WaveShaper_CubicSat( float x )
{
	return (x - x * x * x * 0.333333f) * 1.5f;
}

CAudio*	CAudio::m_pThis = nullptr;

CAudio::CAudio()
{
	m_iFrameInd = 0;
	m_iSampleCounter = 0;
	m_iStartTimeStampNs = 0;
	m_pEchoBuffer = nullptr;
	m_iEchoBufferSize = 0;
}

CAudio::~CAudio()
{
	SAFE_DELETE_ARRAY( m_pEchoBuffer );
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

	if ( m_iEchoBufferSize < sAudioBuffer.iNumFrames * 2 )
	{
		SAFE_DELETE_ARRAY( m_pEchoBuffer );
		m_pEchoBuffer = new float[sAudioBuffer.iNumFrames * 2];
		memset( m_pEchoBuffer, 0, sizeof(float) * sAudioBuffer.iNumFrames * 2 );
		m_iEchoBufferSize = sAudioBuffer.iNumFrames * 2;
	}

	//////////////////////////////////////////////////////////////////////////

	const uint64_t iDelayNs = (uint64_t)(60.0f * 1000.0f * 1000.0f);

	m_iFrameInd++;
	if ( m_iStartTimeStampNs == 0 )
	{
		m_iStartTimeStampNs = GetGlobalTimeStampNs();
	}

	m_iSampleCounter += sAudioBuffer.iNumFrames;

	uint64_t iTimeStampNs = m_iStartTimeStampNs + (uint64_t)((double)m_iSampleCounter * 1e9 / (double)sAudioBuffer.iSampleRate);
		
	const uint64_t iDelayedTimeStampNs = iTimeStampNs - iDelayNs;

	LOG( "AUDIO  %.4f sec ( delayed )\n", (double)(iDelayedTimeStampNs) / 1000.0 / 1000.0 / 1000.0 );

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
			if ( sAudioFrameData0.m_iTimeStampNs <= iDelayedTimeStampNs && iDelayedTimeStampNs < sAudioFrameData1.m_iTimeStampNs )
			{
				float fW = (float)(iDelayedTimeStampNs - sAudioFrameData0.m_iTimeStampNs) / (float)(sAudioFrameData1.m_iTimeStampNs - sAudioFrameData0.m_iTimeStampNs);
				sAudioFrameData.Lerp( sAudioFrameData0, sAudioFrameData1, fW );
				break;
			}
		}
	}

	{
		uint64_t iRealTimeBufferStartNs = iDelayedTimeStampNs;
		uint64_t iRealTimeBufferEndNs = iDelayedTimeStampNs + (uint64_t)((double)sAudioBuffer.iNumFrames * 1000.0 * 1000.0 * 1000.0 / (double)sAudioBuffer.iSampleRate);
		for ( int iEvent = 0; iEvent < m_aAudioEvents.size(); )
		{
			SAudioEvent& sAudioEvent = m_aAudioEvents[iEvent];
			if ( sAudioEvent.iTimeStampNs > iRealTimeBufferStartNs )
			{
				++iEvent;
				continue;										
			}
			if ( sAudioEvent.iTimeStampNs + sAudioEvent.iLifeTimeNs < iRealTimeBufferEndNs )
			{
				m_aAudioEvents[iEvent] = m_aAudioEvents.back();
				m_aAudioEvents.pop_back();
			}
			else
			{
				++iEvent;
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
		}

		for ( int iEvent = 0; iEvent < m_aAudioEvents.size(); iEvent++ )
		{
			SAudioEvent& sAudioEvent = m_aAudioEvents[iEvent];

			const double dLifeTimeSamples = (double)sAudioEvent.iLifeTimeNs * (double)sAudioBuffer.iSampleRate / 1000.0 / 1000.0 / 1000.0;
			float fTimeW = (float)((double)sAudioEvent.iSampleCounter / dLifeTimeSamples);
			if ( fTimeW > 1.0f ) continue;

			float fExpMaster = 1.0f - fabsf( powf( fTimeW, 0.2f ) - 0.5f ) * 2.0f;

			for ( int iChInd = 0; iChInd < 2; iChInd++ )
			{
				float x = sinf( sAudioEvent.fPhase * PI2 );

				switch( sAudioEvent.type )
				{
					case SAudioEvent::ClickDown:
					x = WaveShaper_CubicSat( x );
					break;
					case SAudioEvent::ClickUp:
					x = WaveShaper_CubicSat( x );
					break;
					case SAudioEvent::GunShot:
					x = WaveShaper_CubicSat( x );
					break;
				}
				sAudioBuffer.pData[iFrameInd * 2 + iChInd] += (sAudioEvent.fVolume * 0.5f) * x * fExpMaster;
			}

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
				sAudioEvent.fPhase += Lerp( 200.0f, 1480.0f, fTimeW*fTimeW ) / (float)sAudioBuffer.iSampleRate;
			}

					
			if ( sAudioEvent.fPhase >= 1.0f ) sAudioEvent.fPhase -= 1.0f;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	for (uint32_t iFrameInd = 0; iFrameInd < sAudioBuffer.iNumFrames; iFrameInd++)
	{
		// 1. Calculate exact time in seconds 
		// (since m_iSampleCounter was already incremented at the beginning of the function)
		uint64_t currentSample = m_iSampleCounter - sAudioBuffer.iNumFrames + iFrameInd;
		double t = (double)currentSample / (double)sAudioBuffer.iSampleRate;

		// 32-second global loop phase (0.0 - 1.0)
		float loopPhase = (float)(fmod(t, 32.0) / 32.0);

		// --- KICK DRUM (On every quarter note, 120 BPM = 0.5 sec) ---
		float tk = fmodf((float)t, 0.5f);
		// ADSR-like envelope formula, starting smoothly from zero without pops: t * e^(-t)
		float envK = tk * 50.0f * expf(-tk * 50.0f) * 2.71828f; 
		// FM Kick with integrated phase for the sweep
		float kick = sinf(PI2 * (40.0f * tk - 5.0f * expf(-tk * 30.0f))) * envK;
		kick = WaveShaper_Power(kick, 0.5f) * 0.9f; // Punchy distortion

		// --- BASSLINE (16th note groove) ---
		float tb = fmodf((float)t, 0.25f);
		float envB = tb * 30.0f * expf(-tb * 30.0f) * 2.71828f;
		const float bassFreqs[16] = { 65.41f, 65.41f, 77.78f, 65.41f, 87.31f, 65.41f, 77.78f, 98.00f,
			65.41f, 65.41f, 77.78f, 65.41f, 130.81f,65.41f, 77.78f, 49.00f };
		int bassStep = (int)(t * 4.0) % 16;
		float fB = bassFreqs[bassStep];
		if (loopPhase > 0.5f) fB *= 0.5f; // Octave drop in the second half of the loop (after 16 sec)
		float bass = sinf(PI2 * fB * tb) * envB;
		bass = WaveShaper_CubicSat(bass * 2.5f) * 0.6f;

		// --- PAD / CHORDS (Slowly floating synthesizer) ---
		float padMod = sinf(PI2 * 0.5f * (float)t); // Slow vibrato / FM on the oscillators
		float pad1 = sinf(PI2 * 130.81f * (float)t) + sinf(PI2 * 196.00f * (float)t) + sinf(PI2 * 311.13f * (float)t + padMod);
		float pad2 = sinf(PI2 * 103.83f * (float)t) + sinf(PI2 * 155.56f * (float)t) + sinf(PI2 * 261.63f * (float)t + padMod);
		float padMix = (sinf(PI2 * (float)t / 16.0f) * 0.5f + 0.5f); // 16-second crossfade between the two chords
		float pad = (pad1 * (1.0f - padMix) + pad2 * padMix) * 0.05f;
		pad = WaveShaper_Tan(pad, 2.0f);
		float padEnv = 0.5f - 0.5f * cosf(PI2 * loopPhase); // Smooth fade in and out during the cycle
		pad *= padEnv;

		// --- ARPEGGIO (With ping-pong delay, using a lambda function) ---
		auto GetArp = [](double timeStr) -> float {
			if (timeStr < 0.0) return 0.0f; // <--- bugfix

			float ta = fmodf((float)timeStr, 0.125f);
			float envA = ta * 40.0f * expf(-ta * 40.0f) * 2.71828f;
			const float arpFreqs[16] = { 523.25f, 392.00f, 622.25f, 523.25f, 
				783.99f, 622.25f, 523.25f, 466.16f,
				523.25f, 783.99f, 622.25f, 880.00f,
				783.99f, 622.25f, 523.25f, 392.00f };
			int arpStep = (int)(timeStr * 8.0) % 16;
			float fA = arpFreqs[arpStep];

			// 2-operator FM synthesis
			float arpOsc = sinf(PI2 * fA * ta + sinf(PI2 * fA * 1.5f * ta) * 2.0f * envA);
			return WaveShaper_Atan(arpOsc, 4.0f) * envA * 0.15f;
			};
		float arpL = GetArp(t);
		float arpR = GetArp(t - 0.375); // Mathematical delay of 3/16 notes on the right channel!

		// --- HI-HAT (From pseudo-random noise) ---
		uint32_t seed = (uint32_t)(t * 44100.0) * 1664525 + 1013904223; // Simple LCG hash for noise generation
		float noise = (float)seed * 4.6566129e-10f - 1.0f; // Scaled between -1 and 1
		float th = fmodf((float)t, 0.125f);
		bool accent = ((int)(t * 8.0) % 4) == 2; // Off-beat open hi-hat
		float envH = expf(-th * (accent ? 40.0f : 80.0f));
		float hat = WaveShaper_CubicSat(noise * envH * 0.08f);

		// --- MIXING & MASTERING ---
		float mixL = kick + bass + pad + arpL + hat;
		float mixR = kick + bass + pad + arpR + hat * 0.7f;

		// Using WaveShaper_Tan on the Master bus as a limiter and for subtle saturation (warming)
		mixL = WaveShaper_Tan(mixL, 1.2f) * 0.1f;
		mixR = WaveShaper_Tan(mixR, 1.2f) * 0.1f;

		// ADD the mix to the existing output (the += operator is important to keep events / gunshots)
		sAudioBuffer.pData[iFrameInd * 2 + 0] += mixL;
		sAudioBuffer.pData[iFrameInd * 2 + 1] += mixR;
	}

	//////////////////////////////////////////////////////////////////////////

	for (uint32_t iFrameInd = 0; iFrameInd < sAudioBuffer.iNumFrames; iFrameInd++)
	{
		for ( int iChInd = 0; iChInd < 2; iChInd++ )
		{

		}
	}
	}
