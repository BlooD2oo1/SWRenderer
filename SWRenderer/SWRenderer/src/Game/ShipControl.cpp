#include "ShipControl.h"

#include "Common/Globals.h"
#include "Engine/Engine.h"

CActors::CActors( CSceneGame& sSceneGame )
	: m_sSceneGame( sSceneGame )
	, m_cHashGridShips( 100.0f )
	, m_cHashGridAsteroids( 300.0f )
{
	{
		SShipDesc& sShipDesc = m_pShipDescs[SShip::Interceptor];
		sShipDesc.fDragExponent = 2.0f;
		sShipDesc.fDragCoeff = 0.01f;
		sShipDesc.fBoidMul_Separation = 0.6f;
		sShipDesc.fBoidMul_Alignment = 1.0f;
		sShipDesc.fBoidMul_Cohesion = 0.0013f;
		sShipDesc.fMovSmooth = 1.01f;
		sShipDesc.fMovMul_Boid = 0.001f;
		sShipDesc.fMovMul_AsteroidDeflect = 0.0003f;
		sShipDesc.fMovMul_AsteroidDropOut = 0.0003f;
		sShipDesc.fMovMul_UserCtrl = 0.00013f;
		sShipDesc.fMovMul_Follow = 0.0002f;
		sShipDesc.fSize = 8.0f;
		sShipDesc.fMass = powf( sShipDesc.fSize, 3.0f );
		sShipDesc.m_aTurretPositions.clear();
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.3f, 0.72f, 0.0f ) );
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.7f, 0.0f, 0.0f ) );
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.3f, -0.72f, 0.0f ) );
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.7f, 0.0f, 0.0f ) );
	}
	{
		SShipDesc& sShipDesc = m_pShipDescs[SShip::Scout];
		sShipDesc.fDragExponent = 1.0f;
		sShipDesc.fDragCoeff = 0.001f;		
		sShipDesc.fBoidMul_Separation = 0.6f;
		sShipDesc.fBoidMul_Alignment = 1.0f;
		sShipDesc.fBoidMul_Cohesion = 0.0013f;
		sShipDesc.fMovSmooth = 1.01f;
		sShipDesc.fMovMul_Boid = 0.001f;
		sShipDesc.fMovMul_AsteroidDeflect = 0.0007f;
		sShipDesc.fMovMul_AsteroidDropOut = 0.0003f;
		sShipDesc.fMovMul_UserCtrl = 0.00013f;
		sShipDesc.fMovMul_Follow = 0.0003f;
		sShipDesc.fSize = 3.5f;
		sShipDesc.fMass = powf( sShipDesc.fSize, 3.0f );
		sShipDesc.m_aTurretPositions.clear();
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.3f, 0.0f, 0.0f ) );
	}
	{
		SShipDesc& sShipDesc = m_pShipDescs[SShip::Destroyer];
		sShipDesc.fDragExponent = 1.0f;
		sShipDesc.fDragCoeff = 0.001f;
		sShipDesc.fBoidMul_Separation = 2.6f;
		sShipDesc.fBoidMul_Alignment = 0.2f;
		sShipDesc.fBoidMul_Cohesion = 0.0023f;
		sShipDesc.fMovSmooth = 1.003f;
		sShipDesc.fMovMul_Boid = 0.001f;
		sShipDesc.fMovMul_AsteroidDeflect = 0.004f;
		sShipDesc.fMovMul_AsteroidDropOut = 0.0003f;
		sShipDesc.fMovMul_UserCtrl = 0.00013f;
		sShipDesc.fMovMul_Follow = 0.0002f;
		sShipDesc.fSize = 15.0f;
		sShipDesc.fMass = powf( sShipDesc.fSize, 3.0f );
		sShipDesc.m_aTurretPositions.clear();
		sShipDesc.m_aTurretPositions.push_back( SVector3( 0.3f, 0.0f, 0.0f ) );
	}

	Clear();
}
CActors::~CActors()
{
	Clear();
}

void CActors::Clear()
{
	m_iPlayerShipID = iShipIDInvalid;
	m_mShips.Clear();

	m_aAsteroids.clear();

	m_cHashGridShips.Clear();
	m_cHashGridAsteroids.Clear();
}

