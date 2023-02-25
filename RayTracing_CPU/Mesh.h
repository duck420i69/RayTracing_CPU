#pragma once

#include "object.h"

class Triangle {
public:
	vec4 a, b, c;
	texel ta, tb, tc;
	vec4 normal;
	vec4 col;
	Triangle();
	Triangle(vec4 p1, vec4 p2, vec4 p3);
	Triangle(vec4 p1, vec4 p2, vec4 p3, vec4 col);
	Triangle(vec4 p1, vec4 p2, vec4 p3, texel t1, texel t2, texel t3);
	bool hit(const Ray& ray, Packet& result) const;
	Bound getBound() const;
};

using Mesh = std::vector<Triangle>;

class Object : public Hittable {
public:
	Mesh mesh;
	Bound bound;
	BVHLinear tree;
	bool hit(const Ray& ray, Packet& result) const;
	Bound getBound() const;
	void buildTree();
};
