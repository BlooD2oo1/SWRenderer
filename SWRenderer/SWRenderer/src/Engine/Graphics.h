#pragma once

#include "Common/Globals.h"
#include "Common/Vector.h"
#include "Common/Math.h"

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

	BGRA8()	{}

	BGRA8( uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a )
		: r(_r), g(_g), b(_b), a(_a)
	{
	}

	BGRA8( uint32_t _rgba )
		: rgba(_rgba)
	{
	}

	BGRA8( float fR, float fG, float fB, float fA )
	{
		r = (uint8_t)( Clamp( fR, 0.0f, 1.0f ) * 255.0f );
		g = (uint8_t)( Clamp( fG, 0.0f, 1.0f ) * 255.0f );
		b = (uint8_t)( Clamp( fB, 0.0f, 1.0f ) * 255.0f );
		a = (uint8_t)( Clamp( fA, 0.0f, 1.0f ) * 255.0f );
	}
};

struct SFrameBuffer
{
	SFrameBuffer()
		: pData( nullptr ), iWidth( 0 ), iHeight( 0 )
	{
		vClipScaleInHom = SVector2( 1.0f, 1.0f );
	}
	SFrameBuffer( BGRA8* pData, int iWidth, int iHeight )
		: pData( pData ), iWidth( iWidth ), iHeight( iHeight )
	{
		vClipScaleInHom = SVector2( 1.0f - 0.5f/(float)iWidth, 1.0f - 0.5f/(float)iHeight );
	}
	BGRA8*		pData = nullptr;
	int			iWidth = 0;
	int			iHeight = 0;
	SVector2	vClipScaleInHom;
};

struct STextureIndexed
{
	int32_t  m_iWidth;
	int32_t  m_iHeight;
	uint8_t* m_pData;     // Pixel color indices (m_iWidth * m_iHeight)
	uint8_t* m_pPalette;  // 256 * 3 bytes RGB palette (R, G, B, R, G, B...)

	STextureIndexed()
		: m_iWidth(0)
		, m_iHeight(0)
		, m_pData(nullptr)
		, m_pPalette(nullptr)
	{}

	~STextureIndexed()
	{
		Clear();
	}

	void Clear()
	{
		if (m_pData)    { delete[] m_pData; m_pData = nullptr; }
		if (m_pPalette) { delete[] m_pPalette; m_pPalette = nullptr; }
		m_iWidth = 0;
		m_iHeight = 0;
	}
};

struct SVertexP
{
	SVector3	vPos;
	struct SAttribs
	{
		inline static void Lerp( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float t )
		{
		}
		inline static void LerpPerspective( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float a, float b )
		{
		}
	} sAttribs;
};

struct SVertexPC
{
	SVector3	vPos;
	struct SAttribs
	{
		SVector4	vColor;

		inline static void Lerp( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float t )
		{
			out.vColor = v0.vColor + (v1.vColor - v0.vColor) * t;
		}

		inline static void LerpPerspective( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float a, float b )
		{
			out.vColor = (v0.vColor * a + v1.vColor * b) / (a + b);
		}
	} sAttribs;
};

struct SVertexPW
{
	SVector3	vPos;
	struct SAttribs
	{
		float	fW;

		inline static void Lerp( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float t )
		{
			out.fW = v0.fW + (v1.fW - v0.fW) * t;
		}

		inline static void LerpPerspective( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float a, float b )
		{
			out.fW = (v0.fW * a + v1.fW * b) / (a + b);
		}
	} sAttribs;
};

template<typename TAttribs>
struct SClipVertex
{
	SVector4	vPos;
	TAttribs	sAttribs;

	inline void Lerp( const SClipVertex& v0, const SClipVertex& v1, float t)
	{
		TAttribs::Lerp( sAttribs, v0.sAttribs, v1.sAttribs, t);
	}

	inline void LerpPerspective( const SClipVertex& v0, const SClipVertex& v1, float a, float b )
	{
		TAttribs::LerpPerspective( sAttribs, v0.sAttribs, v1.sAttribs, a, b );
	}
};

// Direct overwrite (Replace / No Blend)
struct SBlendFuncCopy
{
	inline void Execute(BGRA8& dest, BGRA8 src) const
	{
		dest = src;
	}
};

