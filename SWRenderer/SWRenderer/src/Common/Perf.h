#pragma once

class CPerf
{
public:
	CPerf()
	{
		ZeroMemory( &m_liTimeStart, sizeof( LARGE_INTEGER ) );
		ZeroMemory( &m_liTimeEnd, sizeof( LARGE_INTEGER ) );

		LARGE_INTEGER liOneSec;
		QueryPerformanceFrequency(&liOneSec);
		m_fOneSec = (double)liOneSec.QuadPart;
	}
	~CPerf()
	{
	}

	void BeginPerf()
	{
		QueryPerformanceCounter(&m_liTimeStart);
	}

	double EndPerf()
	{
		QueryPerformanceCounter(&m_liTimeEnd);
		return (double)(m_liTimeEnd.QuadPart - m_liTimeStart.QuadPart) / m_fOneSec;
	}

private:
	double			m_fOneSec;
	LARGE_INTEGER	m_liTimeStart;
	LARGE_INTEGER	m_liTimeEnd;
};
