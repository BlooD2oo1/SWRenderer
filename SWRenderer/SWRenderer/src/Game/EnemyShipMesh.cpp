#include "EnemyShipMesh.h"

#include <vector>

CEnemyShipMesh::CEnemyShipMesh()
{
	m_pLineList = nullptr;
}

CEnemyShipMesh::~CEnemyShipMesh()
{
	Clear();
}

void CEnemyShipMesh::Clear()
{
	SAFE_DELETE_ARRAY( m_pLineList );
	m_iLineListCount = 0;
}

void CEnemyShipMesh::Create()
{
	std::vector<SVector3> pts;
	std::vector<std::pair<int, int>> edges;

	auto AddPoint =
		[&](float x, float y, float z) -> int
		{
			// Retaining coordinate transformation (y, -x, z)
			pts.emplace_back(y, -x, z);
			return (int)pts.size() - 1;
		};

	auto AddEdge =
		[&](int a, int b)
		{
			edges.emplace_back(a, b);
		};

	// =====================================================
	// FUSELAGE (Short nose, wide and low-profile rear)
	// =====================================================

	// Shortened nose tip
	const int nose = AddPoint(0.0f, 1.5f, 0.0f);

	// Mid-fuselage (Rhombus ring)
	const int mLeft  = AddPoint(-0.45f, 0.4f,  0.0f);
	const int mRight = AddPoint( 0.45f, 0.4f,  0.0f);
	const int mTop   = AddPoint( 0.0f,  0.4f,  0.25f);
	const int mBot   = AddPoint( 0.0f,  0.4f, -0.15f);

	// Cockpit ridge
	const int crest  = AddPoint(0.0f, 1.2f,  0.25f);

	// Rear fuselage (Wide horizontal stance, low vertical profile for sleekness)
	const int rLeft  = AddPoint(-0.75f, -1.2f,  0.0f);
	const int rRight = AddPoint( 0.75f, -1.2f,  0.0f);
	const int rTop   = AddPoint( 0.0f,  -1.2f,  0.20f);
	const int rBot   = AddPoint( 0.0f,  -1.2f, -0.12f);

	// Nose connections
	AddEdge(nose, mLeft);
	AddEdge(nose, mRight);
	AddEdge(nose, mBot);
	AddEdge(nose, crest);

	// Ridge / Cockpit
	AddEdge(crest, mTop);

	// Mid ring
	AddEdge(mLeft, mTop);
	AddEdge(mTop, mRight);
	AddEdge(mRight, mBot);
	AddEdge(mBot, mLeft);

	// Longitudinal ribs (Mid -> Rear)
	AddEdge(mLeft, rLeft);
	AddEdge(mRight, rRight);
	AddEdge(mTop, rTop);
	AddEdge(mBot, rBot);

	// Rear ring
	AddEdge(rLeft, rTop);
	AddEdge(rTop, rRight);
	AddEdge(rRight, rBot);
	AddEdge(rBot, rLeft);

	// =====================================================
	// CENTRAL LASER CANNON
	// =====================================================

	const int gunBase = AddPoint(0.0f, 1.5f, -0.08f);
	const int gunTip  = AddPoint(0.0f, 1.9f, -0.08f); // Protrudes past short nose

	AddEdge(gunBase, gunTip);
	AddEdge(mBot, gunBase);
	AddEdge(nose, gunBase);

	// =====================================================
	// FORWARD-SWEPT WINGS
	// =====================================================

	auto BuildWing =
		[&](float side)
		{
			float sx = side;

			// Wing root (attached to wide rear fuselage)
			int rootFront = AddPoint(0.45f * sx,  0.4f, 0.0f);
			int rootRear  = AddPoint(0.75f * sx, -1.0f, 0.0f);

			// Wing tip (swept forward)
			int tipFront  = AddPoint(2.1f * sx,  1.1f, -0.05f);
			int tipRear   = AddPoint(1.5f * sx, -0.2f,  0.0f);

			// Wing outline
			AddEdge(rootFront, tipFront);
			AddEdge(tipFront, tipRear);
			AddEdge(tipRear, rootRear);
			AddEdge(rootFront, rootRear);

			// Internal ribs for wireframe definition
			AddEdge(rootFront, tipRear);
			AddEdge(tipFront, rootRear);
		};

	// Build left and right wings
	BuildWing(-1.0f);
	BuildWing( 1.0f);

	// =====================================================
	// SLEEK INTEGRATED THRUSTERS (Lighter, flat-profile nozzles)
	// =====================================================

	auto BuildThruster =
		[&](float sideX)
		{
			const float sx = sideX;
			const float startY = -1.2f;
			const float endY   = -1.8f;
			const float halfW  = 0.18f;
			const float halfH  = 0.08f;

			// Rectangular flat exhaust nozzle
			int n0 = AddPoint(sx - halfW, startY,  halfH);
			int n1 = AddPoint(sx + halfW, startY,  halfH);
			int n2 = AddPoint(sx + halfW, startY, -halfH);
			int n3 = AddPoint(sx - halfW, startY, -halfH);

			int e0 = AddPoint(sx - halfW, endY,  halfH);
			int e1 = AddPoint(sx + halfW, endY,  halfH);
			int e2 = AddPoint(sx + halfW, endY, -halfH);
			int e3 = AddPoint(sx - halfW, endY, -halfH);

			// Connect nozzle frame to exit rim
			AddEdge(n0, e0); AddEdge(n1, e1);
			AddEdge(n2, e2); AddEdge(n3, e3);

			// Rear exit rim
			AddEdge(e0, e1); AddEdge(e1, e2);
			AddEdge(e2, e3); AddEdge(e3, e0);
		};

	// Twin sleek thrusters mounted symmetrically on the wide rear
	BuildThruster(-0.35f);
	BuildThruster( 0.35f);

	// =====================================================
	// Allocation & Color Setup (Red / Orange Enemy Visuals)
	// =====================================================

	m_iLineListCount = (int)edges.size();

	delete[] m_pLineList;
	m_pLineList = new SVertexPC[m_iLineListCount * 2];

	for(uint32_t i = 0; i < m_iLineListCount; ++i)
	{
		m_pLineList[i * 2 + 0].vPos = pts[edges[i].first];
		m_pLineList[i * 2 + 1].vPos = pts[edges[i].second];

		// Dark red base color fading to semi-transparent orange edge endpoints
		m_pLineList[i * 2 + 0].vColor = SVector4(0.1f, 0.1f, 0.9f, 1.0f);
		m_pLineList[i * 2 + 1].vColor = SVector4(0.0f, 0.4f, 1.0f, 0.5f);
	}
}