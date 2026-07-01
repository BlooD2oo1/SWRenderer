#pragma once

#include "Common/Globals.h"
#include "Vector.h"

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

struct SVertexPC
{
	SVector3	vPos;
	SVector4	vColor;
};

struct SVertexPh
{
	SVector4	vPos;
	inline void InterpolateAttribs( const SVertexPh& v0, const SVertexPh& v1, float t )
	{
	}
};

struct SVertexPhC
{
	SVector4	vPos;
	SVector4	vColor;

	inline void InterpolateAttribs( const SVertexPhC& v0, const SVertexPhC& v1, float t )
	{
		vColor = v0.vColor + (v1.vColor - v0.vColor) * t;
	}
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
	void DrawLine( const SVector2& v0o, const SVector2& v1o, BGRA8 sColor );
	void DrawLine( const SVertexPhC& v0o, const SVertexPhC& v1o );	
	void DrawLine3D( const SVertexPC& sV0, const SVertexPC& sV1, const SMatrix& matWorldViewProj );
	void DrawLineList3D( const SVertexPC* pLineList, uint32_t iPrimitiveCount, const SMatrix& matWorldViewProj );
	void DrawLineList3D( const SVertexPC* pVertices, uint32_t* pIndices, uint32_t iPrimitiveCount, const SMatrix& matWorldViewProj );

	template<class TVertex>
	bool ClipLineDepth( TVertex& vPh0, TVertex& vPh1 ) const;
	template<class TVertex>
	bool ClipLineXY( TVertex& vPh0, TVertex& vPh1 ) const;
	bool ClipPixel( SVector4 vPh ) const;

private:
	uint8_t ClipCode( const SVector4& vP4 ) const;

	static uint32_t BlendAdditive( uint32_t dest, BGRA8 src );

private:
	SFrameBuffer	m_sFrameBuffer;
};

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

		TVertex vTemp;
		switch( bit )
		{
		case 1:
		{
			float t = (-m_sFrameBuffer.vClipScaleInHom.x - vPh0.vPos.x) / (vPh1.vPos.x - vPh0.vPos.x);
			vTemp.vPos.x = -m_sFrameBuffer.vClipScaleInHom.x;
			vTemp.vPos.y = vPh0.vPos.y + (vPh1.vPos.y - vPh0.vPos.y) * t;
			vTemp.vPos.z = vPh0.vPos.z + (vPh1.vPos.z - vPh0.vPos.z) * t;
			vTemp.vPos.w = vPh0.vPos.w + (vPh1.vPos.w - vPh0.vPos.w) * t;
			vTemp.InterpolateAttribs( vPh0, vPh1, t );

		}
		break;
		case 2:
		{
			float t = (m_sFrameBuffer.vClipScaleInHom.x - vPh0.vPos.x) / (vPh1.vPos.x - vPh0.vPos.x);
			vTemp.vPos.x = m_sFrameBuffer.vClipScaleInHom.x;
			vTemp.vPos.y = vPh0.vPos.y + (vPh1.vPos.y - vPh0.vPos.y) * t;
			vTemp.vPos.z = vPh0.vPos.z + (vPh1.vPos.z - vPh0.vPos.z) * t;
			vTemp.vPos.w = vPh0.vPos.w + (vPh1.vPos.w - vPh0.vPos.w) * t;
			vTemp.InterpolateAttribs( vPh0, vPh1, t );
		}
		break;
		case 4:
		{
			float t = (-m_sFrameBuffer.vClipScaleInHom.y - vPh0.vPos.y) / (vPh1.vPos.y - vPh0.vPos.y);
			vTemp.vPos.x = vPh0.vPos.x + (vPh1.vPos.x - vPh0.vPos.x) * t;
			vTemp.vPos.y = -m_sFrameBuffer.vClipScaleInHom.y;
			vTemp.vPos.z = vPh0.vPos.z + (vPh1.vPos.z - vPh0.vPos.z) * t;
			vTemp.vPos.w = vPh0.vPos.w + (vPh1.vPos.w - vPh0.vPos.w) * t;
			vTemp.InterpolateAttribs( vPh0, vPh1, t );
		}
		break;
		case 8:
		{
			float t = (m_sFrameBuffer.vClipScaleInHom.y - vPh0.vPos.y) / (vPh1.vPos.y - vPh0.vPos.y);
			vTemp.vPos.x = vPh0.vPos.x + (vPh1.vPos.x - vPh0.vPos.x) * t;
			vTemp.vPos.y = m_sFrameBuffer.vClipScaleInHom.y;
			vTemp.vPos.z = vPh0.vPos.z + (vPh1.vPos.z - vPh0.vPos.z) * t;
			vTemp.vPos.w = vPh0.vPos.w + (vPh1.vPos.w - vPh0.vPos.w) * t;
			vTemp.InterpolateAttribs( vPh0, vPh1, t );
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