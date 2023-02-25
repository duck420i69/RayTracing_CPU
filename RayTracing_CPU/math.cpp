#include "math.h"
#include <cmath>
#include <random>

vec2 vec2::operator+(const vec2& other) const
{
	return { x + other.x, y + other.y};
}

vec2 vec2::operator*(const float& other) const
{
	return { x * other, y * other };
}

vec2 vec2::operator-(const vec2& other) const
{
	return { x - other.x, y - other.y };
}

void vec2::normalize() {
	float lenght = 1/sqrt(x * x + y * y);
	x *= lenght;
	y *= lenght;
}