void CActors::Create()
{
	Clear();

	{
		m_iPlayerShipID = AddShip();
		SShip& sShipPlayer = GetShipByID( m_iPlayerShipID );
		sShipPlayer.m_eControlType = SShip::Player;
		sShipPlayer.m_eShipType = SShip::Interceptor;		
		//sShipPlayer.m_vMov.x = 0.05f;
		sShipPlayer.m_sTurret.m_fShootFreqHz = 30.0f;
		sShipPlayer.m_sTurret.m_fBulletSpeed = 0.2f;
		sShipPlayer.m_sTurret.m_fDamage = 10.0f;
		sShipPlayer.m_sTurret.m_vColor = SVector3( 1.0f, 0.7f, 0.9f ) * 0.6f;
	}

	const int iEnemyCount = 16;
	for ( int i = 0; i < iEnemyCount; i++ )
	{
		ShipID iShipID = AddShip();
		SShip& sShipEnemy = GetShipByID( iShipID );
		sShipEnemy.m_eControlType = SShip::AI;
		sShipEnemy.m_eShipType = ((rand()%3) == 0) ? SShip::Destroyer : SShip::Scout;

		sShipEnemy.m_sTurret.m_fShootFreqHz = 10.0f;
		sShipEnemy.m_sTurret.m_fBulletSpeed = 0.1f;
		sShipEnemy.m_sTurret.m_fDamage = 3.0f;
		sShipEnemy.m_sTurret.m_vColor = SVector3( 0.0f, 1.0f, 1.0f ) * 0.6f;

		const float fScatterRadius = 40.0f * sqrtf( (float)iEnemyCount );
		sShipEnemy.m_vPos.x = ( ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f ) * fScatterRadius;
		sShipEnemy.m_vPos.y = ( ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f ) * fScatterRadius;
		sShipEnemy.m_vPos.z = 0.0f;

		sShipEnemy.m_vPos.x += -150.0f;


		sShipEnemy.m_fYaw = ((float)rand() / (float)RAND_MAX) * PI2;

		/*sShipEnemy.f0 = ((float)rand() / (float)RAND_MAX);
		sShipEnemy.f1 = ((float)rand() / (float)RAND_MAX);
		sShipEnemy.f2 = ((float)rand() / (float)RAND_MAX);
		sShipEnemy.f3 = ((float)rand() / (float)RAND_MAX);*/

		sShipEnemy.m_fPhase_01 = ((float)rand() / (float)RAND_MAX) * PI2;

	}

	m_aAsteroids.reserve( 400 );
	for ( size_t i = 0; i < m_aAsteroids.capacity(); i++ )
	{
		SAsteroid sAsteroid;
		float fScatterRadius = 2000.0f;
		//fScatterRadius = 10.0f;
		sAsteroid.m_vPos.x = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sAsteroid.m_vPos.y = ((float)rand() / (float)RAND_MAX) * fScatterRadius * 2.0f - fScatterRadius;
		sAsteroid.m_vPos.z = 0.0f;

		sAsteroid.m_fSize = ( ((float)rand() / (float)RAND_MAX) * 0.5f + 0.5f );
		sAsteroid.m_fSize *= sAsteroid.m_fSize;
		sAsteroid.m_eModel = ( sAsteroid.m_fSize > 0.8f ) ? SAsteroid::ModelBig : (((rand() % 2) == 0 ) ? SAsteroid::Model01 : SAsteroid::Model02);
		sAsteroid.m_fSize *= 20.0f;

		// uniform distribution of quaternions:
		const float u1 = ((float)rand() / (float)RAND_MAX);
		const float u2 = ((float)rand() / (float)RAND_MAX);
		const float u3 = ((float)rand() / (float)RAND_MAX);
		sAsteroid.m_qRot.w = sqrtf( 1.0f - u1 ) * sinf( 2.0f * PI * u2 );
		sAsteroid.m_qRot.x = sqrtf( 1.0f - u1 ) * cosf( 2.0f * PI * u2 );
		sAsteroid.m_qRot.y = sqrtf( u1 ) * sinf( 2.0f * PI * u3 );
		sAsteroid.m_qRot.z = sqrtf( u1 ) * cosf( 2.0f * PI * u3 );		

		m_aAsteroids.push_back( sAsteroid );
	}

	m_cHashGridAsteroids.Clear();
	for ( size_t i = 0; i < GetAsteroidCount(); i++ )
	{
		const SAsteroid& sAsteroid = GetAsteroid( i );
		m_cHashGridAsteroids.Add( sAsteroid.m_vPos.xy(), i );
	}
}

void CActors::GetField_ShipPlayer( SVector2& vField, const SVector2& p, const SShip& sShip )
{
	SVector2 vD( sShip.m_vPos.xy() - p );
	float lD = SVector2::Length( vD );

	vField = SVector2( 0.0f, 0.0f );

	SVector2 vShipPos( sShip.m_vPos.xy() );
	SVector2 vShipDir( sShip.m_vDir.xy() );
	SVector2 v0( (vShipPos+vShipDir*(2.0f+lD*0.5f)) - p );
	SVector2 v1( (vShipPos+vShipDir*35.0f) - p );
	SVector2 v2( (vShipPos+vShipDir*37.0f) - p );
	float l0 = SVector2::Length( v0 );
	float l1 = SVector2::Length( v1 );
	float l2 = SVector2::Length( v2 );
	SVector2::Normalize( vField, v0*0.01f - v1/l1/l1*4.0f - v2/l2/l2*5.0f );

	{
		float fMulP = Clamp( ( lD - 15.0f ) / 10.0f, -1.0f, 1.0f );
		float fMulN = Clamp( ( lD - 10.0f ) / 4.0f, -1.0f, 1.0f );
		if ( fMulP > 0.0f )
		{
			vField *= fMulP;
		}
		else if ( fMulN < 0.0f )
		{
			vField = vD/lD * fMulN;
		}
		else
		{
			vField = SVector2( 0.0f, 0.0f );
		}

	}

	//float fRepel = Clamp( 1000.0f / l0 / l0 / l0, 0.0f, 0.03f );
	//vField -= v0 * fRepel;
}

