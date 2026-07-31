#include "AsteroidMesh.h"

#include <vector>

CAsteroidMesh::CAsteroidMesh()
{
	m_pLineList = nullptr;
	m_iLineListCount = 0;
}

CAsteroidMesh::~CAsteroidMesh()
{
	Clear();
}

void CAsteroidMesh::Clear()
{
	SAFE_DELETE_ARRAY( m_pLineList );
	m_iLineListCount = 0;
}

void CAsteroidMesh::Create()
{
	{
		std::vector<SVector3> pts;
		std::vector<std::pair<int,int>> edges;

		const float phi = (1.0f + sqrtf(5.0f)) * 0.5f; // Golden ratio
		const float radius = 2.0f;

		auto AddVert = [&](float x, float y, float z)
			{
				float len = sqrtf(x * x + y * y + z * z);
				pts.emplace_back((x / len) * radius, (y / len) * radius, (z / len) * radius);
			};

		// =====================================================
		// GENERATE 60 VERTICES OF A TRUNCATED ICOSAHEDRON (HEX SPHERE)
		// =====================================================

		float signs[2] = { -1.0f, 1.0f };

		// Set 1: Cyclic permutations of (0, +-1, +-3fi)
		for (float a : signs)
		{
			for (float b : signs)
			{
				AddVert(0.0f, a * 1.0f, b * 3.0f * phi);
				AddVert(b * 3.0f * phi, 0.0f, a * 1.0f);
				AddVert(a * 1.0f, b * 3.0f * phi, 0.0f);
			}
		}

		// Set 2: Cyclic permutations of (+-1, +-(2+fi), +-2fi)
		for (float a : signs)
		{
			for (float b : signs)
			{
				for (float c : signs)
				{
					float x = a * 1.0f;
					float y = b * (2.0f + phi);
					float z = c * (2.0f * phi);

					AddVert(x, y, z);
					AddVert(z, x, y);
					AddVert(y, z, x);
				}
			}
		}

		// Set 3: Cyclic permutations of (+-2, +-(1+2fi), +-fi)
		for (float a : signs)
		{
			for (float b : signs)
			{
				for (float c : signs)
				{
					float x = a * 2.0f;
					float y = b * (1.0f + 2.0f * phi);
					float z = c * phi;

					AddVert(x, y, z);
					AddVert(z, x, y);
					AddVert(y, z, x);
				}
			}
		}

		// =====================================================
		// CONNECT 3 NEAREST NEIGHBORS FOR EACH VERTEX (HEX GRID)
		// =====================================================
		const int vertCount = (int)pts.size();

		for (int i = 0; i < vertCount; ++i)
		{
			std::vector<std::pair<float, int>> neighbors;

			for (int j = 0; j < vertCount; ++j)
			{
				if (i == j) continue;

				float dx = pts[i].x - pts[j].x;
				float dy = pts[i].y - pts[j].y;
				float dz = pts[i].z - pts[j].z;
				float distSq = dx * dx + dy * dy + dz * dz;

				neighbors.emplace_back(distSq, j);
			}

			// Sort to find closest vertices
			std::sort(neighbors.begin(), neighbors.end());

			// Connect to 3 closest adjacent vertices
			for (int k = 0; k < 3; ++k)
			{
				int neighborIndex = neighbors[k].second;
				if (i < neighborIndex)
				{
					edges.emplace_back(i, neighborIndex);
				}
			}
		}

		// =====================================================
		// ALLOCATE LINE LIST
		// =====================================================
		m_iLineListCount = (int)edges.size();

		SAFE_DELETE_ARRAY( m_pLineList );

		m_pLineList = new SVertexPC[m_iLineListCount * 2];

		for (uint32_t i = 0; i < (uint32_t)m_iLineListCount; ++i)
		{
			m_pLineList[i * 2 + 0].vPos = pts[edges[i].first];
			m_pLineList[i * 2 + 1].vPos = pts[edges[i].second];

			// Line colors
			m_pLineList[i * 2 + 0].vColor = SVector4( 0.4f, 0.7f, 1.0f, 1.0f );
			m_pLineList[i * 2 + 1].vColor = SVector4( 0.1f, 0.3f, 0.6f, 0.6f );
		}
	}
}