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

// ============================================================================
// DSP & CHIPTUNE OSCILLATORS
// ============================================================================

// Variable width Pulse / Square wave (Classic 8-bit sound: 12.5%, 25%, 50% PWM)
static float Osc_Pulse( float fPhase, float fPwm = 0.5f )
{
	float fP = fmodf( fPhase, 1.0f );
	if ( fP < 0.0f ) fP += 1.0f;
	return ( fP < fPwm ) ? 1.0f : -1.0f;
}

// NES-style 8-bit Triangle wave for smooth chiptune bass
static float Osc_Triangle( float fPhase )
{
	float fP = fmodf( fPhase, 1.0f );
	if ( fP < 0.0f ) fP += 1.0f;
	return 4.0f * fabsf( fP - 0.5f ) - 1.0f;
}

// 8-bit Pseudo-Random Noise generator (LFSR style for chiptune drums)
static float Osc_8BitNoise( uint32_t& uSeed )
{
	uSeed = uSeed * 1664525u + 1013904223u;
	// Quantize noise to 16 discrete levels for gritty arcade texture
	int iLevel = ( uSeed >> 28 ) & 0x0F;
	return ( (float)iLevel / 7.5f ) - 1.0f;
}

// Bitcrusher effect for authentic retro output
static float FX_Bitcrush( float fInput, float fSteps = 16.0f )
{
	return floorf( fInput * fSteps ) / fSteps;
}

// ============================================================================

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