// Additive blending
struct SBlendFuncAdditive
{
	inline void Execute(BGRA8& dest, BGRA8 src) const
	{
		uint32_t rOut = dest.r + ((src.r * src.a) >> 8);
		uint32_t gOut = dest.g + ((src.g * src.a) >> 8);
		uint32_t bOut = dest.b + ((src.b * src.a) >> 8);
		dest.r = (uint8_t)(rOut > 255 ? 255 : rOut);
		dest.g = (uint8_t)(gOut > 255 ? 255 : gOut);
		dest.b = (uint8_t)(bOut > 255 ? 255 : bOut);
	}
};

// Traditional Alpha Blending (SrcAlpha / OneMinusSrcAlpha)
struct SBlendFuncAlpha
{
	inline void Execute(BGRA8& dest, BGRA8 src) const
	{
		uint8_t invA = 255 - src.a;
		dest.r = (uint8_t)(((src.r * src.a) + (dest.r * invA)) >> 8);
		dest.g = (uint8_t)(((src.g * src.a) + (dest.g * invA)) >> 8);
		dest.b = (uint8_t)(((src.b * src.a) + (dest.b * invA)) >> 8);
	}
};

class CGraphics
{
public:
	__forceinline static void		CreateInstance() { SAFE_DELETE( m_pThis ); m_pThis = new CGraphics(); }
	__forceinline static CGraphics&	GetInstance() { return *m_pThis; }
	__forceinline static bool		HasInstance() { return m_pThis == nullptr ? false : true; }
	__forceinline static void		Destroy() { SAFE_DELETE( m_pThis ); }
private:
	static CGraphics*	m_pThis;
	CGraphics();
	~CGraphics();

public:
	void Create( SFrameBuffer& sFrameBuffer );
	void Clear();

	SFrameBuffer&	GetFrameBuffer() { return m_sFrameBuffer; }

	void ClearFrameBuffer( BGRA8 sColor );
	
	// Rasterization functions ( no clipping )
	template<class TBlendFunc>
	void RasterizePixel( int x, int y, BGRA8 sColor, const TBlendFunc& sBlendFunc );
	template<class TAttribs, class TPixelShader, class TBlendFunc>
	void RasterizeLineFlat( const SVector2& v0o, const SVector2& v1o, const TAttribs& sAttribs, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc );
	template<class TAttribs, class TPixelShader, class TBlendFunc>
	void RasterizeLine( const SVector2& vPos0, TAttribs sAttribs0, float fW0, const SVector2& vPos1, TAttribs sAttribs1, float fW1, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc );

	// Draw functions ( with clipping )
	void DrawPixelAA( const SVector2& v, BGRA8 sColor );
	template<class TBlendFunc>
	void DrawLineH( int x, int y, int len, BGRA8 sColor, const TBlendFunc& sBlendFunc );
	template<class TBlendFunc>
	void DrawLineV( int x, int y, int len, BGRA8 sColor, const TBlendFunc& sBlendFunc );
	template<class TVertex, class TVertexShader, class TPixelShader, class TBlendFunc>
	void DrawLine3D( const TVertex& sV0, const TVertex& sV1, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc );
	template<class TVertex, class TVertexShader, class TPixelShader, class TBlendFunc>
	void DrawLineList3D( const TVertex* pLineList, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc );
	template<class TVertex, class TVertexShader, class TPixelShader, class TBlendFunc>
	void DrawLineList3D( const TVertex* pVertices, uint32_t* pIndices, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc );	
	template<class TBlendFunc>
	void DrawRect( int x, int y, int w, int h, BGRA8 sColor, const TBlendFunc& sBlendFunc );
	template<class TBlendFunc>
	void DrawTexture( int x, int y, const TBlendFunc& sBlendFunc, const STextureIndexed& sTex );
	template<class TBlendFunc>
	void DrawText( int x, int y, const char* pText, BGRA8 sColor, const TBlendFunc& sBlendFunc, const STextureIndexed& sTex, int iCharWidth, int iCharHeight, int iSpacing = 0 );

	template<class TAttribs>
	bool ClipLineZ( SClipVertex<TAttribs>& vPh0, SClipVertex<TAttribs>& vPh1 ) const;
	template<class TAttribs>
	bool ClipLineXY( SClipVertex<TAttribs>& vPh0, SClipVertex<TAttribs>& vPh1 ) const;

private:
	uint8_t ClipCode( const SVector4& vP4 ) const
	{
		uint8_t iRet = 0;
		iRet |= ( vP4.x < -vP4.w * m_sFrameBuffer.vClipScaleInHom.x ) ? 1 : 0;
		iRet |= ( vP4.x > vP4.w * m_sFrameBuffer.vClipScaleInHom.x ) ? 2 : 0;
		iRet |= ( vP4.y < -vP4.w * m_sFrameBuffer.vClipScaleInHom.y ) ? 4 : 0;
		iRet |= ( vP4.y > vP4.w * m_sFrameBuffer.vClipScaleInHom.y ) ? 8 : 0;
		return iRet;
	}

private:
	SFrameBuffer	m_sFrameBuffer;
};

