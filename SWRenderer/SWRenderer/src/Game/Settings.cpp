#include "Settings.h"

CSettings::CSettings()
{
	Clear();
}

CSettings::~CSettings()
{
}

void CSettings::Clear()
{

	m_sConstellationColorWire = BGRA8( 24, 16, 2, 255 );
	m_sConstellationColorPoints = BGRA8( 40, 25, 2, 255 );
	m_vAsteroidColor0 = SVector4( 1.0f, 0.6f, 0.6f, 0.4f );
	m_vAsteroidColor1 = SVector4( 1.0f, 0.9f, 0.4f, 0.7f );
	m_fMiniMapScale = 2000.0f;
	m_fGridSpacing = 100.0f;
	m_iGridHalfSize = 6/2;

	{
		SShipDesc& sShipDesc = m_pShipDescs[Interceptor];
		sShipDesc.fDragExponent = 2.0f;
		sShipDesc.fDragCoeff = 0.01f;
		sShipDesc.fBoidMul_Separation = 0.6f;
		sShipDesc.fBoidMul_Alignment = 1.0f;
		sShipDesc.fBoidMul_Cohesion = 0.0013f;
		sShipDesc.fMovSmooth = 1.01f;
		sShipDesc.fMovMul_Boid = 0.001f;
		sShipDesc.fMovMul_AsteroidDeflect = 0.0003f;
		sShipDesc.fMovMul_AsteroidDropOut = 0.0001f;
		sShipDesc.fMovMul_Follow = 0.0002f;
		sShipDesc.fSpeedMin = 0.0f;
		sShipDesc.fAccelMax = 0.0005f;
		sShipDesc.fAngularAccelMax = 100000.0f;
		sShipDesc.fSize = 8.0f;
		sShipDesc.fMass = powf( sShipDesc.fSize, 3.0f );
		sShipDesc.m_aTurretPositions.clear();
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.3f, 0.72f, 0.0f ) );
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.7f, 0.0f, 0.0f ) );
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.3f, -0.72f, 0.0f ) );
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.7f, 0.0f, 0.0f ) );

		sShipDesc.m_vColor0 = SVector3( 1.0f, 0.4f, 0.4f ) * 0.33f;
		sShipDesc.m_vColor1 = SVector3( 1.0f, 0.7f, 0.6f ) * 0.33f;
		sShipDesc.m_vColor2 = SVector3( 1.0f, 0.6f, 0.5f ) * 0.33f;		
		sShipDesc.m_vColor2Shoot = SVector3( 0.1f, 0.4f, 1.0f ) * 0.33f;
		sShipDesc.m_fAlpha = 0.8f;
	}
	{
		SShipDesc& sShipDesc = m_pShipDescs[Scout];
		sShipDesc.fDragExponent = 1.0f;
		sShipDesc.fDragCoeff = 0.001f;		
		sShipDesc.fBoidMul_Separation = 0.6f;
		sShipDesc.fBoidMul_Alignment = 0.3f;
		sShipDesc.fBoidMul_Cohesion = 0.0013f;
		sShipDesc.fMovSmooth = 1.03f;
		sShipDesc.fMovMul_Boid = 0.006f;
		sShipDesc.fMovMul_AsteroidDeflect = 0.002f;
		sShipDesc.fMovMul_AsteroidDropOut = 0.0001f;
		sShipDesc.fMovMul_Follow = 0.0003f;
		sShipDesc.fSpeedMin = 0.02f;
		sShipDesc.fAccelMax = 0.000018f;
		sShipDesc.fAngularAccelMax = 0.002f;
		sShipDesc.fSize = 4.5f;
		sShipDesc.fMass = powf( sShipDesc.fSize, 3.0f );
		sShipDesc.m_aTurretPositions.clear();
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.3f, 0.0f, 0.0f ) );

		sShipDesc.m_vColor0 = SVector3( 1.0f, 0.5f, 0.0f ) * 0.33f;
		sShipDesc.m_vColor1 = SVector3( 0.0f, 0.9f, 1.0f ) * 0.33f;
		sShipDesc.m_vColor2 = SVector3( 0.0f, 0.8f, 1.0f ) * 0.33f;		
		sShipDesc.m_vColor2Shoot = SVector3( 0.1f, 0.4f, 1.0f ) * 0.33f;
		sShipDesc.m_fAlpha = 0.8f;
	}
	{
		SShipDesc& sShipDesc = m_pShipDescs[InterceptorEnemy];
		sShipDesc.fDragExponent = 1.0f;
		sShipDesc.fDragCoeff = 0.001f;
		sShipDesc.fBoidMul_Separation = 1.6f;
		sShipDesc.fBoidMul_Alignment = 0.4f;
		sShipDesc.fBoidMul_Cohesion = 0.0016f;
		sShipDesc.fMovSmooth = 1.03f;
		sShipDesc.fMovMul_Boid = 0.001f;
		sShipDesc.fMovMul_AsteroidDeflect = 0.004f;
		sShipDesc.fMovMul_AsteroidDropOut = 0.0001f;
		sShipDesc.fMovMul_Follow = 0.0002f;
		sShipDesc.fSpeedMin = 0.02f;
		sShipDesc.fAccelMax = 0.000018f;
		sShipDesc.fAngularAccelMax = 0.001f;
		sShipDesc.fSize = 8.0f;
		sShipDesc.fMass = powf( sShipDesc.fSize, 3.0f );
		sShipDesc.m_aTurretPositions.clear();
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.3f, 0.0f, 0.0f ) );

		sShipDesc.m_vColor0 = SVector3( 1.0f, 0.5f, 0.0f ) * 0.33f;
		sShipDesc.m_vColor1 = SVector3( 0.0f, 0.9f, 1.0f ) * 0.33f;
		sShipDesc.m_vColor2 = SVector3( 0.0f, 0.8f, 1.0f ) * 0.33f;		
		sShipDesc.m_vColor2Shoot = SVector3( 0.1f, 0.4f, 1.0f ) * 0.33f;
		sShipDesc.m_fAlpha = 0.8f;
	}
}