// 
// // --- HELPER: MIDI Note -> Frequency (Hz) ---
// static float MidiToFreq( int iMidiNote )
// {
// 	if ( iMidiNote <= 0 ) return 0.0f; // 0 or negative = Pause / Silence
// 	return 440.0f * powf( 2.0f, ( (float)iMidiNote - 69.0f ) / 12.0f );
// }
// 
// // --- ARPEGGIO PATTERN ---
// // C5 (72), G4 (67), D#5 (75), A#4 (70), A5 (81)
// static const int g_ArpSequence[] = {
// 	72, 67, 75, 72,
// 	79, 75, 72, 70,
// 	72, 79, 75, 81,
// 	79, 75, 72, 67
// };
// static const int g_iArpSequenceCount = sizeof( g_ArpSequence ) / sizeof( g_ArpSequence[0] );
// 
// // --- BASSLINE PATTERN ---
// // C2 (36), D#2 (39), F2 (41), G2 (43), C3 (48), G1 (31)
// static const int g_BassSequence[] = {
// 	36, 36, 39, 36, 41, 36, 39, 43,
// 	36, 36, 39, 36, 48, 36, 39, 31
// };
// static const int g_iBassSequenceCount = sizeof( g_BassSequence ) / sizeof( g_BassSequence[0] );
// 
// // --- PAD / CHORD STRUCTURE ---
// struct SChord
// {
// 	int aNotes[4]; // 4-note chord voicing
// };
// 
// static const SChord g_PadChords[] = {
// 	{ { 48, 55, 63, 48 } }, // C minor (C3, G3, D#4, C3)
// 	{ { 44, 51, 60, 44 } }  // G# major (G#2, D#3, C4, G#2)
// };
// static const int g_iPadChordCount = sizeof( g_PadChords ) / sizeof( g_PadChords[0] );
// 
// // --- ARPEGGIO SYNTHESIZER ---
// static float Synth_GetArp( float timeStr, float fAction, float fClimax )
// {
// 	float ta = fmodf( timeStr, 0.125f );
// 	float envA = ta * 40.0f * expf( -ta * 40.0f ) * 2.71828f;
// 
// 	int arpStep = (int)( timeStr * 8.0f ) % g_iArpSequenceCount;
// 	float fA = MidiToFreq( g_ArpSequence[arpStep] );
// 
// 	// FM modulation depth driven by action and climax
// 	float fmDepth = 1.0f + fClimax * 0.8f + fAction * 1.2f;
// 
// 	// 2-operator FM synthesis for the arpeggio
// 	float arpOsc = sinf( PI2 * fA * ta + sinf( PI2 * fA * 1.5f * ta ) * fmDepth * envA );
// 
// 	float vol = 0.05f + fClimax * 0.03f + fAction * 0.02f;
// 	return WaveShaper_Atan( arpOsc, 4.0f ) * envA * vol;
// }
// 
// // --- MAIN MUSIC GENERATOR ---
// void CAudio::Music( SAudioBuffer& sAudioBuffer, float fAction, float fClimax )
// {
// 	// Calculate how many samples make up exactly 32 seconds
// 	const uint32_t loopLenSamples = 32 * sAudioBuffer.iSampleRate;
// 
// 	for ( uint32_t iFrameInd = 0; iFrameInd < sAudioBuffer.iNumFrames; iFrameInd++ )
// 	{
// 		// 1. Get current absolute sample, then wrap around the 32 sec loop length
// 		uint64_t currentSample = m_iSampleCounter - sAudioBuffer.iNumFrames + iFrameInd;
// 		uint32_t loopSample = (uint32_t)( currentSample % loopLenSamples );
// 
// 		// 2. Calculate time in seconds, strictly between 0.0f and 31.999f
// 		float t = (float)loopSample / (float)sAudioBuffer.iSampleRate;
// 
// 		// Normalized phase (0.0 - 1.0) for the whole loop
// 		float loopPhase = t / 32.0f;
// 
// 		float tk = fmodf( t, 0.5f );
// 
// 		// --- KICK DRUM ---
// 		float kick = 0.0f;
// 		{
// 			float envK = tk * 50.0f * expf( -tk * 50.0f ) * 2.71828f;
// 
// 			float kickPunch = 40.0f + fAction * 20.0f;
// 			kick = sinf( PI2 * ( kickPunch * tk - 5.0f * expf( -tk * 30.0f ) ) ) * envK;
// 
// 			float kickDistortion = 0.6f - fAction * 0.05f;
// 			float kickVol = 0.45f + fClimax * 0.05f + fAction * 0.05f;
// 			kick = WaveShaper_Power( kick, kickDistortion ) * kickVol;
// 		}
// 
// 		// --- BASSLINE ---
// 		float bass = 0.0f;
// 		{
// 			float tb = fmodf( t, 0.25f );
// 			float envB = tb * 30.0f * expf( -tb * 30.0f ) * 2.71828f;
// 
// 			int bassStep = (int)( t * 4.0f ) % g_iBassSequenceCount;
// 			float fB = MidiToFreq( g_BassSequence[bassStep] );
// 
// 			if ( loopPhase > 0.5f ) fB *= 0.5f; // Octave drop in the second half of the loop
// 
// 			bool isOffBeat = ( bassStep % 2 != 0 );
// 			float intensity = ( fAction > fClimax ) ? fAction : fClimax;
// 			float stepActivityMultiplier = 1.0f;
// 			if ( isOffBeat )
// 			{
// 				stepActivityMultiplier = 0.2f + 0.8f * intensity;
// 			}
// 
// 			// Gentle FM growl for bass
// 			float bassFmEnv = tb * 20.0f * expf( -tb * 20.0f );
// 			float bassMod = sinf( PI2 * fB * 2.0f * tb ) * fAction * 1.5f * bassFmEnv;
// 			bass = sinf( PI2 * fB * tb + bassMod ) * envB;
// 
// 			float bassDrive = 1.1f + fClimax * 0.3f + fAction * 0.4f;
// 			float bassVol = ( 0.3f + fClimax * 0.05f ) * stepActivityMultiplier;
// 			bass = WaveShaper_CubicSat( bass * bassDrive ) * bassVol;
// 		}
// 
// 		// --- PAD / CHORDS ---
// 		float pad = 0.0f;
// 		{
// 			float padMod = sinf( PI2 * 0.5f * t );
// 
// 			// Fetch frequencies for Chord 1 (C minor) and Chord 2 (G# major) from MIDI data
// 			const SChord& c1 = g_PadChords[0];
// 			const SChord& c2 = g_PadChords[1];
// 
// 			float pad1 = sinf( PI2 * MidiToFreq( c1.aNotes[0] ) * t ) +
// 				sinf( PI2 * MidiToFreq( c1.aNotes[1] ) * t ) +
// 				sinf( PI2 * MidiToFreq( c1.aNotes[2] ) * t + padMod ) +
// 				sinf( PI2 * ( MidiToFreq( c1.aNotes[3] ) + 0.39f ) * t ); // slight detune
// 
// 			float pad2 = sinf( PI2 * MidiToFreq( c2.aNotes[0] ) * t ) +
// 				sinf( PI2 * MidiToFreq( c2.aNotes[1] ) * t ) +
// 				sinf( PI2 * MidiToFreq( c2.aNotes[2] ) * t + padMod ) +
// 				sinf( PI2 * ( MidiToFreq( c2.aNotes[3] ) + 0.27f ) * t ); // slight detune
// 
// 			float padMix = ( sinf( PI2 * t / 16.0f ) * 0.5f + 0.5f );
// 			pad = ( pad1 * ( 1.0f - padMix ) + pad2 * padMix ) * 0.04f;
// 
// 			// Warmer saturation profile
// 			pad = WaveShaper_Tan( pad, 2.0f );
// 			float padEnv = 0.5f - 0.5f * cosf( PI2 * loopPhase );
// 
// 			float climaxCurve = fClimax * fClimax;
// 			float pumpEffect = 1.0f - ( fAction * 0.4f * expf( -tk * 15.0f ) );
// 			float padVolMultiplier = 0.2f + 1.3f * climaxCurve;
// 			pad *= padEnv * padVolMultiplier * pumpEffect;
// 		}
// 
// 		// --- ARPEGGIO ---
// 		float arpL = 0.0f;
// 		float arpR = 0.0f;
// 		{
// 			arpL = Synth_GetArp( t, fAction, fClimax );
// 
// 			float tR = t - 0.375f;
// 			if ( tR < 0.0f ) tR += 32.0f;
// 
// 			arpR = Synth_GetArp( tR, fAction, fClimax );
// 		}
// 
// 		// --- HI-HAT ---
// 		float hat = 0.0f;
// 		{
// 			uint32_t seed = loopSample * 1664525 + 1013904223;
// 			float noise = (float)seed * 4.6566129e-10f - 1.0f;
// 			float th = fmodf( t, 0.125f );
// 			bool accent = ( (int)( t * 8.0f ) % 4 ) == 2;
// 
// 			float decayBase = accent ? 40.0f : 80.0f;
// 			float decay = decayBase - fAction * ( decayBase * 0.2f );
// 			float envH = expf( -th * decay );
// 			float hatVol = 0.035f + fClimax * 0.015f + fAction * 0.01f;
// 			hat = WaveShaper_CubicSat( noise * envH ) * hatVol;
// 		}
// 
// 		// --- MIXING & MASTERING ---
// 		float mixL = kick + bass + pad + arpL + hat;
// 		float mixR = kick + bass + pad + arpR + hat * 0.7f;
// 
// 		float masterDrive = 1.0f + fAction * 0.1f;
// 		mixL = WaveShaper_Tan( mixL * masterDrive, 1.2f ) * 0.2f;
// 		mixR = WaveShaper_Tan( mixR * masterDrive, 1.2f ) * 0.2f;
// 
// 		sAudioBuffer.pData[iFrameInd * 2 + 0] += mixL;
// 		sAudioBuffer.pData[iFrameInd * 2 + 1] += mixR;
// 	}
// }

