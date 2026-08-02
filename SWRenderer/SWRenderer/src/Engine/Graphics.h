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

	struct SVertexh
	{
		SVector4	vPos;
		inline void InterpolateAttribs( const SVertexh& v0, const SVertexh& v1, float t )
		{
		}
	};
};

struct SVertexPC
{
	SVector3	vPos;
	SVector4	vColor;

	struct SVertexh
	{
		SVector4	vPos;
		SVector4	vColor;

		inline void InterpolateAttribs( const SVertexh& v0, const SVertexh& v1, float t )
		{
			vColor = v0.vColor + (v1.vColor - v0.vColor) * t;
		}
	};
};


struct SFrameBuffer
{
	SFrameBuffer()
		: pData( nullptr ), iWidth( 0 ), iHeight( 0 )
	{
		vClipScaleInHom = SVector2( 1.0f, 1.0f );
	}
	SFrameBuffer( uint32_t* pData, int iWidth, int iHeight )
		: pData( pData ), iWidth( iWidth ), iHeight( iHeight )
	{
		vClipScaleInHom = SVector2( 1.0f - 0.5f/(float)iWidth, 1.0f - 0.5f/(float)iHeight );
	}
	uint32_t*	pData = nullptr;
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
	
	void DrawPixel( int x, int y, BGRA8 sColor );
	void DrawPixelAA( const SVector2& v, BGRA8 sColor );
	void DrawLineH( int x, int y, int len, BGRA8 sColor );
	void DrawLineV( int x, int y, int len, BGRA8 sColor );
	void DrawLine( const SVector2& v0o, const SVector2& v1o, BGRA8 sColor );
	void DrawLine( const SVertexPC::SVertexh& v0o, const SVertexPC::SVertexh& v1o );	
	template<class TVertex, class TVertexShader>
	void DrawLine3D( const TVertex& sV0, const TVertex& sV1, const TVertexShader& sVertexShader );
	template<class TVertex, class TVertexShader>
	void DrawLineList3D( const TVertex* pLineList, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader );
	template<class TVertex, class TVertexShader>
	void DrawLineList3D( const TVertex* pVertices, uint32_t* pIndices, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader );

	void DrawRect( int x, int y, int w, int h, BGRA8 sColor );

	void DrawTexture( int x, int y, const STextureIndexed& sTex );

	void DrawText( int x, int y, const char* pText, BGRA8 sColor, const STextureIndexed& sTex, int iCharWidth, int iCharHeight, int iSpacing = 0 );

	template<class TVertex>
	bool ClipLineDepth( TVertex& vPh0, TVertex& vPh1 ) const;
	template<class TVertex>
	bool ClipLineXY( TVertex& vPh0, TVertex& vPh1 ) const;
	bool ClipPixel( SVector4 vPh ) const
	{
		uint8_t iClipCode = ClipCode( vPh );
		if ( iClipCode != 0 )
		{
			return false;
		}
		return true;
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

	static uint32_t BlendAdditive( uint32_t dest, BGRA8 src );

private:
	SFrameBuffer	m_sFrameBuffer;
};

template<class TVertex, class TVertexShader>
void CGraphics::DrawLine3D( const TVertex& sV0, const TVertex& sV1, const TVertexShader& sVertexShader )
{
	SVertexPC::SVertexh vPh0;
	SVertexPC::SVertexh vPh1;
	sVertexShader.Process( vPh0, sV0 );
	sVertexShader.Process( vPh1, sV1 );

	if ( ClipLineDepth<SVertexPC::SVertexh>( vPh0, vPh1 ) )
	{
		if ( ClipLineXY<SVertexPC::SVertexh>( vPh0, vPh1 ) )
		{
			{
				float fWRec0 = 1.0f / vPh0.vPos.w;
				vPh0.vPos.x = vPh0.vPos.x * fWRec0;
				vPh0.vPos.y = vPh0.vPos.y * fWRec0;

				float fWRec1 = 1.0f / vPh1.vPos.w;
				vPh1.vPos.x = vPh1.vPos.x * fWRec1;
				vPh1.vPos.y = vPh1.vPos.y * fWRec1;
			}

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

template<class TVertex, class TVertexShader>
void CGraphics::DrawLineList3D( const TVertex* pLineList, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader )
{
	assert( pLineList != nullptr && iPrimitiveCount > 0 );
	for ( uint32_t i = 0; i < iPrimitiveCount; i++ )
	{
		int iInd0 = i*2+0;
		int iInd1 = i*2+1;
		DrawLine3D( pLineList[iInd0], pLineList[iInd1], sVertexShader );
	}
}

template<class TVertex, class TVertexShader>
void CGraphics::DrawLineList3D( const TVertex* pVertices, uint32_t* pIndices, uint32_t iPrimitiveCount, const TVertexShader& sVertexShader )
{
	assert( pVertices != nullptr && pIndices != nullptr && iPrimitiveCount > 0 );
	for ( uint32_t i = 0; i < iPrimitiveCount; i++ )
	{
		uint32_t iInd0 = pIndices[i * 2 + 0];
		uint32_t iInd1 = pIndices[i * 2 + 1];
		DrawLine3D( pVertices[iInd0], pVertices[iInd1], sVertexShader );
	}
}

template<class TVertex>
bool CGraphics::ClipLineDepth( TVertex& vPh0, TVertex& vPh1 ) const
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

		TVertex vTemp;
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

template<class TVertex>
bool CGraphics::ClipLineXY( TVertex& vPh0, TVertex& vPh1 ) const
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

		TVertex vTemp;

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