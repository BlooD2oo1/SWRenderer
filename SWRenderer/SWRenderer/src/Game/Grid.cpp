#include "Grid.h"
#include "Common/Defines.h"

CGrid::CGrid()
{
	Clear();
}

CGrid::~CGrid()
{
	Clear();
}

void CGrid::Clear()
{
}

void CGrid::Create()
{
	Clear();
}

void CGrid::RenderToScene( float fSpacing, int iHalfGridSize, const SMatrix& matViewProj, const SViewPort& sViewport, const SVector3& vPos )
{
	SVector4 vColor = SVector4( 0.3f, 0.2f, 0.1f, 1.0f );
	struct SVertexShaderGrid
	{
		using AttribsType = SVertexPCW::SAttribs;
		SMatrix matWorldViewProj;
		void Execute( SClipVertex<AttribsType>& out, const SVertexPCW& in ) const
		{
			SVector4 vPhSrc( in.vPos, 1.0f );
			SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProj );
			out.sAttribs.vColor = in.sAttribs.vColor;
			out.sAttribs.fW = in.sAttribs.fW;
		}
	} sVertexShaderGrid;

	struct SPixelShaderGrid
	{
		BGRA8 Execute( const SVertexPCW::SAttribs& in ) const
		{
			return BGRA8( in.vColor.x, in.vColor.y, in.vColor.z, in.vColor.w * ( ( ( ((int)(in.fW*40.0f)) % 5 ) < 3 ) ? 0.5f : 0.1f ) );
			//return BGRA8( in.vColor.x, in.vColor.y, in.vColor.z, fabsf( (in.fW*5.0f-floorf(in.fW*5.0f)) * 2.0f - 1.0f ) * in.vColor.w );
		}
	};

	SMatrix matScale;
	SMatrix::Identity(matScale);
	SMatrix::Scale( matScale, fSpacing );

	{
		SVector3 vCenter = vPos / fSpacing;
		SVector3 vCenterQ;
		vCenterQ.x = vCenter.x;
		vCenterQ.y = floorf( vCenter.y );
		vCenterQ.z = floorf( vCenter.z );

		SMatrix matWorld( matScale );
		matWorld.m30 = vCenterQ.x*fSpacing;
		matWorld.m31 = vCenterQ.y*fSpacing;
		matWorld.m32 = vCenterQ.z*fSpacing;
		SMatrix::Mul( matWorld, matWorld, matViewProj );
		SMatrix::Mul( matWorld, matWorld, sViewport.GetViewPortMatrix() );
		sVertexShaderGrid.matWorldViewProj = matWorld;

		float fi = vCenter.y - floorf(vCenter.y);
		float fj = vCenter.z - floorf(vCenter.z);
		float fk = vCenter.x - floorf(vCenter.x);

		for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
		{
			//for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
			int j = 0;
			{
				SVector3 vOffset( 0.0f, (float)i, (float)j );

				SVertexPCW pVertices[3];

				pVertices[0].vPos = SVector3( vOffset );
				pVertices[0].sAttribs.vColor = vColor;
				pVertices[0].sAttribs.fW = fk;
				pVertices[1].vPos = pVertices[0].vPos;
				pVertices[1].sAttribs.vColor = vColor;
				pVertices[1].sAttribs.fW = (float)iHalfGridSize + fk;
				pVertices[2].vPos = pVertices[0].vPos;
				pVertices[2].sAttribs.vColor = vColor;
				pVertices[2].sAttribs.fW = (float)(iHalfGridSize*2) + fk;

				pVertices[0].vPos.x -= (float)iHalfGridSize;
				pVertices[2].vPos.x += (float)iHalfGridSize;

				float di = (float)i - fi;
				float dj = (float)j - fj;
				float d = di*di+dj*dj;
				float t = d / (iHalfGridSize*iHalfGridSize);
				t = Clamp( t, 0.0f, 1.0f );

				float fAlpha = 1.0f-t;

				pVertices[0].sAttribs.vColor.w *= 0.0f;
				pVertices[1].sAttribs.vColor.w *= fAlpha;
				pVertices[2].sAttribs.vColor.w *= 0.0f;

				uint32_t pIndices[4] = { 0, 1, 1, 2 };

				CGraphics::GetInstance().DrawLineList3D( pVertices, 3, pIndices, 2, sViewport, sVertexShaderGrid, SPixelShaderGrid(), SBlendFuncAdditive() );
			}
		}
	}

	{
		SVector3 vCenter = vPos / fSpacing;
		SVector3 vCenterQ;
		vCenterQ.x = floorf( vCenter.x );
		vCenterQ.y = vCenter.y;
		vCenterQ.z = floorf( vCenter.z );

		SMatrix matWorld( matScale );
		matWorld.m30 = vCenterQ.x*fSpacing;
		matWorld.m31 = vCenterQ.y*fSpacing;
		matWorld.m32 = vCenterQ.z*fSpacing;
		SMatrix::Mul( matWorld, matWorld, matViewProj );
		SMatrix::Mul( matWorld, matWorld, sViewport.GetViewPortMatrix() );
		sVertexShaderGrid.matWorldViewProj = matWorld;

		float fi = vCenter.x - floorf(vCenter.x);
		float fj = vCenter.z - floorf(vCenter.z);
		float fk = vCenter.y - floorf(vCenter.y);

		for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
		{
			//for ( int j = -iHalfGridSize; j <= iHalfGridSize; j++ )
			int j = 0;
			{
				SVector3 vOffset( (float)i, 0.0f, (float)j );

				SVertexPCW pVertices[3];

				pVertices[0].vPos = SVector3( vOffset );
				pVertices[0].sAttribs.vColor = vColor;
				pVertices[0].sAttribs.fW = fk;

				pVertices[1].vPos = pVertices[0].vPos;
				pVertices[1].sAttribs.vColor = vColor;
				pVertices[1].sAttribs.fW = (float)iHalfGridSize + fk;
				pVertices[2].vPos = pVertices[0].vPos;
				pVertices[2].sAttribs.vColor = vColor;
				pVertices[2].sAttribs.fW = (float)(iHalfGridSize*2) + fk;

				pVertices[0].vPos.y -= (float)iHalfGridSize;
				pVertices[2].vPos.y += (float)iHalfGridSize;

				float di = (float)i - fi;
				float dj = (float)j - fj;
				float d = di*di+dj*dj;
				float t = d / (iHalfGridSize*iHalfGridSize);
				t = Clamp( t, 0.0f, 1.0f );

				float fAlpha = 1.0f-t;

				pVertices[0].sAttribs.vColor.w *= 0.0f;
				pVertices[1].sAttribs.vColor.w *= fAlpha;
				pVertices[2].sAttribs.vColor.w *= 0.0f;

				uint32_t pIndices[4] = { 0, 1, 1, 2 };

				CGraphics::GetInstance().DrawLineList3D( pVertices, 3, pIndices, 2, sViewport, sVertexShaderGrid, SPixelShaderGrid(), SBlendFuncAdditive() );
			}
		}
	}
}

