#pragma once

#include "Common/Vector.h"
#include "Engine/Camera.h"
#include "Engine/Graphics.h"
#include "Game/ShipControl.h"
#include "Game/ShipMesh.h"

class CScene01
{
public:
	CScene01();
	~CScene01();

	void Clear();
	void Create();

	void Update();
	void Render();

	bool On_KeyDown( uint32_t key );
	bool On_KeyUp( uint32_t key );
	bool On_MouseMove( int deltax, int deltay );
	bool On_MouseButtonDown( uint32_t button );
	bool On_MouseButtonUp( uint32_t button );
	bool On_MouseWheel( int iDelta );

private:

	SVertexPC*		m_pStars;
	uint32_t		m_iStarsCount;

	SVertexPC*		m_pBGStars;
	uint32_t		m_iBGStarsCount;

	CShipControl	m_cShipControl;
	CShipMesh		m_cShipMesh;
	CCameraFree		m_cCamera;
	CCameraShip		m_cCameraShip;

	SVertexPC*		m_pVBCircle;
	uint32_t		m_iVBCircleCount;
	uint32_t*		m_pIBCircle;
	uint32_t		m_iIBCircleCount;

	SVector3*		m_pCirclePos;
	int				m_iCirclePosCount;

	float			m_fTimeMultiplierW;
	float			m_fTimeMultiplier;
};
