#include "ShipControl.h"

#include "Common/Globals.h"
#include "Engine/Engine.h"

#define HASH

CActors::CActors( CSceneGame& sSceneGame )
	: m_sSceneGame( sSceneGame )
	, m_cHashGridShips( 100.0f )
	, m_cHashGridAsteroids( 300.0f )
{
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
		sShipPlayer.m_fSize = 10.0f;
		sShipPlayer.m_fMass = powf( sShipPlayer.m_fSize, 3.0f );
		//sShipPlayer.m_vMov.x = 0.05f;
		sShipPlayer.m_sTurret.m_aTurretPositions.push_back( SVector3( 0.5f, 2.7f, 0.0f ) );
		sShipPlayer.m_sTurret.m_aTurretPositions.push_back( SVector3( 1.0f, 0.0f, 0.0f ) );
		sShipPlayer.m_sTurret.m_aTurretPositions.push_back( SVector3( 0.5f, -2.7f, 0.0f ) );
		sShipPlayer.m_sTurret.m_aTurretPositions.push_back( SVector3( 1.0f, 0.0f, 0.0f ) );
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

		sShipEnemy.m_fSize = 7.0f;
		sShipEnemy.m_fMass = powf( sShipEnemy.m_fSize, 3.0f );

		sShipEnemy.m_sTurret.m_aTurretPositions.push_back( SVector3( 0.3f, 0.0f, 0.0f ) );
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

	m_aAsteroids.reserve( 1000 );
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
		//sAsteroid.m_fMass = powf( sAsteroid.m_fSize, 3.0f )*8.0f;

		//sAsteroid.m_vMov = SVector3( 0.0f, 0.0f, 0.0f );

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

	const float fWidth = std::min( sAsteroid.m_fSize * 2.0f + sShip.m_fSize, m_cHashGridAsteroids.GetGridSize() );
	const float fRad = std::min( sAsteroid.m_fSize * 2.4f + sShip.m_fSize, m_cHashGridAsteroids.GetGridSize() );

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
	const float fRad0 = std::min( sAsteroid.m_fSize * 0.5f + sShip.m_fSize, m_cHashGridAsteroids.GetGridSize() );
	const float fRad1 = std::min( sAsteroid.m_fSize * 5.0f + sShip.m_fSize, m_cHashGridAsteroids.GetGridSize() );

	SVector2 vAst( p - sAsteroid.m_vPos.xy() );
	float fAstL = SVector2::Length( vAst );
	if ( fAstL > 0.0f )
	{
		float f = Clamp( (fRad1-fAstL)/(fRad1-fRad0), 0.0f, 1.0f );
		f = powf( f, 4.0f );
		vField = vAst/fAstL*f;
	}
	else
	{
		vField = SVector2( 0.0f, 0.0f );
	}
}

