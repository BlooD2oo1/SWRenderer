#pragma once

#include "Common/Vector.h"
#include "Engine/Graphics.h"

class CAsteroidMesh
{
public:
	CAsteroidMesh();
	~CAsteroidMesh();

	void Create();
	void Clear();

	SVertexPC*		GetLineList() const { return m_pLineList; }
	uint32_t		GetLineListCount() const { return m_iLineListCount; }

private:
	SVertexPC*		m_pLineList;
	uint32_t		m_iLineListCount;
};