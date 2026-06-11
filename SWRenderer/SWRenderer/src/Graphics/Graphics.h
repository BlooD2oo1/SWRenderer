#pragma once

#include "Common/Globals.h"
#include "Vector.h"

#define PI05		1.5707963267948966192313216916398f
#define PI			3.1415926535897932384626433832795f
#define PI2			6.283185307179586476925286766559f
#define PIRECIP		0.31830988618379067153776752674503f
#define SQRT2		1.4142135623730950488016887242097f
#define E_NUMBER	2.7182818284590452353602874713527f

struct BGRA8
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
	uint32_t*	pData = nullptr;
	int			iWidth = 0;
	int			iHeight = 0;
};

static uint32_t BlendAdditive( uint32_t dest, BGRA8 src )
{
	BGRA8 sDest;
	sDest.rgba = dest;
	uint16_t rOut = sDest.r + ((src.r*src.a)>>8);
	uint16_t gOut = sDest.g + ((src.g*src.a)>>8);
	uint16_t bOut = sDest.b + ((src.b*src.a)>>8);
	rOut = rOut > 255 ? 255 : rOut;
	gOut = gOut > 255 ? 255 : gOut;
	bOut = bOut > 255 ? 255 : bOut;
	return (rOut) | (gOut << 8) | (bOut << 16);
}

static void ClearFrameBuffer( SFrameBuffer& sFrameBuffer, BGRA8 sColor )
{
	for ( int y = 0; y < sFrameBuffer.iHeight; y++ )
	{
		for ( int x = 0; x < sFrameBuffer.iWidth; x++ )
		{
			sFrameBuffer.pData[y * sFrameBuffer.iWidth + x] = sColor.rgba;
		}
	}	
}

static void DrawPixel( SFrameBuffer& sFrameBuffer, int x, int y, BGRA8 sColor )
{
	if ( x >= 0 && x < sFrameBuffer.iWidth &&
		y >= 0 && y < sFrameBuffer.iHeight )
	{
		sFrameBuffer.pData[y * sFrameBuffer.iWidth + x] = BlendAdditive( sFrameBuffer.pData[y * sFrameBuffer.iWidth + x], sColor );
	}
}

static void DrawPixelAA( SFrameBuffer& sFrameBuffer, const SVector2& v, BGRA8 sColor )
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

		sFrameBuffer.pData[iy * sFrameBuffer.iWidth + ix] = BlendAdditive( sFrameBuffer.pData[iy * sFrameBuffer.iWidth + ix], BGRA8{ i00, i00, i00, 0 } );
		sFrameBuffer.pData[iy * sFrameBuffer.iWidth + ix + 1] = BlendAdditive( sFrameBuffer.pData[iy * sFrameBuffer.iWidth + ix + 1], BGRA8{ i10, i10, i10, 0 } );
		sFrameBuffer.pData[(iy + 1) * sFrameBuffer.iWidth + ix] = BlendAdditive( sFrameBuffer.pData[(iy + 1) * sFrameBuffer.iWidth + ix], BGRA8{ i01, i01, i01, 0 } );
		sFrameBuffer.pData[(iy + 1) * sFrameBuffer.iWidth + ix + 1] = BlendAdditive( sFrameBuffer.pData[(iy + 1) * sFrameBuffer.iWidth + ix + 1], BGRA8{ i11, i11, i11, 0 } );
	}
}

static void DrawLine( SFrameBuffer& sFrameBuffer, const SVector2& v0o, const SVector2& v1o, BGRA8 sColor )
{
	bool bSwizzle = abs(v1o.x - v0o.x) < abs(v1o.y - v0o.y);

	SVector2 v0( v0o );
	SVector2 v1( v1o );
	if ( bSwizzle )
	{
		std::swap( v0.x, v0.y );
		std::swap( v1.x, v1.y );
	}
	if ( v1.x < v0.x ) std::swap( v0, v1 );
	SVector2 v;
	SVector2::Sub( v, v1, v0 );

	//if ( bSwizzle ) sColor.r=sColor.r>>2;

	int iXStart = (int)(v0.x+0.5f);
	int iXEnd = (int)(v1.x-0.5f);
	assert( iXEnd-iXStart < 1000 );
	for ( int iX = iXStart; iX <= iXEnd; iX++ )
	{
		float fY = v.y * ( ((float)iX+0.5f) - v0.x ) / v.x + v0.y;

		int x = iX;
		int y = (int)fY;
		if ( bSwizzle )
		{
			std::swap( x, y );
		}
		
		DrawPixel( sFrameBuffer, x, y, sColor );
	}
}

