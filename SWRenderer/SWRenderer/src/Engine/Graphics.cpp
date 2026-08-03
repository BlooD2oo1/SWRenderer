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

void CGraphics::BlendAdditive( BGRA8& dest, BGRA8 src )
{
	uint32_t rOut = dest.r + ((src.r*src.a)>>8);
	uint32_t gOut = dest.g + ((src.g*src.a)>>8);
	uint32_t bOut = dest.b + ((src.b*src.a)>>8);
	rOut = rOut > 255 ? 255 : rOut;
	gOut = gOut > 255 ? 255 : gOut;
	bOut = bOut > 255 ? 255 : bOut;
	dest.r = (uint8_t)rOut;
	dest.g = (uint8_t)gOut;
	dest.b = (uint8_t)bOut;
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

void CGraphics::RasterizePixel( int x, int y, BGRA8 sColor )
{
	assert( x >= 0 && x < m_sFrameBuffer.iWidth && y >= 0 && y < m_sFrameBuffer.iHeight );
	//if ( x >= 0 && x < m_sFrameBuffer.iWidth && y >= 0 && y < m_sFrameBuffer.iHeight )
	{
		BlendAdditive( m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x], sColor );
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

		BlendAdditive( m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix], BGRA8{ sColor.r, sColor.g, sColor.b, i00 } );
		BlendAdditive( m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix + 1], BGRA8{ sColor.r, sColor.g, sColor.b, i10 } );
		BlendAdditive( m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix], BGRA8{ sColor.r, sColor.g, sColor.b, i01 } );
		BlendAdditive( m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix + 1], BGRA8{ sColor.r, sColor.g, sColor.b, i11 } );
	}
}

void CGraphics::DrawLineH( int x, int y, int len, BGRA8 sColor )
{
	if ( y < 0 || y >= m_sFrameBuffer.iHeight || len == 0 )
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
		RasterizePixel( i, y, sColor );
	}
}

void CGraphics::DrawLineV( int x, int y, int len, BGRA8 sColor )
{
	if ( x < 0 || x >= m_sFrameBuffer.iWidth || len == 0 )
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
		RasterizePixel( x, i, sColor );
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

void CGraphics::DrawText( int x, int y, const char* pText, BGRA8 sColor, const STextureIndexed& sTex, int iCharWidth, int iCharHeight, int iSpacing /*= 0*/ )
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
		x += iCharWidth + iSpacing;
		pText++;
	}
}

