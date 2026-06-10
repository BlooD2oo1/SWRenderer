#pragma once

#include "Common/Globals.h"

struct SVector2
{
	union
	{
		struct { float x, y; };
		float m[2];
	};

	SVector2()
	{}

	SVector2(float _x, float _y)
		: x(_x), y(_y)
	{}

	constexpr static SVector2& Add( SVector2& out, const SVector2& a, const SVector2& b )
	{
		out.x = a.x + b.x;
		out.y = a.y + b.y;
		return out;
	}

	constexpr static SVector2& Sub( SVector2& out, const SVector2& a, const SVector2& b )
	{
		out.x = a.x - b.x;
		out.y = a.y - b.y;
		return out;
	}
};

struct SVector3
{
	union
	{
		struct { float x, y, z; };
		float m[3];
	};

	SVector3()
	{}

	SVector3(float _x, float _y, float _z)
		: x(_x), y(_y), z(_z)
	{}

	constexpr static SVector3& Mul( SVector3& out, const SVector3& a, const float b )
	{
		out.x = a.x * b;
		out.y = a.y * b;
		out.z = a.z * b;
		return out;
	}

	constexpr static SVector3& Add( SVector3& out, const SVector3& a, const SVector3& b )
	{
		out.x = a.x + b.x;
		out.y = a.y + b.y;
		out.z = a.z + b.z;
		return out;
	}

	constexpr static SVector3& Sub( SVector3& out, const SVector3& a, const SVector3& b )
	{
		out.x = a.x - b.x;
		out.y = a.y - b.y;
		out.z = a.z - b.z;
		return out;
	}

	constexpr static float Dot( const SVector3& a, const SVector3& b )
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	constexpr static SVector3& Cross( SVector3& out, const SVector3& a, const SVector3& b )
	{
		out.x = a.y * b.z - a.z * b.y;
		out.y = a.z * b.x - a.x * b.z;
		out.z = a.x * b.y - a.y * b.x;
		return out;
	}

	constexpr static float LengthSq( const SVector3& v )
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	static float Length( const SVector3& v ) noexcept
	{
		return sqrtf( LengthSq( v ) );
	}

	static SVector3& Normalize( SVector3& out, const SVector3& v ) noexcept
	{
		float len = Length( v );
		if ( len > 0.000001f )
		{
			out.x = v.x / len;
			out.y = v.y / len;
			out.z = v.z / len;
		}
		else
		{
			out.x = 0.0f;
			out.y = 0.0f;
			out.z = 0.0f;
		}
		return out;
	}
};

struct SVector4
{
	union
	{
		struct { float x, y, z, w; };
		float m[4];
	};

	SVector4()
	{}

	SVector4(float _x, float _y, float _z, float _w)
		: x(_x), y(_y), z(_z), w(_w)
	{}

	constexpr static float Dot( const SVector4& a, const SVector4& b )
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}
};

struct SMatrix
{
	union
	{
		struct
		{
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
		};
		float m[4][4];
	};

	SMatrix()
	{}

	constexpr static SMatrix& Identity( SMatrix& out )
	{
		out.m00 = 1.0f; out.m01 = 0.0f; out.m02 = 0.0f; out.m03 = 0.0f;
		out.m10 = 0.0f; out.m11 = 1.0f; out.m12 = 0.0f; out.m13 = 0.0f;
		out.m20 = 0.0f; out.m21 = 0.0f; out.m22 = 1.0f; out.m23 = 0.0f;
		out.m30 = 0.0f; out.m31 = 0.0f; out.m32 = 0.0f; out.m33 = 1.0f;
		return out;
	}

	static SMatrix& Mul( SMatrix& out, const SMatrix& a, const SMatrix& b )
	{
		SMatrix r;

		r.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20 + a.m03 * b.m30;
		r.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21 + a.m03 * b.m31;
		r.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22 + a.m03 * b.m32;
		r.m03 = a.m00 * b.m03 + a.m01 * b.m13 + a.m02 * b.m23 + a.m03 * b.m33;

		r.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20 + a.m13 * b.m30;
		r.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31;
		r.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32;
		r.m13 = a.m10 * b.m03 + a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33;

		r.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20 + a.m23 * b.m30;
		r.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31;
		r.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32;
		r.m23 = a.m20 * b.m03 + a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33;

		r.m30 = a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20 + a.m33 * b.m30;
		r.m31 = a.m30 * b.m01 + a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31;
		r.m32 = a.m30 * b.m02 + a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32;
		r.m33 = a.m30 * b.m03 + a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33;

		out = r;
		return out;
	}

	constexpr static SVector4& Mul( SVector4& out, const SMatrix& m, const SVector4& v )
	{
		assert( &out != &v );
		out.x = m.m00 * v.x + m.m01 * v.y + m.m02 * v.z + m.m03 * v.w;
		out.y = m.m10 * v.x + m.m11 * v.y + m.m12 * v.z + m.m13 * v.w;
		out.z = m.m20 * v.x + m.m21 * v.y + m.m22 * v.z + m.m23 * v.w;
		out.w = m.m30 * v.x + m.m31 * v.y + m.m32 * v.z + m.m33 * v.w;
		return out;
	}

	constexpr static SVector4& Mul( SVector4& out, const SVector4& v, const SMatrix& m )
	{
		assert( &out != &v );
		out.x = v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30;
		out.y = v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31;
		out.z = v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32;
		out.w = v.x * m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33;
		return out;
	}

	static SMatrix& BuildViewMatrix( SMatrix& out, const SVector3& eye, const SVector3& target, const SVector3& up )
	{
		SVector3 zaxis;
		SVector3::Sub( zaxis, eye, target );
		SVector3::Normalize( zaxis, zaxis );
		SVector3 xaxis;
		SVector3::Cross( xaxis, up, zaxis );
		SVector3::Normalize( xaxis, xaxis );
		SVector3 yaxis;
		SVector3::Cross( yaxis, zaxis, xaxis );
		out.m00 = xaxis.x; out.m01 = yaxis.x; out.m02 = zaxis.x; out.m03 = 0.0f;
		out.m10 = xaxis.y; out.m11 = yaxis.y; out.m12 = zaxis.y; out.m13 = 0.0f;
		out.m20 = xaxis.z; out.m21 = yaxis.z; out.m22 = zaxis.z; out.m23 = 0.0f;
		out.m30 = -SVector3::Dot( xaxis, eye ); out.m31 = -SVector3::Dot( yaxis, eye ); out.m32 = -SVector3::Dot( zaxis, eye ); out.m33 = 1.0f;
		return out;
	}

	static SMatrix& BuildProjectionMatrix( SMatrix& out, float fovY, float aspect, float znear, float zfar )
	{
		const float f = 1.0f / tanf(fovY * 0.5f);

		out.m00 = f / aspect;
		out.m01 = 0.0f;
		out.m02 = 0.0f;
		out.m03 = 0.0f;

		out.m10 = 0.0f;
		out.m11 = f;
		out.m12 = 0.0f;
		out.m13 = 0.0f;

		out.m20 = 0.0f;
		out.m21 = 0.0f;
		out.m22 = (zfar + znear) / (znear - zfar);
		out.m23 = -1.0f;

		out.m30 = 0.0f;
		out.m31 = 0.0f;
		out.m32 = (2.0f * zfar * znear) / (znear - zfar);
		out.m33 = 0.0f;

		return out;
	}	
};