void CGrid::RenderToMiniMap( float fSpacing, int iHalfGridSize, const SMatrix& matViewProj, const SViewPort& sViewport, const SVector3& vPos )
{
	SVector4 vColor = SVector4( 0.3f, 0.2f, 0.1f, 0.5f );

	struct SVertexShaderGrid
	{
		using AttribsType = SVertexP::SAttribs;
		SMatrix matWorldViewProj;
		void Execute( SClipVertex<AttribsType>& out, const SVertexP& in ) const
		{
			SVector4 vPhSrc( in.vPos, 1.0f );
			SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProj );
		}
	} sVertexShaderGrid;

	struct SPixelShaderGrid
	{
		BGRA8 sColor;
		BGRA8 Execute( const SVertexP::SAttribs& in ) const
		{
			return sColor;
		}
	} sPixelShaderGrid;
	sPixelShaderGrid.sColor = BGRA8( vColor.x, vColor.y, vColor.z, vColor.w );

	SVector3 vCenterGrid( floorf(vPos.x / fSpacing), floorf(vPos.y / fSpacing), floorf(vPos.z / fSpacing) );

	SMatrix matScale;
	SMatrix::Identity(matScale);
	SMatrix::Scale( matScale, fSpacing );

	SMatrix matWorld( matScale );
	matWorld.m30 = vCenterGrid.x * fSpacing;
	matWorld.m31 = vCenterGrid.y * fSpacing;
	matWorld.m32 = vCenterGrid.z * fSpacing;

	SMatrix::Mul( sVertexShaderGrid.matWorldViewProj, matWorld, matViewProj );

	for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
	{
		SVertexP sVertex0, sVertex2;
		sVertex0.vPos = SVector3( -(float)iHalfGridSize, (float)i, 0.0f );
		sVertex2.vPos = SVector3(  (float)iHalfGridSize, (float)i, 0.0f );

		CGraphics::GetInstance().DrawLine3D( sVertex0, sVertex2, sViewport, sVertexShaderGrid, sPixelShaderGrid, SBlendFuncAdditive() );
	}

	for ( int i = -iHalfGridSize; i <= iHalfGridSize; i++ )
	{
		SVertexP sVertex0, sVertex2;
		sVertex0.vPos = SVector3( (float)i, -(float)iHalfGridSize, 0.0f );
		sVertex2.vPos = SVector3( (float)i,  (float)iHalfGridSize, 0.0f );

		CGraphics::GetInstance().DrawLine3D( sVertex0, sVertex2, sViewport, sVertexShaderGrid, sPixelShaderGrid, SBlendFuncAdditive() );
	}
}

