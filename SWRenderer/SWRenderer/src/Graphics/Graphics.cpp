#include "Graphics.h"

CGraphics* CGraphics::m_pThis = nullptr;

CGraphics::CGraphics()
{
}

CGraphics::~CGraphics()
{
}

void CGraphics::Create( SFrameBuffer& sFrameBuffer )
{
	m_sFrameBuffer = sFrameBuffer;
}

void CGraphics::Clear()
{
}

void CGraphics::ClearFrameBuffer( BGRA8 sColor )
{
	for ( int y = 0; y < m_sFrameBuffer.iHeight; y++ )
	{
		for ( int x = 0; x < m_sFrameBuffer.iWidth; x++ )
		{
			m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x] = sColor.rgba;
		}
	}	
}

void CGraphics::DrawPixel( int x, int y, BGRA8 sColor )
{
	assert( x >= 0 && x < m_sFrameBuffer.iWidth && y >= 0 && y < m_sFrameBuffer.iHeight );
	//if ( x >= 0 && x < m_sFrameBuffer.iWidth && y >= 0 && y < m_sFrameBuffer.iHeight )
	{
		m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x] = BlendAdditive( m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x], sColor );
	}
}

void CGraphics::DrawPixelAA( const SVector2& v, BGRA8 sColor )
{
	int ix = (int)v.x;
	int iy = (int)v.y;
	if ( ix >= 0 && ix < m_sFrameBuffer.iWidth - 1 &&
		iy >= 0 && iy < m_sFrameBuffer.iHeight - 1 )
	{
		float fxmod = v.x - (float)ix;
		float fymod = v.y - (float)iy;
		float fxmodinv = 1.0f - fxmod;
		float fymodinv = 1.0f - fymod;
		uint8_t i00 = (uint8_t)( sqrtf( fxmodinv*fymodinv ) * sColor.a);
		uint8_t i01 = (uint8_t)( sqrtf( fxmodinv*fymod ) * sColor.a);
		uint8_t i10 = (uint8_t)( sqrtf( fxmod * fymodinv ) * sColor.a);
		uint8_t i11 = (uint8_t)( sqrtf( fxmod * fymod ) * sColor.a);

		m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix] = BlendAdditive( m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix], BGRA8{ sColor.r, sColor.g, sColor.b, i00 } );
		m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix + 1] = BlendAdditive( m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix + 1], BGRA8{ sColor.r, sColor.g, sColor.b, i10 } );
		m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix] = BlendAdditive( m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix], BGRA8{ sColor.r, sColor.g, sColor.b, i01 } );
		m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix + 1] = BlendAdditive( m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix + 1], BGRA8{ sColor.r, sColor.g, sColor.b, i11 } );
	}
}

void CGraphics::DrawLine( const SVector2& v0o, const SVector2& v1o, BGRA8 sColor )
{
	SVector2 v( v1o - v0o );

	if ( v.x == 0.0f && v.y == 0.0f )
	{
		return;
	}

	bool bSwizzle = fabsf(v.x) < fabsf(v.y);

	SVector2 v0( v0o );
	SVector2 v1( v1o );
	if ( bSwizzle )
	{
		std::swap( v0.x, v0.y );
		std::swap( v1.x, v1.y );
	}
	if ( v1.x < v0.x ) std::swap( v0, v1 );
	v = v1 - v0;

	int iXStart = (int)(v0.x+0.5f);
	int iXEnd = (int)(v1.x+0.5f);
	//assert( iXEnd-iXStart < 1000 );
	for ( int iX = iXStart; iX < iXEnd; iX++ )
	{
		float fY = v.y * ( ((float)iX+0.5f) - v0.x ) / v.x + v0.y;

		int x = iX;
		int y = (int)fY;
		if ( bSwizzle )
		{
			std::swap( x, y );
		}

		DrawPixel( x, y, sColor );
	}
}

void CGraphics::DrawLine( const SVertexPhC& v0o, const SVertexPhC& v1o )
{
	SVector2 v( v1o.vPos.x - v0o.vPos.x, v1o.vPos.y - v0o.vPos.y );

	if ( v.x == 0.0f && v.y == 0.0f )
	{
		return;
	}

	bool bSwizzle = fabsf(v.x) < fabsf(v.y);

	SVertexPhC v0( v0o );
	SVertexPhC v1( v1o );
	if ( bSwizzle )
	{
		std::swap( v0.vPos.x, v0.vPos.y );
		std::swap( v1.vPos.x, v1.vPos.y );
	}
	if ( v1.vPos.x < v0.vPos.x )
	{
		std::swap( v0, v1 );
	}
	v.x = v1.vPos.x - v0.vPos.x;
	v.y = v1.vPos.y - v0.vPos.y;

	int iXStart = (int)(v0.vPos.x+0.5f);
	int iXEnd = (int)(v1.vPos.x+0.5f);
	//assert( iXEnd-iXStart < 1000 );
	for ( int iX = iXStart; iX < iXEnd; iX++ )
	{
		float t = (((float)iX + 0.5f) - v0.vPos.x) / v.x;
		float fY = v.y * t + v0.vPos.y;

		int x = iX;
		int y = (int)fY;

		float a = (1.0f - t) / v0.vPos.w;
		float b = t / v1.vPos.w;

		SVector4 vColor = (v0.vColor * a + v1.vColor * b) / (a + b);
		BGRA8 sColor( vColor.x, vColor.y, vColor.z, vColor.w );

		if ( bSwizzle )
		{
			std::swap( x, y );
		}

		DrawPixel( x, y, sColor );
	}
}

uint32_t CGraphics::BlendAdditive( uint32_t dest, BGRA8 src )
{
	BGRA8 sDest;
	sDest.rgba = dest;
	uint32_t rOut = sDest.r + ((src.r*src.a)>>8);
	uint32_t gOut = sDest.g + ((src.g*src.a)>>8);
	uint32_t bOut = sDest.b + ((src.b*src.a)>>8);
	rOut = rOut > 255 ? 255 : rOut;
	gOut = gOut > 255 ? 255 : gOut;
	bOut = bOut > 255 ? 255 : bOut;
	return (rOut) | (gOut << 8) | (bOut << 16);
}