static float MidiToFreq( int iMidiNote )
{
	if ( iMidiNote <= 0 ) return 0.0f;
	return 440.0f * powf( 2.0f, ( (float)iMidiNote - 69.0f ) / 12.0f );
}

// ============================================================================
// CHORD SYSTEM & PROGRESSION ENGINE (PO-20 Arcade Core)
// ============================================================================

struct SChord
{
	const char* pName;
	int iRootMidi;      // Base MIDI root note (e.g., 57 = A3)
	int aIntervals[4];  // Semitone offsets from root: {0, 3, 7, 12}
};

// PO-20 Style Arcade Chords Palette
static const SChord g_ArcadeChords[] = {
	{ "Am",  57, { 0, 3, 7, 12 } }, // 0: A minor
	{ "F",   53, { 0, 4, 7, 12 } }, // 1: F major
	{ "C",   60, { 0, 4, 7, 12 } }, // 2: C major
	{ "G",   55, { 0, 4, 7, 12 } }, // 3: G major
	{ "Dm",  50, { 0, 3, 7, 12 } }, // 4: D minor
	{ "Em",  52, { 0, 3, 7, 12 } }, // 5: E minor
	{ "E7",  52, { 0, 4, 7, 10 } }, // 6: E dominant 7
	{ "Fm",  53, { 0, 3, 7, 12 } }  // 7: F minor
};
static const int g_iNumArcadeChords = sizeof( g_ArcadeChords ) / sizeof( g_ArcadeChords[0] );

