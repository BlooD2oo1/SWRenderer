#pragma once

#include <cmath>
#include "Common/Globals.h"
#include "Common/Vector.h"

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

constexpr float SmoothConverge( float fSrc, float fDst, float fIncSmoothness, float fDecSmoothness, float fElapsedTimeMs )
{
    return Lerp( fDst, fSrc, fDst > fSrc ? CalcSmoothUpdateWeight( fIncSmoothness, fElapsedTimeMs ) : CalcSmoothUpdateWeight( fDecSmoothness, fElapsedTimeMs ) );
}

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

static bool SegmentSphereTest( const SVector2& v0, const SVector2& v1, const SVector2& vCenter, float fRadiusSq, float& fT )
{
    SVector2 d = v1 - v0;
    SVector2 f = v0 - vCenter;
    float a = SVector2::Dot(d, d);
    float b = 2.0f * SVector2::Dot(f, d);
    float c = SVector2::Dot(f, f) - fRadiusSq;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        return false; // No intersection
    }
    discriminant = sqrtf(discriminant);
    float t1 = (-b - discriminant) / (2.0f * a);
    float t2 = (-b + discriminant) / (2.0f * a);
    if (t1 >= 0 && t1 <= 1)
    {
        fT = t1;
        return true;
    }
    if (t2 >= 0 && t2 <= 1)
    {
        fT = t2;
        return true;
    }
    return false;
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
