#include "BData.h"

#include <cstdio>
#include <cstring>
#include <cstdint>

#include "Globals.h"

#define BDATA_MAX_STRING_LEN	(uint32_t)4095		// TCHAR-ban szamolva ( plusz a '\0' )

const uint32_t CBData::g_dwAppVersion = 1;

//55-56-57-58 version bug

CBData::CBData()
{
	m_pData = nullptr;
	Clear();
}

CBData::~CBData()
{
	Clear();
}

void CBData::Clear()
{
	SAFE_DELETE_ARRAY( m_pData );
	m_dwIOPointer = 0;
	m_dwSize = 0;
	m_dwValidSize = 0;
	m_dwVersion = g_dwAppVersion;
}

void CBData::SetToReadMode()
{
	m_dwIOPointer = 0;
}

void CBData::Allocate( uint32_t dwMin )
{
	if ( m_dwSize == 0 )
	{
		m_dwSize = std::max( dwMin, (uint32_t)1024 );
		m_pData = new uint8_t[m_dwSize];
		memset( m_pData, 0, m_dwSize );
	}
	else
	{
		m_dwSize = std::max( m_dwSize*2, m_dwSize+dwMin );
		uint8_t* pNewData = new uint8_t[m_dwSize];
		memset( pNewData, 0, m_dwSize );
		memcpy( pNewData, m_pData, m_dwIOPointer );
		SAFE_DELETE_ARRAY( m_pData );
		m_pData = pNewData;
		pNewData = NULL;
	}
}

uint32_t CBData::GetValidSize() const
{
	return m_dwValidSize;
}

const uint8_t* CBData::GetDataPointer() const
{
	return m_pData;
}
/*
void CBData::Write( const CString& sData )
{
	uint16_t wStringLen = (uint16_t)sData.GetLength();

	wStringLen = min( wStringLen, BDATA_MAX_STRING_LEN );
	
	uint16_t wLen = wStringLen * sizeof(TCHAR);

	uint16_t wAllDataLen = sizeof( uint16_t ) + wLen + sizeof(TCHAR);

	if ( m_dwSize - m_dwIOPointer < wAllDataLen )
	{
		Allocate( wAllDataLen );
	}

	if ( m_dwSize - m_dwIOPointer < wAllDataLen )
	{
		assert( 0 );
	}

	// a string karektereinek szama ( a '\0'-val egyutt ):
	uint16_t* pwDataLen = (uint16_t*)&m_pData[m_dwIOPointer];
	*pwDataLen = wLen + sizeof( TCHAR );
	m_dwIOPointer += sizeof( uint16_t );
	m_dwValidSize += sizeof( uint16_t );

	// a string karakterei:
	//memcpy( &m_pData[m_dwIOPointer], (uint8_t*)(const TCHAR*)sData, wLen );
	const TCHAR* pxstr = sData.GetString();
	memcpy( &m_pData[m_dwIOPointer], (uint8_t*)pxstr, wLen );
	m_dwIOPointer += wLen;
	m_dwValidSize += wLen;

	// a lezaro '\0' a vegere:
	TCHAR cNull = _T ( '\0' );
	memcpy( &m_pData[m_dwIOPointer], &cNull, sizeof( TCHAR ) );
	m_dwIOPointer += sizeof( TCHAR );
	m_dwValidSize += sizeof( TCHAR );
}

void CBData::Write( const TCHAR* pData )
{
	CString sData = pData;
	Write( sData );
}

void CBData::Read( CString& sData )
{
	if ( m_dwValidSize <= m_dwIOPointer )	// elo-ellenorzes a biztonsag kedveer
	{
		assert( 0 );
		return;
	}

	uint16_t* pwDataLen = (uint16_t*)&m_pData[m_dwIOPointer];
	uint16_t wLen = *pwDataLen;				// kiolvassuk a string meretet ( lezaro 0-al egyutt )
	m_dwIOPointer += sizeof( uint16_t );

	if ( m_dwValidSize < m_dwIOPointer + wLen )
	{
		assert( 0 );	// itt valami orjasi baj fog tortenni a tovabbi read-eknel
		return;
	}

	TCHAR* cMsg = new TCHAR[wLen];
	ZeroMemory( cMsg, wLen );
	memcpy( (uint8_t*)cMsg, &m_pData[m_dwIOPointer], wLen );	// kimasoljuk a databol a string-et ( lezaro 0-al egyutt )
	
	if ( cMsg[wLen/sizeof(TCHAR) - 1] != _T( '\0' ) )
	{
		assert( 0 );	// nem talaltuk meg a lezaro 0-t, itt valami orjasi baj fog tortenni a tovabbi read-eknel
		cMsg[wLen-1] = _T( '\0' );
	}

	m_dwIOPointer += wLen;
	sData = cMsg;
	SAFE_DELETE_ARRAY( cMsg );
}
*/
void CBData::Write( const void* pData, uint32_t dwSize )
{
	if ( m_dwSize - m_dwIOPointer < dwSize )
	{
		Allocate( dwSize );
	}

	if ( m_dwSize - m_dwIOPointer < dwSize )
	{
		assert( 0 );
	}

	memcpy( &m_pData[m_dwIOPointer], pData, dwSize );
	m_dwIOPointer += dwSize;
	m_dwValidSize += dwSize;
}

