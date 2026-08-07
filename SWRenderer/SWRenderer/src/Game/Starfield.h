#pragma once

#include "Engine/Graphics.h"
#include "Game/Camera.h"

class CStarfield
{
public:

	CStarfield();
	~CStarfield();

	void Clear();
	void Create();

	void Render( const SCamera& sCamera, const SViewPort& sViewport );

	SVertexPC*		m_pStars;
	uint32_t		m_iStarsCount;
};