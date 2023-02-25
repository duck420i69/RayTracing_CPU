#include "Sphere.h"

bool Sphere::hit(const Ray& ray, Packet& result) const {
	// a always equal to 1 when ray direction is always normalized
	vec4 oc = ray.o - center;
	float half_b = oc.dot(ray.d);
	float c = oc.dot(oc) - r*r;
	float discriminant = half_b * half_b - c;
	if (discriminant < 0) return false;
	else {
		float sqrtd = sqrt(discriminant);
		float t = (-half_b - sqrtd);
		if (t < 0) {
			t = (-half_b + sqrtd);
			if (t < 0) return false;
		}
		ray.tMax = t;
		result.color = { 1,1,1,1 };
		result.last_hit = ray.o + ray.d * t;
		result.normal = result.last_hit - center;
		result.normal.normalize();
		result.material = material;
		return true;
	}
}

Bound Sphere::getBound() const {
	return Bound(center + vec4(-r, -r, -r, 1.0f), center + vec4(r, r, r, 1.0f));
}
