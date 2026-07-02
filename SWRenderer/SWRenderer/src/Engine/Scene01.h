#pragma once

#include "Graphics/Vector.h"
#include "Graphics/Camera.h"
#include "Graphics/Graphics.h"

struct SShip
{
	SShip();
	void Init();
	void Accelerate( float fValue );
	void MoveUp( float fValue );
	void MoveRight( float fValue );

	void Update( float fElapsedTimeMs, const SMatrix& matView000 );

	void GetMatrix( SMatrix& sOut );

	SVector3	m_vPos;
	SVector3	m_vDir;
	SVector3	m_vUp;
	SVector3	m_vRight;

	float		m_fSpeedForward;
	float		m_fSpeedUp;
	float		m_fSpeedRight;
};

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

	SVertexPC*		m_pLineListSpaceShip;
	uint32_t		m_iLineListSpaceShipCount;
	SShip			m_sShip;
	CCameraFree		m_cCamera;
	CCameraShip		m_cCameraShip;

	SVertexPC*		m_pVBCircle;
	uint32_t		m_iVBCircleCount;
	uint32_t*		m_pIBCircle;
	uint32_t		m_iIBCircleCount;

	SVector3*		m_pCirclePos;
	int				m_iCirclePosCount;

	float			m_fTimeMultiplier;
};
