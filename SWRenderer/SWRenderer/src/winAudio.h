#pragma once

bool Audio_Init();
void Audio_UpdateBegin( float*& pDataOut, uint32_t& iNumFramesOut, uint32_t& iSampleRateOut );
void Audio_UpdateEnd( uint32_t iNumFrames );
void Audio_Shutdown();

