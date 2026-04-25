#pragma once

#include "rtmath.h"

class Ray {
public:
	Ray(vec3 origin, vec3 direction, float tMaximum = infinity, float refraction_term = 1.0f) : o(origin), d(direction), tMax(tMaximum), refraction(refraction_term) {};

	vec3 o;
	vec3 d;
	mutable float tMax;
	mutable float refraction;
};