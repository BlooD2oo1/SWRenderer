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

	inline SVector2& operator+=(const SVector2& rhs) noexcept { x += rhs.x; y += rhs.y; return *this; }
	inline SVector2& operator-=(const SVector2& rhs) noexcept { x -= rhs.x; y -= rhs.y; return *this; }
	inline SVector2& operator*=(const SVector2& rhs) noexcept { x *= rhs.x; y *= rhs.y; return *this; }
	inline SVector2& operator/=(const SVector2& rhs) noexcept { x /= rhs.x; y /= rhs.y; return *this; }

	inline SVector2& operator*=(float s) noexcept { x *= s; y *= s; return *this; }
	inline SVector2& operator/=(float s) noexcept { x /= s; y /= s; return *this; }

	constexpr static float Dot( const SVector2& a, const SVector2& b )
	{
		return a.x * b.x + a.y * b.y;
	}

	constexpr static float LengthSq( const SVector2& v )
	{
		return v.x * v.x + v.y * v.y;
	}

	static float Length( const SVector2& v )
	{
		return sqrtf( v.x * v.x + v.y * v.y );
	}

	static SVector2& Normalize( SVector2& out, const SVector2& v ) noexcept
	{
		float len = Length( v );
		if ( len > 0.000001f )
		{
			out.x = v.x / len;
			out.y = v.y / len;
		}
		else
		{
			out.x = 0.0f;
			out.y = 0.0f;
		}
		return out;
	}
};

inline SVector2 operator+(const SVector2& a, const SVector2& b) { return SVector2(a.x + b.x, a.y + b.y); }
inline SVector2 operator-(const SVector2& a, const SVector2& b) { return SVector2(a.x - b.x, a.y - b.y); }

inline SVector2 operator*(const SVector2& a, const SVector2& b) { return SVector2(a.x * b.x, a.y * b.y); }
inline SVector2 operator*(const SVector2& a, float s)            { return SVector2(a.x * s,    a.y * s); }
inline SVector2 operator*(float s, const SVector2& a)            { return SVector2(a.x * s,    a.y * s); }