void CBData::Read( void* pData, uint32_t dwSize )
{
	if ( m_dwValidSize < m_dwIOPointer + dwSize )
	{
		assert( 0 );
		return;
	}

	memcpy( (uint8_t*)pData, &m_pData[m_dwIOPointer], dwSize );
	m_dwIOPointer += dwSize;
}

void CBData::Write( const uint32_t& dwData )
{
	if ( m_dwSize - m_dwIOPointer < sizeof( uint32_t ) )
	{
		Allocate( sizeof( uint32_t ) );
	}

	if ( m_dwSize - m_dwIOPointer < sizeof( uint32_t ) )
	{
		assert( 0 );
	}

	memcpy( &m_pData[m_dwIOPointer], &dwData, sizeof( uint32_t ) );
	m_dwIOPointer += sizeof( uint32_t );
	m_dwValidSize += sizeof( uint32_t );
}

void CBData::Read( uint32_t& dwData )
{
	uint32_t dwTypeSize = sizeof( uint32_t );

	if ( m_dwValidSize < m_dwIOPointer + dwTypeSize )
	{
		assert( 0 );
		return;
	}


	memcpy( (uint8_t*)&dwData, &m_pData[m_dwIOPointer], dwTypeSize );
	m_dwIOPointer += dwTypeSize;
}


void CBData::Write( const uint16_t& wData )
{
	if ( m_dwSize - m_dwIOPointer < sizeof( uint16_t ) )
	{
		Allocate( sizeof( uint16_t ) );
	}

	if ( m_dwSize - m_dwIOPointer < sizeof( uint16_t ) )
	{
		assert( 0 );
	}

	memcpy( &m_pData[m_dwIOPointer], &wData, sizeof( uint16_t ) );
	m_dwIOPointer += sizeof( uint16_t );
	m_dwValidSize += sizeof( uint16_t );
}

void CBData::Read( uint16_t& wData )
{
	uint32_t dwTypeSize = sizeof( uint16_t );

	if ( m_dwValidSize < m_dwIOPointer + dwTypeSize )
	{
		assert( 0 );
		return;
	}


	memcpy( (uint8_t*)&wData, &m_pData[m_dwIOPointer], dwTypeSize );
	m_dwIOPointer += dwTypeSize;
}

void CBData::Write( const uint8_t& bData )
{
	if ( m_dwSize - m_dwIOPointer < sizeof( uint8_t ) )
	{
		Allocate( sizeof( uint8_t ) );
	}

	if ( m_dwSize - m_dwIOPointer < sizeof( uint8_t ) )
	{
		assert( 0 );
	}

	memcpy( &m_pData[m_dwIOPointer], &bData, sizeof( uint8_t ) );
	m_dwIOPointer += sizeof( uint8_t );
	m_dwValidSize += sizeof( uint8_t );
}

