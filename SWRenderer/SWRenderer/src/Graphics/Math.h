#pragma once

#include <stdint.h>
#include <cmath>

#define PI05		1.5707963267948966192313216916398f
#define PI			3.1415926535897932384626433832795f
#define PI2			6.283185307179586476925286766559f
#define PIRECIP		0.31830988618379067153776752674503f
#define SQRT2		1.4142135623730950488016887242097f
#define E_NUMBER	2.7182818284590452353602874713527f

template< class T, class T2 >
constexpr T	Lerp( const T& f0, const T& f1, const T2 fX )
{ 
	//return ( T )( fA * ( 1.0f - fW ) + fB * fW );
	return (T)( f0 + fX * ( f1-f0 ) );

}

template< class T >
constexpr T	Lerp( const T& f00, const T& f01, const T& f10, const T& f11, const float fX, const float fY )
{ 
	float f0 = Lerp( f00, f10, fX );
	float f1 = Lerp( f01, f11, fX );

	return Lerp( f0, f1, fY );
}


template< class T >
constexpr T Clamp( const T& x, const T& a, const T& b )
{
	T ret = (((x) > (a)) ? (x) : (a));
	ret = (((ret) < (b)) ? (ret) : (b));
	return ret;
}

__forceinline static float CalcSmoothUpdateWeight(float fSmoothness, float fElapsedTime) { return powf(fSmoothness, -fElapsedTime); }
__forceinline static float CalcSmoothUpdateWeight(double fSmoothness, double fElapsedTime) { return (float)pow(fSmoothness, -fElapsedTime); }

static constexpr uint32_t Hash(uint32_t x, uint32_t y)
{
    uint32_t h = x * 0x8da6b343u + y * 0xd8163841u;
    h ^= (h >> 13);
    h *= 0xcb1ab31fu;
    h ^= (h >> 16);
    return h;
}

static constexpr float Rand(uint32_t x, uint32_t y)
{
    return (Hash(x, y) & 0xffffff) * (1.0f / 16777215.0f);
}

static constexpr float Smooth(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

static float Noise2DPeriodic(float x, float y, int period)
{
    int ix = (int)floorf(x);
    int iy = (int)floorf(y);

    float fx = x - ix;
    float fy = y - iy;

    int x0 = ix % period;
    int y0 = iy % period;
    int x1 = (x0 + 1) % period;
    int y1 = (y0 + 1) % period;

    if (x0 < 0) x0 += period;
    if (y0 < 0) y0 += period;

    float v00 = Rand(x0, y0);
    float v10 = Rand(x1, y0);
    float v01 = Rand(x0, y1);
    float v11 = Rand(x1, y1);

    float sx = Smooth(fx);
    float sy = Smooth(fy);

    float a = v00 + (v10 - v00) * sx;
    float b = v01 + (v11 - v01) * sx;

    return a + (b - a) * sy;
}
