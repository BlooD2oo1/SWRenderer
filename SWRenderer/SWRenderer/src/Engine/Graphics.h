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
		inline static void InterpolateAttribs( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float t )
		{
		}
		inline static void InterpolateAttribs( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float a, float b )
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

		inline static void InterpolateAttribs( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float t )
		{
			out.vColor = v0.vColor + (v1.vColor - v0.vColor) * t;
		}

		inline static void InterpolateAttribs( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float a, float b )
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
		float		fW;

		inline static void InterpolateAttribs( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float t )
		{
			out.fW = v0.fW + (v1.fW - v0.fW) * t;
		}

		inline static void InterpolateAttribs( SAttribs& out, const SAttribs& v0, const SAttribs& v1, float a, float b )
		{
			out.fW = (v0.fW * a + v1.fW * b) / (a + b);
		}
	} sAttribs;
};

template<typename TAttribs>
struct SClipVertex
{
	SVector4   vPos;
	TAttribs  sAttribs;

	inline void InterpolateAttribs( const SClipVertex& v0, const SClipVertex& v1, float t)
	{
		TAttribs::InterpolateAttribs( sAttribs, v0.sAttribs, v1.sAttribs, t);
	}

	inline void InterpolateAttribs( const SClipVertex& v0, const SClipVertex& v1, float a, float b )
	{
		TAttribs::InterpolateAttribs( sAttribs, v0.sAttribs, v1.sAttribs, a, b );
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
	
	// Rasterize functions ( no clipping )
	void RasterizePixel( int x, int y, BGRA8 sColor );

	template<class TAttribs, class TPixelShader>
	void RasterizePixel( int x, int y, const TAttribs& sAttribs, const TPixelShader& sPixelShader );
	template<class TAttribs, class TPixelShader>
	void RasterizeLine( const SVector2& v0o, const SVector2& v1o, const TAttribs& sAttribs, const TPixelShader& sPixelShader );
	template<class TAttribs, class TPixelShader>
	void RasterizeLine( const SVector2& vPos0, TAttribs sAttribs0, float fW0, const SVector2& vPos1, TAttribs sAttribs1, float fW1, const TPixelShader& sPixelShader );

	// Draw functions ( with clipping )
	void DrawPixelAA( const SVector2& v, BGRA8 sColor );
	void DrawLineH( int x, int y, int len, BGRA8 sColor );
	void DrawLineV( int x, int y, int len, BGRA8 sColor );
	template<class TVertex, class TVertexShader, class TPixelShader>
	void DrawLine3D( const TVertex& sV0, const TVertex& sV1, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader );
	template<class TVertex, class TVertexShader, class TPixelShader>
	void DrawLineList3D( const TVertex* pLineList, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader );
	template<class TVertex, class TVertexShader, class TPixelShader>
	void DrawLineList3D( const TVertex* pVertices, uint32_t* pIndices, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader );
	
	void DrawRect( int x, int y, int w, int h, BGRA8 sColor );

	void DrawTexture( int x, int y, const STextureIndexed& sTex );

	void DrawText( int x, int y, const char* pText, BGRA8 sColor, const STextureIndexed& sTex, int iCharWidth, int iCharHeight, int iSpacing = 0 );

	template<class TAttribs>
	bool ClipLineDepth( SClipVertex<TAttribs>& vPh0, SClipVertex<TAttribs>& vPh1 ) const;
	template<class TAttribs>
	bool ClipLineXY( SClipVertex<TAttribs>& vPh0, SClipVertex<TAttribs>& vPh1 ) const;
	bool ClipPixel( SVector4 vPh ) const
	{
		uint8_t iClipCode = ClipCode( vPh );
		if ( iClipCode != 0 )
		{
			return false;
		}
		return true;
	}

	inline static void BlendAdditive( BGRA8& dest, BGRA8 src )
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

template<class TAttribs, class TPixelShader>
void CGraphics::RasterizePixel( int x, int y, const TAttribs& sAttribs, const TPixelShader& sPixelShader )
{
	sPixelShader.Process( m_sFrameBuffer.pData[y * m_sFrameBuffer.iWidth + x], x, y, sAttribs );
}

template<class TAttribs, class TPixelShader>
void CGraphics::RasterizeLine( const SVector2& v0o, const SVector2& v1o, const TAttribs& sAttribs, const TPixelShader& sPixelShader )
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

		RasterizePixel( x, y, sAttribs, sPixelShader );
	}
}

template<class TAttribs, class TPixelShader>
void CGraphics::RasterizeLine( const SVector2& vPos0, TAttribs sAttribs0, float fW0, const SVector2& vPos1, TAttribs sAttribs1, float fW1, const TPixelShader& sPixelShader )
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
		TAttribs::InterpolateAttribs( sAttribs, sAttribs0, sAttribs1, a, b );

		if ( bSwizzle )
		{
			std::swap( x, y );
		}

		RasterizePixel( x, y, sAttribs, sPixelShader );
	}
}