template<class TBlendFunc>
void CGraphics::RasterizePixel( int x, int y, BGRA8 sColor, const TBlendFunc& sBlendFunc )
{
	sBlendFunc.Execute( m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x], sColor );
}

template<class TAttribs, class TPixelShader, class TBlendFunc>
void CGraphics::RasterizeLineFlat( const SVector2& v0o, const SVector2& v1o, const TAttribs& sAttribs, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc )
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

	for ( int iX = iXStart; iX < iXEnd; iX++ )
	{
		float fY = v.y * ( ((float)iX+0.5f) - v0.x ) / v.x + v0.y;

		int x = iX;
		int y = (int)fY;
		if ( bSwizzle )
		{
			std::swap( x, y );
		}

		RasterizePixel( x, y, sPixelShader.Execute( sAttribs ), sBlendFunc );
	}
}

template<class TAttribs, class TPixelShader, class TBlendFunc>
void CGraphics::RasterizeLine( const SVector2& vPos0, TAttribs sAttribs0, float fW0, const SVector2& vPos1, TAttribs sAttribs1, float fW1, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc )
{
	SVector2 v( vPos1 - vPos0 );

	if ( v.x == 0.0f && v.y == 0.0f )
	{
		return;
	}

	bool bSwizzle = fabsf(v.x) < fabsf(v.y);

	SVector2 v0( vPos0 );
	SVector2 v1( vPos1 );

	if ( bSwizzle )
	{
		std::swap( v0.x, v0.y );
		std::swap( v1.x, v1.y );
	}
	if ( v1.x < v0.x )
	{
		std::swap( v0, v1 );
		std::swap( sAttribs0, sAttribs1 );
		std::swap( fW0, fW1 );
	}
	v = v1 - v0;

	int iXStart = (int)(v0.x+0.5f);
	int iXEnd = (int)(v1.x+0.5f);

	for ( int iX = iXStart; iX < iXEnd; iX++ )
	{
		float t = (((float)iX + 0.5f) - v0.x) / v.x;
		float fY = v.y * t + v0.y;

		int x = iX;
		int y = (int)fY;

		float a = (1.0f - t) / fW0;
		float b = t / fW1;

		TAttribs sAttribs;
		TAttribs::LerpPerspective( sAttribs, sAttribs0, sAttribs1, a, b );

		if ( bSwizzle )
		{
			std::swap( x, y );
		}

		RasterizePixel( x, y, sPixelShader.Execute( sAttribs ), sBlendFunc );
	}
}

template<class TVertex, class TVertexShader, class TPixelShader, class TBlendFunc>
void CGraphics::DrawLine3D( const TVertex& sV0, const TVertex& sV1, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc )
{
	using TAttribs = typename TVertexShader::AttribsType;
	using TClipVertex = SClipVertex<TAttribs>;

	TClipVertex vPh0;
	TClipVertex vPh1;
	sVertexShader.Execute( vPh0, sV0 );
	sVertexShader.Execute( vPh1, sV1 );

	if ( ClipLineZ( vPh0, vPh1 ) )
	{
		if ( ClipLineXY( vPh0, vPh1 ) )
		{
			{
				float fWRec0 = 1.0f / vPh0.vPos.w;
				vPh0.vPos.x = vPh0.vPos.x * fWRec0;
				vPh0.vPos.y = vPh0.vPos.y * fWRec0;

				float fWRec1 = 1.0f / vPh1.vPos.w;
				vPh1.vPos.x = vPh1.vPos.x * fWRec1;
				vPh1.vPos.y = vPh1.vPos.y * fWRec1;
			}

			SVector2 vScreen0( vPh0.vPos.x, vPh0.vPos.y );
			vScreen0.x = vScreen0.x * 0.5f + 0.5f;
			vScreen0.y = -(vScreen0.y) * 0.5f + 0.5f;
			vScreen0.x *= (float)m_sFrameBuffer.iWidth;
			vScreen0.y *= (float)m_sFrameBuffer.iHeight;

			SVector2 vScreen1( vPh1.vPos.x, vPh1.vPos.y );
			vScreen1.x = vScreen1.x * 0.5f + 0.5f;
			vScreen1.y = -(vScreen1.y) * 0.5f + 0.5f;
			vScreen1.x *= (float)m_sFrameBuffer.iWidth;
			vScreen1.y *= (float)m_sFrameBuffer.iHeight;

			RasterizeLine( vScreen0, vPh0.sAttribs, vPh0.vPos.w, vScreen1, vPh1.sAttribs, vPh1.vPos.w, sPixelShader, sBlendFunc );
		}
	}
}

