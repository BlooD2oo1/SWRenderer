#pragma once

#include "Graphics/Vector.h"
#include "Graphics/Camera.h"

struct SVertex
{
	SVector3 vPos;
};

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

	struct SParticle
	{
		SVector3 vPos;
		float a;
	};
	SParticle*		m_pStars;
	int				m_iStarsCount;

	SParticle*		m_pBGStars;
	int				m_iBGStarsCount;

	SVertex*		m_pLineListSpaceShip;
	int				m_iLineListSpaceShipCount;
	SShip			m_sShip;
	CCameraFree		m_cCamera;
	CCameraShip		m_cCameraShip;
};