void CBData::Read( uint8_t& bData )
{
	uint32_t dwTypeSize = sizeof( uint8_t );

	if ( m_dwValidSize < m_dwIOPointer + dwTypeSize )
	{
		assert( 0 );
		return;
	}

	memcpy( (uint8_t*)&bData, &m_pData[m_dwIOPointer], dwTypeSize );
	m_dwIOPointer += dwTypeSize;
}

void CBData::Write( const float& fData )
{
	if ( m_dwSize - m_dwIOPointer < sizeof( float ) )
	{
		Allocate( sizeof( float ) );
	}

	if ( m_dwSize - m_dwIOPointer < sizeof( float ) )
	{
		assert( 0 );
	}

	memcpy( &m_pData[m_dwIOPointer], &fData, sizeof( float ) );
	m_dwIOPointer += sizeof( float );
	m_dwValidSize += sizeof( float );
}

void CBData::Read( float& fData )
{
	uint32_t dwTypeSize = sizeof( float );	

	if ( m_dwValidSize < m_dwIOPointer + dwTypeSize )
	{
		assert( 0 );
		return;
	}

	memcpy( (uint8_t*)&fData, &m_pData[m_dwIOPointer], dwTypeSize );
	m_dwIOPointer += dwTypeSize;
}

void CBData::Write( const int& iData )
{
	if ( m_dwSize - m_dwIOPointer < sizeof( int ) )
	{
		Allocate( sizeof( int ) );
	}

	if ( m_dwSize - m_dwIOPointer < sizeof( int ) )
	{
		assert( 0 );
	}

	memcpy( &m_pData[m_dwIOPointer], &iData, sizeof( int ) );
	m_dwIOPointer += sizeof( int );
	m_dwValidSize += sizeof( int );
}

void CBData::Read( int& iData )
{
	uint32_t dwTypeSize = sizeof( int );	

	if ( m_dwValidSize < m_dwIOPointer + dwTypeSize )
	{
		assert( 0 );
		return;
	}

	memcpy( (uint8_t*)&iData, &m_pData[m_dwIOPointer], dwTypeSize );
	m_dwIOPointer += dwTypeSize;
}

void CBData::Write( const bool& bData )
{
	if ( m_dwSize - m_dwIOPointer < sizeof( bool ) )
	{
		Allocate( sizeof( bool ) );
	}

	if ( m_dwSize - m_dwIOPointer < sizeof( bool ) )
	{
		assert( 0 );
	}

	memcpy( &m_pData[m_dwIOPointer], &bData, sizeof( bool ) );
	m_dwIOPointer += sizeof( bool );
	m_dwValidSize += sizeof( bool );
}

void CBData::Read( bool& bData )
{
	uint32_t dwTypeSize = sizeof( bool );	

	if ( m_dwValidSize < m_dwIOPointer + dwTypeSize )
	{
		assert( 0 );
		return;
	}

	memcpy( (uint8_t*)&bData, &m_pData[m_dwIOPointer], dwTypeSize );
	m_dwIOPointer += dwTypeSize;
}

void CBData::Write( const SVector3& vData )
{
	if ( m_dwSize - m_dwIOPointer < sizeof( SVector3 ) )
	{
		Allocate( sizeof( SVector3 ) );
	}

	if ( m_dwSize - m_dwIOPointer < sizeof( SVector3 ) )
	{
		assert( 0 );
	}

	memcpy( &m_pData[m_dwIOPointer], &vData, sizeof( SVector3 ) );
	m_dwIOPointer += sizeof( SVector3 );
	m_dwValidSize += sizeof( SVector3 );
}

