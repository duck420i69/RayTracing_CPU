#pragma once

#include "object.h"

class Triangle {
public:
	vec3 a, b, c;
	texel ta, tb, tc;
	vec3 normal;
	Color col;
	Triangle();
	Triangle(vec3 p1, vec3 p2, vec3 p3);
	Triangle(vec3 p1, vec3 p2, vec3 p3, Color col);
	Triangle(vec3 p1, vec3 p2, vec3 p3, texel t1, texel t2, texel t3);
	bool hit(const Ray& ray, Packet& result) const;
	Bound getBound() const;
};

using Mesh = std::vector<Triangle>;

class Object : public Hittable {
public:
	Mesh mesh;
	Bound bound;
	BVHLinear tree;
	HittableType getType() const override;
	bool hit(const Ray& ray, Packet& result) const override;
	Bound getBound() const override;
	float getCost() const override;
	void buildTree();
};
