#pragma once

#include "Common/Defines.h"
#include "Engine/Graphics.h"

struct SMesh
{
public:
	SMesh()
	{
		m_pVertices = nullptr;
		m_iVertexCount = 0;
		m_pIndices = nullptr;
		m_iIndexCount = 0;
	}
	~SMesh()
	{
		SAFE_DELETE_ARRAY( m_pVertices );
		SAFE_DELETE_ARRAY( m_pIndices );
	}

	void Clear()
	{
		SAFE_DELETE_ARRAY( m_pVertices );
		m_iVertexCount = 0;
		SAFE_DELETE_ARRAY( m_pIndices );
		m_iIndexCount = 0;
	}

	SVertexPC*		m_pVertices;
	uint32_t		m_iVertexCount;
	uint32_t*		m_pIndices;
	uint32_t		m_iIndexCount;
};