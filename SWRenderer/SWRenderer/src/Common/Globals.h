#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <vector>
#include <chrono>
#include <atomic>

#include "Common/Math.h"

static std::atomic<bool> g_bRunning( true );

#define KEY_ESCAPE		0x1B
#define KEY_SPACE		0x20
#define KEY_ENTER		0x0D
#define KEY_LEFT		0x25
#define KEY_UP			0x26
#define KEY_RIGHT		0x27
#define KEY_DOWN		0x28


#define SAFE_DELETE(p)       { if(p) { delete	(p);   (p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=nullptr; } }

inline uint64_t GetGlobalTimeStampNs()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::steady_clock::now().time_since_epoch()
	).count();
}

#ifdef _DEBUG
#define NOMINMAX
#include <windows.h>
inline void LOG(const char* fmt, ...)
{
	char buf[1024];

	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	OutputDebugStringA(buf);
}
#else
#define LOG(fmt, ...)
#endif