template<class TVertex, class TVertexShader, class TPixelShader, class TBlendFunc>
void CGraphics::DrawLineList3D( const TVertex* pLineList, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc )
{
	assert( pLineList != nullptr && iPrimitiveCount > 0 );
	for ( uint32_t i = 0; i < iPrimitiveCount; i++ )
	{
		int iInd0 = i*2+0;
		int iInd1 = i*2+1;
		DrawLine3D( pLineList[iInd0], pLineList[iInd1], sVertexShader, sPixelShader, sBlendFunc );
	}
}

template<class TVertex, class TVertexShader, class TPixelShader, class TBlendFunc>
void CGraphics::DrawLineList3D( const TVertex* pVertices, uint32_t* pIndices, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader, const TBlendFunc& sBlendFunc )
{
	assert( pVertices != nullptr && pIndices != nullptr && iPrimitiveCount > 0 );
	for ( uint32_t i = 0; i < iPrimitiveCount; i++ )
	{
		uint32_t iInd0 = pIndices[i * 2 + 0];
		uint32_t iInd1 = pIndices[i * 2 + 1];
		DrawLine3D( pVertices[iInd0], pVertices[iInd1], sVertexShader, sPixelShader, sBlendFunc );
	}
}

template<class TBlendFunc>
void CGraphics::DrawLineH( int x, int y, int len, BGRA8 sColor, const TBlendFunc& sBlendFunc )
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
		RasterizePixel( i, y, sColor, sBlendFunc );
	}
}

template<class TBlendFunc>
void CGraphics::DrawLineV( int x, int y, int len, BGRA8 sColor, const TBlendFunc& sBlendFunc )
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
		RasterizePixel( x, i, sColor, sBlendFunc );
	}
}

template<class TBlendFunc>
void CGraphics::DrawRect( int x, int y, int w, int h, BGRA8 sColor, const TBlendFunc& sBlendFunc )
{
	DrawLineH( x, y, w, sColor, sBlendFunc );
	DrawLineH( x, y + h - 1, w, sColor, sBlendFunc );
	DrawLineV( x, y+1, h-2, sColor, sBlendFunc );
	DrawLineV( x + w - 1, y+1, h-2, sColor, sBlendFunc );
}

template<class TBlendFunc>
void CGraphics::DrawTexture( int x, int y, const TBlendFunc& sBlendFunc, const STextureIndexed& sTex )
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
				RasterizePixel( iDestX, iDestY, sColor, sBlendFunc );
			}			
		}
	}
}

template<class TBlendFunc>
void CGraphics::DrawText( int x, int y, const char* pText, BGRA8 sColor, const TBlendFunc& sBlendFunc, const STextureIndexed& sTex, int iCharWidth, int iCharHeight, int iSpacing )
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
					RasterizePixel( iDestX, iDestY, sColor, sBlendFunc );
				}			
			}
		}
		x += iCharWidth + iSpacing;
		pText++;
	}
}