void CActors::GetField_Asteroid( SVector2& vField, const SVector2& p, const SShip& sShip, const SAsteroid& sAsteroid )
{
	/*const float fWidth = std::min( sAsteroid.m_fSize * 1.0f + sShip.m_fSize, m_fHashGridAsteroids_Size );
	const float fHeightN = std::min( sAsteroid.m_fSize * 10.0f + sShip.m_fSize, m_fHashGridAsteroids_Size );
	const float fHeightP = std::min( sAsteroid.m_fSize * 0.5f + sShip.m_fSize, m_fHashGridAsteroids_Size );
	const float fRad = std::min( sAsteroid.m_fSize * 0.8f + sShip.m_fSize, m_fHashGridAsteroids_Size );

	SVector2 vAst( p - sAsteroid.m_vPos.xy() );
	SVector2 vShipMovNorm( sShip.m_vMov.xy() );
	float fShipMovL = SVector2::Length( vShipMovNorm );
	if ( fShipMovL > 0.0f )
	{
		vShipMovNorm /= fShipMovL;
	}

	SVector2 vT( -vShipMovNorm.y, vShipMovNorm.x );
	SVector2 vR( 0.0f, 0.0f );
	float fX = 0.0f;
	float fY = 0.0f;
	float fZ = 0.0f;

	if ( fShipMovL > 0.0f )
	{
		float t = SVector2::Cross( vShipMovNorm, vAst );
		float tSign = (t > 0.0f) ? +1.0f : -1.0f;
		
		vT *= tSign;
		fX = SVector2::Dot( vAst, vT );
		fX = (fWidth - fX) / fWidth;
		fX = Clamp( fX, 0.0f, 1.0f );
	}
	if ( fShipMovL > 0.0f )
	{
		fY = SVector2::Dot( vAst, vShipMovNorm );
		if ( fY > 0.0f )
		{
			fY = (fHeightP - fY) / fHeightP;
		}
		else
		{
			fY = (fHeightN + fY) / fHeightN;
		}
		fY = Clamp( fY, 0.0f, 1.0f );

	}	
	{
		float fRadial = SVector2::Length( vAst );
		if ( fRadial > 0.0f )
		{
			fZ = (fRad - fRadial) / fRad;
			fZ = Clamp( fZ, 0.0f, 1.0f );
			vR = vAst / fRadial;
		}
	}

	vT *= fX;
	vT *= fY;
	vR *= fX * fY;

	SVector2::Lerp( vField, vT, vR, 0.5f );
	*/

	const float fWidth = std::min( sAsteroid.m_fSize * 2.0f + GetShipDesc( sShip.m_eShipType ).fSize, m_cHashGridAsteroids.GetGridSize() );
	const float fRad = std::min( sAsteroid.m_fSize * 2.4f + GetShipDesc( sShip.m_eShipType ).fSize, m_cHashGridAsteroids.GetGridSize() );

	SVector2 vShipMovNorm( sShip.m_vMov.xy() );
	SVector2::Normalize( vShipMovNorm, vShipMovNorm );

	SVector2 vAst( sAsteroid.m_vPos.xy() - p );
	float t = SVector2::Cross( vAst, vShipMovNorm );
	t /= fWidth;

	if ( t > 0.0f )
	{
		t = +1.0f - t;
		t = std::max( 0.0f, t );
	}
	else
	{
		t = -1.0f - t;
		t = std::min( 0.0f, t );
	}

	float dotv = SVector2::Dot( vAst, vShipMovNorm );
	if ( dotv < 1.0f ) dotv *= 25.0f;
	t /= dotv;
	t = Clamp( t * fRad, -1.0f, 1.0f );

	float fAstL = SVector2::Length( vAst );
	SVector2 vAstNorm( vAst / fAstL );

	float r = Clamp( (fRad - fAstL) / fRad, 0.0f, 1.0f );

	t *= r;

	SVector2 vT( -vShipMovNorm.y, vShipMovNorm.x );	
	vT *= t;
	SVector2 vN( vAstNorm*-r );
	SVector2::Lerp( vField, vT, vN, 0.3f );
}

void CActors::GetField_Asteroid2( SVector2& vField, const SVector2& p, const SShip& sShip, const SAsteroid& sAsteroid )
{
	const float fRad0 = std::min( sAsteroid.m_fSize * 0.5f + GetShipDesc( sShip.m_eShipType ).fSize, m_cHashGridAsteroids.GetGridSize() );
	const float fRad1 = std::min( sAsteroid.m_fSize * 10.0f + GetShipDesc( sShip.m_eShipType ).fSize, m_cHashGridAsteroids.GetGridSize() );

	SVector2 vAst( p - sAsteroid.m_vPos.xy() );
	float fAstL = SVector2::Length( vAst );
	if ( fAstL > 0.0f )
	{
		float f = Clamp( (fRad1-fAstL)/(fRad1-fRad0), 0.0f, 1.0f );
		f = powf( f, 3.0f );
		vField = vAst/fAstL*f;
		SVector2 vShipMovNorm( sShip.m_vMov.xy() );
		SVector2::Normalize( vShipMovNorm, vShipMovNorm );
		vField *= Clamp( -SVector2::Dot( vField, vShipMovNorm ), 0.0f, 1.0f );
	}
	else
	{
		vField = SVector2( 0.0f, 0.0f );
	}
}

void CActors::Update()
{
	_updateHashGrids();
	_updateShips();
}

void CActors::_updateHashGrids()
{
	m_cHashGridShips.Clear();
	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShip = GetShip( i );
		m_cHashGridShips.Add( sShip.m_vPos.xy(), i );
	}
}

