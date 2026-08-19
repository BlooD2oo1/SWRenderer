#pragma once

#include <vector>
#include "Vector.h"

#define SKA
#ifdef SKA
#include "Common/flat_hash_map.hpp"
#define spatial_hash_map ska::flat_hash_map
#else
#include <unordered_map>
#define spatial_hash_map std::unordered_map
#endif

using SpatialHash = uint32_t;
template <typename TType>
class CSpatialHashGrid
{
	CSpatialHashGrid( float fGridSize )
		: m_fGridSize( fGridSize )
	{
	}

	void Clear()
	{

	}

	float	GetGridSize() const { return m_fGridSize; }

	void Add( SVector2 vPos, const TType& obj )
	{
		SpatialHash iHash = _getHash( vPos );
		m_mapHashGrid[iHash].push_back( obj );
	}

	std::vector< TType >* Find( SpatialHash iHash )
	{
		auto it = m_mapHashGrid.find( iHash );
		if ( it != m_mapHashGrid.end() )
			return &it->second;
		return nullptr;
	}

private:
	inline void _getHash( int32_t& iHashX, int32_t& iHashY, const SVector2& vPos )
	{
		iHashX = (int32_t)floorf( vPos.x / m_fGridSize );
		iHashY = (int32_t)floorf( vPos.y / m_fGridSize );
	}
	inline SpatialHash _getHash( const SVector2& vPos )
	{
		int32_t iHashX;
		int32_t iHashY;
		_getHash( iHashX, iHashY, vPos );
		SpatialHash iHash = ((SpatialHash)iHashX << 16) | ((SpatialHash)iHashY & 0xFFFF);
		return iHash;
	}
	SpatialHash _getHash( int32_t iHashX, int32_t iHashY )
	{
		SpatialHash iHash = ((SpatialHash)iHashX << 16) | ((SpatialHash)iHashY & 0xFFFF);
		return iHash;
	}

private:
	const float m_fGridSize;
	spatial_hash_map< SpatialHash, std::vector< TType > > m_mapHashGrid;
};