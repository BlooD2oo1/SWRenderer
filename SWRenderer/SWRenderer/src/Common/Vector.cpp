#include "Vector.h"

BGRA8::BGRA8( const SVector4& vColor )
{
	r = (uint8_t)(std::max( 0.0f, std::min( vColor.x, 1.0f ) ) * 255.0f);
	g = (uint8_t)(std::max( 0.0f, std::min( vColor.y, 1.0f ) ) * 255.0f);
	b = (uint8_t)(std::max( 0.0f, std::min( vColor.z, 1.0f ) ) * 255.0f);
	a = (uint8_t)(std::max( 0.0f, std::min( vColor.w, 1.0f ) ) * 255.0f);
}