void CActors::_updateShips()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	for ( size_t iShipInd = 0; iShipInd < GetShipCount(); iShipInd++ )
	{
		SShip& sShip = GetShip( iShipInd );
		sShip.m_vMov_CurrPrev = sShip.m_vMov_Curr;
		sShip.m_vMov_Curr = SVector3( 0.0f, 0.0f, 0.0f );
	}

	// ============================================================================
	// Ship Boid Update
	// ============================================================================

	for ( size_t iShipInd0 = 0; iShipInd0 < GetShipCount(); iShipInd0++ )
	{
		SShip& sShip0 = GetShip( iShipInd0 );

		SVector2 vSeparation( 0.0f, 0.0f );
		SVector2 vAlignment( 0.0f, 0.0f );
		SVector2 vCohesion( 0.0f, 0.0f );

		int iNeighborCount = 0;
		SVector2 vAvgPos( 0.0f, 0.0f );
		SVector2 vAvgMov( 0.0f, 0.0f );

		const auto aNeighborGrids = m_cHashGridShips.Get3x3Neighbors( sShip0.m_vPos.xy() );
		for ( size_t iNeighbourGridInd = 0; iNeighbourGridInd < aNeighborGrids.size(); ++iNeighbourGridInd )
		{
			const std::vector< size_t >* pShipInds = aNeighborGrids[iNeighbourGridInd];
			if ( !pShipInds )
			{
				continue;
			}
			for ( size_t iShipIndInd = 0; iShipIndInd < pShipInds->size(); iShipIndInd++ )
			{
				size_t iShipInd1 = (*pShipInds)[iShipIndInd];
				if ( iShipInd0 == iShipInd1 ) continue;
				SShip& sShip1 = GetShip( iShipInd1 );

				if ( sShip0.m_eControlType != sShip1.m_eControlType ) continue;

				SVector2 vDist( sShip1.m_vPos.x - sShip0.m_vPos.x, sShip1.m_vPos.y - sShip0.m_vPos.y );
				float fDistSq = SVector2::LengthSq( vDist );
				if ( fDistSq < powf( m_cHashGridShips.GetGridSize(), 2 ) )
				{
					if ( fDistSq < powf( m_cHashGridShips.GetGridSize()/2.0f, 2 ) )
					{
						SVector2 vDirAway( -vDist.x, -vDist.y );
						vSeparation += vDirAway * (1.0f / (fDistSq + 0.00001f));
					}
					iNeighborCount++;
					vAvgPos += sShip1.m_vPos.xy();
					vAvgMov += sShip1.m_vMov.xy();
				}
			}
		}

		if ( iNeighborCount > 0 )
		{
			vAvgMov /= (float)iNeighborCount;
			vAlignment = vAvgMov - sShip0.m_vMov.xy();

			vAvgPos /= (float)iNeighborCount;
			vCohesion = vAvgPos - sShip0.m_vPos.xy();
		}

		SVector2 vBoidMov =	vSeparation * GetShipDesc( sShip0.m_eShipType ).fBoidMul_Separation +
							vAlignment * GetShipDesc( sShip0.m_eShipType ).fBoidMul_Alignment +
							vCohesion * GetShipDesc( sShip0.m_eShipType ).fBoidMul_Cohesion;

		sShip0.m_vMov_Curr += SVector3( vBoidMov.x, vBoidMov.y, 0.0f ) * GetShipDesc( sShip0.m_eShipType ).fMovMul_Boid;
	}

	// ============================================================================
	// Ship Asteroid Update
	// ============================================================================

	for ( size_t iShipInd = 0; iShipInd < GetShipCount(); iShipInd++ )
	{
		SShip& sShip = GetShip( iShipInd );

		SVector2 vAsteroidField( 0.0f, 0.0f );
		SVector2 vDropOutMov( 0.0f, 0.0f );

		const auto aNeighborGrid = m_cHashGridAsteroids.Get3x3Neighbors( sShip.m_vPos.xy() );
		for ( size_t iNeighbourGridInd = 0; iNeighbourGridInd < aNeighborGrid.size(); ++iNeighbourGridInd )
		{
			const std::vector< size_t >* pAsteroidInds = aNeighborGrid[iNeighbourGridInd];
			if ( !pAsteroidInds )
			{
				continue;
			}
			for ( size_t iAsteroidIndInd = 0; iAsteroidIndInd < pAsteroidInds->size(); iAsteroidIndInd++ )
			{
				size_t iAsteroidInd = (*pAsteroidInds)[iAsteroidIndInd];
				SAsteroid& sAsteroid = GetAsteroid( iAsteroidInd );
						
				{
					SVector2 vField;
					//GetField_Asteroid( vField, sShip.m_vPos.xy(), sShip, sAsteroid );
					//vSeparation += vField*2.0f;

					GetField_Asteroid2( vField, sShip.m_vPos.xy(), sShip, sAsteroid );
					vAsteroidField += vField;
				}

				SVector2 vDist( sAsteroid.m_vPos.x - sShip.m_vPos.x, sAsteroid.m_vPos.y - sShip.m_vPos.y );
				float fDistSq = SVector2::LengthSq( vDist );
				float fSizeSq = sAsteroid.m_fSize + GetShipDesc( sShip.m_eShipType ).fSize;
				fSizeSq *= fSizeSq;
				if ( fDistSq < fSizeSq )
				{
					float fDamage = 0.8f * fElapsedTimeMs;
					if ( sShip.m_eControlType == SShip::Player )
					{
						fDamage = 0.05f * fElapsedTimeMs;
					}

					SVector2 vNormal( vDist/sqrtf(fDistSq) );
					vDropOutMov += -vNormal;

					ApplyCustomDrag( sShip.m_vMov, 3, 0.6f, fElapsedTimeMs );

					if ( _onDamageShipByCollision( sShip, fDamage ) )
					{
					}
				}
			}
		}

		const float fHeight = 10.0f;
		SVector2 vShipMovNorm( sShip.m_vMov.xy() );
		SVector2::Normalize( vShipMovNorm, vShipMovNorm );
		float f = SVector2::Cross( vShipMovNorm, vAsteroidField );
		if ( f >= 0.0f )
		{
			f = std::max( 0.0f, (fHeight - (+f)) / fHeight );
		}
		else
		{
			f = -std::max( 0.0f, (fHeight - (-f)) / fHeight );
		}
		f = Clamp( f, -1.0f, 1.0f );
		
		vAsteroidField += SVector2( -vShipMovNorm.y, vShipMovNorm.x ) * f * SVector2::Length( vAsteroidField );

		if ( sShip.m_eControlType == SShip::AI )
		{
			sShip.m_vMov_Curr += SVector3( vAsteroidField.x, vAsteroidField.y, 0.0f ) * GetShipDesc( sShip.m_eShipType ).fMovMul_AsteroidDeflect;
		}

		sShip.m_vMov_Curr += SVector3( vDropOutMov.x, vDropOutMov.y, 0.0f ) * GetShipDesc( sShip.m_eShipType ).fMovMul_AsteroidDropOut;
	}

	// ============================================================================
	// Ship - Ship Weapon Damage Update
	// ============================================================================

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShipPlayer = GetShipPlayer();

		SShip& sShip = GetShip( i );

		if ( sShip.m_eControlType == SShip::Player ) continue;

		for ( int iBulletInd = 0; iBulletInd < sShipPlayer.m_sTurret.m_aBullets.size(); iBulletInd++ )
		{
			STurret::SBullet& sBullet = sShipPlayer.m_sTurret.m_aBullets[iBulletInd];
			float fT = 0.0f;
			if ( SegmentSphereTest( sBullet.m_vPosPrev.xy(), sBullet.m_vPos.xy(), sShip.m_vPos.xy(), GetShipDesc( sShip.m_eShipType ).fSize, fT ) )
			{
				SVector2 vSegmentDir( sBullet.m_vPos.xy() - sBullet.m_vPosPrev.xy() );
				SVector2::Normalize( vSegmentDir, vSegmentDir );
				SVector2 vAttackPoint( sBullet.m_vPosPrev.xy() + vSegmentDir * fT );
				float fAttackForce = SVector2::Dot( vSegmentDir, sBullet.m_vMov.xy() );
				SVector2 vMov = (sShip.m_vPos.xy() - vAttackPoint) * fAttackForce / GetShipDesc( sShip.m_eShipType ).fSize * sBullet.m_fMass;
				vMov /= fElapsedTimeMs;
				sShip.m_vMov_Curr.xy() += vMov;

				_onDamageShipByBullet( sShip, sShipPlayer.m_sTurret.m_fDamage, sBullet.m_vMov );

				SAudioEvent sAudioEvent;
				sAudioEvent.type = SAudioEvent::GunHit;
				sAudioEvent.fVolume = 0.15f;
				sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
				sAudioEvent.iLifeTimeNs = 1000 * 1000 * 500;
				sAudioEvent.iSampleCounter = 0;
				sAudioEvent.fPhase = 0.0f;	
				sAudioEvent.sClick.iButton = 1;
				sAudioEvent.sGun.vPos = sBullet.m_vPos;
				sAudioEvent.sGun.fPitch = 600.0f;
				CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );

				sBullet.m_fTime = sBullet.m_fTimer;
			}
		}
	}

	// ============================================================================
	// Bullet - Asteroid Damage Update
	// ============================================================================

	for ( size_t i = 0; i < GetAsteroidCount(); i++ )
	{
		SShip& sShipPlayer = GetShipPlayer();

		SAsteroid& sAsteroid = GetAsteroid( i );
		for ( int iBulletInd = 0; iBulletInd < sShipPlayer.m_sTurret.m_aBullets.size(); iBulletInd++ )
		{
			STurret::SBullet& sBullet = sShipPlayer.m_sTurret.m_aBullets[iBulletInd];
			float fT = 0.0f;
			if ( SegmentSphereTest( sBullet.m_vPosPrev.xy(), sBullet.m_vPos.xy(), sAsteroid.m_vPos.xy(), sAsteroid.m_fSize, fT ) )
			{
				SAudioEvent sAudioEvent;
				sAudioEvent.type = SAudioEvent::GunHit;
				sAudioEvent.fVolume = 0.1f;
				sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
				sAudioEvent.iLifeTimeNs = 1000 * 1000 * 200;
				sAudioEvent.iSampleCounter = 0;
				sAudioEvent.fPhase = 0.0f;	
				sAudioEvent.sClick.iButton = 1;
				sAudioEvent.sGun.vPos = sBullet.m_vPos;
				sAudioEvent.sGun.fPitch = 600.0f;
				CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );

				sBullet.m_fTime = sBullet.m_fTimer;
			}
		}
	}

	// ============================================================================
	// Delete Dead Ships
	// ============================================================================

	for ( size_t i = 0; i < GetShipCount(); )
	{
		SShip& sShip = GetShip( i );
		if ( sShip.m_eControlType == SShip::AI )
		{
			if ( sShip.m_bDead )
			{
				m_mShips.Delete( sShip.m_iID );
				continue;
			}
		}
		i++;
	}

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShip = GetShip( i );

		// ============================================================================
		// Player Ship Update
		// ============================================================================
		if ( sShip.m_eControlType == SShip::Player )
		{
			float fYawMultiplier = sShip.m_sTurret.m_bShoot ? 0.5f : 1.0f;

			sShip.m_fYawSpeed = SmoothConverge( sShip.m_fYawSpeed, sShip.m_fYaw_ctrl * fYawMultiplier, 1.01f, 1.01f, fElapsedTimeMs );
			sShip.m_fAccForward = SmoothConverge( sShip.m_fAccForward, sShip.m_fAccForward_ctrl, 1.01f, 1.01f, fElapsedTimeMs );
			sShip.m_fAccRight = SmoothConverge( sShip.m_fAccRight, sShip.m_fAccRight_ctrl, 1.001f, 1.01f, fElapsedTimeMs );

			sShip.m_fYaw += sShip.m_fYawSpeed * 0.004f * fElapsedTimeMs;
			SVector3 vShipForward( cosf( sShip.m_fYaw ), sinf( sShip.m_fYaw ), 0.0f );
			SVector3 vShipRight( -vShipForward.y, vShipForward.x, 0.0f );
			sShip.m_vMov_Curr += vShipForward * sShip.m_fAccForward * GetShipDesc( sShip.m_eShipType ).fMovMul_UserCtrl;
			sShip.m_vMov_Curr += vShipRight * sShip.m_fAccRight * GetShipDesc( sShip.m_eShipType ).fMovMul_UserCtrl;




			// m_vMov felbontasa m_vDir es m_vRight iranyara, hogy a ship ne tudjon "csuszni" a levegoben
			{
				SVector3 vMovForward( sShip.m_vDir );
				vMovForward = vMovForward * SVector3::Dot( sShip.m_vMov, vMovForward );
				SVector3 vMovRight( -sShip.m_vDir.y, sShip.m_vDir.x, 0.0f );
				vMovRight = vMovRight * SVector3::Dot( sShip.m_vMov, vMovRight );
				vMovRight *= CalcSmoothUpdateWeight( 1.0f + fabsf( sShip.m_fAccForward ) * 0.0005f, fElapsedTimeMs );
				sShip.m_vMov = vMovForward + vMovRight;
			}
		}

		// ============================================================================
		// Enemy Ship Update
		// ============================================================================
		if ( sShip.m_eControlType == SShip::AI )
		{

			SShip& sShipPlayer = GetShipPlayer();

			SVector3 vEnemyToPlayerDir( sShipPlayer.m_vPos - sShip.m_vPos );
			const float fEnemyToPlayerDist = SVector3::Length( vEnemyToPlayerDir );
			SVector3::Normalize( vEnemyToPlayerDir, vEnemyToPlayerDir );
			const float fSin_Phase_01 = sinf( sShip.m_fPhase_01 );

			SVector2 vField( 0.0f, 0.0f );
			GetField_ShipPlayer( vField, sShip.m_vPos.xy(), sShipPlayer );
			float fFollowAmount = GetShipDesc( sShip.m_eShipType ).fMovMul_Follow;

			switch ( sShip.m_eShipType )
			{
			case SShip::Scout:
			{
				fFollowAmount *= ( fSin_Phase_01 * 0.5f + 0.5f ) * 0.7f + 0.3f;

				if ( fSin_Phase_01 > 0.98f && fSin_Phase_01 < 1.0f && SVector3::Dot( sShip.m_vDir, vEnemyToPlayerDir ) > 0.8f )
				{
					if ( !sShip.m_sTurret.m_bShoot )
					{
						sShip.m_sTurret.m_iLastBulletTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
					}
					sShip.m_sTurret.m_bShoot = true;
				}
				else
				{
					sShip.m_sTurret.m_bShoot = false;
				}
			}
			break;
			}

			
			sShip.m_vMov_Curr.xy() += vField * fFollowAmount;

			const float fYaw = atan2f( sShip.m_vMov.y, sShip.m_vMov.x );
			const float fYawPrev = sShip.m_fYaw;
			sShip.m_fYaw = LerpAngle( fYaw, sShip.m_fYaw, CalcSmoothUpdateWeight( 1.02f, fElapsedTimeMs ) );
			sShip.m_fYawSpeed = (sShip.m_fYaw - fYawPrev) / (fElapsedTimeMs * 0.004f);
		}
	}

	// ============================================================================
	// Final Ship Update
	// ============================================================================

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShip = GetShip( i );

		// Get current speed and direction
		/*float fSpeed = SVector2::Length( sShip.m_vMov.xy() );
		if ( fSpeed > 0.0001f )
		{
			SVector2 vForward = sShip.m_vMov.xy() / fSpeed;
			SVector2 vRight   = SVector2( -vForward.y, vForward.x );

			// Decompose m_vMov_Curr into tangential and lateral acceleration
			float fForwardAccel = SVector2::Dot( sShip.m_vMov_Curr.xy(), vForward );
			float fLateralAccel = SVector2::Dot( sShip.m_vMov_Curr.xy(), vRight );

			// Limit maximum lateral acceleration based on current speed: a_perp_max = speed * omega_max
			float fMaxAngularVel  = 0.001f;//GetShipDesc( sShip.m_eShipType ).fMaxAngularVel; // Max turn rate in rad/ms
			float fMaxLatAccel    = fSpeed * fMaxAngularVel;

			fLateralAccel = Clamp( fLateralAccel, -fMaxLatAccel, fMaxLatAccel );

			// Reconstruct the constrained acceleration vector
			sShip.m_vMov_Curr.xy() = ( vForward * fForwardAccel ) + ( vRight * fLateralAccel );
		}*/

		SVector3::Lerp( sShip.m_vMov_Curr, sShip.m_vMov_Curr, sShip.m_vMov_CurrPrev, CalcSmoothUpdateWeight( GetShipDesc( sShip.m_eShipType ).fMovSmooth, fElapsedTimeMs ) );
		sShip.m_vMov += sShip.m_vMov_Curr * fElapsedTimeMs;
		ApplyCustomDrag( sShip.m_vMov, GetShipDesc( sShip.m_eShipType ).fDragExponent, GetShipDesc( sShip.m_eShipType ).fDragCoeff, fElapsedTimeMs );
		// Update position using average velocity (trapezoidal integration)
		sShip.m_vPos += ( sShip.m_vMovPrev + sShip.m_vMov ) * 0.5f * fElapsedTimeMs;

		sShip.m_fRoll = SmoothConverge( -sShip.m_fYawSpeed, sShip.m_fRoll, 1.0002f, fElapsedTimeMs );

		sShip.m_vMovPrev = sShip.m_vMov;
		sShip.m_vDirPrev = sShip.m_vDir;
		sShip.m_matShipPrev = sShip.m_matShip;
		SMatrix::BuildEulerXYZ( sShip.m_matShip, sShip.m_fRoll, 0.0f, sShip.m_fYaw );
		sShip.m_matShip.m30 = sShip.m_vPos.x;	sShip.m_matShip.m31 = sShip.m_vPos.y;	sShip.m_matShip.m32 = sShip.m_vPos.z;	sShip.m_matShip.m33 = 1.0f;

		sShip.m_vDir.x = sShip.m_matShip.m00;	sShip.m_vDir.y = sShip.m_matShip.m01;	sShip.m_vDir.z = sShip.m_matShip.m02;
		sShip.m_vUp.x = sShip.m_matShip.m20;	sShip.m_vUp.y = sShip.m_matShip.m21;	sShip.m_vUp.z = sShip.m_matShip.m22;
		SVector3::Cross( sShip.m_vRight, sShip.m_vDir, sShip.m_vUp );

		SMatrix::Scale( sShip.m_matShip, GetShipDesc( sShip.m_eShipType ).fSize );

		sShip.m_fDamageTimerMs = std::max( sShip.m_fDamageTimerMs - fElapsedTimeMs, 0.0f );

		sShip.m_fPhase_01 += fElapsedTimeMs * 0.001f;
		if ( sShip.m_fPhase_01 > PI2 ) sShip.m_fPhase_01 -= PI2;
	}

	// ============================================================================
	// Ship Weapons Update
	// ============================================================================

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShip = GetShip( i );

		{
			STurret& sTurret = sShip.m_sTurret;

			if ( sTurret.m_bShoot )
			{
				SVector3 vGunDir( sShip.m_vDir );
				SVector3 vGunDirPrev( sShip.m_vDirPrev );

				SVector3 vGunMov( sShip.m_vMov );
				SVector3 vGunMovPrev( sShip.m_vMovPrev );

				const float fShootFreqHz = sTurret.m_fShootFreqHz;
				const uint64_t iShootPeriodNs = (uint64_t)(1.0f / fShootFreqHz * 1000.0f * 1000.0f * 1000.0f);

				uint64_t iTNs = sTurret.m_iLastBulletTimeStampNs;
				for ( ; iTNs < CEngine::GetInstance().GetTimeStampNs(); iTNs += iShootPeriodNs )
				{
					const std::vector< SVector3 >& aTurretPositions = GetShipDesc( sShip.m_eShipType ).m_aTurretPositions;
					if ( aTurretPositions.size() == 0 )
					{
						break;
					}			

					SVector3 vGunPosWorld;
					SVector3 vGunPosWorldPrev;
					{
						SVector3 vGunPos = aTurretPositions[sTurret.m_iBulletCounter%aTurretPositions.size()];
						SMatrix::TransformCoord( vGunPosWorld, vGunPos, sShip.m_matShip );
						SMatrix::TransformCoord( vGunPosWorldPrev, vGunPos, sShip.m_matShipPrev );
					}

					float fFrameW = (float)(iTNs - sTurret.m_iLastBulletTimeStampNs) / (float)(CEngine::GetInstance().GetTimeStampNs() - sTurret.m_iLastBulletTimeStampNs);

					sTurret.m_aBullets.emplace_back();
					STurret::SBullet& sBullet = sTurret.m_aBullets.back();
					sBullet.m_vPos = Lerp( vGunPosWorldPrev, vGunPosWorld, fFrameW );

					SVector3 vBulletDir = Lerp( vGunDirPrev, vGunDir, fFrameW );
					sBullet.m_vMov = Lerp( vGunMovPrev, vGunMov, fFrameW ) + vBulletDir * sTurret.m_fBulletSpeed;

					sBullet.m_fMass = 0.01f;

					sBullet.m_fTime = 1000.0f;
					sBullet.m_fTimer = 0.0f;

					SAudioEvent sAudioEvent;
					sAudioEvent.type = SAudioEvent::GunShot;
					sAudioEvent.fVolume = 0.06f;
					sAudioEvent.iTimeStampNs = iTNs;
					sAudioEvent.iLifeTimeNs = 1000 * 1000 * 1500;
					sAudioEvent.iSampleCounter = 0;
					sAudioEvent.fPhase = 0.0f;			
					sAudioEvent.sGun.vPos = sBullet.m_vPos;
					sAudioEvent.sGun.fPitch = 400.0f + (rand() % 1000 - 500) * 0.1f;
					CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );

					sTurret.m_iBulletCounter++;
				}

				sTurret.m_iLastBulletTimeStampNs = iTNs;
			}

			float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

			for ( size_t iBulletInd = 0; iBulletInd < sTurret.m_aBullets.size(); )
			{
				STurret::SBullet& sBullet = sTurret.m_aBullets[iBulletInd];
				sBullet.m_fTimer += fElapsedTimeMs;
				if ( sBullet.m_fTimer > sBullet.m_fTime )
				{
					sTurret.m_aBullets[iBulletInd] = sTurret.m_aBullets.back();
					sTurret.m_aBullets.pop_back();
					continue;
				}
				sBullet.m_vPosPrev = sBullet.m_vPos;
				sBullet.m_vPos += sBullet.m_vMov * fElapsedTimeMs;

				++iBulletInd;
			}
		}
	}
}

