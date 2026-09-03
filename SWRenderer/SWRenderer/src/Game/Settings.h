#pragma once

#include <vector>
#include "Common/Globals.h"
#include "Common/Vector.h"
#include "Engine/Graphics.h"

#define EDITOR

enum EShipType
{
	Interceptor,
	Scout,
	InterceptorEnemy,
	ShipType_Count,
};

struct SShipDesc
{
	float		fDragExponent;
	float		fDragCoeff;
	float		fBoidMul_Separation;
	float		fBoidMul_Alignment;
	float		fBoidMul_Cohesion;
	float		fMovSmooth;
	float		fMovMul_Boid;
	float		fMovMul_AsteroidDeflect;
	float		fMovMul_AsteroidDropOut;
	float		fMovMul_Follow;
	float		fSpeedMin;
	float		fAccelMax;
	float		fAngularAccelMax;
	float		fSize;
	float		fMass;

	std::vector< SVector3 >	m_aTurretPositions;

	SVector3	m_vColor0;
	SVector3	m_vColor1;
	SVector3	m_vColor2;
	SVector3	m_vColor2Shoot;
	float		m_fAlpha;
};

class CSettings
{
	CSettings();
	~CSettings();
public:
	static CSettings& GetInstance()
	{
		static CSettings cInstance;
		return cInstance;
	}

	void Clear();

	void ImGui();

public:
	BGRA8					m_sConstellationColorWire;
	BGRA8					m_sConstellationColorPoints;
	SVector4				m_vAsteroidColor0;
	SVector4				m_vAsteroidColor1;

	float					m_fMiniMapScale;
	float					m_fGridSpacing;
	int						m_iGridHalfSize;



	SShipDesc				m_pShipDescs[ShipType_Count];


};