template<class TAttribs>
bool CGraphics::ClipLineZ( SClipVertex<TAttribs>& vPh0, SClipVertex<TAttribs>& vPh1 ) const
{
	while ( 1 )
	{
		uint8_t iClipCode0 = ( vPh0.vPos.z < 0.0f ) | ( (vPh0.vPos.z > vPh0.vPos.w ) << 1 );
		uint8_t iClipCode1 = ( vPh1.vPos.z < 0.0f ) | ( (vPh1.vPos.z > vPh1.vPos.w ) << 1 );
		if ( iClipCode0 == 0 && iClipCode1 == 0 )
		{
			break;
		}
		if ( ( iClipCode0 & iClipCode1 ) != 0 )
		{
			return false;
		}

		int bit = ( ( iClipCode0 & 1 ) != ( iClipCode1 & 1 ) ) ? 1 : 2;

		SClipVertex<TAttribs> vTemp;
		{
			switch ( bit )
			{
			case 1:
			{
				float t = ( 0.0f - vPh0.vPos.z ) / ( vPh1.vPos.z - vPh0.vPos.z );
				vTemp.vPos.x = vPh0.vPos.x + ( vPh1.vPos.x - vPh0.vPos.x ) * t;
				vTemp.vPos.y = vPh0.vPos.y + ( vPh1.vPos.y - vPh0.vPos.y ) * t;
				vTemp.vPos.w = vPh0.vPos.w + ( vPh1.vPos.w - vPh0.vPos.w ) * t;
				vTemp.vPos.z = 0.0f;
				vTemp.Lerp( vPh0, vPh1, t );
			}
			break;
			case 2:
			{
				float t = ( vPh0.vPos.w - vPh0.vPos.z ) / ( vPh1.vPos.z - vPh0.vPos.z - vPh1.vPos.w + vPh0.vPos.w );
				vTemp.vPos.x = vPh0.vPos.x + ( vPh1.vPos.x - vPh0.vPos.x ) * t;
				vTemp.vPos.y = vPh0.vPos.y + ( vPh1.vPos.y - vPh0.vPos.y ) * t;
				vTemp.vPos.w = vPh0.vPos.w + ( vPh1.vPos.w - vPh0.vPos.w ) * t;
				vTemp.vPos.z = vTemp.vPos.w;
				vTemp.Lerp( vPh0, vPh1, t );
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

template<class TAttribs>
bool CGraphics::ClipLineXY( SClipVertex<TAttribs>& vPh0, SClipVertex<TAttribs>& vPh1 ) const
{
	while ( 1 )
	{
		uint8_t iClipCode0 = ClipCode( vPh0.vPos );
		uint8_t iClipCode1 = ClipCode( vPh1.vPos );

		if ( ( iClipCode0 | iClipCode1 ) == 0 )
		{
			return true;
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

		SClipVertex<TAttribs> vTemp;

		switch (bit)
		{
		case 1:
			{
				float d0 = vPh0.vPos.x + vPh0.vPos.w * m_sFrameBuffer.vClipScaleInHom.x;
				float d1 = vPh1.vPos.x + vPh1.vPos.w * m_sFrameBuffer.vClipScaleInHom.x;
				float t = d0 / (d0 - d1);
				vTemp.vPos = vPh0.vPos + (vPh1.vPos - vPh0.vPos) * t;
				vTemp.vPos.x = -vTemp.vPos.w * m_sFrameBuffer.vClipScaleInHom.x;
				vTemp.Lerp(vPh0, vPh1, t);
			}
			break;

		case 2:
			{
				float d0 = vPh0.vPos.w * m_sFrameBuffer.vClipScaleInHom.x - vPh0.vPos.x;
				float d1 = vPh1.vPos.w * m_sFrameBuffer.vClipScaleInHom.x - vPh1.vPos.x;
				float t = d0 / (d0 - d1);
				vTemp.vPos = vPh0.vPos + (vPh1.vPos - vPh0.vPos) * t;
				vTemp.vPos.x = vTemp.vPos.w * m_sFrameBuffer.vClipScaleInHom.x;
				vTemp.Lerp(vPh0, vPh1, t);
			}
			break;

		case 4:
			{
				float d0 = vPh0.vPos.y + vPh0.vPos.w * m_sFrameBuffer.vClipScaleInHom.y;
				float d1 = vPh1.vPos.y + vPh1.vPos.w * m_sFrameBuffer.vClipScaleInHom.y;
				float t = d0 / (d0 - d1);
				vTemp.vPos = vPh0.vPos + (vPh1.vPos - vPh0.vPos) * t;
				vTemp.vPos.y = -vTemp.vPos.w * m_sFrameBuffer.vClipScaleInHom.y;
				vTemp.Lerp(vPh0, vPh1, t);
			}
			break;

		default:
			{
				float d0 = vPh0.vPos.w * m_sFrameBuffer.vClipScaleInHom.y - vPh0.vPos.y;
				float d1 = vPh1.vPos.w * m_sFrameBuffer.vClipScaleInHom.y - vPh1.vPos.y;
				float t = d0 / (d0 - d1);
				vTemp.vPos = vPh0.vPos + (vPh1.vPos - vPh0.vPos) * t;
				vTemp.vPos.y = vTemp.vPos.w * m_sFrameBuffer.vClipScaleInHom.y;
				vTemp.Lerp(vPh0, vPh1, t);
			}
			break;
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