bool CActors::_onDamageShipByBullet( SShip& sShip, float fDamage, const SVector3& vMovBullet )
{
	if ( fDamage <= 0.0f )
	{
		return false;
	}

	sShip.m_fHP -= fDamage;
	sShip.m_fHP = std::max( sShip.m_fHP, 0.0f );

	sShip.m_fDamageTimerMs = 200.0f;

	if ( sShip.m_fHP <= 0.0f && !sShip.m_bDead )
	{
		SAudioEvent sAudioEvent;
		sAudioEvent.type = SAudioEvent::GunHit;
		sAudioEvent.fVolume = 0.2f;
		sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
		sAudioEvent.iLifeTimeNs = 1000 * 1000 * 2000;
		sAudioEvent.iSampleCounter = 0;
		sAudioEvent.fPhase = 0.0f;	
		sAudioEvent.sClick.iButton = 1;
		sAudioEvent.sGun.vPos = sShip.m_vPos;
		sAudioEvent.sGun.fPitch = 200.0f;
		CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );

		SEffect_ShipExplosion& sEffect = m_sSceneGame.GetEffects().CreateShipExplosion();
		sEffect.m_vPos = sShip.m_vPos;
		sEffect.m_vMovShip = sShip.m_vMov;
		sEffect.m_vMovBullet = vMovBullet;
		sEffect.Create();
		
		sShip.m_bDead = true;
		return true;
	}

	return false;
}