#ifdef EDITOR


#include "ImGui/imgui.h"
#include "Engine/Engine.h"

void CSettings::ImGui()
{
	if ( ImGui::Button( "Restart" ) )
	{
		CEngine::GetInstance().GetSceneGame().Create();
		ImGui::SetWindowFocus(nullptr);
	}

	SVector4 v4;

	v4 = CSettings::GetInstance().m_sConstellationColorWire;
	if ( ImGui::ColorEdit4( "ConstellationColorWire", &v4.x ) )
	{
		CSettings::GetInstance().m_sConstellationColorWire = v4;
	}
	v4 = CSettings::GetInstance().m_sConstellationColorPoints;
	if ( ImGui::ColorEdit4( "ConstellationColorPoints", &v4.x ) )
	{
		CSettings::GetInstance().m_sConstellationColorPoints = v4;
	}
	ImGui::ColorEdit4( "AsteroidColor0", &CSettings::GetInstance().m_vAsteroidColor0.x );
	ImGui::ColorEdit4( "AsteroidColor1", &CSettings::GetInstance().m_vAsteroidColor1.x );
	ImGui::SliderFloat( "MiniMapScale", &CSettings::GetInstance().m_fMiniMapScale, 100.0f, 10000.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
	ImGui::SliderFloat( "GridSpacing", &CSettings::GetInstance().m_fGridSpacing, 1.0f, 1000.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
	ImGui::SliderInt( "GridHalfSize", &CSettings::GetInstance().m_iGridHalfSize, 1, 100 );

	ImGui::SeparatorText( "ShipDescs");
	static EShipType eShipType = EShipType::Interceptor;
	ImGui::Combo( "ShipType", (int*)&eShipType, "Interceptor\0Scout\0InterceptorEnemy\0\0" );
	{
		ImGui::Separator();
		SShipDesc& sShipDesc = CSettings::GetInstance().m_pShipDescs[eShipType];
		ImGui::SliderFloat( "fDragExponent", &sShipDesc.fDragExponent, 0.0f, 10.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fDragCoeff", &sShipDesc.fDragCoeff, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fBoidMul_Separation", &sShipDesc.fBoidMul_Separation, 0.0f, 10.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fBoidMul_Alignment", &sShipDesc.fBoidMul_Alignment, 0.0f, 10.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fBoidMul_Cohesion", &sShipDesc.fBoidMul_Cohesion, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fMovSmooth", &sShipDesc.fMovSmooth, 1.0f, 1.2f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fMovMul_Boid", &sShipDesc.fMovMul_Boid, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fMovMul_AsteroidDeflect", &sShipDesc.fMovMul_AsteroidDeflect, 0.0f, 0.01f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fMovMul_AsteroidDropOut", &sShipDesc.fMovMul_AsteroidDropOut, 0.0f, 0.01f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fMovMul_Follow", &sShipDesc.fMovMul_Follow, 0.0f, 0.01f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fSpeedMin", &sShipDesc.fSpeedMin, 0.0f, 10.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fAccelMax", &sShipDesc.fAccelMax, 0.0f, 0.10f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fAngularAccelMax", &sShipDesc.fAngularAccelMax, 0.0f, 10.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::SliderFloat( "fSize", &sShipDesc.fSize, 0.0f, 100.0f, "%.5f", ImGuiSliderFlags_Logarithmic );
		ImGui::ColorEdit3( "m_vColor0", &sShipDesc.m_vColor0.x );
		ImGui::ColorEdit3( "m_vColor1", &sShipDesc.m_vColor1.x );
		ImGui::ColorEdit3( "m_vColor2", &sShipDesc.m_vColor2.x );
		ImGui::ColorEdit3( "m_vColor2Shoot", &sShipDesc.m_vColor2Shoot.x );
		ImGui::SliderFloat( "m_fAlpha", &sShipDesc.m_fAlpha, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic );

		ImGui::Separator();
	}
}
#endif