void CGrid::RenderCoordSys( const SMatrix& matViewProj, const SViewPort& sViewport, const SVector3& vPos, float fSize )
{
	struct SVertexShaderBasic
	{
		using AttribsType = SVertexPC::SAttribs;
		SMatrix matWorldViewProjViewPort;
		void Execute( SClipVertex<AttribsType>& out, const SVertexPC& in ) const
		{
			SVector4 vPhSrc( in.vPos, 1.0f );
			SMatrix::Mul( out.vPos, vPhSrc, matWorldViewProjViewPort );
			out.sAttribs.vColor = in.sAttribs.vColor;
		}
	} sVertexShaderBasic;
	SMatrix matViewProjViewPort;
	SMatrix::Identity( matViewProjViewPort );
	matViewProjViewPort.m30 = vPos.x;
	matViewProjViewPort.m31 = vPos.y;
	matViewProjViewPort.m32 = vPos.z;
	SMatrix::Mul( matViewProjViewPort, matViewProjViewPort, matViewProj );
	SMatrix::Mul( matViewProjViewPort, matViewProjViewPort, sViewport.GetViewPortMatrix() );

	struct SPixelShaderBasic
	{
		BGRA8 Execute( const SVertexPC::SAttribs& in ) const
		{
			return BGRA8( in.vColor.x, in.vColor.y, in.vColor.z, in.vColor.w );
		}
	};

	sVertexShaderBasic.matWorldViewProjViewPort = matViewProjViewPort;                                                            
	SVertexPC sLine[2];

	sLine[0].vPos = SVector3( 0.0f, 0.0f, 0.0f );
	sLine[1].vPos = SVector3( fSize, 0.0f, 0.0f );
	sLine[0].sAttribs.vColor = SVector4( 0.0f, 0.0f, 1.0f, 1.0f );
	sLine[1].sAttribs.vColor = SVector4( 0.0f, 0.0f, 1.0f, 0.5f );
	CGraphics::GetInstance().DrawLine3D( sLine[0], sLine[1], sViewport, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );

	sLine[0].vPos = SVector3( 0.0f, 0.0f, 0.0f );
	sLine[1].vPos = SVector3( 0.0f, fSize, 0.0f );
	sLine[0].sAttribs.vColor = SVector4( 0.0f, 1.0f, 0.0f, 1.0f );
	sLine[1].sAttribs.vColor = SVector4( 0.0f, 1.0f, 0.0f, 0.5f );
	CGraphics::GetInstance().DrawLine3D( sLine[0], sLine[1], sViewport, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );

	sLine[0].vPos = SVector3( 0.0f, 0.0f, 0.0f );
	sLine[1].vPos = SVector3( 0.0f, 0.0f, fSize );
	sLine[0].sAttribs.vColor = SVector4( 1.0f, 0.0f, 0.0f, 1.0f );
	sLine[1].sAttribs.vColor = SVector4( 1.0f, 0.0f, 0.0f, 0.5f );
	CGraphics::GetInstance().DrawLine3D( sLine[0], sLine[1], sViewport, sVertexShaderBasic, SPixelShaderBasic(), SBlendFuncAdditive() );
}