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

void CGraphics::DrawLineH( int x, int y, int len, BGRA8 sColor )
{
	if ( y < 0 || y >= m_sFrameBuffer.iHeight )
	{
		return;
	}

	int iXStart = x;
	int iXEnd = (len > 0) ? (x + len - 1) : (x + len + 1);
	if ( iXStart > iXEnd )
	{
		std::swap( iXStart, iXEnd );
	}

	iXStart = std::max( 0, iXStart );
	iXEnd = std::min( m_sFrameBuffer.iWidth - 1, iXEnd );

	if ( iXStart > iXEnd )
	{
		return;
	}

	for ( int i = iXStart; i <= iXEnd; i++ )
	{
		DrawPixel( i, y, sColor );
	}
}

void CGraphics::DrawLineV( int x, int y, int len, BGRA8 sColor )
{
	if ( x < 0 || x >= m_sFrameBuffer.iWidth )
	{
		return;
	}
	int iYStart = y;
	int iYEnd = (len > 0) ? (y + len - 1) : (y + len + 1);
	if ( iYStart > iYEnd )
	{
		std::swap( iYStart, iYEnd );
	}

	iYStart = std::max( 0, iYStart );
	iYEnd = std::min( m_sFrameBuffer.iHeight - 1, iYEnd );
	
	if ( iYStart > iYEnd )
	{
		return;
	}

	for ( int i = iYStart; i <= iYEnd; i++ )
	{
		DrawPixel( x, i, sColor );
	}
}

void CGraphics::DrawRect( int x, int y, int w, int h, BGRA8 sColor )
{
	DrawLineH( x, y, w, sColor );
	DrawLineH( x, y + h - 1, w, sColor );
	DrawLineV( x, y+1, h-2, sColor );
	DrawLineV( x + w - 1, y+1, h-2, sColor );
}

void CGraphics::DrawTexture( int x, int y, const STextureIndexed& sTex )
{
	for ( int iy = 0; iy < sTex.m_iHeight; iy++ )
	{
		for ( int ix = 0; ix < sTex.m_iWidth; ix++ )
		{
			int iDestX = x + ix;
			int iDestY = y + iy;
			if ( iDestX >= 0 && iDestX < m_sFrameBuffer.iWidth && iDestY >= 0 && iDestY < m_sFrameBuffer.iHeight )
			{
				uint8_t uIndex = sTex.m_pData[iy * sTex.m_iWidth + ix];
				BGRA8 sColor;
				sColor.r = sTex.m_pPalette[uIndex*3+2];
				sColor.g = sTex.m_pPalette[uIndex*3+1];
				sColor.b = sTex.m_pPalette[uIndex*3+0];
				((BGRA8*)m_sFrameBuffer.pData)[iDestY * m_sFrameBuffer.iWidth + iDestX] = sColor;
			}			
		}
	}
}

void CGraphics::DrawText( int x, int y, const char* pText, BGRA8 sColor, const STextureIndexed& sTex, int iCharWidth, int iCharHeight )
{
	//text length:
	if ( pText == nullptr || *pText == '\0' )
	{
		return;
	}

	while ( *pText != '\0' )
	{
		char c = *pText;
		if ( c < 32 || c > 127 )
		{
			c = '?';
		}
		int iCharIndex = c-32;
		int iCharX = (iCharIndex % 16) * iCharWidth;
		int iCharY = (iCharIndex / 16) * iCharHeight;
		for ( int iy = 0; iy < iCharHeight; iy++ )
		{
			for ( int ix = 0; ix < iCharWidth; ix++ )
			{
				int iDestX = x + ix;
				int iDestY = y + iy;
				if ( iDestX >= 0 && iDestX < m_sFrameBuffer.iWidth && iDestY >= 0 && iDestY < m_sFrameBuffer.iHeight )
				{
					uint8_t uIndex = sTex.m_pData[(iCharY+iy) * sTex.m_iWidth + (iCharX+ix)];
					if ( uIndex == 0 )
					{
						continue;
					}
					((BGRA8*)m_sFrameBuffer.pData)[iDestY * m_sFrameBuffer.iWidth + iDestX] = sColor;
				}			
			}
		}
		x += iCharWidth;
		pText++;
	}
}

