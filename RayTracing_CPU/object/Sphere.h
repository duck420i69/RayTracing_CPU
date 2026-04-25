
#pragma once

#include "object.h"

class Sphere : public Hittable {
public:
	HittableType getType() const override;
	bool hit(const Ray& ray, Packet& result) const override;
	Bound getBound() const override;
	float getCost() const override;

	vec3 center;
	float r;
};

