
#pragma once

#include "object.h"

class Sphere : public Hittable {
public:
	bool hit(const Ray& ray, Packet& result) const;
	Bound getBound() const;

	vec4 center;
	std::shared_ptr<Material> material;
	float r;
};

