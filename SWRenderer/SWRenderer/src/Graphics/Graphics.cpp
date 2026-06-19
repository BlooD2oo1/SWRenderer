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
	m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x] = BlendAdditive( m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x], sColor );
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
		uint8_t i00 = (uint8_t)( sqrtf( fxmodinv*fymodinv ) * 255.0f );
		uint8_t i01 = (uint8_t)( sqrtf( fxmodinv*fymod ) * 255.0f );
		uint8_t i10 = (uint8_t)( sqrtf( fxmod * fymodinv ) * 255.0f);
		uint8_t i11 = (uint8_t)( sqrtf( fxmod * fymod ) * 255.0f);

		m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix] = BlendAdditive( m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix], BGRA8{ i00, i00, i00, 0 } );
		m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix + 1] = BlendAdditive( m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix + 1], BGRA8{ i10, i10, i10, 0 } );
		m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix] = BlendAdditive( m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix], BGRA8{ i01, i01, i01, 0 } );
		m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix + 1] = BlendAdditive( m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix + 1], BGRA8{ i11, i11, i11, 0 } );
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

void CGraphics::DrawLine3D( SVector4& vPh0, SVector4 vPh1, BGRA8 sColor )
{
	if ( ClipLineDepth( vPh0, vPh1 ) )
	{
		{
			float fWRec0 = 1.0f / vPh0.w;
			vPh0.x = vPh0.x * fWRec0;
			vPh0.y = vPh0.y * fWRec0;
			//vPh0.z = vPh0.z * fWRec0;
			vPh0.w = 1.0f;

			float fWRec1 = 1.0f / vPh1.w;
			vPh1.x = vPh1.x * fWRec1;
			vPh1.y = vPh1.y * fWRec1;
			//vPh1.z = vPh1.z * fWRec1;
			vPh1.w = 1.0f;
		}

		if ( ClipLineXY( vPh0, vPh1 ) )
		{
			SVector2 vP0( (vPh0.x)*0.5f + 0.5f, -(vPh0.y)*0.5f + 0.5f );			
			vP0.x *= (float)m_sFrameBuffer.iWidth;
			vP0.y *= (float)m_sFrameBuffer.iHeight;

			SVector2 vP1( (vPh1.x)*0.5f + 0.5f, -(vPh1.y)*0.5f + 0.5f );
			vP1.x *= (float)m_sFrameBuffer.iWidth;
			vP1.y *= (float)m_sFrameBuffer.iHeight;

			DrawLine( vP0, vP1, sColor );
		}
	}
}

void CGraphics::DrawLine3D( const SVector3& vP0, const SVector3 vP1, const SMatrix& matViewProj, BGRA8 sColor )
{
	SVector4 vPh0;
	SVector4 vPh1;
	{
		SVector4 vPh0Src( vP0, 1.0f );
		SMatrix::Mul( vPh0, vPh0Src, matViewProj );			
		SVector4 vPh1Src( vP1, 1.0f );
		SMatrix::Mul( vPh1, vPh1Src, matViewProj );				
	}

	DrawLine3D( vPh0, vPh1, sColor );
}

bool CGraphics::ClipLineDepth( SVector4& vPh0, SVector4& vPh1 )
{
	while ( 1 )
	{
		uint8_t iClipCode0 = ( vPh0.z < 0.0f ) | ( (vPh0.z > vPh0.w ) << 1 );
		uint8_t iClipCode1 = ( vPh1.z < 0.0f ) | ( (vPh1.z > vPh1.w ) << 1 );
		if ( iClipCode0 == 0 && iClipCode1 == 0 )
		{
			break;
		}
		if ( ( iClipCode0 & iClipCode1 ) != 0 )
		{
			return false;
		}

		int bit = ( ( iClipCode0 & 1 ) != ( iClipCode1 & 1 ) ) ? 1 : 2;

		SVector4 vTemp;
		{
			switch ( bit )
			{
			case 1:
				{
					float t = ( 0.0f - vPh0.z ) / ( vPh1.z - vPh0.z );
					vTemp.x = vPh0.x + ( vPh1.x - vPh0.x ) * t;
					vTemp.y = vPh0.y + ( vPh1.y - vPh0.y ) * t;
					vTemp.w = vPh0.w + ( vPh1.w - vPh0.w ) * t;
					vTemp.z = 0.0f;
				}
				break;
			case 2:
				{
					float t = ( vPh0.w - vPh0.z ) / ( vPh1.z - vPh0.z - vPh1.w + vPh0.w );
					vTemp.x = vPh0.x + ( vPh1.x - vPh0.x ) * t;
					vTemp.y = vPh0.y + ( vPh1.y - vPh0.y ) * t;
					vTemp.w = vPh0.w + ( vPh1.w - vPh0.w ) * t;
					vTemp.z = vTemp.w;
				}
				break;
			}
		}

		if ( iClipCode0 & bit )
		{
			vPh0 = vTemp;
		}
		else
		{
			vPh1 = vTemp;
		}
	}

	return true;
}

