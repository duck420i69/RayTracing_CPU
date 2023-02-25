#pragma once

#include "math.h"

class Ray {
public:
	Ray(vec4 origin, vec4 direction, float tMaximum = infinity, float refraction_term = 1.0f) : o(origin), d(direction), tMax(tMaximum), refraction(refraction_term) {};

	vec4 o;
	vec4 d;
	mutable float tMax;
	mutable float refraction;
};