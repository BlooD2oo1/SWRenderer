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
		for ( int iChInd = 0; iChInd < 2; iChInd++ )
		{

		}
	}

	Music( sAudioBuffer, sAudioFrameData.m_fMusic_Action, sAudioFrameData.m_fMusic_Climax );
}
static float Synth_GetArp( float timeStr, float fAction, float fClimax )
{
	float ta = fmodf(timeStr, 0.125f);
	float envA = ta * 40.0f * expf(-ta * 40.0f) * 2.71828f;
	const float arpFreqs[16] = { 523.25f, 392.00f, 622.25f, 523.25f, 
		783.99f, 622.25f, 523.25f, 466.16f,
		523.25f, 783.99f, 622.25f, 880.00f,
		783.99f, 622.25f, 523.25f, 392.00f };
	int arpStep = (int)(timeStr * 8.0f) % 16;
	float fA = arpFreqs[arpStep];

	// FM modulation depth driven by action and climax
	float fmDepth = 1.0f + fClimax * 0.8f + fAction * 1.2f;

	// 2-operator FM synthesis for the arpeggio
	float arpOsc = sinf(PI2 * fA * ta + sinf(PI2 * fA * 1.5f * ta) * fmDepth * envA);

	float vol = 0.05f + fClimax * 0.03f + fAction * 0.02f;
	return WaveShaper_Atan(arpOsc, 4.0f) * envA * vol;
}

void CAudio::Music( SAudioBuffer& sAudioBuffer, float fAction, float fClimax )
{
	// Calculate how many samples make up exactly 32 seconds
	const uint32_t loopLenSamples = 32 * sAudioBuffer.iSampleRate;

	for (uint32_t iFrameInd = 0; iFrameInd < sAudioBuffer.iNumFrames; iFrameInd++)
	{
		// 1. Get the current absolute sample, then wrap it around the 32 sec loop length
		uint64_t currentSample = m_iSampleCounter - sAudioBuffer.iNumFrames + iFrameInd;
		uint32_t loopSample = (uint32_t)(currentSample % loopLenSamples);

		// 2. Calculate time in seconds, strictly between 0.0f and 31.999f
		float t = (float)loopSample / (float)sAudioBuffer.iSampleRate;

		// 0.0 - 1.0 phase of the whole loop
		float loopPhase = t / 32.0f; 

		// --- KICK DRUM ---
		float tk = fmodf(t, 0.5f); 
		float envK = tk * 50.0f * expf(-tk * 50.0f) * 2.71828f; 

		float kickPunch = 40.0f + fAction * 20.0f;
		float kick = sinf(PI2 * (kickPunch * tk - 5.0f * expf(-tk * 30.0f))) * envK;

		float kickDistortion = 0.6f - fAction * 0.05f; 
		float kickVol = 0.45f + fClimax * 0.05f + fAction * 0.05f;
		kick = WaveShaper_Power(kick, kickDistortion) * kickVol;

		// --- BASSLINE ---
		float tb = fmodf(t, 0.25f);
		float envB = tb * 30.0f * expf(-tb * 30.0f) * 2.71828f;

		const float bassFreqs[16] = { 65.41f, 65.41f, 77.78f, 65.41f, 87.31f, 65.41f, 77.78f, 98.00f,
			65.41f, 65.41f, 77.78f, 65.41f, 130.81f,65.41f, 77.78f, 49.00f };

		int bassStep = (int)(t * 4.0f) % 16;
		float fB = bassFreqs[bassStep];
		if (loopPhase > 0.5f) fB *= 0.5f; 

		bool isOffBeat = (bassStep % 2 != 0);
		float intensity = (fAction > fClimax) ? fAction : fClimax;
		float stepActivityMultiplier = 1.0f;
		if (isOffBeat)
		{
			stepActivityMultiplier = 0.2f + 0.8f * intensity;
		}

		// Gentle FM growl for bass
		float bassFmEnv = tb * 20.0f * expf(-tb * 20.0f);
		float bassMod = sinf(PI2 * fB * 2.0f * tb) * fAction * 1.5f * bassFmEnv;
		float bass = sinf(PI2 * fB * tb + bassMod) * envB;

		float bassDrive = 1.1f + fClimax * 0.3f + fAction * 0.4f; 
		float bassVol = (0.3f + fClimax * 0.05f) * stepActivityMultiplier;
		bass = WaveShaper_CubicSat(bass * bassDrive) * bassVol;

		// --- PAD / CHORDS (Stronger, prominent feature, blooms towards high climax) ---
		float padMod = sinf(PI2 * 0.5f * t); 
		// Richer unison/detune oscillators for a powerful, cinematic chord body
		float pad1 = sinf(PI2 * 130.81f * t) + sinf(PI2 * 196.00f * t) + sinf(PI2 * 311.13f * t + padMod) 
			+ sinf(PI2 * 131.20f * t); // slightly detuned for thickness
		float pad2 = sinf(PI2 * 103.83f * t) + sinf(PI2 * 155.56f * t) + sinf(PI2 * 261.63f * t + padMod)
			+ sinf(PI2 * 104.10f * t); // slightly detuned for thickness

		float padMix = (sinf(PI2 * t / 16.0f) * 0.5f + 0.5f); 
		float pad = (pad1 * (1.0f - padMix) + pad2 * padMix) * 0.04f;

		// Warmer saturation profile to give the pad a bold, defining character
		pad = WaveShaper_Tan(pad, 2.0f);
		float padEnv = 0.5f - 0.5f * cosf(PI2 * loopPhase); 

		// Strong scaling based on fClimax (powers up heavily near 1.0) with mild action sidechain pump
		float climaxCurve = fClimax * fClimax; 
		float pumpEffect = 1.0f - (fAction * 0.4f * expf(-tk * 15.0f));
		float padVolMultiplier = 0.2f + 1.3f * climaxCurve; // Becomes a dominant lead element at high climax
		pad *= padEnv * padVolMultiplier * pumpEffect;

		// --- ARPEGGIO ---
		float arpL = Synth_GetArp(t, fAction, fClimax);

		float tR = t - 0.375f;
		if (tR < 0.0f) tR += 32.0f;
		float arpR = Synth_GetArp(tR, fAction, fClimax); 

		// --- HI-HAT ---
		uint32_t seed = loopSample * 1664525 + 1013904223; 
		float noise = (float)seed * 4.6566129e-10f - 1.0f; 
		float th = fmodf(t, 0.125f);
		bool accent = ((int)(t * 8.0f) % 4) == 2; 

		float decayBase = accent ? 40.0f : 80.0f;
		float decay = decayBase - fAction * (decayBase * 0.2f);
		float envH = expf(-th * decay);
		float hatVol = 0.035f + fClimax * 0.015f + fAction * 0.01f;
		float hat = WaveShaper_CubicSat(noise * envH) * hatVol;

		// --- MIXING & MASTERING ---
		// Combined sum of all layers
		float mixL = kick + bass + pad + arpL + hat;
		float mixR = kick + bass + pad + arpR + hat * 0.7f;

		// Master limiting stage optimized to handle the richer, louder pad without digital clipping
		float masterDrive = 1.0f + fAction * 0.1f;
		mixL = WaveShaper_Tan(mixL * masterDrive, 1.2f) * 0.2f;
		mixR = WaveShaper_Tan(mixR * masterDrive, 1.2f) * 0.2f;

		sAudioBuffer.pData[iFrameInd * 2 + 0] += mixL;
		sAudioBuffer.pData[iFrameInd * 2 + 1] += mixR;
	}
}