static uint8_t ClipCode( const SVector4& vP4 )
{
	uint8_t iRet = 0;
	iRet |= ( vP4.x <= -vP4.w ) ? 1 : 0;
	iRet |= ( vP4.x >= vP4.w ) ? 2 : 0;
	iRet |= ( vP4.y <= -vP4.w ) ? 4 : 0;
	iRet |= ( vP4.y >= vP4.w ) ? 8 : 0;
	iRet |= ( vP4.z <= 0.0f ) ? 16 : 0;
	iRet |= ( vP4.z >= vP4.w ) ? 32 : 0;
	return iRet;
}

static bool ProjectPoint( SVector2& vOut, const SVector3& vP, const SMatrix& matViewProj, const int iWidth, const int iHeight )
{
	SVector4 vPhSrc( vP.x, vP.y, vP.z, 1.0f );
	SVector4 vPh;
	SMatrix::Mul( vPh, vPhSrc, matViewProj );
	
	uint8_t iClipCode = ClipCode( vPh );
	if ( iClipCode != 0 )
	{
		return false;
	}
	
	float fWRec = 1.0f / vPh.w;

	vOut.x = vPh.x * fWRec;
	vOut.y = vPh.y * fWRec;

	vOut.x = vOut.x*0.5f + 0.5f;
	vOut.y = -vOut.y*0.5f + 0.5f;
	vOut.x *= (float)iWidth;
	vOut.y *= (float)iHeight;

	return true;
}

static bool ProjectLine( SVector2& vOut0, SVector2& vOut1, const SVector3& vP0, const SVector3& vP1, const SMatrix& matViewProj, const int iWidth, const int iHeight )
{
	SVector4 vPh0Src( vP0.x, vP0.y, vP0.z, 1.0f );
	SVector4 vPh0;
	SMatrix::Mul( vPh0, vPh0Src, matViewProj );

	SVector4 vPh1Src( vP1.x, vP1.y, vP1.z, 1.0f );
	SVector4 vPh1;
	SMatrix::Mul( vPh1, vPh1Src, matViewProj );

	uint8_t iClipCode0 = ClipCode( vPh0 );
	uint8_t iClipCode1 = ClipCode( vPh1 );

	if ( iClipCode0 != 0 || iClipCode1 != 0 )
	{
		return false;
	}

	/*while ( 1 )
	{
		if ( iClipCode0 == 0 && iClipCode1 == 0 )
		{
			break;
		}
		if ( ( iClipCode0 & iClipCode1 ) != 0 )
		{
			return false;
		}

		uint8_t iSideCode;
		if		( iClipCode0 & 1  != iClipCode1 & 1  )	iSideCode = 1;
		else if	( iClipCode0 & 2  != iClipCode1 & 2  )	iSideCode = 2;
		else if	( iClipCode0 & 4  != iClipCode1 & 4  )	iSideCode = 4;
		else if	( iClipCode0 & 8  != iClipCode1 & 8  )	iSideCode = 8;
		else if	( iClipCode0 & 16 != iClipCode1 & 16 )	iSideCode = 16;
		else											iSideCode = 32;
	}*/

	float fWRec0 = 1.0f / vPh0.w;
	vOut0.x = vPh0.x * fWRec0;
	vOut0.y = vPh0.y * fWRec0;

	float fWRec1 = 1.0f / vPh1.w;
	vOut1.x = vPh1.x * fWRec1;
	vOut1.y = vPh1.y * fWRec1;

	vOut0.x = vOut0.x*0.5f + 0.5f;
	vOut0.y = -vOut0.y*0.5f + 0.5f;
	vOut0.x *= (float)iWidth;
	vOut0.y *= (float)iHeight;

	vOut1.x = vOut1.x*0.5f + 0.5f;
	vOut1.y = -vOut1.y*0.5f + 0.5f;
	vOut1.x *= (float)iWidth;
	vOut1.y *= (float)iHeight;

	return true;
}