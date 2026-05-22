#pragma once

struct SVector2
{
	float x;
	float y;

	SVector2() noexcept
	{}

	constexpr SVector2(float _x, float _y) noexcept
		: x(_x), y(_y)
	{}

	constexpr SVector2 operator+(const SVector2& rhs) const noexcept
	{
		return SVector2(x + rhs.x, y + rhs.y);
	}

	constexpr SVector2 operator-(const SVector2& rhs) const noexcept
	{
		return SVector2(x - rhs.x, y - rhs.y);
	}

	constexpr SVector2 operator+(float s) const noexcept
	{
		return SVector2(x + s, y + s);
	}

	constexpr SVector2 operator-(float s) const noexcept
	{
		return SVector2(x - s, y - s);
	}

	constexpr SVector2 operator*(float s) const noexcept
	{
		return SVector2(x * s, y * s);
	}

	constexpr SVector2 operator/(float s) const noexcept
	{
		const float inv = 1.0f / s;
		return SVector2(x * inv, y * inv);
	}

	SVector2& operator+=(const SVector2& rhs) noexcept
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	SVector2& operator-=(const SVector2& rhs) noexcept
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	SVector2& operator*=(float s) noexcept
	{
		x *= s;
		y *= s;
		return *this;
	}

	SVector2& operator/=(float s) noexcept
	{
		const float inv = 1.0f / s;
		x *= inv;
		y *= inv;
		return *this;
	}
};

constexpr SVector2 operator*(float s, const SVector2& v) noexcept
{
	return SVector2(v.x * s, v.y * s);
}

constexpr SVector2 operator+(float s, const SVector2& v) noexcept
{
	return SVector2(v.x + s, v.y + s);
}

constexpr SVector2 operator-(float s, const SVector2& v) noexcept
{
	return SVector2(s - v.x, s - v.y);
}