inline SVector2 operator/(const SVector2& a, const SVector2& b) { return SVector2(a.x / b.x, a.y / b.y); }
inline SVector2 operator/(const SVector2& a, float s)            { return SVector2(a.x / s,    a.y / s); }


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

	inline SVector3& operator+=(const SVector3& rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	inline SVector3& operator-=(const SVector3& rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	inline SVector3& operator*=(const SVector3& rhs) noexcept { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
	inline SVector3& operator/=(const SVector3& rhs) noexcept { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

	inline SVector3& operator*=(float s) noexcept { x *= s; y *= s; z *= s; return *this; }
	inline SVector3& operator/=(float s) noexcept { x /= s; y /= s; z /= s; return *this; }

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

	static SVector3& Slerp( SVector3& out, const SVector3& a, const SVector3& b, float w)
	{
		float dot = Dot(a, b);

		dot = Clamp(dot, -1.0f, 1.0f);

		float theta = acosf(dot);

		if (theta < 0.0001f)
		{
			out = a;
			return out;
		}

		float sinTheta = sinf(theta);

		float w0 = sinf((1.0f - w) * theta) / sinTheta;
		float w1 = sinf(w * theta) / sinTheta;

		out = SVector3( a.x * w0 + b.x * w1, a.y * w0 + b.y * w1, a.z * w0 + b.z * w1 );
		return out;
	}
};

inline SVector3 operator+(const SVector3& a, const SVector3& b) { return SVector3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline SVector3 operator-(const SVector3& a, const SVector3& b) { return SVector3(a.x - b.x, a.y - b.y, a.z - b.z); }

inline SVector3 operator*(const SVector3& a, const SVector3& b) { return SVector3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline SVector3 operator*(const SVector3& a, float s)            { return SVector3(a.x * s,    a.y * s,    a.z * s); }
inline SVector3 operator*(float s, const SVector3& a)            { return SVector3(a.x * s,    a.y * s,    a.z * s); }

inline SVector3 operator/(const SVector3& a, const SVector3& b) { return SVector3(a.x / b.x, a.y / b.y, a.z / b.z); }
inline SVector3 operator/(const SVector3& a, float s)            { return SVector3(a.x / s,    a.y / s,    a.z / s); }


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

	SVector4( const SVector3& v, float _w)
		: x(v.x), y(v.y), z(v.z), w(_w)
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

	constexpr static SMatrix& Scale( SMatrix& out, float f )
	{
		out.m00 *= f;		out.m01 *= f;		out.m02 *= f;
		out.m10 *= f;		out.m11 *= f;		out.m12 *= f;
		out.m20 *= f;		out.m21 *= f;		out.m22 *= f;
		return out;

	}

	static SVector3& TransformCoord( SVector3& out, const SVector3& v, const SMatrix& m )
	{
		assert( &out != &v );
		out.x = v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30;
		out.y = v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31;
		out.z = v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32;
		return out;
	}

	static SVector3& TransformNormal( SVector3& out, const SVector3& v, const SMatrix& m )
	{
		assert( &out != &v );
		out.x = v.x * m.m00 + v.y * m.m10 + v.z * m.m20;
		out.y = v.x * m.m01 + v.y * m.m11 + v.z * m.m21;
		out.z = v.x * m.m02 + v.y * m.m12 + v.z * m.m22;
		return out;
	}

	static SMatrix& BuildViewMatrix( SMatrix& out, const SVector3& eye, const SVector3& target, const SVector3& up )
	{
		SVector3 zaxis = eye - target;
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

	constexpr static SMatrix& Transpose( SMatrix& out, const SMatrix& m )
	{
		assert( &out != &m );
		out.m00 = m.m00;
		out.m01 = m.m10;
		out.m02 = m.m20;
		out.m03 = m.m30;

		out.m10 = m.m01;
		out.m11 = m.m11;
		out.m12 = m.m21;
		out.m13 = m.m31;

		out.m20 = m.m02;
		out.m21 = m.m12;
		out.m22 = m.m22;
		out.m23 = m.m32;

		out.m30 = m.m03;
		out.m31 = m.m13;
		out.m32 = m.m23;
		out.m33 = m.m33;

		return out;
	}

	static bool Inverse( SMatrix& out, const SMatrix& m )
	{
		float inv[16];
		const float* a = &m.m00;

		inv[0] =
			a[5] * a[10] * a[15] -
			a[5] * a[11] * a[14] -
			a[9] * a[6] * a[15] +
			a[9] * a[7] * a[14] +
			a[13] * a[6] * a[11] -
			a[13] * a[7] * a[10];

		inv[4] =
			-a[4] * a[10] * a[15] +
			a[4] * a[11] * a[14] +
			a[8] * a[6] * a[15] -
			a[8] * a[7] * a[14] -
			a[12] * a[6] * a[11] +
			a[12] * a[7] * a[10];

		inv[8] =
			a[4] * a[9] * a[15] -
			a[4] * a[11] * a[13] -
			a[8] * a[5] * a[15] +
			a[8] * a[7] * a[13] +
			a[12] * a[5] * a[11] -
			a[12] * a[7] * a[9];

		inv[12] =
			-a[4] * a[9] * a[14] +
			a[4] * a[10] * a[13] +
			a[8] * a[5] * a[14] -
			a[8] * a[6] * a[13] -
			a[12] * a[5] * a[10] +
			a[12] * a[6] * a[9];

		inv[1] =
			-a[1] * a[10] * a[15] +
			a[1] * a[11] * a[14] +
			a[9] * a[2] * a[15] -
			a[9] * a[3] * a[14] -
			a[13] * a[2] * a[11] +
			a[13] * a[3] * a[10];

		inv[5] =
			a[0] * a[10] * a[15] -
			a[0] * a[11] * a[14] -
			a[8] * a[2] * a[15] +
			a[8] * a[3] * a[14] +
			a[12] * a[2] * a[11] -
			a[12] * a[3] * a[10];

		inv[9] =
			-a[0] * a[9] * a[15] +
			a[0] * a[11] * a[13] +
			a[8] * a[1] * a[15] -
			a[8] * a[3] * a[13] -
			a[12] * a[1] * a[11] +
			a[12] * a[3] * a[9];

		inv[13] =
			a[0] * a[9] * a[14] -
			a[0] * a[10] * a[13] -
			a[8] * a[1] * a[14] +
			a[8] * a[2] * a[13] +
			a[12] * a[1] * a[10] -
			a[12] * a[2] * a[9];

		inv[2] =
			a[1] * a[6] * a[15] -
			a[1] * a[7] * a[14] -
			a[5] * a[2] * a[15] +
			a[5] * a[3] * a[14] +
			a[13] * a[2] * a[7] -
			a[13] * a[3] * a[6];

		inv[6] =
			-a[0] * a[6] * a[15] +
			a[0] * a[7] * a[14] +
			a[4] * a[2] * a[15] -
			a[4] * a[3] * a[14] -
			a[12] * a[2] * a[7] +
			a[12] * a[3] * a[6];

		inv[10] =
			a[0] * a[5] * a[15] -
			a[0] * a[7] * a[13] -
			a[4] * a[1] * a[15] +
			a[4] * a[3] * a[13] +
			a[12] * a[1] * a[7] -
			a[12] * a[3] * a[5];

		inv[14] =
			-a[0] * a[5] * a[14] +
			a[0] * a[6] * a[13] +
			a[4] * a[1] * a[14] -
			a[4] * a[2] * a[13] -
			a[12] * a[1] * a[6] +
			a[12] * a[2] * a[5];

		inv[3] =
			-a[1] * a[6] * a[11] +
			a[1] * a[7] * a[10] +
			a[5] * a[2] * a[11] -
			a[5] * a[3] * a[10] -
			a[9] * a[2] * a[7] +
			a[9] * a[3] * a[6];

		inv[7] =
			a[0] * a[6] * a[11] -
			a[0] * a[7] * a[10] -
			a[4] * a[2] * a[11] +
			a[4] * a[3] * a[10] +
			a[8] * a[2] * a[7] -
			a[8] * a[3] * a[6];

		inv[11] =
			-a[0] * a[5] * a[11] +
			a[0] * a[7] * a[9] +
			a[4] * a[1] * a[11] -
			a[4] * a[3] * a[9] -
			a[8] * a[1] * a[7] +
			a[8] * a[3] * a[5];

		inv[15] =
			a[0] * a[5] * a[10] -
			a[0] * a[6] * a[9] -
			a[4] * a[1] * a[10] +
			a[4] * a[2] * a[9] +
			a[8] * a[1] * a[6] -
			a[8] * a[2] * a[5];

		float det =
			a[0] * inv[0] +
			a[1] * inv[4] +
			a[2] * inv[8] +
			a[3] * inv[12];

		if (fabsf(det) < 1e-8f)
			return false;

		det = 1.0f / det;

		float* o = &out.m00;
		for (int i = 0; i < 16; ++i)
			o[i] = inv[i] * det;

		return true;
	}
};

struct SQuaternion
{
	union
	{
		struct { float x, y, z, w; };
		float m[4];
	};

	SQuaternion()
	{}

	SQuaternion( float _x, float _y, float _z, float _w )
		: x( _x ), y( _y ), z( _z ), w( _w )
	{}

	constexpr static SQuaternion& Identity( SQuaternion& out )
	{
		out.x = 0.0f;
		out.y = 0.0f;
		out.z = 0.0f;
		out.w = 1.0f;
		return out;
	}

	static float Length( const SQuaternion& q )
	{
		return sqrtf(
			q.x * q.x +
			q.y * q.y +
			q.z * q.z +
			q.w * q.w );
	}

	static SQuaternion& Normalize( SQuaternion& out, const SQuaternion& q )
	{
		float len = Length( q );

		if ( len > 0.000001f )
		{
			float invLen = 1.0f / len;

			out.x = q.x * invLen;
			out.y = q.y * invLen;
			out.z = q.z * invLen;
			out.w = q.w * invLen;
		}
		else
		{
			Identity( out );
		}

		return out;
	}

	static SQuaternion& Conjugate( SQuaternion& out, const SQuaternion& q )
	{
		out.x = -q.x;
		out.y = -q.y;
		out.z = -q.z;
		out.w =  q.w;
		return out;
	}

	static SQuaternion& Mul( SQuaternion& out, const SQuaternion& a, const SQuaternion& b )
	{
		SQuaternion r;

		r.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
		r.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
		r.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
		r.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;

		out = r;
		return out;
	}

	static SQuaternion& FromAxisAngle(
		SQuaternion& out,
		const SVector3& axis,
		float angleRadians )
	{
		SVector3 n;
		SVector3::Normalize( n, axis );

		float half = angleRadians * 0.5f;
		float s = sinf( half );

		out.x = n.x * s;
		out.y = n.y * s;
		out.z = n.z * s;
		out.w = cosf( half );

		return out;
	}

	static SQuaternion& FromEulerXYZ(
		SQuaternion& out,
		float pitch,
		float yaw,
		float roll )
	{
		SQuaternion qx, qy, qz, temp;

		FromAxisAngle( qx, SVector3(1,0,0), pitch );
		FromAxisAngle( qy, SVector3(0,1,0), yaw );
		FromAxisAngle( qz, SVector3(0,0,1), roll );

		Mul( temp, qx, qy );
		Mul( out, temp, qz );

		return out;
	}

	static SMatrix& ToMatrix( SMatrix& out, const SQuaternion& qin )
	{
		SQuaternion q;
		Normalize( q, qin );

		float xx = q.x * q.x;
		float yy = q.y * q.y;
		float zz = q.z * q.z;

		float xy = q.x * q.y;
		float xz = q.x * q.z;
		float yz = q.y * q.z;

		float wx = q.w * q.x;
		float wy = q.w * q.y;
		float wz = q.w * q.z;

		out.m00 = 1.0f - 2.0f * (yy + zz);
		out.m01 = 2.0f * (xy + wz);
		out.m02 = 2.0f * (xz - wy);
		out.m03 = 0.0f;

		out.m10 = 2.0f * (xy - wz);
		out.m11 = 1.0f - 2.0f * (xx + zz);
		out.m12 = 2.0f * (yz + wx);
		out.m13 = 0.0f;

		out.m20 = 2.0f * (xz + wy);
		out.m21 = 2.0f * (yz - wx);
		out.m22 = 1.0f - 2.0f * (xx + yy);
		out.m23 = 0.0f;

		out.m30 = 0.0f;
		out.m31 = 0.0f;
		out.m32 = 0.0f;
		out.m33 = 1.0f;

		return out;
	}
};