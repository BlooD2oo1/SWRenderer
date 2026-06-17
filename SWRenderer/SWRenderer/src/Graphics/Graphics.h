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
	void DrawLine3D( SVector4& vPh0, SVector4 vPh1, BGRA8 sColor );
	void DrawLine3D( const SVector3& vP0, const SVector3 vP1, const SMatrix& matViewProj, BGRA8 sColor );

	static bool ClipLineDepth( SVector4& vPh0, SVector4& vPh1 );
	static bool ClipLineXY( SVector4& vPh0, SVector4& vPh1 );

private:
		static uint32_t BlendAdditive( uint32_t dest, BGRA8 src );
		static uint8_t ClipCode( const SVector4& vP4 );

private:
	SFrameBuffer	m_sFrameBuffer;
};
