#pragma once

#include <chrono>

inline uint64_t GetGlobalTimeStampNs()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::steady_clock::now().time_since_epoch()
	).count();
}

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