bool CActors::_onDamageShipByCollision( SShip& sShip, float fDamage )
{
	if ( fDamage <= 0.0f )
	{
		return false;
	}

	sShip.m_fHP -= fDamage;
	sShip.m_fHP = std::max( sShip.m_fHP, 0.0f );

	sShip.m_fDamageTimerMs = 200.0f;

	if ( sShip.m_fHP <= 0.0f && !sShip.m_bDead )
	{
		SAudioEvent sAudioEvent;
		sAudioEvent.type = SAudioEvent::GunHit;
		sAudioEvent.fVolume = 0.2f;
		sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
		sAudioEvent.iLifeTimeNs = 1000 * 1000 * 2000;
		sAudioEvent.iSampleCounter = 0;
		sAudioEvent.fPhase = 0.0f;	
		sAudioEvent.sClick.iButton = 1;
		sAudioEvent.sGun.vPos = sShip.m_vPos;
		sAudioEvent.sGun.fPitch = 200.0f;
		CAudio::GetInstance().MainThread_PushAudioEvent( sAudioEvent );

		SEffect_ShipExplosion& sEffect = m_sSceneGame.GetEffects().CreateShipExplosion();
		sEffect.m_vPos = sShip.m_vPos;
		sEffect.m_vMovShip = sShip.m_vMov;
		sEffect.m_vMovBullet = SVector3( 0.0f, 0.0f, 0.0f );
		sEffect.Create();

		sShip.m_bDead = true;
		return true;
	}

	return false;
}

