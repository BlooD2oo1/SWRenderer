#pragma once

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