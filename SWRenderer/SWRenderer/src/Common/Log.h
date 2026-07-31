#pragma once

#ifdef _DEBUG

#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <cstdarg>

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

#define LOG(...)

#endif