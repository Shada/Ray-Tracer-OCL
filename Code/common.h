#pragma once
#include <cmath>

struct Pixel
{
	float r;
	float g;
	float b;
};

struct Vec4
{
	float x;
	float y;
	float z;
	float w;

	Vec4()
	{
		x = y = z = w = 0;
	}
	Vec4(float _x, float _y, float _z, float _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	Vec4 operator+(Vec4 _a)
	{
		Vec4 result = Vec4();
		result.x = x + _a.x;
		result.y = y + _a.y;
		result.z = z + _a.z;
		result.w = w + _a.w;
		return result;
	}
	void operator+=(Vec4 _a)
	{
		x += _a.x;
		y += _a.y;
		z += _a.z;
		w += _a.w;
	}
	Vec4 operator-(Vec4 _a)
	{
		Vec4 result = Vec4();
		result.x = x - _a.x;
		result.y = y - _a.y;
		result.z = z - _a.z;
		result.w = w - _a.w;
		return result;
	}
	void operator-=(Vec4 _a)
	{
		x -= _a.x;
		y -= _a.y;
		z -= _a.z;
		w -= _a.w;
	}
	Vec4 operator*(float _a)
	{
		Vec4 result = Vec4();
		result.x = x * _a;
		result.y = y * _a;
		result.z = z * _a;
		result.w = w * _a;
		return result;
	}
	void operator*=(float _a)
	{
		x *= _a;
		y *= _a;
		z *= _a;
		w *= _a;
	}
	Vec4 operator*(Vec4 _a)
	{
		Vec4 result = Vec4();
		result.x = y * _a.z - z * _a.y;
		result.y = z * _a.x - x * _a.z;
		result.z = x * _a.y - y * _a.x;
		result.w = 0;
		return result;
	}
	Vec4 operator/(float _a)
	{
		Vec4 result = Vec4();
		if(_a == 0)
		{
			return result;
		}
		result.x = x / _a;
		result.y = y / _a;
		result.z = z / _a;
		result.w = w / _a;

		return result;
	}
	void operator/=(float _a)
	{
		if(_a == 0)
		{
			return;
		}
		x /= _a;
		y /= _a;
		z /= _a;
		w /= _a;
	}
};

inline Vec4 rotateVec4(Vec4 _v, Vec4 _axis, float _radians)
{
	Vec4 result = Vec4();

	float c = std::cos(_radians);
	float l_c = 1 - c;

	float s = std::sin(_radians);
	
	result.x = _v.x * (_axis.x * _axis.x + (1 - _axis.x * _axis.x) * c) + _v.y * (_axis.x * _axis.y * l_c + _axis.z * s) + _v.z * (_axis.x * _axis.z * l_c - _axis.y * s);
	result.y = _v.x * (_axis.x * _axis.y * l_c - _axis.z * s) + _v.y * (_axis.y * _axis.y + (1 - _axis.y * _axis.y) * c) + _v.z * (_axis.y * _axis.z * l_c + _axis.x * s);
	result.z = _v.x * (_axis.x * _axis.z * l_c + _axis.y * s) + _v.y * (_axis.y * _axis.z * l_c - _axis.x * s) + _v.z * (_axis.z * _axis.z + (1 - _axis.z * _axis.z) * c);

	return result;
};