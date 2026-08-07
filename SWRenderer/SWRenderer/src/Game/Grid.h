#pragma once

#include "Engine/Graphics.h"
#include "Game/Camera.h"

class CGrid
{
public:

	CGrid();
	~CGrid();

	void Clear();
	void Create();

	void RenderToScene( float fSpacing, int iHalfGridSize, const SMatrix& matViewProj, const SViewPort& sViewport, const SVector3& vPos );
	void RenderToMiniMap( float fSpacing, int iHalfGridSize, const SMatrix& matViewProj, const SViewPort& sViewport, const SVector3& vPos );

	void RenderCoordSys( const SMatrix& matViewProj, const SViewPort& sViewport, const SVector3& vPos, float fSize );
};