// 16-Bar Chord Progression Chain (Controls global song harmony)
static const int g_ChordProgression[16] = {
	0, 1, 2, 3, // Am -> F  -> C -> G
	0, 1, 2, 3, // Am -> F  -> C -> G
	4, 5, 0, 6, // Dm -> Em -> Am -> E7
	1, 3, 0, 0  // F  -> G  -> Am -> Am
};

// ============================================================================
// 16-STEP SEQUENCER PATTERNS
// ============================================================================

// Drum Pattern (16th notes: 1 = Kick, 2 = Snare, 3 = HiHat)
static const int g_PatternDrums[16] = {
	1, 3, 3, 3,  2, 3, 1, 3,  1, 3, 2, 3,  2, 3, 1, 3
};

// Bass Pattern (Relative chord note index: 0 = Root, 1 = 3rd, 2 = 5th, 3 = Octave, -1 = Rest)
static const int g_PatternBass[16] = {
	0, -1, 0, 0,  2, -1, 0, -1,  3, -1, 0, 0,  2, 0, 1, 2
};

// Lead Pattern (Relative chord note index)
static const int g_PatternLead[16] = {
	0, 1, 2, 3,  2, 1, 0, 2,  3, 2, 1, 0,  1, 2, 3, 1
};

// ============================================================================
// MAIN PO-ARCADE SYNTHESIS ENGINE
// ============================================================================