void CBData::Read( SVector3& vData )
{
	uint32_t dwTypeSize = sizeof( SVector3 );	

	if ( m_dwValidSize < m_dwIOPointer + dwTypeSize )
	{
		assert( 0 );
		return;
	}

	memcpy( (uint8_t*)&vData, &m_pData[m_dwIOPointer], dwTypeSize );
	m_dwIOPointer += dwTypeSize;
}

void CBData::Write( const SVector4& vData )
{
	if ( m_dwSize - m_dwIOPointer < sizeof( SVector4 ) )
	{
		Allocate( sizeof( SVector4 ) );
	}

	if ( m_dwSize - m_dwIOPointer < sizeof( SVector4 ) )
	{
		assert( 0 );
	}

	memcpy( &m_pData[m_dwIOPointer], &vData, sizeof( SVector4 ) );
	m_dwIOPointer += sizeof( SVector4 );
	m_dwValidSize += sizeof( SVector4 );
}

void CBData::Read( SVector4& vData )
{
	uint32_t dwTypeSize = sizeof( SVector4 );	

	if ( m_dwValidSize < m_dwIOPointer + dwTypeSize )
	{
		assert( 0 );
		return;
	}

	memcpy( (uint8_t*)&vData, &m_pData[m_dwIOPointer], dwTypeSize );
	m_dwIOPointer += dwTypeSize;
}

bool CBData::OpenFromFile( const char* szFilename )
{
	Clear();
	
	FILE* pFile = nullptr;
	if ( fopen_s( &pFile, szFilename, "rb" ) != 0 || !pFile )
	{
		return false;
	}

	fseek( pFile, 0, SEEK_END );
	long dwFileLen = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );

	if ( dwFileLen <= 0 || (size_t)dwFileLen < sizeof( m_dwVersion ) )
	{
		fclose( pFile );
		return false;
	}

	// Verzió beolvasása
	if ( fread( &m_dwVersion, sizeof( m_dwVersion ), 1, pFile ) != 1 )
	{
		fclose( pFile );
		return false;
	}

	dwFileLen -= sizeof( m_dwVersion );

	m_dwSize = dwFileLen;
	m_dwValidSize = dwFileLen;
	m_pData = new uint8_t[m_dwSize];

	// Adatok beolvasása
	if ( fread( m_pData, 1, m_dwSize, pFile ) != m_dwSize )
	{
		fclose( pFile );
		return false;
	}

	fclose( pFile );

	return true;
}

bool CBData::OpenFromMemory( uint8_t* pData, uint32_t dwSize )
{
	Clear();
	if ( dwSize < sizeof( m_dwVersion ) )
	{
		return false;
	}
	memcpy( &m_dwVersion, pData, sizeof( m_dwVersion ) );
	m_dwSize = dwSize - sizeof( m_dwVersion );
	m_dwValidSize = m_dwSize;
	m_pData = new uint8_t[m_dwSize];
	memcpy( m_pData, pData + sizeof( m_dwVersion ), m_dwSize );
	return true;
}

bool CBData::SaveToFile( const char* szFilename ) const
{
	FILE* pFile = nullptr;
	if ( fopen_s( &pFile, szFilename, "wb" ) != 0 || !pFile )
	{
		return false;
	}

	// Verzió kiírása
	if ( fwrite( &g_dwAppVersion, sizeof( g_dwAppVersion ), 1, pFile ) != 1 )
	{
		fclose( pFile );
		return false;
	}

	// Adatok kiírása
	if ( fwrite( m_pData, 1, m_dwValidSize, pFile ) != m_dwValidSize )
	{
		fclose( pFile );
		return false;
	}

	fclose( pFile );

	return true;
}

void CBData::SaveToMemory( uint8_t*& pData, uint32_t& dwSize ) const
{
	SAFE_DELETE_ARRAY( pData );
	dwSize = sizeof( m_dwVersion ) + m_dwValidSize;
	pData = new uint8_t[dwSize];
	memcpy( pData, &m_dwVersion, sizeof( m_dwVersion ) );
	memcpy( pData + sizeof( m_dwVersion ), m_pData, m_dwValidSize );
}

