#include "Starfield.h"
#include "Common/Defines.h"

CStarfield::CStarfield()
{
	m_pStars = nullptr;
	m_pStarsBG = nullptr;
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
	SAFE_DELETE_ARRAY( m_pStarsBG );
	m_iStarsBGCount = 0;
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
		a = powf( a, 2.0f );
		a = a * 0.9f + 0.1f;
		a = a*0.7f;
		//m_pStars[i].sAttribs.vColor = SVector4( ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, ((float)rand()/(float)RAND_MAX)*0.1f+0.7f, ((float)rand()/(float)RAND_MAX)*0.1f+0.5f, a*2.0f );
		m_pStars[i].sAttribs.vColor = SVector4( ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, 1.0f );
		m_pStars[i].sAttribs.vColor.x = powf( m_pStars[i].sAttribs.vColor.x*a, 1.3f )*1.0f;
		m_pStars[i].sAttribs.vColor.y = powf( m_pStars[i].sAttribs.vColor.y*a, 2.0f )*1.0f;
		m_pStars[i].sAttribs.vColor.z = powf( m_pStars[i].sAttribs.vColor.z*a, 2.2f )*1.0f;
	}					

	m_iStarsBGCount = 1000;
	m_pStarsBG = new SVertexPC[m_iStarsBGCount];
	// random stars on a hemisphere:
	for ( uint32_t i = 0; i < m_iStarsBGCount; i++ )
	{
		float u = ((float)rand() / (float)RAND_MAX);
		float v = ((float)rand() / (float)RAND_MAX);

		float theta = 2.0f * PI * u;
		float phi = acosf(2.0f * v - 1.0f);

		m_pStarsBG[i].vPos.x = sinf( phi ) * cosf( theta );
		m_pStarsBG[i].vPos.y = sinf( phi ) * sinf( theta );
		m_pStarsBG[i].vPos.z = -abs( cosf( phi ) );

		float a = ((float)rand()/(float)RAND_MAX);
		a = powf( a, 2.0f );
		a = a * 0.8f + 0.2f;
		a = a*0.4f;
		m_pStarsBG[i].sAttribs.vColor = SVector4( ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, ((float)rand()/(float)RAND_MAX)*0.1f+0.9f, 1.0f );
		m_pStarsBG[i].sAttribs.vColor.x = powf( m_pStarsBG[i].sAttribs.vColor.x*a, 1.3f )*1.0f;
		m_pStarsBG[i].sAttribs.vColor.y = powf( m_pStarsBG[i].sAttribs.vColor.y*a, 1.8f )*1.0f;
		m_pStarsBG[i].sAttribs.vColor.z = powf( m_pStarsBG[i].sAttribs.vColor.z*a, 2.2f )*1.0f;
	}
}																		   

void CStarfield::Render( const SCamera& sCamera, const SViewPort& sViewport )
{
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

		const int iSteps = 3;
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
							sPixelShaderBasic.sColor = BGRA8( m_pStars[i].sAttribs.vColor.x, m_pStars[i].sAttribs.vColor.y, m_pStars[i].sAttribs.vColor.z, m_pStars[i].sAttribs.vColor.w / (fL * 0.2f + 1.0f) );
							CGraphics::GetInstance().RasterizeLineFlat( SVector2( sPh0.vPos.x, sPh0.vPos.y ), SVector2( sPh1.vPos.x, sPh1.vPos.y ), sPh0.sAttribs, sPixelShaderBasic, SBlendFuncAdditive() );
						}
						else
						{
							BGRA8 sColor = BGRA8( m_pStars[i].sAttribs.vColor.x , m_pStars[i].sAttribs.vColor.y, m_pStars[i].sAttribs.vColor.z, m_pStars[i].sAttribs.vColor.w );
							CGraphics::GetInstance().RasterizePixel( (int)sPh0.vPos.x, (int)sPh0.vPos.y, sColor, SBlendFuncAdditive() );
						}
					}
				}
			}
		}
	}

	{
		struct SVertexShaderStarsBG
		{
			using AttribsType = SVertexPC::SAttribs;
			SMatrix matWorldViewProjViewPort;
			inline void Execute( SClipVertex<AttribsType>& out, const SVertexPC& in ) const
			{
				SVector4 vPhSrc( in.vPos, 1.0f );
				SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProjViewPort );
				out.sAttribs.vColor = in.sAttribs.vColor;
			}
		} sVertexShaderStarsBG;	

		struct SPixelShaderStarsBG
		{
			inline BGRA8 Execute( const SVertexPC::SAttribs& in ) const
			{
				return BGRA8( in.vColor.x, in.vColor.y, in.vColor.z, in.vColor.w );
			}
		} sPixelShaderStarsBG;

		{
			SMatrix matWorld;
			SMatrix::BuildEulerXYZ( matWorld, 0.0f, 0.0f, 0.0f );
			SMatrix::Scale( matWorld, 1000.0f );
			SVector3 vPos = sCamera.m_vLookAt;
			vPos.z -= 400.0f;
			SMatrix::Translate( matWorld, vPos );
			SMatrix::Mul( sVertexShaderStarsBG.matWorldViewProjViewPort, matWorld, sCamera.m_matViewProj );
			SMatrix::Mul( sVertexShaderStarsBG.matWorldViewProjViewPort, sVertexShaderStarsBG.matWorldViewProjViewPort, sViewport.GetViewPortMatrix() );

			CGraphics::GetInstance().DrawPointList3D( m_pStarsBG, m_iStarsBGCount, sViewport, sVertexShaderStarsBG, sPixelShaderStarsBG, SBlendFuncAdditive() );
		}
	}
}
