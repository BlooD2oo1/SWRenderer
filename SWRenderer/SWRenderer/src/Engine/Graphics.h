#pragma once

#include "Common/Globals.h"

//struct for uint32_t as RGBA color:
struct RGBA8
{
	union
	{
		struct
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};
		uint32_t rgba;
	};
};

struct SFrameBuffer
{
	SFrameBuffer( uint32_t* pData, int iWidth, int iHeight )
		: pData( pData ), iWidth( iWidth ), iHeight( iHeight )
	{
	}
	uint32_t*		pData = nullptr;
	int			iWidth = 0;
	int			iHeight = 0;
};

static uint32_t BlendAdditive( uint32_t dest, RGBA8 src )
{
	RGBA8 sDest;
	sDest.rgba = dest;
	uint16_t rOut = sDest.r + src.r;
	uint16_t gOut = sDest.g + src.g;
	uint16_t bOut = sDest.b + src.b;
	rOut = rOut > 255 ? 255 : rOut;
	gOut = gOut > 255 ? 255 : gOut;
	bOut = bOut > 255 ? 255 : bOut;
	return (bOut) | (gOut << 8) | (rOut << 16);
}

static void ClearFrameBuffer( SFrameBuffer& sFrameBuffer, RGBA8 sColor )
{
	for ( int y = 0; y < sFrameBuffer.iHeight; y++ )
	{
		for ( int x = 0; x < sFrameBuffer.iWidth; x++ )
		{
			sFrameBuffer.pData[y * sFrameBuffer.iWidth + x] = sColor.rgba;
		}
	}	
}

static void DrawPixel( SFrameBuffer& sFrameBuffer, int x, int y, RGBA8 sColor )
{
	if ( x >= 0 && x < sFrameBuffer.iWidth &&
		y >= 0 && y < sFrameBuffer.iHeight )
	{
		sFrameBuffer.pData[y * sFrameBuffer.iWidth + x] = BlendAdditive( sFrameBuffer.pData[y * sFrameBuffer.iWidth + x], sColor );
	}
}

static void DrawPixelAA( SFrameBuffer& sFrameBuffer, float fx, float fy, RGBA8 sColor )
{
	int ix = (int)fx;
	int iy = (int)fy;
	if ( ix >= 0 && ix < sFrameBuffer.iWidth - 1 &&
		iy >= 0 && iy < sFrameBuffer.iHeight - 1 )
	{
		float fxmod = fx - (float)ix;
		float fymod = fy - (float)iy;
		float fxmodinv = 1.0f - fxmod;
		float fymodinv = 1.0f - fymod;
		uint8_t i00 = (uint8_t)( sqrtf( fxmodinv*fymodinv ) * 255.0f );
		uint8_t i01 = (uint8_t)( sqrtf( fxmodinv*fymod ) * 255.0f );
		uint8_t i10 = (uint8_t)( sqrtf( fxmod * fymodinv ) * 255.0f);
		uint8_t i11 = (uint8_t)( sqrtf( fxmod * fymod ) * 255.0f);

		sFrameBuffer.pData[iy * sFrameBuffer.iWidth + ix] = BlendAdditive( sFrameBuffer.pData[iy * sFrameBuffer.iWidth + ix], RGBA8{ i00, i00, i00, 0 } );
		sFrameBuffer.pData[iy * sFrameBuffer.iWidth + ix + 1] = BlendAdditive( sFrameBuffer.pData[iy * sFrameBuffer.iWidth + ix + 1], RGBA8{ i10, i10, i10, 0 } );
		sFrameBuffer.pData[(iy + 1) * sFrameBuffer.iWidth + ix] = BlendAdditive( sFrameBuffer.pData[(iy + 1) * sFrameBuffer.iWidth + ix], RGBA8{ i01, i01, i01, 0 } );
		sFrameBuffer.pData[(iy + 1) * sFrameBuffer.iWidth + ix + 1] = BlendAdditive( sFrameBuffer.pData[(iy + 1) * sFrameBuffer.iWidth + ix + 1], RGBA8{ i11, i11, i11, 0 } );
	}
}