void CActors::Update()
{
	_updateHashGrids();
	_updateBoids();
	_updateShips();
	//_updateAsteroids();
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

void CActors::_updateBoids()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShip = GetShip( i );
		sShip.m_vBoidMov = SVector3( 0.0f, 0.0f, 0.0f );
	}

	for ( size_t i0 = 0; i0 < GetShipCount(); i0++ )
	{
		SShip& sShip0 = GetShip( i0 );

		if ( sShip0.m_iID == GetShipIDPlayer() ) continue;	

		SVector2 vSeparation( 0.0f, 0.0f );
		SVector2 vAlignment( 0.0f, 0.0f );
		SVector2 vCohesion( 0.0f, 0.0f );

		int iNeighborCount = 0;
		SVector2 vAvgPos( 0.0f, 0.0f );
		SVector2 vAvgMov( 0.0f, 0.0f );


		const auto aNeighbors = m_cHashGridShips.Get3x3Neighbors( sShip0.m_vPos.xy() );
		for ( size_t i = 0; i < aNeighbors.size(); ++i )
		{
			const std::vector< size_t >* pShipInds = aNeighbors[i];
			if ( !pShipInds )
			{
				continue;
			}
			for ( size_t j = 0; j < pShipInds->size(); j++ )
			{
				size_t i1 = (*pShipInds)[j];
				if ( i0 == i1 ) continue;
				SShip& sShip1 = GetShip( i1 );
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
					vAvgPos += SVector2( sShip1.m_vPos.x, sShip1.m_vPos.y );
					vAvgMov += SVector2( sShip1.m_vMov.x, sShip1.m_vMov.y );
				}
			}
		}

		if ( iNeighborCount > 0 )
		{
			vAvgMov /= (float)iNeighborCount;
			vAlignment = vAvgMov - SVector2( sShip0.m_vMov.x, sShip0.m_vMov.y );

			vAvgPos /= (float)iNeighborCount;
			vCohesion = vAvgPos - SVector2( sShip0.m_vPos.x, sShip0.m_vPos.y );
		}

		SVector2 vBoidMov =	vSeparation * 600.0f +
							vAlignment * 2000.0f +
							vCohesion * 1.3f;

		sShip0.m_vBoidMov += SVector3( vBoidMov.x, vBoidMov.y, 0.0f );
	}

	for ( size_t i0 = 0; i0 < GetShipCount(); i0++ )
	{
		//if ( i0 == m_iPlayerShipInd ) continue;

		SShip& sShip0 = GetShip( i0 );

		SVector2 vAsteroidField( 0.0f, 0.0f );

		const auto aNeighbors = m_cHashGridAsteroids.Get3x3Neighbors( sShip0.m_vPos.xy() );
		for ( size_t i = 0; i < aNeighbors.size(); ++i )
		{
			const std::vector< size_t >* pAsteroidInds = aNeighbors[i];
			if ( !pAsteroidInds )
			{
				continue;
			}
			for ( size_t j = 0; j < pAsteroidInds->size(); j++ )
			{
				size_t i1 = (*pAsteroidInds)[j];
				SAsteroid& sAsteroid1 = GetAsteroid( i1 );
						
				//if ( i0 != m_iPlayerShipInd )
				{
					SVector2 vField;
					//GetField_Asteroid( vField, SVector2( sShip0.m_vPos.x, sShip0.m_vPos.y ), sShip0, sAsteroid1 );
					//vSeparation += vField*2.0f;

					GetField_Asteroid2( vField, SVector2( sShip0.m_vPos.x, sShip0.m_vPos.y ), sShip0, sAsteroid1 );
					vAsteroidField += vField;
				}

				SVector2 vDist( sAsteroid1.m_vPos.x - sShip0.m_vPos.x, sAsteroid1.m_vPos.y - sShip0.m_vPos.y );
				float fDistSq = SVector2::LengthSq( vDist );
				if ( fDistSq < sAsteroid1.m_fSize * sAsteroid1.m_fSize )
				{
					float fDamage = 0.8f * fElapsedTimeMs;
					if ( sShip0.m_iID == GetShipIDPlayer() )
					{
						fDamage = 0.05f * fElapsedTimeMs;
					}
					if ( _onDamageShipByCollision( sShip0, fDamage ) )
					{
						/*if ( fDistSq > 0.0f )
						{
							SVector2 vAttackDir( vDist/sqrtf(fDistSq) );
							float fAttackForce = SVector2::Dot( vAttackDir, sShip0.m_vMov.xy() );
							SVector2 vMov = vAttackDir / sAsteroid1.m_fMass * sShip0.m_fMass;
							sAsteroid1.m_vMov.xy() += vMov;
						}*/
					}
				}
			}
		}

		const float fHeight = 10.0f;
		float f = SVector2::Cross( sShip0.m_vMov.xy(), vAsteroidField );
		if ( f >= 0.0f )
		{
			f = std::max( 0.0f, (fHeight - (+f)) / fHeight );
		}
		else
		{
			f = -std::max( 0.0f, (fHeight - (-f)) / fHeight );
		}
		f = Clamp( f, -1.0f, 1.0f );
		vAsteroidField = SVector2( -sShip0.m_vMov.y, sShip0.m_vMov.x ) * f * SVector2::Length( vAsteroidField );

		SVector2 vBoidMov = vAsteroidField * 5000.0f;


		sShip0.m_vBoidMov += SVector3( vBoidMov.x, vBoidMov.y, 0.0f );
	}
}

