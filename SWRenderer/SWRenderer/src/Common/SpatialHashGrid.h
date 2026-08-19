#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <utility>
#include "Vector.h"

#include "Common/flat_hash_map.hpp"

template <typename TType>
class CSpatialHashGrid
{
	using SpatialHash = uint32_t;
public:
	explicit CSpatialHashGrid( float fGridSize )
		: m_fGridSize( fGridSize )
		, m_fInvGridSize( fGridSize > 0.0f ? ( 1.0f / fGridSize ) : 1.0f )
	{
	}

	// Clears vector contents without deallocating memory capacity (zero allocations per frame)
	void SoftClear()
	{
		for ( auto& pair : m_mapHashGrid )
		{
			pair.second.clear();
		}
	}

	// Completely removes all buckets and deallocates memory
	void Clear()
	{
		m_mapHashGrid.clear();
	}

	float GetGridSize() const { return m_fGridSize; }

	void Add( const SVector2& vPos, const TType& obj )
	{
		SpatialHash iHash = _getHash( vPos );
		m_mapHashGrid[iHash].push_back( obj );
	}

	void Add( const SVector2& vPos, TType&& obj )
	{
		SpatialHash iHash = _getHash( vPos );
		m_mapHashGrid[iHash].push_back( std::move( obj ) );
	}

	const std::vector< TType >* Find( SpatialHash iHash ) const
	{
		auto it = m_mapHashGrid.find( iHash );
		if ( it != m_mapHashGrid.end() )
			return &it->second;
		return nullptr;
	}

	// Returns pointers to the 3x3 neighbor vectors on the stack (zero dynamic memory allocation)
	std::array< const std::vector< TType >*, 9 > Get3x3Neighbors( const SVector2& vPos ) const
	{
		std::array< const std::vector< TType >*, 9 > neighbors;
		int32_t iCenterX, iCenterY;
		_getGridCoords( iCenterX, iCenterY, vPos );

		uint32_t iIdx = 0;
		for ( int32_t iY = -1; iY <= 1; ++iY )
		{
			for ( int32_t iX = -1; iX <= 1; ++iX )
			{
				SpatialHash iHash = _getHash( iCenterX + iX, iCenterY + iY );
				neighbors[iIdx++] = Find( iHash );
			}
		}

		return neighbors;
	}

private:
	inline void _getGridCoords( int32_t& iX, int32_t& iY, const SVector2& vPos ) const
	{
		iX = (int32_t)floorf( vPos.x * m_fInvGridSize );
		iY = (int32_t)floorf( vPos.y * m_fInvGridSize );
	}

	// Prime-based spatial hashing without magic shifts or complex sign handling
	inline SpatialHash _getHash( int32_t iX, int32_t iY ) const
	{
		return ( (uint32_t)iX * 73856093u ) ^ ( (uint32_t)iY * 19349663u );
	}

	inline SpatialHash _getHash( const SVector2& vPos ) const
	{
		int32_t iX, iY;
		_getGridCoords( iX, iY, vPos );
		return _getHash( iX, iY );
	}

private:
	const float m_fGridSize;
	const float m_fInvGridSize;
	ska::flat_hash_map< SpatialHash, std::vector< TType > > m_mapHashGrid;
};