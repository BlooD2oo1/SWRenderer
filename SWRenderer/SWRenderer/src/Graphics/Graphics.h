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
	void DrawLine3D( SVector4& vPh0, SVector4 vPh1, BGRA8 sColor );
	void DrawLine3D( const SVector3& vP0, const SVector3 vP1, const SMatrix& matViewProj, BGRA8 sColor );

	bool ClipLineDepth( SVector4& vPh0, SVector4& vPh1 );
	bool ClipLineXY( SVector4& vPh0, SVector4& vPh1 );
	bool ClipPixel( SVector4 vPh );

private:
	uint8_t ClipCode( const SVector4& vP4 );	

	static uint32_t BlendAdditive( uint32_t dest, BGRA8 src );		

private:
	SFrameBuffer	m_sFrameBuffer;
};