void CActors::_updateShips()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	SShip& sShipPlayer = GetShipPlayer();

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShip = GetShip( i );

		if ( sShip.m_iID == GetShipIDPlayer() )
		{
			float fYawMultiplier = sShip.m_sTurret.m_bShoot ? 0.5f : 1.0f;

			sShip.m_fYawSpeed = SmoothConverge( sShip.m_fYawSpeed, sShip.m_fYaw_ctrl * fYawMultiplier, 1.01f, 1.01f, fElapsedTimeMs );
			sShip.m_fAccForward = SmoothConverge( sShip.m_fAccForward, sShip.m_fAccForward_ctrl, 1.01f, 1.01f, fElapsedTimeMs );
			sShip.m_fAccRight = SmoothConverge( sShip.m_fAccRight, sShip.m_fAccRight_ctrl, 1.001f, 1.01f, fElapsedTimeMs );

			sShip.m_fYaw += sShip.m_fYawSpeed * 0.004f * fElapsedTimeMs;
			SVector3 vShipForward( cosf( sShip.m_fYaw ), sinf( sShip.m_fYaw ), 0.0f );
			SVector3 vShipRight( -vShipForward.y, vShipForward.x, 0.0f );
			sShip.m_vMov += vShipForward * sShip.m_fAccForward * 0.0001f * fElapsedTimeMs;
			sShip.m_vMov += vShipRight * sShip.m_fAccRight * 0.0001f * fElapsedTimeMs;

			// m_vMov felbontasa m_vDir es m_vRight iranyara, hogy a ship ne tudjon "csuszni" a levegoben
			SVector3 vMovForward( sShip.m_vDir );
			vMovForward = vMovForward * SVector3::Dot( sShip.m_vMov, vMovForward );
			SVector3 vMovRight( -sShip.m_vDir.y, sShip.m_vDir.x, 0.0f );
			vMovRight = vMovRight * SVector3::Dot( sShip.m_vMov, vMovRight );
			vMovRight = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), vMovRight, CalcSmoothUpdateWeight( 1.0f + fabsf( sShip.m_fAccForward ) * 0.0005f, fElapsedTimeMs ) );
			sShip.m_vMov = vMovForward + vMovRight;

			//sShip.m_vMov += sShip.m_vBoidMov * 0.001f;

			float fSpeedWeight = 1.00005f + SVector3::LengthSq( sShip.m_vMov ) * 0.05f;// * fabsf( sShip.m_fAccForward );
			sShip.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), sShip.m_vMov, CalcSmoothUpdateWeight( fSpeedWeight, fElapsedTimeMs ) );

			//m_fRoll = m_fYaw*0.5f;
			sShip.m_fRoll = Lerp( sShip.m_fYawSpeed, sShip.m_fRoll, CalcSmoothUpdateWeight( 1.01f, fElapsedTimeMs ) );
			//m_fRoll = SmoothConverge( m_fRoll, -m_fYaw * 3.5f, 1.0002f, 1.0002f, fElapsedTimeMs );
		}
		else
		{
			SVector3 vEnemyToPlayerDir( sShipPlayer.m_vPos - sShip.m_vPos );
			const float fEnemyToPlayerDist = SVector3::Length( vEnemyToPlayerDir );
			SVector3::Normalize( vEnemyToPlayerDir, vEnemyToPlayerDir );
			const float fSin_Phase_01 = sinf( sShip.m_fPhase_01 );

			SVector2 vField( 0.0f, 0.0f );
			GetField_ShipPlayer( vField, SVector2( sShip.m_vPos.x, sShip.m_vPos.y ), sShipPlayer );
			vField *= ( fSin_Phase_01 * 0.5f + 0.5f ) * 0.7f + 0.3f;
			//const float fFollowAmount = Clamp( (fEnemyToPlayerDist-Lerp(20.0f, 110.0f, fSin_Phase_01))*0.02f, -0.4f, 1.0f );
			const float fFollowAmount = 0.2f;
			SVector3 vFollowMov = SVector3( vField.x, vField.y, 0.0f );

			// ha allunk az urhajoval ne alljanak kukan egy helybe
			//SVector2 vRotateFollowMov( -vEnemyToPlayerDir.y, vEnemyToPlayerDir.x );
			//SVector2::Normalize( vRotateFollowMov, vRotateFollowMov );

			SVector3 vMov = sShip.m_vBoidMov * 0.001f + vFollowMov * fFollowAmount;// + SVector3( vRotateFollowMov, 0.0f ) * 0.05f;

			sShip.m_vMov = Lerp( vMov, sShip.m_vMov, CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );

			sShip.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), sShip.m_vMov, CalcSmoothUpdateWeight( 1.000002f, fElapsedTimeMs ) );

			const float fYaw = atan2f( sShip.m_vMov.y, sShip.m_vMov.x );
			const float fYawPrev = sShip.m_fYaw;
			sShip.m_fYaw = LerpAngle( fYaw, sShip.m_fYaw, CalcSmoothUpdateWeight( 1.05f, fElapsedTimeMs ) );
			
			sShip.m_fYawSpeed = (sShip.m_fYaw - fYawPrev) / (fElapsedTimeMs * 0.004f);			

			sShip.m_fRoll = Lerp( -sShip.m_fYawSpeed, sShip.m_fRoll, CalcSmoothUpdateWeight( 1.002f, fElapsedTimeMs ) );


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

			sShip.m_fPhase_01 += fElapsedTimeMs * 0.001f;
			if ( sShip.m_fPhase_01 > PI2 ) sShip.m_fPhase_01 -= PI2;
		}
	}

	for ( size_t i = 0; i < GetShipCount(); i++ )
	{
		SShip& sShip = GetShip( i );

		if ( sShip.m_iID == GetShipIDPlayer() ) continue;		

		for ( int iBulletInd = 0; iBulletInd < sShipPlayer.m_sTurret.m_aBullets.size(); iBulletInd++ )
		{
			STurret::SBullet& sBullet = sShipPlayer.m_sTurret.m_aBullets[iBulletInd];
			SVector2 vBulletPosPrev( sBullet.m_vPosPrev.x, sBullet.m_vPosPrev.y );
			SVector2 vBulletPos( sBullet.m_vPos.x, sBullet.m_vPos.y );
			float fT = 0.0f;
			if ( SegmentSphereTest( vBulletPosPrev, vBulletPos, SVector2( sShip.m_vPos.x, sShip.m_vPos.y ), sShip.m_fSize, fT ) )
			{
				SVector2 vSegmentDir( vBulletPos - vBulletPosPrev );
				SVector2::Normalize( vSegmentDir, vSegmentDir );
				SVector2 vAttackPoint( vBulletPosPrev + vSegmentDir * fT );
				float fAttackForce = SVector2::Dot( vSegmentDir, SVector2( sBullet.m_vMov.x, sBullet.m_vMov.y ) );
				SVector2 vMov = (sShip.m_vPos.xy() - vAttackPoint) * fAttackForce / sShip.m_fMass * sBullet.m_fMass;
				sShip.m_vMov.x += vMov.x;
				sShip.m_vMov.y += vMov.y;

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

	for ( size_t i = 0; i < GetAsteroidCount(); i++ )
	{
		SAsteroid& sAsteroid = GetAsteroid( i );
		for ( int iBulletInd = 0; iBulletInd < sShipPlayer.m_sTurret.m_aBullets.size(); iBulletInd++ )
		{
			STurret::SBullet& sBullet = sShipPlayer.m_sTurret.m_aBullets[iBulletInd];
			SVector2 vBulletPosPrev( sBullet.m_vPosPrev.x, sBullet.m_vPosPrev.y );
			SVector2 vBulletPos( sBullet.m_vPos.x, sBullet.m_vPos.y );
			float fT = 0.0f;
			if ( SegmentSphereTest( vBulletPosPrev, vBulletPos, SVector2( sAsteroid.m_vPos.x, sAsteroid.m_vPos.y ), sAsteroid.m_fSize, fT ) )
			{
				/*SVector2 vSegmentDir( vBulletPos - vBulletPosPrev );
				SVector2::Normalize( vSegmentDir, vSegmentDir );
				SVector2 vAttackPoint( vBulletPosPrev + vSegmentDir * fT );
				float fAttackForce = SVector2::Dot( vSegmentDir, SVector2( sBullet.m_vMov.x, sBullet.m_vMov.y ) );
				SVector2 vMov = ( sAsteroid.m_vPos.xy() - vAttackPoint ) * fAttackForce / sAsteroid.m_fMass * sBullet.m_fMass;
				sAsteroid.m_vMov.x += vMov.x;
				sAsteroid.m_vMov.y += vMov.y;*/

				SAudioEvent sAudioEvent;
				sAudioEvent.type = SAudioEvent::GunHit;
				sAudioEvent.fVolume = 0.15f;
				sAudioEvent.iTimeStampNs = CEngine::GetInstance().GetTimeStampNs();
				sAudioEvent.iLifeTimeNs = 1000 * 1000 * 300;
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


	for ( size_t i = 0; i < GetShipCount(); )
	{
		SShip& sShip = GetShip( i );
		if ( sShip.m_iID != GetShipIDPlayer() )
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
		sShip.Update();
	}
}

/*void CActors::_updateAsteroids()
{
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	for ( size_t i = 0; i < GetAsteroidCount(); i++ )
	{
		SAsteroid& sAsteroid = GetAsteroid( i );
		sAsteroid.m_vPos += sAsteroid.m_vMov * fElapsedTimeMs;
		sAsteroid.m_vMov = Lerp( SVector3( 0.0f, 0.0f, 0.0f ), sAsteroid.m_vMov, CalcSmoothUpdateWeight( 1.001f, fElapsedTimeMs ) );
	}
}*/

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
	m_vBoidMov = SVector3( 0.0f, 0.0f, 0.0f );
	m_vMov = SVector3( 0.0f, 0.0f, 0.0f );
	m_vMovPrev = m_vMov;	
	m_vDir = SVector3( 1.0f, 0.0f, 0.0f );
	m_vDirPrev = m_vDir;
	m_vUp = SVector3( 0.0f, 0.0f, 1.0f );
	m_vRight = SVector3( 0.0f, 1.0f, 0.0f );
	SMatrix::Identity( m_matShip );
	m_matShipPrev = m_matShip;

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

void SShip::Update()
{
	m_vMovPrev = m_vMov;
	m_vDirPrev = m_vDir;
	m_matShipPrev = m_matShip;
	SMatrix::BuildEulerXYZ( m_matShip, m_fRoll, 0.0f, m_fYaw );
	m_matShip.m30 = m_vPos.x;	m_matShip.m31 = m_vPos.y;	m_matShip.m32 = m_vPos.z;	m_matShip.m33 = 1.0f;

	m_vDir.x = m_matShip.m00;	m_vDir.y = m_matShip.m01;	m_vDir.z = m_matShip.m02;
	m_vUp.x = m_matShip.m20;	m_vUp.y = m_matShip.m21;	m_vUp.z = m_matShip.m22;
	SVector3::Cross( m_vRight, m_vDir, m_vUp );	
	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();
	m_vPos += m_vMov * fElapsedTimeMs;

	m_fDamageTimerMs = std::max( m_fDamageTimerMs - fElapsedTimeMs, 0.0f );

	m_sTurret.Update( *this );
}

////////////////////////////////////////////////////////////////

void STurret::Clear()
{
	m_aTurretPositions.clear();
	m_iBulletCounter = 0;

	m_fShootFreqHz = 1.0f;
	m_fBulletSpeed = 0.1f;
	m_vColor = SVector3( 1.0f, 1.0f, 1.0f );

	m_fDamage = 1.0f;

	m_aBullets.clear();
	m_bShoot = false;
	m_iLastBulletTimeStampNs = 0;
}

void STurret::Update( const SShip& sShip )
{
	if ( m_bShoot )
	{
		SVector3 vGunDir( sShip.m_vDir );
		SVector3 vGunDirPrev( sShip.m_vDirPrev );

		SVector3 vGunMov( sShip.m_vMov );
		SVector3 vGunMovPrev( sShip.m_vMovPrev );

		const float fShootFreqHz = m_fShootFreqHz;
		const uint64_t iShootPeriodNs = (uint64_t)(1.0f / fShootFreqHz * 1000.0f * 1000.0f * 1000.0f);

		uint64_t iTNs = m_iLastBulletTimeStampNs;
		for ( ; iTNs < CEngine::GetInstance().GetTimeStampNs(); iTNs += iShootPeriodNs )
		{
			if ( m_aTurretPositions.size() == 0 )
			{
				break;
			}			

			SVector3 vGunPosWorld;
			SVector3 vGunPosWorldPrev;
			{
				SVector3 vGunPos = m_aTurretPositions[m_iBulletCounter%m_aTurretPositions.size()];
				SMatrix::TransformCoord( vGunPosWorld, vGunPos, sShip.m_matShip );
				SMatrix::TransformCoord( vGunPosWorldPrev, vGunPos, sShip.m_matShipPrev );
			}

			float fFrameW = (float)(iTNs - m_iLastBulletTimeStampNs) / (float)(CEngine::GetInstance().GetTimeStampNs() - m_iLastBulletTimeStampNs);

			m_aBullets.emplace_back();
			SBullet& sBullet = m_aBullets.back();
			sBullet.m_vPos = Lerp( vGunPosWorldPrev, vGunPosWorld, fFrameW );

			SVector3 vBulletDir = Lerp( vGunDirPrev, vGunDir, fFrameW );
			sBullet.m_vMov = Lerp( vGunMovPrev, vGunMov, fFrameW ) + vBulletDir * m_fBulletSpeed;

			sBullet.m_fMass = 1.0f;

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

			m_iBulletCounter++;
		}

		m_iLastBulletTimeStampNs = iTNs;
	}

	float fElapsedTimeMs = CEngine::GetInstance().GetElapsedTimeMs();

	for ( size_t iBulletInd = 0; iBulletInd < m_aBullets.size(); )
	{
 		SBullet& sBullet = m_aBullets[iBulletInd];
		sBullet.m_fTimer += fElapsedTimeMs;
		if ( sBullet.m_fTimer > sBullet.m_fTime )
		{
			m_aBullets[iBulletInd] = m_aBullets.back();
			m_aBullets.pop_back();
			continue;
		}
		sBullet.m_vPosPrev = sBullet.m_vPos;
		sBullet.m_vPos += sBullet.m_vMov * fElapsedTimeMs;

		++iBulletInd;
	}
}

////////////////////////////////////////////////////////////////

void SLaserGun::Clear()
{
	m_vGunPosition = SVector3( 0.0f, 0.0f, 0.0f );

	m_vColor = SVector3( 1.0f, 1.0f, 1.0f );

	m_bShoot = false;
}