bool CGraphics::ClipLineXY( SVector4& vPh0, SVector4& vPh1 )
{
	/*if ( iClipCode0 != 0 || iClipCode1 != 0 )
	{
	return false;
	}*/

	while ( 1 )
	{
		uint8_t iClipCode0 = ClipCode( vPh0 );
		uint8_t iClipCode1 = ClipCode( vPh1 );

		if ( iClipCode0 == 0 && iClipCode1 == 0 )
		{
			break;
		}
		if ( ( iClipCode0 & iClipCode1 ) != 0 )
		{
			return false;
		}

		uint8_t bit;
		if		( ( iClipCode0 & 1 ) != ( iClipCode1 & 1 ) )	bit = 1;
		else if	( ( iClipCode0 & 2 ) != ( iClipCode1 & 2 ) )	bit = 2;
		else if	( ( iClipCode0 & 4 ) != ( iClipCode1 & 4 ) )	bit = 4;
		else													bit = 8;

		SVector2 vTemp;
		switch( bit )
		{
		case 1:
			vTemp.x = -m_sFrameBuffer.vClipScaleInHom.x;//vPh0.x + ( vPh1.x - vPh0.x ) * ( -1.0f - vPh0.x ) / ( vPh1.x - vPh0.x );
			vTemp.y = vPh0.y + ( vPh1.y - vPh0.y ) * ( -m_sFrameBuffer.vClipScaleInHom.x - vPh0.x ) / ( vPh1.x - vPh0.x );
			break;
		case 2:
			vTemp.x = m_sFrameBuffer.vClipScaleInHom.x;//vPh0.x + ( vPh1.x - vPh0.x ) * ( 1.0f - vPh0.x ) / ( vPh1.x - vPh0.x );
			vTemp.y = vPh0.y + ( vPh1.y - vPh0.y ) * ( m_sFrameBuffer.vClipScaleInHom.x - vPh0.x ) / ( vPh1.x - vPh0.x );
			break;
		case 4:
			vTemp.x = vPh0.x + ( vPh1.x - vPh0.x ) * ( -m_sFrameBuffer.vClipScaleInHom.y - vPh0.y ) / ( vPh1.y - vPh0.y );
			vTemp.y = -m_sFrameBuffer.vClipScaleInHom.y;//vPh0.y + ( vPh1.y - vPh0.y ) * ( -1.0f - vPh0.y ) / ( vPh1.y - vPh0.y );
			break;
		case 8:
			vTemp.x = vPh0.x + ( vPh1.x - vPh0.x ) * ( m_sFrameBuffer.vClipScaleInHom.y - vPh0.y ) / ( vPh1.y - vPh0.y );
			vTemp.y = m_sFrameBuffer.vClipScaleInHom.y;//vPh0.y + ( vPh1.y - vPh0.y ) * ( 1.0f - vPh0.y ) / ( vPh1.y - vPh0.y );
			break;
		}

		if ( iClipCode0 & bit )
		{
			vPh0.x = vTemp.x;
			vPh0.y = vTemp.y;
		}
		else
		{
			vPh1.x = vTemp.x;
			vPh1.y = vTemp.y;
		}
	}

	return true;
}

uint8_t CGraphics::ClipCode( const SVector4& vP4 )
{
	uint8_t iRet = 0;
	iRet |= ( vP4.x < -m_sFrameBuffer.vClipScaleInHom.x ) ? 1 : 0;
	iRet |= ( vP4.x > m_sFrameBuffer.vClipScaleInHom.x ) ? 2 : 0;
	iRet |= ( vP4.y < -m_sFrameBuffer.vClipScaleInHom.y ) ? 4 : 0;
	iRet |= ( vP4.y > m_sFrameBuffer.vClipScaleInHom.y ) ? 8 : 0;
	return iRet;
}

uint32_t CGraphics::BlendAdditive( uint32_t dest, BGRA8 src )
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

