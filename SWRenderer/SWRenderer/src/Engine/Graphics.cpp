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

		SBlendFuncAdditive  sBlendFunc;
		sBlendFunc.Execute( m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix], BGRA8{ sColor.r, sColor.g, sColor.b, i00 } );
		sBlendFunc.Execute( m_sFrameBuffer.pData[iy * m_sFrameBuffer.iWidth + ix + 1], BGRA8{ sColor.r, sColor.g, sColor.b, i10 } );
		sBlendFunc.Execute( m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix], BGRA8{ sColor.r, sColor.g, sColor.b, i01 } );
		sBlendFunc.Execute( m_sFrameBuffer.pData[(iy + 1) * m_sFrameBuffer.iWidth + ix + 1], BGRA8{ sColor.r, sColor.g, sColor.b, i11 } );
	}
}
