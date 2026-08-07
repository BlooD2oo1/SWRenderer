#include "Starfield.h"
#include "Common/Defines.h"

CStarfield::CStarfield()
{
	m_pStars = nullptr;
	Clear();
}

CStarfield::~CStarfield()
{
	Clear();
}

void CStarfield::Clear()
{
	SAFE_DELETE_ARRAY( m_pStars );
	m_iStarsCount = 0;
}

void CStarfield::Create()
{
	Clear();

	m_iStarsCount = 1000;
	m_pStars = new SVertexPC[m_iStarsCount];
	for ( uint32_t i = 0; i < m_iStarsCount; i++ )
	{
		m_pStars[i].vPos.x = ((float)rand()/(float)RAND_MAX);
		m_pStars[i].vPos.y = ((float)rand()/(float)RAND_MAX);
		m_pStars[i].vPos.z = ((float)rand()/(float)RAND_MAX);

		float a = ((float)rand()/(float)RAND_MAX);
		a = powf( a, 40.0f );
		a = a * 0.8f + 0.2f;
		m_pStars[i].sAttribs.vColor = SVector4( ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, ((float)rand()/(float)RAND_MAX)*0.1f+0.7f, ((float)rand()/(float)RAND_MAX)*0.1f+0.6f, a*2.0f );
	}
}

void CStarfield::Render( const SCamera& sCamera, const SViewPort& sViewport )
{
	struct SPixelShaderBasic
	{
		BGRA8 sColor;
		BGRA8 Execute( const SVertexP::SAttribs& in ) const
		{
			return sColor;
		}
	} sPixelShaderBasic;

	SMatrix matViewProjViewport;
	SMatrix::Mul( matViewProjViewport, sCamera.m_matViewProj, sViewport.GetViewPortMatrix() );
	SMatrix matViewProjViewportPrev;
	SMatrix::Mul( matViewProjViewportPrev, sCamera.m_matViewProjPrev, sViewport.GetViewPortMatrix() );

	float fAlpha = 1.0f;
	const int iSteps = 2;
	for ( int j =0; j < iSteps; j++ )
	{
		float fStarBoxSize = powf( (float)(j+1), 3.0f ) * 200.0f;
		float fStarBoxSizeInv = 1.0f / fStarBoxSize;
		for ( uint32_t i = 0; i < m_iStarsCount; i++ )
		{
			SClipVertex<SVertexP::SAttribs> sPh0;
			SClipVertex<SVertexP::SAttribs> sPh1;
			{
				SVector4 vPhSrc( m_pStars[i].vPos * fStarBoxSize, 1.0f );
				vPhSrc.x = vPhSrc.x - floorf((vPhSrc.x - sCamera.m_vEyeSmooth.x) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
				vPhSrc.y = vPhSrc.y - floorf((vPhSrc.y - sCamera.m_vEyeSmooth.y) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
				vPhSrc.z = vPhSrc.z - floorf((vPhSrc.z - sCamera.m_vEyeSmooth.z) * fStarBoxSizeInv + 0.5f) * fStarBoxSize;
				SMatrix::Mul( sPh0.vPos, vPhSrc, matViewProjViewport );
				SMatrix::Mul( sPh1.vPos, vPhSrc, matViewProjViewportPrev );

			}

			if ( CGraphics::GetInstance().ClipLineZ( sPh0, sPh1 ) )
			{
				if ( CGraphics::GetInstance().ClipLineXY( sPh0, sPh1, sViewport ) )
				{
					{
						float fWRec0 = 1.0f / sPh0.vPos.w;
						sPh0.vPos.x = sPh0.vPos.x * fWRec0;
						sPh0.vPos.y = sPh0.vPos.y * fWRec0;

						float fWRec1 = 1.0f / sPh1.vPos.w;
						sPh1.vPos.x = sPh1.vPos.x * fWRec1;
						sPh1.vPos.y = sPh1.vPos.y * fWRec1;
					}

					SVector2 vL( sPh0.vPos.x - sPh1.vPos.x, sPh0.vPos.y - sPh1.vPos.y );
					float fL = SVector2::Length( vL );

					if ( fL > 1.5f )
					{
						sPixelShaderBasic.sColor = BGRA8( m_pStars[i].sAttribs.vColor.x * fAlpha, m_pStars[i].sAttribs.vColor.y, m_pStars[i].sAttribs.vColor.z * fAlpha, m_pStars[i].sAttribs.vColor.w / (fL * 0.2f + 1.0f) );
						CGraphics::GetInstance().RasterizeLineFlat( SVector2( sPh0.vPos.x, sPh0.vPos.y ), SVector2( sPh1.vPos.x, sPh1.vPos.y ), sPh0.sAttribs, sPixelShaderBasic, SBlendFuncAdditive() );
					}
					else
					{
						BGRA8 sColor = BGRA8( m_pStars[i].sAttribs.vColor.x * fAlpha, m_pStars[i].sAttribs.vColor.y, m_pStars[i].sAttribs.vColor.z * fAlpha, m_pStars[i].sAttribs.vColor.w );
						CGraphics::GetInstance().RasterizePixel( (int)sPh0.vPos.x, (int)sPh0.vPos.y, sColor, SBlendFuncAdditive() );
					}
				}
			}
		}
	}
}