void CActors::Render()
{
}

ShipID CActors::AddShip()
{
	ShipID iRetID;
	m_mShips.Add( &iRetID );
	return iRetID;
}

////////////////////////////////////////////////////////////////

SShip::SShip()
{
	Clear();
}

void SShip::Clear()
{
	m_sTurret.Clear();

	m_fYaw = 0.0f;
	m_fRoll = 0.0f;

	m_vPos = SVector3( 0.0f, 0.0f, 0.0f );
	m_vMov = SVector3( 0.0f, 0.0f, 0.0f );
	m_vMovPrev = m_vMov;	
	m_vDir = SVector3( 1.0f, 0.0f, 0.0f );
	m_vDirPrev = m_vDir;
	m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
	m_vRight = SVector3( 0.0f, 1.0f, 0.0f );
	SMatrix::Identity( m_matShip );
	m_matShipPrev = m_matShip;

	m_vMov_Curr = SVector3( 0.0f, 0.0f, 0.0f );
	m_vMov_CurrPrev = SVector3( 0.0f, 0.0f, 0.0f );

	m_fHP = 100.0f;
	m_fDamageTimerMs = 0.0f;
	m_bDead = false;
	m_fPhase_01 = 0.0f;

	m_fYawSpeed = 0.0f;
	m_fYaw_ctrl = 0.0f;	
	m_fAccForward = 0.0f;
	m_fAccForward_ctrl = 0.0f;
	m_fAccRight = 0.0f;
	m_fAccRight_ctrl = 0.0f;
}

////////////////////////////////////////////////////////////////

void STurret::Clear()
{
	m_iBulletCounter = 0;

	m_fShootFreqHz = 1.0f;
	m_fBulletSpeed = 0.1f;
	m_vColor = SVector3( 1.0f, 1.0f, 1.0f );

	m_fDamage = 1.0f;

	m_aBullets.clear();
	m_bShoot = false;
	m_iLastBulletTimeStampNs = 0;
}

////////////////////////////////////////////////////////////////

void SLaserGun::Clear()
{
	m_vGunPosition = SVector3( 0.0f, 0.0f, 0.0f );

	m_vColor = SVector3( 1.0f, 1.0f, 1.0f );

	m_bShoot = false;
}