void CAudio::Music( SAudioBuffer& sAudioBuffer, float fAction, float fClimax )
{
	const float fBPM = 60.0f; // Classic high-tempo arcade speed
	const float fSecondsPerBeat = 60.0f / fBPM;
	const float fSecondsPer16th = fSecondsPerBeat / 4.0f;
	const float fSecondsPerBar = fSecondsPerBeat * 4.0f;

	// Total length of the 16-bar chord chain in seconds
	const float fTotalLoopSec = fSecondsPerBar * 16.0f;

	for ( uint32_t iFrameInd = 0; iFrameInd < sAudioBuffer.iNumFrames; iFrameInd++ )
	{
		// Calculate current time modulo loop length
		uint64_t currentSample = m_iSampleCounter - sAudioBuffer.iNumFrames + iFrameInd;
		float t = fmodf( (float)currentSample / (float)sAudioBuffer.iSampleRate, fTotalLoopSec );

		// --- 1. SEQUENCER TIMING & CHORD CHAIN TRACKING ---
		int iCurrentBar = (int)( t / fSecondsPerBar ) % 16;
		int iCurrent16th = (int)( t / fSecondsPer16th ) % 16;
		float fStepTime = fmodf( t, fSecondsPer16th ); // Time within current 16th step

		// Retrieve active chord from the PO-20 chord progression chain
		int iChordIdx = g_ChordProgression[iCurrentBar];
		const SChord& activeChord = g_ArcadeChords[iChordIdx];

		// Dynamic pulse width modulation driven by performance params
		float fGlobalPwm = 0.25f + 0.2f * sinf( PI2 * 0.2f * t ) + 0.15f * fAction;

		// --- 2. CHIPTUNE KICK / SNARE / HI-HAT (DRUMS) ---
		float fDrums = 0.0f;
		{
			int iDrumStep = g_PatternDrums[iCurrent16th];
			uint32_t uNoiseSeed = (uint32_t)( currentSample + iFrameInd * 17 );

			if ( iDrumStep == 1 ) // Arcade Kick (Pitch-dropping pulse/sine drop)
			{
				float fEnv = expf( -fStepTime * 35.0f );
				float fFreq = 120.0f * expf( -fStepTime * 40.0f ) + 30.0f;
				float fKickOsc = Osc_Pulse( fFreq * fStepTime, 0.5f );
				fDrums += fKickOsc * fEnv * 0.45f;
			}
			else if ( iDrumStep == 2 ) // Arcade Snare (Noise + low-pitched pop)
			{
				float fEnv = expf( -fStepTime * 25.0f );
				float fNoise = Osc_8BitNoise( uNoiseSeed );
				float fTone = Osc_Triangle( 180.0f * fStepTime );
				fDrums += ( fNoise * 0.7f + fTone * 0.3f ) * fEnv * 0.35f;
			}
			else if ( iDrumStep == 3 ) // Arcade Hi-Hat (Short metallic noise burst)
			{
				float fEnv = expf( -fStepTime * 80.0f );
				float fNoise = Osc_8BitNoise( uNoiseSeed );
				fDrums += fNoise * fEnv * ( 0.12f + fClimax * 0.08f );
			}
		}

		// --- 3. BASSLINE (TRIANGLE WAVE) ---
		float fBass = 0.0f;
		{
			int iBassNoteIdx = g_PatternBass[iCurrent16th];
			if ( iBassNoteIdx >= 0 )
			{
				float fEnv = expf( -fStepTime * 12.0f );
				int iMidiNote = activeChord.iRootMidi - 24 + activeChord.aIntervals[iBassNoteIdx % 4];
				float fFreq = MidiToFreq( iMidiNote );

				float fBassOsc = Osc_Triangle( fFreq * t );
				fBass = fBassOsc * fEnv * 0.35f;
			}
		}

		// --- 4. CHIPTUNE FAST-ARP CHORD PAD (1-OSC FAST ARPEGGIO) ---
		// Classic NES trick: Cycling chord notes rapidly (30-40 Hz) on a single channel
		float fChordArp = 0.0f;
		{
			float fArpSpeedHz = 32.0f + fAction * 16.0f; // Arp speed accelerates with Action
			int iArpSubStep = (int)( t * fArpSpeedHz ) % 4;
			int iMidiNote = activeChord.iRootMidi + activeChord.aIntervals[iArpSubStep];
			float fFreq = MidiToFreq( iMidiNote );

			float fPadEnv = 0.6f + 0.4f * sinf( PI2 * ( t / fSecondsPerBar ) );
			float fPulse = Osc_Pulse( fFreq * t, fGlobalPwm );

			fChordArp = fPulse * 0.08f * fPadEnv * ( 0.4f + 0.6f * fClimax );
		}

		// --- 5. LEAD SYNTH (PWM PULSE WAVE) ---
		float fLead = 0.0f;
		{
			int iLeadNoteIdx = g_PatternLead[iCurrent16th];
			if ( iLeadNoteIdx >= 0 )
			{
				float fEnv = expf( -fStepTime * 18.0f );
				int iMidiNote = activeChord.iRootMidi + 12 + activeChord.aIntervals[iLeadNoteIdx % 4];
				float fFreq = MidiToFreq( iMidiNote );

				// Dual pulse with slight octave/detune layering for a rich lead sound
				float fLeadOsc1 = Osc_Pulse( fFreq * t, fGlobalPwm );
				float fLeadOsc2 = Osc_Pulse( ( fFreq * 1.003f ) * t, 0.125f );

				fLead = ( fLeadOsc1 + fLeadOsc2 * 0.5f ) * fEnv * 0.15f;
			}
		}

		// --- 6. MIXING & MASTERING (LO-FI BITCRUSHER) ---
		float fMixL = fDrums + fBass + fChordArp + fLead;
		float fMixR = fDrums + fBass + fChordArp * 0.7f + fLead;

		// Master drive and 8-bit quantization / bitcrushing stage
		float fMasterDrive = 1.0f + fAction * 0.2f;
		fMixL = FX_Bitcrush( WaveShaper_Tan( fMixL * fMasterDrive, 1.1f ), 24.0f ) * 0.25f;
		fMixR = FX_Bitcrush( WaveShaper_Tan( fMixR * fMasterDrive, 1.1f ), 24.0f ) * 0.25f;

		sAudioBuffer.pData[iFrameInd * 2 + 0] += fMixL;
		sAudioBuffer.pData[iFrameInd * 2 + 1] += fMixR;
	}
}