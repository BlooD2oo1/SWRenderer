#pragma once

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