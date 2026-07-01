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

void CGraphics::DrawLine3D( const SVertexPC& sV0, const SVertexPC& sV1, const SMatrix& matWorldViewProj )
{
	SVertexPhC vPh0;
	SVertexPhC vPh1;
	{
		SVector4 vPhSrc0( sV0.vPos, 1.0f );
		SMatrix::Mul( vPh0.vPos, vPhSrc0, matWorldViewProj );
		vPh0.vColor = sV0.vColor;

		SVector4 vPhSrc1( sV1.vPos, 1.0f );
		SMatrix::Mul( vPh1.vPos, vPhSrc1, matWorldViewProj );
		vPh1.vColor = sV1.vColor;
	}

	if ( ClipLineDepth<SVertexPhC>( vPh0, vPh1 ) )
	{
		{
			float fWRec0 = 1.0f / vPh0.vPos.w;
			vPh0.vPos.x = vPh0.vPos.x * fWRec0;
			vPh0.vPos.y = vPh0.vPos.y * fWRec0;

			float fWRec1 = 1.0f / vPh1.vPos.w;
			vPh1.vPos.x = vPh1.vPos.x * fWRec1;
			vPh1.vPos.y = vPh1.vPos.y * fWRec1;
		}

		if ( ClipLineXY<SVertexPhC>( vPh0, vPh1 ) )
		{
			vPh0.vPos.x = vPh0.vPos.x * 0.5f + 0.5f;
			vPh0.vPos.y = -(vPh0.vPos.y) * 0.5f + 0.5f;
			vPh0.vPos.x *= (float)m_sFrameBuffer.iWidth;
			vPh0.vPos.y *= (float)m_sFrameBuffer.iHeight;

			vPh1.vPos.x = vPh1.vPos.x * 0.5f + 0.5f;
			vPh1.vPos.y = -(vPh1.vPos.y) * 0.5f + 0.5f;
			vPh1.vPos.x *= (float)m_sFrameBuffer.iWidth;
			vPh1.vPos.y *= (float)m_sFrameBuffer.iHeight;

			DrawLine( vPh0, vPh1 );
		}
	}
}

void CGraphics::DrawLineList3D( const SVertexPC* pLineList, uint32_t iPrimitiveCount, const SMatrix& matWorldViewProj )
{
	assert( pLineList != nullptr && iPrimitiveCount > 0 );
	for ( uint32_t i = 0; i < iPrimitiveCount; i++ )
	{
		int iInd0 = i*2+0;
		int iInd1 = i*2+1;
		DrawLine3D( pLineList[iInd0], pLineList[iInd1], matWorldViewProj );
	}
}

void CGraphics::DrawLineList3D( const SVertexPC* pVertices, uint32_t* pIndices, uint32_t iPrimitiveCount, const SMatrix& matWorldViewProj )
{
	assert( pVertices != nullptr && pIndices != nullptr && iPrimitiveCount > 0 );
	for ( uint32_t i = 0; i < iPrimitiveCount; i++ )
	{
		uint32_t iInd0 = pIndices[i * 2 + 0];
		uint32_t iInd1 = pIndices[i * 2 + 1];
		DrawLine3D( pVertices[iInd0], pVertices[iInd1], matWorldViewProj );
	}
}

bool CGraphics::ClipPixel( SVector4 vPh ) const
{
	uint8_t iClipCode = ClipCode( vPh );
	if ( iClipCode != 0 )
	{
		return false;
	}
	return true;
}

uint8_t CGraphics::ClipCode( const SVector4& vP4 ) const
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

