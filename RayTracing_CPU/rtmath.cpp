#include "rtmath.h"
#include <cmath>
#include <random>
#include <iostream>
#include <string>

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

std::string to_string(vec3 v) {
	return std::to_string(v.x()) + " " + std::to_string(v.y()) + " " + std::to_string(v.z());
}

std::string to_string(vec4 v) {
	return to_string(v.v);
}