#pragma once

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
// VOICES
// ============================================================================

struct SChord
{
	const char* pName;
	int iRootMidi;      // Base MIDI root note (e.g., 57 = A3)
	int aIntervals[4];  // Semitone offsets from root: {0, 3, 7, 12}
};

// --- DRUMS & PERCUSSION ---

// Arcade Kick: Pitch-dropping pulse/sine drop
float Voice_Kick( float fStepTime )
{
	float fEnv = expf( -fStepTime * 35.0f );
	float fFreq = 120.0f * expf( -fStepTime * 40.0f ) + 30.0f;
	return Osc_Pulse( fFreq * fStepTime, 0.5f ) * fEnv * 0.45f;
}

// Arcade Snare: Noise + low-pitched pop
float Voice_Snare( float fStepTime, uint32_t uNoiseSeed )
{
	float fEnv = expf( -fStepTime * 25.0f );
	float fNoise = Osc_8BitNoise( uNoiseSeed );
	float fTone = Osc_Triangle( 180.0f * fStepTime );
	return ( fNoise * 0.7f + fTone * 0.3f ) * fEnv * 0.35f;
}

// Arcade Hi-Hat: Short metallic noise burst
float Voice_HiHat( float fStepTime, uint32_t uNoiseSeed, float fClimax )
{
	float fEnv = expf( -fStepTime * 80.0f );
	float fNoise = Osc_8BitNoise( uNoiseSeed );
	return fNoise * fEnv * ( 0.12f + fClimax * 0.08f );
}

// Arcade Tom / Blip Percussion: Pitch-drop woodblock / tom sound
float Voice_PercTom( float fStepTime )
{
	float fEnv = expf( -fStepTime * 45.0f );
	float fFreq = 320.0f * expf( -fStepTime * 20.0f ) + 80.0f;
	return Osc_Pulse( fFreq * fStepTime, 0.25f ) * fEnv * 0.30f;
}

// Noise Sweep / Explosion FX: Longer decay noise effect
float Voice_NoiseSweep( float fStepTime, uint32_t uNoiseSeed )
{
	float fEnv = expf( -fStepTime * 6.0f );
	float fNoise = Osc_8BitNoise( uNoiseSeed );
	return fNoise * fEnv * 0.25f;
}

// --- SYNTHS & LEAD / BASS / CHORDS ---

// Bassline: Triangle wave with exponential decay
float Voice_Bass( float fStepTime, float fFreq, float t )
{
	float fEnv = expf( -fStepTime * 12.0f );
	return Osc_Triangle( fFreq * t ) * fEnv * 0.35f;
}

// Lead Synth: Dual pulse with slight octave/detune layering
float Voice_Lead( float fStepTime, float fFreq, float t, float fPwm )
{
	float fEnv = expf( -fStepTime * 18.0f );
	float fLeadOsc1 = Osc_Pulse( fFreq * t, fPwm );
	float fLeadOsc2 = Osc_Pulse( ( fFreq * 1.003f ) * t, 0.125f );
	return ( fLeadOsc1 + fLeadOsc2 * 0.5f ) * fEnv * 0.15f;
}

// Chiptune Fast-Arp Pad: 1-OSC Rapid Arpeggio (NES Style)
float Voice_ChordArp( float t, const SChord& activeChord, float fSecondsPerBar, float fPwm, float fAction, float fClimax )
{
	float fArpSpeedHz = 32.0f + fAction * 16.0f;
	int iArpSubStep = (int)( t * fArpSpeedHz ) % 4;
	int iMidiNote = activeChord.iRootMidi + activeChord.aIntervals[iArpSubStep];
	float fFreq = MidiToFreq( iMidiNote );

	float fPadEnv = 0.6f + 0.4f * sinf( PI2 * ( t / fSecondsPerBar ) );
	float fPulse = Osc_Pulse( fFreq * t, fPwm );

	return fPulse * 0.08f * fPadEnv * ( 0.4f + 0.6f * fClimax );
}

// Chord Stab: Simultaneous polyphonic chord hit (alternative to Arp)
float Voice_ChordStab( float fStepTime, const SChord& activeChord, float t )
{
	float fEnv = expf( -fStepTime * 14.0f );
	float fMix = 0.0f;

	// Triád összegzése (Root, 3rd, 5th)
	for ( int i = 0; i < 3; ++i )
	{
		float fFreq = MidiToFreq( activeChord.iRootMidi + activeChord.aIntervals[i] );
		fMix += Osc_Pulse( fFreq * t, 0.25f );
	}

	return ( fMix * 0.33f ) * fEnv * 0.20f;
}

// A short, high-pitched laser sound with exponential decay
float Voice_Laser( float fStepTime )
{
	float fEnv = expf( -fStepTime * 16.0f );
	float fFreq = 1400.0f * expf( -fStepTime * 35.0f ) + 120.0f;
	return Osc_Pulse( fFreq * fStepTime, 0.25f ) * fEnv * 0.20f;
}
