#include "Audio.h"
#include "Common/Globals.h"
#include "Common/Time.h"

// ============================================================================
// ENVELOPE HELPERS (ZERO-BOUNDED EXPONENTIAL DECAY)
// ============================================================================

// Guarantees clean 1.0 -> 0.0 decay at fProgress == 1.0f, eliminating pops/clicks
static inline float Env_ExpDecay( float fProgress, float fDecayRate )
{
	if ( fProgress >= 1.0f ) return 0.0f;
	if ( fProgress <= 0.0f ) return 1.0f;

	float fVal = expf( -fProgress * fDecayRate );
	float fEnd = expf( -fDecayRate );

	return ( fVal - fEnd ) / ( 1.0f - fEnd );
}

// ============================================================================
// WAVESHAPERS & DSP HELPERS
// ============================================================================

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
	return ( x - x * x * x * 0.333333f ) * 1.5f;
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
static float Osc_Pulse( float fPhase, float fPwmDuty = 0.5f )
{
	float fP = fmodf( fPhase, 1.0f );
	if ( fP < 0.0f ) fP += 1.0f;
	return ( fP < fPwmDuty ) ? 1.0f : -1.0f;
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
// VOICES (MODULAR INSTRUMENT & SFX GENERATORS)
// ============================================================================

struct SChord
{
	const char* pName;
	int iRootMidi;      // Base MIDI root note (e.g., 57 = A3)
	int aIntervals[4];  // Semitone offsets from root: {0, 3, 7, 12}
};

// --- DRUMS & PERCUSSION ---

// Arcade Kick: Pitch-dropping pulse/sine drop with zero-ending envelope
float Voice_Kick( float fStepTimeSec, float fSecPer16th, float fDecayRate = 1.0f )
{
	float fStepProgress = fStepTimeSec / fSecPer16th;
	float fEnv = Env_ExpDecay( fStepProgress, 3.5f * fDecayRate );
	float fFreqHz = 120.0f * expf( -fStepProgress * 4.0f ) + 30.0f;
	return Osc_Pulse( fFreqHz * fStepTimeSec, 0.5f ) * fEnv * 0.45f;
}

// Arcade Snare: Noise + low-pitched pop with zero-ending envelope
float Voice_Snare( float fStepTimeSec, float fSecPer16th, uint32_t uNoiseSeed, float fDecayRate = 1.0f )
{
	float fStepProgress = fStepTimeSec / fSecPer16th;
	float fEnv = Env_ExpDecay( fStepProgress, 2.5f * fDecayRate );
	float fNoise = Osc_8BitNoise( uNoiseSeed );
	float fTone = Osc_Triangle( 180.0f * fStepTimeSec );
	return ( fNoise * 0.7f + fTone * 0.3f ) * fEnv * 0.35f;
}

// Arcade Hi-Hat: Short metallic noise burst with zero-ending envelope
float Voice_HiHat( float fStepTimeSec, float fSecPer16th, uint32_t uNoiseSeed, float fClimax, float fDecayRate = 1.0f )
{
	float fStepProgress = fStepTimeSec / fSecPer16th;
	float fEnv = Env_ExpDecay( fStepProgress, 8.0f * fDecayRate );
	float fNoise = Osc_8BitNoise( uNoiseSeed );
	return fNoise * fEnv * ( 0.12f + fClimax * 0.08f );
}

// Arcade Tom / Blip Percussion: Pitch-drop woodblock sound
float Voice_PercTom( float fStepTimeSec, float fSecPer16th, float fDecayRate = 1.0f )
{
	float fStepProgress = fStepTimeSec / fSecPer16th;
	float fEnv = Env_ExpDecay( fStepProgress, 4.5f * fDecayRate );
	float fFreqHz = 320.0f * expf( -fStepProgress * 2.0f ) + 80.0f;
	return Osc_Pulse( fFreqHz * fStepTimeSec, 0.25f ) * fEnv * 0.30f;
}

// Noise Sweep / Explosion FX: Longer decay noise effect
float Voice_NoiseSweep( float fStepTimeSec, float fSecPer16th, uint32_t uNoiseSeed, float fDecayRate = 1.0f )
{
	float fStepProgress = fStepTimeSec / fSecPer16th;
	float fEnv = Env_ExpDecay( fStepProgress, 0.6f * fDecayRate );
	float fNoise = Osc_8BitNoise( uNoiseSeed );
	return fNoise * fEnv * 0.25f;
}

// --- SYNTHS & LEAD / BASS / CHORDS ---

// Bassline: Triangle wave with gate length mapped to clean zero-bound decay
float Voice_Bass( float fStepTimeSec, float fSecPer16th, float fFreqHz, float fTimeSec, float fGateLength = 0.8f )
{
	float fStepProgress = fStepTimeSec / fSecPer16th;
	float fDecayCoeff = 2.5f / fGateLength;
	float fEnv = Env_ExpDecay( fStepProgress, fDecayCoeff );
	return Osc_Triangle( fFreqHz * fTimeSec ) * fEnv * 0.35f;
}

// Lead Synth: Dual pulse with customizable gate duration
float Voice_Lead( float fStepTimeSec, float fSecPer16th, float fFreqHz, float fTimeSec, float fPwmDuty, float fGateLength = 0.7f )
{
	float fStepProgress = fStepTimeSec / fSecPer16th;
	float fDecayCoeff = 2.0f / fGateLength;
	float fEnv = Env_ExpDecay( fStepProgress, fDecayCoeff );

	float fLeadOsc1 = Osc_Pulse( fFreqHz * fTimeSec, fPwmDuty );
	float fLeadOsc2 = Osc_Pulse( ( fFreqHz * 1.003f ) * fTimeSec, 0.125f );
	return ( fLeadOsc1 + fLeadOsc2 * 0.5f ) * fEnv * 0.15f;
}

// Chiptune Fast-Arp Pad: 1-OSC Rapid Arpeggio (NES Style)
float Voice_ChordArp( float fTimeSec, const SChord& activeChord, float fSecPerBar, float fPwmDuty, float fAction, float fClimax )
{
	float fArpSpeedHz = 32.0f + fAction * 16.0f;
	int iArpSubStep = (int)( fTimeSec * fArpSpeedHz ) % 4;
	int iMidiNote = activeChord.iRootMidi + activeChord.aIntervals[iArpSubStep];
	float fFreqHz = MidiToFreq( iMidiNote );

	float fPadEnv = 0.6f + 0.4f * sinf( PI2 * ( fTimeSec / fSecPerBar ) );
	float fPulse = Osc_Pulse( fFreqHz * fTimeSec, fPwmDuty );

	return fPulse * 0.08f * fPadEnv * ( 0.4f + 0.6f * fClimax );
}

// Chord Stab: Polyphonic chord hit with clean zero-ending decay
float Voice_ChordStab( float fStepTimeSec, float fSecPer16th, const SChord& activeChord, float fTimeSec, float fGateLength = 0.6f )
{
	float fStepProgress = fStepTimeSec / fSecPer16th;
	float fEnv = Env_ExpDecay( fStepProgress, 2.0f / fGateLength );
	float fMix = 0.0f;

	for ( int i = 0; i < 3; ++i )
	{
		float fFreqHz = MidiToFreq( activeChord.iRootMidi + activeChord.aIntervals[i] );
		fMix += Osc_Pulse( fFreqHz * fTimeSec, 0.25f );
	}

	return ( fMix * 0.33f ) * fEnv * 0.20f;
}

// Laser SFX: Exponential pitch drop with zero-ending envelope
float Voice_Laser( float fStepTimeSec, float fSecPer16th, float fDecayRate = 1.0f )
{
	float fStepProgress = fStepTimeSec / fSecPer16th;
	float fEnv = Env_ExpDecay( fStepProgress, 2.0f * fDecayRate );
	float fFreqHz = 1400.0f * expf( -fStepProgress * 4.0f ) + 120.0f;
	return Osc_Pulse( fFreqHz * fStepTimeSec, 0.25f ) * fEnv * 0.20f;
}

// ============================================================================
// CHORD SYSTEM & PROGRESSION ENGINE (PO-20 Arcade Core)
// ============================================================================

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

static const int g_ChordProgression[16] = {
	0, 1, 2, 3, // Am -> F  -> C -> G
	0, 1, 2, 3, // Am -> F  -> C -> G
	4, 5, 0, 6, // Dm -> Em -> Am -> E7
	1, 3, 0, 0  // F  -> G  -> Am -> Am
};

// ============================================================================
// 16-STEP SEQUENCER PATTERNS
// ============================================================================

static const int g_PatternDrums[16] = {
	1, 3, 3, 3,  2, 3, 1, 3,  1, 3, 2, 3,  2, 3, 1, 3
};

static const int g_PatternBass[16] = {
	0, -1, 0, 0,  2, -1, 0, -1,  3, -1, 0, 0,  2, 0, 1, 2
};

static const int g_PatternLead[16] = {
	0, 1, 2, 3,  2, 1, 0, 2,  3, 2, 1, 0,  1, 2, 3, 1
};

// ============================================================================
// MAIN MUSIC SYNTHESIS ENGINE
// ============================================================================

void CAudio::Music( SAudioBuffer& sAudioBuffer, float fAction, float fClimax )
{
	//LOG( "MUSIC  %.4f sec ( action=%.2f, climax=%.2f )\n", (double)( m_iSampleCounter ) / 1000.0 / 1000.0 / 1000.0, fAction, fClimax );
	const float fBPM = 90.0f;
	const float fSecPerBeat = 60.0f / fBPM;
	const float fSecPer16th = fSecPerBeat / 4.0f;
	const float fSecPerBar  = fSecPerBeat * 4.0f;
	const float fTotalLoopSec = fSecPerBar * 16.0f;

	const double dSampleRate = (double)sAudioBuffer.iSampleRate;
	const uint64_t uSamplesPer16th   = (uint64_t)( fSecPer16th * dSampleRate );
	const uint64_t uSamplesPerBar    = (uint64_t)( fSecPerBar * dSampleRate );
	const uint64_t uTotalLoopSamples = (uint64_t)( fTotalLoopSec * dSampleRate );

	for ( uint32_t iFrameInd = 0; iFrameInd < sAudioBuffer.iNumFrames; iFrameInd++ )
	{
		uint64_t currentSample = m_iSampleCounter - sAudioBuffer.iNumFrames + iFrameInd;

		// Sample-exact position inside the current 16-bar loop
		uint64_t uLoopSample = currentSample % uTotalLoopSamples;

		// Loop time in seconds (0.0s to fTotalLoopSec)
		float fTimeSec = (float)uLoopSample / (float)sAudioBuffer.iSampleRate;

		int iCurrentBar  = (int)( uLoopSample / uSamplesPerBar ) % 16;
		int iCurrent16th = (int)( uLoopSample / uSamplesPer16th ) % 16;

		// Sample-exact position inside the current 16th note step
		uint64_t uStepSample = uLoopSample % uSamplesPer16th;

		// Elapsed time in seconds since the start of the current 16th step
		float fStepTimeSec = (float)uStepSample / (float)sAudioBuffer.iSampleRate;

		const SChord& activeChord = g_ArcadeChords[g_ChordProgression[iCurrentBar]];
		float fGlobalPwmDuty = 0.25f + 0.2f * sinf( PI2 * 0.2f * fTimeSec ) + 0.15f * fAction;
		uint32_t uNoiseSeed = (uint32_t)( currentSample + iFrameInd * 17 );

		// --- 2. DRUMS ---
		float fDrums = 0.0f;
		int iDrumStep = g_PatternDrums[iCurrent16th];

		if ( iDrumStep == 1 )      fDrums += Voice_Kick( fStepTimeSec, fSecPer16th );
		else if ( iDrumStep == 2 ) fDrums += Voice_Snare( fStepTimeSec, fSecPer16th, uNoiseSeed );
		else if ( iDrumStep == 3 ) fDrums += Voice_HiHat( fStepTimeSec, fSecPer16th, uNoiseSeed, fClimax );

		// --- 3. BASS ---
		float fBass = 0.0f;
		int iBassNoteIdx = g_PatternBass[iCurrent16th];
		if ( iBassNoteIdx >= 0 )
		{
			int iMidiNote = activeChord.iRootMidi - 24 + activeChord.aIntervals[iBassNoteIdx % 4];
			float fBassGate = ( iCurrent16th % 4 == 0 ) ? 0.95f : 0.60f; // Accent on beat start
			fBass = Voice_Bass( fStepTimeSec, fSecPer16th, MidiToFreq( iMidiNote ), fTimeSec, fBassGate );
		}

		// --- 4. CHORD PAD / ARPEGGIO ---
		float fChordPad = Voice_ChordArp( fTimeSec, activeChord, fSecPerBar, fGlobalPwmDuty, fAction, fClimax );

		// --- 5. LEAD SYNTH ---
		float fLead = 0.0f;
		int iLeadNoteIdx = g_PatternLead[iCurrent16th];
		if ( iLeadNoteIdx >= 0 )
		{
			int iMidiNote = activeChord.iRootMidi + 12 + activeChord.aIntervals[iLeadNoteIdx % 4];
			fLead = Voice_Lead( fStepTimeSec, fSecPer16th, MidiToFreq( iMidiNote ), fTimeSec, fGlobalPwmDuty, 0.7f );
		}

		// --- 6. MIXING & MASTERING ---
		float fMixL = fDrums + fBass + fChordPad + fLead;
		float fMixR = fDrums + fBass + fChordPad * 0.7f + fLead;

		float fMasterDrive = 1.0f + fAction * 0.2f;

		fMixL *= 0.2f;
		fMixR *= 0.2f;

		sAudioBuffer.pData[iFrameInd * 2 + 0] += fMixL;
		sAudioBuffer.pData[iFrameInd * 2 + 1] += fMixR;
	}
}

// ============================================================================
// AUDIO SYSTEM CLASS IMPLEMENTATION
// ============================================================================

CAudio* CAudio::m_pThis = nullptr;

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
		memset( m_pEchoBuffer, 0, sizeof( float ) * sAudioBuffer.iNumFrames * 2 );
		m_iEchoBufferSize = sAudioBuffer.iNumFrames * 2;
	}

	//////////////////////////////////////////////////////////////////////////

	const uint64_t iDelayNs = (uint64_t)( 60.0f * 1000.0f * 1000.0f );

	m_iFrameInd++;
	if ( m_iStartTimeStampNs == 0 )
	{
		m_iStartTimeStampNs = GetGlobalTimeStampNs();
	}

	m_iSampleCounter += sAudioBuffer.iNumFrames;

	uint64_t iTimeStampNs = m_iStartTimeStampNs + (uint64_t)( (double)m_iSampleCounter * 1e9 / (double)sAudioBuffer.iSampleRate );

	const uint64_t iDelayedTimeStampNs = iTimeStampNs - iDelayNs;

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
		for ( int i = 0; i < m_aAudioFrameData.size() - 1; i++ )
		{
			SAudioFrameData& sAudioFrameData0 = m_aAudioFrameData[i];
			SAudioFrameData& sAudioFrameData1 = m_aAudioFrameData[i + 1];
			if ( sAudioFrameData0.m_iTimeStampNs <= iDelayedTimeStampNs && iDelayedTimeStampNs < sAudioFrameData1.m_iTimeStampNs )
			{
				float fW = (float)( iDelayedTimeStampNs - sAudioFrameData0.m_iTimeStampNs ) / (float)( sAudioFrameData1.m_iTimeStampNs - sAudioFrameData0.m_iTimeStampNs );
				sAudioFrameData.Lerp( sAudioFrameData0, sAudioFrameData1, fW );
				break;
			}
		}
	}

	{
		uint64_t iRealTimeBufferStartNs = iDelayedTimeStampNs;
		uint64_t iRealTimeBufferEndNs = iDelayedTimeStampNs + (uint64_t)( (double)sAudioBuffer.iNumFrames * 1000.0 * 1000.0 * 1000.0 / (double)sAudioBuffer.iSampleRate );
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

	const float fSpeed = ( std::max( fabsf( sAudioFrameData.m_fShipAcc ), fabsf( sAudioFrameData.m_fShipYawSpeed ) )*0.3f + sAudioFrameData.m_fShipSpeed ) * 10.0f;
	const float fVolume = sqrtf( fabsf( sAudioFrameData.m_fShipAcc ) + fabsf( sAudioFrameData.m_fShipYawSpeed ) );

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
		float fEngineVol = 0.01f * fVolume;

		sAudioBuffer.pData[iFrameInd * 2 + 0] = FX_Bitcrush( fEngineL, 16.0f ) * fEngineVol;
		sAudioBuffer.pData[iFrameInd * 2 + 1] = FX_Bitcrush( fEngineR, 16.0f ) * fEngineVol;
		// ====================================================================
		// RETRO ARCADE AUDIO EVENTS (GUNSHOT, CLICKS)
		// ====================================================================

		for ( int iEvent = 0; iEvent < m_aAudioEvents.size(); iEvent++ )
		{
			SAudioEvent& sAudioEvent = m_aAudioEvents[iEvent];

			const double dLifeTimeSamples = (double)sAudioEvent.iLifeTimeNs * (double)sAudioBuffer.iSampleRate / 1000.0 / 1000.0 / 1000.0;
			float fTimeW = (float)( (double)sAudioEvent.iSampleCounter / dLifeTimeSamples );
			if ( fTimeW > 1.0f ) continue;

			float fSampleOut = 0.0f;

			if ( sAudioEvent.type == SAudioEvent::GunShot )
			{
				float fFreqHz = 500.0f * expf( -fTimeW * 12.0f ) + 50.0f;

				sAudioEvent.fPhase += fFreqHz / (float)sAudioBuffer.iSampleRate;
				if ( sAudioEvent.fPhase >= 1.0f ) sAudioEvent.fPhase -= 1.0f;

				float fEnv = Env_ExpDecay( fTimeW, 14.0f );
				float fPulsePWM = 0.125f + 0.05f * sinf( fTimeW * 40.0f );
				float fLaserTone = Osc_Pulse( sAudioEvent.fPhase, fPulsePWM );

				uint32_t uNoiseSeed = (uint32_t)( sAudioEvent.iSampleCounter * 1664525u + iEvent * 1013904223u );
				float fNoiseBurst = Osc_8BitNoise( uNoiseSeed ) * Env_ExpDecay( fTimeW, 45.0f );

				float fRawSample = fLaserTone * 0.75f + fNoiseBurst * 0.45f;
				fSampleOut = FX_Bitcrush( fRawSample, 12.0f ) * fEnv;
			}
			if ( sAudioEvent.type == SAudioEvent::GunHit )
			{
				float fFreqHz = 100.0f * expf( -fTimeW * 12.0f ) + 50.0f;

				sAudioEvent.fPhase += fFreqHz / (float)sAudioBuffer.iSampleRate;
				if ( sAudioEvent.fPhase >= 1.0f ) sAudioEvent.fPhase -= 1.0f;

				float fEnv = Env_ExpDecay( fTimeW, 4.0f );
				float fPulsePWM = 0.125f + 0.05f * sinf( fTimeW * 140.0f );
				float fLaserTone = Osc_Pulse( sAudioEvent.fPhase, fPulsePWM );

				uint32_t uNoiseSeed = (uint32_t)( sAudioEvent.iSampleCounter * 1664525u + iEvent * 1013904223u );
				float fNoiseBurst = Osc_8BitNoise( uNoiseSeed ) * Env_ExpDecay( fTimeW, 45.0f );

				float fRawSample = fLaserTone * 0.75f + fNoiseBurst * 0.45f;
				fSampleOut = FX_Bitcrush( fRawSample, 12.0f ) * fEnv;
			}
			else if ( sAudioEvent.type == SAudioEvent::ClickDown || sAudioEvent.type == SAudioEvent::ClickUp )
			{
				float fFreqHz = ( sAudioEvent.type == SAudioEvent::ClickDown )
					? Lerp( 1200.0f, 300.0f, fTimeW )
					: Lerp( 400.0f, 1100.0f, fTimeW );

				sAudioEvent.fPhase += fFreqHz / (float)sAudioBuffer.iSampleRate;
				if ( sAudioEvent.fPhase >= 1.0f ) sAudioEvent.fPhase -= 1.0f;

				float fEnv = Env_ExpDecay( fTimeW, 22.0f );
				float fClickOsc = Osc_Pulse( sAudioEvent.fPhase, 0.25f );
				fSampleOut = FX_Bitcrush( fClickOsc, 16.0f ) * fEnv;
			}
			else if ( sAudioEvent.type == SAudioEvent::MenuSelect )
			{
				// Use iLifeTimeNs to modulate the sound characteristics
				const float fLifeTimeMs = (float)sAudioEvent.iLifeTimeNs / 1000000.0f;

				// Frequency sweep influenced by lifetime - longer lifetime = slower sweep
				float fSweepRate = 8.0f * (150.0f / fLifeTimeMs);
				float fFreqHz = 800.0f * expf( -fTimeW * fSweepRate ) + 200.0f;

				sAudioEvent.fPhase += fFreqHz / (float)sAudioBuffer.iSampleRate;
				if ( sAudioEvent.fPhase >= 1.0f ) sAudioEvent.fPhase -= 1.0f;

				// Envelope decay rate also influenced by lifetime
				float fDecayRate = 18.0f * (150.0f / fLifeTimeMs);
				float fEnv = Env_ExpDecay( fTimeW, fDecayRate );

				float fMenuTone = Osc_Pulse( sAudioEvent.fPhase, 0.25f );
				fSampleOut = FX_Bitcrush( fMenuTone, 14.0f ) * fEnv;
			}

			for ( int iChInd = 0; iChInd < 2; iChInd++ )
			{
				sAudioBuffer.pData[iFrameInd * 2 + iChInd] += ( sAudioEvent.fVolume * 0.35f ) * fSampleOut;
			}

			sAudioEvent.iSampleCounter++;
		}
	}

	//Music( sAudioBuffer, sAudioFrameData.m_fMusic_Action, sAudioFrameData.m_fMusic_Climax );
}