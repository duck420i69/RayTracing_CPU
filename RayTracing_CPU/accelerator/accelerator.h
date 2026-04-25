#pragma once

#include "../rtmath.h"
#include <deque>


class Bound {
public:
	Bound() : min({ infinity, infinity, infinity }), max({ -infinity, -infinity, -infinity }) {};
	Bound(vec3 low_bound, vec3 high_bound) : min(low_bound), max(high_bound) {};

	inline bool hit(const vec3& origin, const vec3& invRayDir, const float& tMax) const {
		float t0 = 0.0f;
		float t1 = tMax;
		for (int i = 0; i < 3; i++) {
			float tNear = (min[i] - origin[i]) * invRayDir[i];
			float tFar = (max[i] - origin[i]) * invRayDir[i];
			if (tNear > tFar) std::swap(tNear, tFar);
			t0 = tNear > t0 ? tNear : t0;
			t1 = tFar < t1 ? tFar : t1;
			if (t0 > t1) return false;
		}
		return true;
	}

	inline float surfaceArea() const {
		float x = max[0] - min[0];
		float y = max[1] - min[1];
		float z = max[2] - min[2];
		return 2 * (x * y + y * z + x * z);
	}

	vec3 min;
	vec3 max;
};

inline Bound Union(const Bound& b1, const Bound& b2) {
	vec3 min, max;
	for (int i = 0; i < 3; i++) {
		min[i] = std::min(b1.min[i], b2.min[i]);
		max[i] = std::max(b1.max[i], b2.max[i]);
	}
	return Bound(min, max);
}

class BVHNode {
public:
	BVHNode() = default;
	BVHNode(Bound b, uint32_t index) : bound(b), obj_index(index), cost(0.0f) { centroid = (b.max + b.min) * 0.5f; }
	Bound bound;
	vec3 centroid;
	
	float cost;
	uint32_t obj_index;

	std::vector<std::unique_ptr<BVHNode>> child;
};

struct BVHLinearNode {
public:
	BVHLinearNode(Bound b, uint32_t obj) : bound(b), obj_index(obj), next_index(0) {}
	Bound bound;
	uint32_t next_index;
	uint32_t obj_index;
};

struct BVHLinear {
	std::vector<BVHLinearNode> bvh;
};

enum class BuildStrat {
	SAH = 0,
	TOPDOWN = 1
};

std::unique_ptr<BVHNode> constructBVH(std::deque<std::unique_ptr<BVHNode>> component, BuildStrat buildStrat);

void constructLinearBVH(const std::unique_ptr<BVHNode>& root, BVHLinear& arr, size_t& iter);