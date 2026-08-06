#include "ShipMeshDestroyer.h"

#include <vector>

CShipMeshDestroyer::CShipMeshDestroyer()
{
	m_pLineList = nullptr;
}

CShipMeshDestroyer::~CShipMeshDestroyer()
{
	Clear();
}

void CShipMeshDestroyer::Clear()
{
	SAFE_DELETE_ARRAY( m_pLineList );
	m_iLineListCount = 0;
}

void CShipMeshDestroyer::Create()
{
	std::vector<SVector3> pts;
	std::vector<std::pair<int, int>> edges;

	auto AddPoint =
		[&](float x, float y, float z) -> int
		{
			// Direct assignment matching coordinate system (+X = Front, Y = Side, +Z = Up)
			pts.emplace_back(x, y, z);
			return (int)pts.size() - 1;
		};

	auto AddEdge =
		[&](int a, int b)
		{
			edges.emplace_back(a, b);
		};

	// =====================================================
	// TAPERED ARMORED PROW / NOSE (Sleeker wedge nose at +X)
	// =====================================================

	// Narrow frontmost chisel tip (X = +5.0f, narrowed from 1.2f down to 0.4f)
	const int fn0 = AddPoint(5.0f, -0.4f,  0.20f);
	const int fn1 = AddPoint(5.0f,  0.4f,  0.20f);
	const int fn2 = AddPoint(5.0f,  0.4f, -0.20f);
	const int fn3 = AddPoint(5.0f, -0.4f, -0.20f);

	// Flared armored shoulders (X = +3.2f, controlled width = 1.1f)
	const int hn0 = AddPoint(3.2f, -1.1f,  0.35f);
	const int hn1 = AddPoint(3.2f,  1.1f,  0.35f);
	const int hn2 = AddPoint(3.2f,  1.1f, -0.25f);
	const int hn3 = AddPoint(3.2f, -1.1f, -0.25f);

	// Front chisel tip frame
	AddEdge(fn0, fn1); AddEdge(fn1, fn2); AddEdge(fn2, fn3); AddEdge(fn3, fn0);

	// Tapered wedge connections
	AddEdge(fn0, hn0); AddEdge(fn1, hn1); AddEdge(fn2, hn2); AddEdge(fn3, hn3);
	AddEdge(hn0, hn1); AddEdge(hn1, hn2); AddEdge(hn2, hn3); AddEdge(hn3, hn0);

	// Front armor reinforcement cross-bracing
	AddEdge(fn0, fn2); AddEdge(fn1, fn3);

	// Central nose prow line for extra visual detail
	const int noseProw = AddPoint(5.4f, 0.0f, 0.0f);
	AddEdge(noseProw, fn0); AddEdge(noseProw, fn1);
	AddEdge(noseProw, fn2); AddEdge(noseProw, fn3);

	// =====================================================
	// MAIN FUSELAGE & BOMB BAY (Central body)
	// =====================================================

	// Mid-body section (X = +0.5f)
	const int mb0 = AddPoint(0.5f, -0.95f,  0.45f);
	const int mb1 = AddPoint(0.5f,  0.95f,  0.45f);
	const int mb2 = AddPoint(0.5f,  0.95f, -0.35f);
	const int mb3 = AddPoint(0.5f, -0.95f, -0.35f);

	// Rear body section (X = -2.2f)
	const int rb0 = AddPoint(-2.2f, -1.10f,  0.40f);
	const int rb1 = AddPoint(-2.2f,  1.10f,  0.40f);
	const int rb2 = AddPoint(-2.2f,  1.10f, -0.30f);
	const int rb3 = AddPoint(-2.2f, -1.10f, -0.30f);

	// Connect shoulders to mid-body
	AddEdge(hn0, mb0); AddEdge(hn1, mb1); AddEdge(hn2, mb2); AddEdge(hn3, mb3);

	// Connect mid-body to rear body
	AddEdge(mb0, rb0); AddEdge(mb1, rb1); AddEdge(mb2, rb2); AddEdge(mb3, rb3);

	// Body rings
	AddEdge(mb0, mb1); AddEdge(mb1, mb2); AddEdge(mb2, mb3); AddEdge(mb3, mb0);
	AddEdge(rb0, rb1); AddEdge(rb1, rb2); AddEdge(rb2, rb3); AddEdge(rb3, rb0);

	// Spine ridge
	const int topRidge0 = AddPoint( 2.5f, 0.0f, 0.60f);
	const int topRidge1 = AddPoint(-1.0f, 0.0f, 0.55f);
	AddEdge(topRidge0, topRidge1);
	AddEdge(topRidge0, hn0); AddEdge(topRidge0, hn1);
	AddEdge(topRidge1, rb0); AddEdge(topRidge1, rb1);

	// =====================================================
	// DUAL HEAVY PLASMA / TORPEDO CANNONS
	// =====================================================

	auto BuildHeavyCannon =
		[&](float sideY)
		{
			const float sy = sideY;
			int baseR = AddPoint(2.8f, 1.0f * sy, -0.1f);
			int tipR  = AddPoint(5.2f, 1.0f * sy, -0.1f);

			int barrelTop = AddPoint(4.0f, 1.0f * sy,  0.1f);
			int barrelBot = AddPoint(4.0f, 1.0f * sy, -0.3f);

			AddEdge(baseR, tipR);
			AddEdge(baseR, barrelTop); AddEdge(barrelTop, tipR);
			AddEdge(baseR, barrelBot); AddEdge(barrelBot, tipR);
			AddEdge(hn2, baseR); AddEdge(hn1, baseR);
		};

	BuildHeavyCannon(-1.0f);
	BuildHeavyCannon( 1.0f);

	// =====================================================
	// BROAD BOMBER WINGS & ORDNANCE PODS
	// =====================================================

	auto BuildHeavyWing =
		[&](float side)
		{
			float sy = side;

			// Wing root
			int wRootFront = AddPoint( 1.2f, 0.95f * sy,  0.1f);
			int wRootRear  = AddPoint(-2.0f, 1.10f * sy,  0.1f);

			// Outer wing structure
			int wMidOuter  = AddPoint( 0.2f, 3.6f * sy,  0.0f);
			int wTipFront  = AddPoint(-0.8f, 4.0f * sy, -0.05f);
			int wTipRear   = AddPoint(-2.8f, 3.0f * sy,  0.0f);

			// Outer frame
			AddEdge(wRootFront, wMidOuter);
			AddEdge(wMidOuter, wTipFront);
			AddEdge(wTipFront, wTipRear);
			AddEdge(wTipRear, wRootRear);
			AddEdge(wRootFront, wRootRear);

			// Structural truss reinforcement
			AddEdge(wRootFront, wTipFront);
			AddEdge(wRootRear, wMidOuter);
			AddEdge(wRootRear, wTipFront);

			// Ordnance pod underneath
			int podFront = AddPoint( 0.8f, 2.3f * sy, -0.30f);
			int podRear  = AddPoint(-1.8f, 2.3f * sy, -0.30f);
			AddEdge(wRootFront, podFront);
			AddEdge(wMidOuter, podFront);
			AddEdge(podFront, podRear);
			AddEdge(wRootRear, podRear);
			AddEdge(wTipRear, podRear);
		};

	BuildHeavyWing(-1.0f);
	BuildHeavyWing( 1.0f);

	// =====================================================
	// QUAD HEAVY THRUSTERS (Mounted at rear -X)
	// =====================================================

	auto BuildThrusterNozzle =
		[&](float offsetY, float offsetZ)
		{
			const float startX = -2.2f;
			const float endX   = -3.4f;
			const float size   = 0.26f;

			int n0 = AddPoint(startX, offsetY - size, offsetZ + size);
			int n1 = AddPoint(startX, offsetY + size, offsetZ + size);
			int n2 = AddPoint(startX, offsetY + size, offsetZ - size);
			int n3 = AddPoint(startX, offsetY - size, offsetZ - size);

			int e0 = AddPoint(endX, offsetY - size * 1.2f, offsetZ + size * 1.2f);
			int e1 = AddPoint(endX, offsetY + size * 1.2f, offsetZ + size * 1.2f);
			int e2 = AddPoint(endX, offsetY + size * 1.2f, offsetZ - size * 1.2f);
			int e3 = AddPoint(endX, offsetY - size * 1.2f, offsetZ - size * 1.2f);

			// Frame lines
			AddEdge(n0, e0); AddEdge(n1, e1);
			AddEdge(n2, e2); AddEdge(n3, e3);

			// Nozzle rims
			AddEdge(n0, n1); AddEdge(n1, n2); AddEdge(n2, n3); AddEdge(n3, n0);
			AddEdge(e0, e1); AddEdge(e1, e2); AddEdge(e2, e3); AddEdge(e3, e0);

			// Exhaust grid
			AddEdge(e0, e2); AddEdge(e1, e3);
		};

	// Upper left, Upper right, Lower left, Lower right engines
	BuildThrusterNozzle(-0.55f,  0.20f);
	BuildThrusterNozzle( 0.55f,  0.20f);
	BuildThrusterNozzle(-0.55f, -0.20f);
	BuildThrusterNozzle( 0.55f, -0.20f);

	// =====================================================
	// Allocation & Color Setup (Dark Crimson / Orange)
	// =====================================================

	m_iLineListCount = (int)edges.size();

	delete[] m_pLineList;
	m_pLineList = new SVertexPC[m_iLineListCount * 2];

	for(uint32_t i = 0; i < m_iLineListCount; ++i)
	{
		m_pLineList[i * 2 + 0].vPos = pts[edges[i].first];
		m_pLineList[i * 2 + 1].vPos = pts[edges[i].second];

		// Dark crimson base with energetic orange highlights at line endpoints
		m_pLineList[i * 2 + 0].sAttribs.vColor = SVector4(0.7f, 0.5f, 0.4f, 1.0f);
		m_pLineList[i * 2 + 1].sAttribs.vColor = SVector4(0.8f, 0.4f, 0.5f, 1.0f);
	}
}