#pragma once

#include "Common/Globals.h"
#include "Vector.h"

#define PI05		1.5707963267948966192313216916398f
#define PI			3.1415926535897932384626433832795f
#define PI2			6.283185307179586476925286766559f
#define PIRECIP		0.31830988618379067153776752674503f
#define SQRT2		1.4142135623730950488016887242097f
#define E_NUMBER	2.7182818284590452353602874713527f

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

static void DrawPixelAA( SFrameBuffer& sFrameBuffer, const SVector2& v, RGBA8 sColor )
{
	int ix = (int)v.x;
	int iy = (int)v.y;
	if ( ix >= 0 && ix < sFrameBuffer.iWidth - 1 &&
		iy >= 0 && iy < sFrameBuffer.iHeight - 1 )
	{
		float fxmod = v.x - (float)ix;
		float fymod = v.y - (float)iy;
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

static void DrawLine( SFrameBuffer& sFrameBuffer, const SVector2& v0, const SVector2& v1, RGBA8 sColor )
{
	SVector2 v( v1 - v0 );

	if ( v.x > 0.0f && v.y > 0.0f && v.x >= v.y )
	{
		int iXCount = (int)v.x;
		int iY = 0;
		float m = v.y/v.x;
		int step = 0;
		for ( int iX = 0; iX <= iXCount; iX++ )
		{
			int step0 = (int)((float)(iX+1)*m);
			if ( step != step0 )
			{
				iY++;
			}
			step = step0;
			DrawPixel( sFrameBuffer, iX+(int)v0.x, iY+(int)v0.y, sColor );
		}
	}
}