template<class TVertex, class TVertexShader, class TPixelShader>
void CGraphics::DrawLine3D( const TVertex& sV0, const TVertex& sV1, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader )
{
	using TAttribs = typename TVertexShader::AttribsType;
	using TClipVertex = SClipVertex<TAttribs>;

	TClipVertex vPh0;
	TClipVertex vPh1;
	sVertexShader.Process( vPh0, sV0 );
	sVertexShader.Process( vPh1, sV1 );

	if ( ClipLineDepth( vPh0, vPh1 ) )
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

			RasterizeLine( vScreen0, vPh0.sAttribs, vPh0.vPos.w, vScreen1, vPh1.sAttribs, vPh1.vPos.w, sPixelShader );
		}
	}
}

template<class TVertex, class TVertexShader, class TPixelShader>
void CGraphics::DrawLineList3D( const TVertex* pLineList, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader )
{
	assert( pLineList != nullptr && iPrimitiveCount > 0 );
	for ( uint32_t i = 0; i < iPrimitiveCount; i++ )
	{
		int iInd0 = i*2+0;
		int iInd1 = i*2+1;
		DrawLine3D( pLineList[iInd0], pLineList[iInd1], sVertexShader, sPixelShader );
	}
}

template<class TVertex, class TVertexShader, class TPixelShader>
void CGraphics::DrawLineList3D( const TVertex* pVertices, uint32_t* pIndices, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader, const TPixelShader& sPixelShader )
{
	assert( pVertices != nullptr && pIndices != nullptr && iPrimitiveCount > 0 );
	for ( uint32_t i = 0; i < iPrimitiveCount; i++ )
	{
		uint32_t iInd0 = pIndices[i * 2 + 0];
		uint32_t iInd1 = pIndices[i * 2 + 1];
		DrawLine3D( pVertices[iInd0], pVertices[iInd1], sVertexShader, sPixelShader );
	}
}

template<class TAttribs>
bool CGraphics::ClipLineDepth( SClipVertex<TAttribs>& vPh0, SClipVertex<TAttribs>& vPh1 ) const
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
				vTemp.InterpolateAttribs( vPh0, vPh1, t );
			}
			break;
			case 2:
			{
				float t = ( vPh0.vPos.w - vPh0.vPos.z ) / ( vPh1.vPos.z - vPh0.vPos.z - vPh1.vPos.w + vPh0.vPos.w );
				vTemp.vPos.x = vPh0.vPos.x + ( vPh1.vPos.x - vPh0.vPos.x ) * t;
				vTemp.vPos.y = vPh0.vPos.y + ( vPh1.vPos.y - vPh0.vPos.y ) * t;
				vTemp.vPos.w = vPh0.vPos.w + ( vPh1.vPos.w - vPh0.vPos.w ) * t;
				vTemp.vPos.z = vTemp.vPos.w;
				vTemp.InterpolateAttribs( vPh0, vPh1, t );
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
				vTemp.InterpolateAttribs(vPh0, vPh1, t);
			}
			break;

		case 2:
			{
				float d0 = vPh0.vPos.w * m_sFrameBuffer.vClipScaleInHom.x - vPh0.vPos.x;
				float d1 = vPh1.vPos.w * m_sFrameBuffer.vClipScaleInHom.x - vPh1.vPos.x;
				float t = d0 / (d0 - d1);
				vTemp.vPos = vPh0.vPos + (vPh1.vPos - vPh0.vPos) * t;
				vTemp.vPos.x = vTemp.vPos.w * m_sFrameBuffer.vClipScaleInHom.x;
				vTemp.InterpolateAttribs(vPh0, vPh1, t);
			}
			break;

		case 4:
			{
				float d0 = vPh0.vPos.y + vPh0.vPos.w * m_sFrameBuffer.vClipScaleInHom.y;
				float d1 = vPh1.vPos.y + vPh1.vPos.w * m_sFrameBuffer.vClipScaleInHom.y;
				float t = d0 / (d0 - d1);
				vTemp.vPos = vPh0.vPos + (vPh1.vPos - vPh0.vPos) * t;
				vTemp.vPos.y = -vTemp.vPos.w * m_sFrameBuffer.vClipScaleInHom.y;
				vTemp.InterpolateAttribs(vPh0, vPh1, t);
			}
			break;

		default:
			{
				float d0 = vPh0.vPos.w * m_sFrameBuffer.vClipScaleInHom.y - vPh0.vPos.y;
				float d1 = vPh1.vPos.w * m_sFrameBuffer.vClipScaleInHom.y - vPh1.vPos.y;
				float t = d0 / (d0 - d1);
				vTemp.vPos = vPh0.vPos + (vPh1.vPos - vPh0.vPos) * t;
				vTemp.vPos.y = vTemp.vPos.w * m_sFrameBuffer.vClipScaleInHom.y;
				vTemp.InterpolateAttribs(vPh0, vPh1, t);
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