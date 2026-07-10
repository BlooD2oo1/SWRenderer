#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <vector>

#include "Common/Math.h"

#define SAFE_DELETE(p)       { if(p) { delete	(p);   (p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=nullptr; } }


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
#endif
