#pragma once

#include <cstdint>
#include "Vector.h"

class CBData
{
public:
	CBData();
	~CBData();

	void			Clear();
	void			SetToReadMode();

	uint32_t			GetValidSize() const;
	const uint8_t*		GetDataPointer() const;

	//void			Write( const CString& sData );
	//void			Write( const TCHAR* pData );
	//void			Read( CString& sData );

	void			Write( const void* pData, uint32_t dwSize );
	void			Read( void* pData, uint32_t dwSize );

	void			Write( const uint16_t& wData );
	void			Read( uint16_t& wData );

	void			Write( const uint32_t& dwData );
	void			Read( uint32_t& dwData );

	void			Write( const uint8_t& bData );
	void			Read( uint8_t& bData );

	void			Write( const float& fData );
	void			Read( float& fData );

	void			Write( const int& iData );
	void			Read( int& iData );

	void			Write( const bool& bData );
	void			Read( bool& bData );

	void			Write( const SVector3& vData );
	void			Read( SVector3& vData );

	void			Write( const SVector4& vData );
	void			Read( SVector4& vData );

	bool			OpenFromFile( const char* szFilename );
	bool			OpenFromMemory( uint8_t* pData, uint32_t dwSize );
	bool			SaveToFile( const char* szFilename ) const;
	void			SaveToMemory( uint8_t*& pData, uint32_t& dwSize ) const;

	uint32_t		GetVersion() const { return m_dwVersion; }
	void			SetVersion( uint32_t dwVersion ) { m_dwVersion = dwVersion; }
	static uint32_t	GetAppVersion() { return g_dwAppVersion; }

private:
	void			Allocate( uint32_t dwMin );

private:
	uint8_t*		m_pData;		// unified data
	uint32_t		m_dwSize;		// m_pData teljes merete ( mindig kicsit tobb van lefoglalva )
	uint32_t		m_dwIOPointer;	// az adat iras/olvasas az m_pData-bol epp hol tart
	uint32_t		m_dwValidSize;	// az m_pDataban levo ertelmes adatok merete
	uint32_t				m_dwVersion;	// megnyitott/elmentett fileokhoz verziozas
	static const uint32_t	g_dwAppVersion;	// a progi verzioja
};