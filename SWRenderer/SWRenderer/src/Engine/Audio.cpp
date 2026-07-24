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

static float MidiToFreq( int iMidiNote )
{
	if ( iMidiNote <= 0 ) return 0.0f;
	return 440.0f * powf( 2.0f, ( (float)iMidiNote - 69.0f ) / 12.0f );
}

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

// 8-bit Pseudo-Random Noise generator (LFSR style for chiptune drums & sfx)
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

void CAudio::AudioThread_Update( SAudioBuffer& sAudioBuffer )
{
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

	//LOG( "AUDIO  %.4f sec ( delayed )\n", (double)(iDelayedTimeStampNs) / 1000.0 / 1000.0 / 1000.0 );

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
	// AMBIENT ENGINE SOUND GENERATION (RETRO SPACESHIP THRUSTER & TURBINE)
	//////////////////////////////////////////////////////////////////////////

	// Persistent engine synthesis states
	static float s_fEnginePhaseSub = 0.0f;
	static float s_fEnginePhaseTurbineL = 0.0f;
	static float s_fEnginePhaseTurbineR = 0.0f;
	static float s_fEngineLFO = 0.0f;
	static uint32_t s_uEngineNoiseSeed = 1337u;

	const float fSpeed = sAudioFrameData.m_fShipSpeed;

	// Dynamic pitch targets based on ship velocity
	const float fSubFreq     = 35.0f + fSpeed * 45.0f;      // Deep sub-thrust rumble (35Hz -> 80Hz)
	const float fTurbineFreq = 110.0f + fSpeed * 520.0f;    // Sci-Fi plasma turbine whine (110Hz -> 630Hz)
	const float fLFOFreq     = 8.0f + fSpeed * 14.0f;       // Engine throb / LFO rate

	const float fDt = 1.0f / (float)sAudioBuffer.iSampleRate;

	for (uint32_t iFrameInd = 0; iFrameInd < sAudioBuffer.iNumFrames; iFrameInd++)
	{
		// Advance oscillator phases
		s_fEnginePhaseSub += fSubFreq * fDt;
		if ( s_fEnginePhaseSub >= 1.0f ) s_fEnginePhaseSub -= 1.0f;

		s_fEnginePhaseTurbineL += fTurbineFreq * fDt;
		if ( s_fEnginePhaseTurbineL >= 1.0f ) s_fEnginePhaseTurbineL -= 1.0f;

		// Stereo detune for wider spatial reactor field
		s_fEnginePhaseTurbineR += ( fTurbineFreq * 1.006f ) * fDt;
		if ( s_fEnginePhaseTurbineR >= 1.0f ) s_fEnginePhaseTurbineR -= 1.0f;

		s_fEngineLFO += fLFOFreq * fDt;
		if ( s_fEngineLFO >= 1.0f ) s_fEngineLFO -= 1.0f;

		// 1. Sub-Bass Rumble (Modulated triangle wave + 8-bit noise texture)
		float fLfoVal = 0.7f + 0.3f * sinf( s_fEngineLFO * PI2 );
		float fNoiseVal = Osc_8BitNoise( s_uEngineNoiseSeed );
		float fSubRumble = Osc_Triangle( s_fEnginePhaseSub ) * ( 0.65f + 0.35f * fNoiseVal ) * fLfoVal;

		// 2. Plasma Turbine Whine (PWM Pulse wave)
		float fPwm = 0.15f + 0.10f * sinf( s_fEngineLFO * PI2 * 0.5f );
		float fTurbineL = Osc_Pulse( s_fEnginePhaseTurbineL, fPwm );
		float fTurbineR = Osc_Pulse( s_fEnginePhaseTurbineR, fPwm );

		// 3. Exhaust Noise Grit
		float fExhaustNoise = fNoiseVal * ( 0.15f + 0.65f * fSpeed );

		// Mix channels
		float fEngineL = ( fSubRumble * 0.55f ) + ( fTurbineL * 0.22f ) + ( fExhaustNoise * 0.15f );
		float fEngineR = ( fSubRumble * 0.55f ) + ( fTurbineR * 0.22f ) + ( fExhaustNoise * 0.15f );

		// Master engine volume (scales slightly with speed)
		float fEngineVol = 0.003f + 0.2f * fSpeed;

		sAudioBuffer.pData[iFrameInd * 2 + 0] = FX_Bitcrush( fEngineL, 16.0f ) * fEngineVol;
		sAudioBuffer.pData[iFrameInd * 2 + 1] = FX_Bitcrush( fEngineR, 16.0f ) * fEngineVol;

		// ====================================================================
		// RETRO ARCADE AUDIO EVENTS (GUNSHOT, CLICKS)
		// ====================================================================

		for ( int iEvent = 0; iEvent < m_aAudioEvents.size(); iEvent++ )
		{
			SAudioEvent& sAudioEvent = m_aAudioEvents[iEvent];

			const double dLifeTimeSamples = (double)sAudioEvent.iLifeTimeNs * (double)sAudioBuffer.iSampleRate / 1000.0 / 1000.0 / 1000.0;
			float fTimeW = (float)((double)sAudioEvent.iSampleCounter / dLifeTimeSamples);
			if ( fTimeW > 1.0f ) continue;

			float fSampleOut = 0.0f;

			if ( sAudioEvent.type == SAudioEvent::GunShot )
			{
				// --- ARCADE SPACESHIP RAILGUN / LASER ---
				float fFreq = 1600.0f * expf( -fTimeW * 12.0f ) + 150.0f;

				sAudioEvent.fPhase += fFreq / (float)sAudioBuffer.iSampleRate;
				if ( sAudioEvent.fPhase >= 1.0f ) sAudioEvent.fPhase -= 1.0f;

				float fEnv = expf( -fTimeW * 14.0f );
				float fPulsePWM = 0.125f + 0.05f * sinf( fTimeW * 20.0f );
				float fLaserTone = Osc_Pulse( sAudioEvent.fPhase, fPulsePWM );

				uint32_t uNoiseSeed = (uint32_t)( sAudioEvent.iSampleCounter * 1664525u + iEvent * 1013904223u );
				float fNoiseBurst = Osc_8BitNoise( uNoiseSeed ) * expf( -fTimeW * 45.0f );

				float fRawSample = fLaserTone * 0.75f + fNoiseBurst * 0.45f;
				fSampleOut = FX_Bitcrush( fRawSample, 12.0f ) * fEnv;
			}
			else if ( sAudioEvent.type == SAudioEvent::ClickDown || sAudioEvent.type == SAudioEvent::ClickUp )
			{
				// --- RETRO ARCADE UI CLICKS ---
				float fFreq = ( sAudioEvent.type == SAudioEvent::ClickDown )
					? Lerp( 1200.0f, 300.0f, fTimeW )
					: Lerp( 400.0f, 1100.0f, fTimeW );

				sAudioEvent.fPhase += fFreq / (float)sAudioBuffer.iSampleRate;
				if ( sAudioEvent.fPhase >= 1.0f ) sAudioEvent.fPhase -= 1.0f;

				float fEnv = expf( -fTimeW * 22.0f );
				float fClickOsc = Osc_Pulse( sAudioEvent.fPhase, 0.25f );
				fSampleOut = FX_Bitcrush( fClickOsc, 16.0f ) * fEnv;
			}

			// Add sound to left and right channels
			for ( int iChInd = 0; iChInd < 2; iChInd++ )
			{
				sAudioBuffer.pData[iFrameInd * 2 + iChInd] += ( sAudioEvent.fVolume * 0.35f ) * fSampleOut;
			}

			sAudioEvent.iSampleCounter++;
		}
	}

	Music( sAudioBuffer, sAudioFrameData.m_fMusic_Action, sAudioFrameData.m_fMusic_Climax );
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
	LOG( "MUSIC  %.4f sec ( action=%.2f, climax=%.2f )\n", (double)(m_iSampleCounter) / 1000.0 / 1000.0 / 1000.0, fAction, fClimax );
	const float fBPM = 80.0f; // Tempo
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
			//float fSecondVolume = iCurrent16th % 2 == 0 ? fAction+0.5f : 1.0f;
			if ( iLeadNoteIdx >= 0 )
			{
				float fEnv = expf( -fStepTime * 18.0f );
				int iMidiNote = activeChord.iRootMidi + 12 + activeChord.aIntervals[iLeadNoteIdx % 4];
				float fFreq = MidiToFreq( iMidiNote );

				// Dual pulse with slight octave/detune layering for a rich lead sound
				float fLeadOsc1 = Osc_Pulse( fFreq * t, fGlobalPwm );
				float fLeadOsc2 = Osc_Pulse( ( fFreq * 1.003f ) * t, 0.125f );

				fLead = ( fLeadOsc1 + fLeadOsc2 * 0.5f ) * fEnv * 0.15f;// * fSecondVolume;
			}
		}

		// --- 6. MIXING & MASTERING (LO-FI BITCRUSHER) ---
		float fMixL = fDrums + fBass + fChordArp + fLead;
		float fMixR = fDrums + fBass + fChordArp * 0.7f + fLead;

		// Master drive and 8-bit quantization / bitcrushing stage
		float fMasterDrive = 1.0f + fAction * 0.2f;
		fMixL = FX_Bitcrush( WaveShaper_Tan( fMixL * fMasterDrive, 1.1f ), 24.0f ) * 0.10f;
		fMixR = FX_Bitcrush( WaveShaper_Tan( fMixR * fMasterDrive, 1.1f ), 24.0f ) * 0.10f;

		sAudioBuffer.pData[iFrameInd * 2 + 0] += fMixL;
		sAudioBuffer.pData[iFrameInd * 2 + 1] += fMixR;
	}
}