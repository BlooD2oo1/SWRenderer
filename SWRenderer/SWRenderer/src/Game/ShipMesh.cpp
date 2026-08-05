#include "ShipMesh.h"

#include <vector>

CShipMesh::CShipMesh()
{
	m_pLineList = nullptr;
}

CShipMesh::~CShipMesh()
{
	Clear();
}

void CShipMesh::Clear()
{
	SAFE_DELETE_ARRAY( m_pLineList );
	m_iLineListCount = 0;
}

void CShipMesh::Create()
{
	{
		std::vector<SVector3> pts;
		std::vector<std::pair<int,int>> edges;

		auto AddPoint =
			[&](float x, float y, float z) -> int
			{
				// X = elore/hatra, Y = bal/jobb, Z = fel/le
				pts.emplace_back(x, y, z);
				return (int)pts.size() - 1;
			};

		auto AddEdge =
			[&](int a, int b)
			{
				edges.emplace_back(a, b);
			};

		// =====================================================
		// FUSELAGE
		// =====================================================

		const int nose =
			AddPoint(4.8f, 0.0f, 0.0f);

		const int n0 = AddPoint(3.6f, -0.15f, -0.12f);
		const int n1 = AddPoint(3.6f,  0.15f, -0.12f);
		const int n2 = AddPoint(3.6f,  0.15f,  0.12f);
		const int n3 = AddPoint(3.6f, -0.15f,  0.12f);

		const int m0 = AddPoint(1.4f, -0.42f, -0.22f);
		const int m1 = AddPoint(1.4f,  0.42f, -0.22f);
		const int m2 = AddPoint(1.4f,  0.42f,  0.22f);
		const int m3 = AddPoint(1.4f, -0.42f,  0.22f);

		const int r0 = AddPoint(-1.3f, -0.35f, -0.20f);
		const int r1 = AddPoint(-1.3f,  0.35f, -0.20f);
		const int r2 = AddPoint(-1.3f,  0.35f,  0.20f);
		const int r3 = AddPoint(-1.3f, -0.35f,  0.20f);

		// nose connections
		AddEdge(nose,n0);
		AddEdge(nose,n1);
		AddEdge(nose,n2);
		AddEdge(nose,n3);

		// front ring
		AddEdge(n0,n1);
		AddEdge(n1,n2);
		AddEdge(n2,n3);
		AddEdge(n3,n0);

		// mid
		AddEdge(n0,m0);
		AddEdge(n1,m1);
		AddEdge(n2,m2);
		AddEdge(n3,m3);

		AddEdge(m0,m1);
		AddEdge(m1,m2);
		AddEdge(m2,m3);
		AddEdge(m3,m0);

		// rear
		AddEdge(m0,r0);
		AddEdge(m1,r1);
		AddEdge(m2,r2);
		AddEdge(m3,r3);

		AddEdge(r0,r1);
		AddEdge(r1,r2);
		AddEdge(r2,r3);
		AddEdge(r3,r0);

		// diagonals for detail
		AddEdge(m0,m2);
		AddEdge(m1,m3);
		AddEdge(r0,r2);
		AddEdge(r1,r3);

		// =====================================================
		// COCKPIT
		// =====================================================

		int c0 = AddPoint(2.6f, 0.0f, 0.45f);
		int c1 = AddPoint(1.8f, 0.0f, 0.58f);
		int c2 = AddPoint(1.0f, 0.0f, 0.42f);

		AddEdge(nose,c0);
		AddEdge(c0,c1);
		AddEdge(c1,c2);
		AddEdge(c2,m2);
		AddEdge(c2,m3);

		// =====================================================
		// WINGS
		// =====================================================

		auto BuildWing =
			[&](float side, float zOffset)
			{
				float sy = side;

				int rootA = AddPoint(
					0.8f, 0.55f * sy, zOffset);

				int rootB = AddPoint(
					-0.8f, 0.55f * sy, zOffset);

				int tipFront = AddPoint(
					0.9f, 2.8f * sy, zOffset);

				int tipRear = AddPoint(
					-1.2f, 2.2f * sy, zOffset);

				int midOuter = AddPoint(
					-0.2f, 2.0f * sy, zOffset);

				// outer frame
				AddEdge(rootA,tipFront);
				AddEdge(tipFront,midOuter);
				AddEdge(midOuter,tipRear);
				AddEdge(tipRear,rootB);
				AddEdge(rootA,rootB);

				// internal ribs
				AddEdge(rootA,midOuter);
				AddEdge(rootB,midOuter);
				AddEdge(rootA,tipRear);
				AddEdge(rootB,tipFront);

				// attach to body
				AddEdge(m0,rootA);
				AddEdge(r0,rootB);

				AddEdge(m1,rootA);
				AddEdge(r1,rootB);

				// laser cannon
				int gun0 = AddPoint(
					1.0f,
					2.8f * sy,
					zOffset);

				int gun1 = AddPoint(
					2.0f,
					2.8f * sy,
					zOffset);

				AddEdge(tipFront, gun0);
				AddEdge(gun0, gun1);

				// engine pod
				const float ex = -0.7f;
				const float ey = 1.35f * sy;
				const float ez = zOffset;

				const float radius = 0.16f;

				int prev = -1;
				int first = -1;

				for(int i=0;i<8;i++)
				{
					float a =
						float(i) / 8.0f *
						6.2831853f;

					// YZ kor, X iranyba nez
					int p = AddPoint(
						ex,
						ey + cosf(a) * radius,
						ez + sinf(a) * radius);

					if(i == 0)
						first = p;

					if(prev != -1)
						AddEdge(prev, p);

					prev = p;
				}

				AddEdge(prev,first);

				// engine mount
				AddEdge(rootB,first);
				AddEdge(midOuter,first);
			};

		// top wings
		BuildWing(-1.6f, 0.12f);
		BuildWing( 1.6f, 0.12f);

		// bottom wings
		BuildWing(-1.0f,-0.62f);
		BuildWing( 1.0f,-0.62f);

		// =====================================================
		// Allocate
		// =====================================================

		m_iLineListCount =
			(int)edges.size();

		delete[] m_pLineList;

		m_pLineList = new SVertexPC[m_iLineListCount * 2];

		for(uint32_t i=0; i<m_iLineListCount; ++i)
		{
			m_pLineList[i*2+0]
				.vPos =
				pts[edges[i].first];

			m_pLineList[i*2+1]
				.vPos =
				pts[edges[i].second];

			m_pLineList[i * 2 + 0].sAttribs.vColor = SVector4( 0.4f, 0.4f, 0.4f, 1.0f );
			m_pLineList[i * 2 + 1].sAttribs.vColor = SVector4( 0.0f, 0.3f, 1.0f, 0.4f );
		}
	}
}