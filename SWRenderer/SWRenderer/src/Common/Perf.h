#pragma once

#include <chrono>

class CPerf
{
public:
	using Clock = std::chrono::steady_clock;

	void BeginPerf()
	{
		m_TimeStart = Clock::now();
	}

	uint64_t EndPerfNs()
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>( Clock::now() - m_TimeStart ).count();
	}

private:
	Clock::time_point m_TimeStart;
};