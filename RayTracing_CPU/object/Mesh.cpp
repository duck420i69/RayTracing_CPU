#include "Mesh.h"

Triangle::Triangle() {
	a = {}; b = {}; c = {};
	ta = {}; tb = {}; tc = {};
}

Triangle::Triangle(vec3 p1, vec3 p2, vec3 p3) : Triangle(p1, p2, p3, {}, {}, {}) {
	col = { 1.0f, 1.0f, 1.0f, 1.0f };
	normal = (b - a).cross(c - a);
	normal.normalize();
}

Triangle::Triangle(vec3 p1, vec3 p2, vec3 p3, Color col) : a(p1), b(p2), c(p3), col(col) {
	normal = (b - a).cross(c - a);
	normal.normalize();
}

Triangle::Triangle(vec3 p1, vec3 p2, vec3 p3, texel t1, texel t2, texel t3) : a(p1), b(p2), c(p3), ta(t1), tb(t2), tc(t3) {
	col = { 1.0f, 1.0f, 1.0f, 1.0f };
	normal = (b - a).cross(c - a);
	normal.normalize();
}

bool Triangle::hit(const Ray& ray, Packet& result) const {
	vec3 E1 = b - a;
	vec3 E2 = c - a;
	vec3 N = E1.cross(E2);
	float det = -ray.d.dot(N);
	
	if (-1e-6f <= det && det <= 1e-6f) return false;
	
	float invdet = 1.0 / det;
	vec3 AO = ray.o - a;
	vec3 DAO;
	DAO = AO.cross(ray.d);
	
	float u = E2.dot(DAO) * invdet;
	float v = -E1.dot(DAO) * invdet;
	float t = AO.dot(N) * invdet;
	
	if (t < ray.tMax && t > 0.0f && u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f) {
		result.u = u;
		result.v = v;
		result.normal = normal;
		result.last_hit = a + E1 * result.u + E2 * result.v;
		ray.tMax = t;
		return true;
	}
	return false;
}

Bound Triangle::getBound() const {
	vec3 min;
	vec3 max;
	for (int i = 0; i < 3; i++) {
		max[i] = std::max(a[i], std::max(b[i], c[i]));
		min[i] = std::min(a[i], std::min(b[i], c[i]));
	}
	return Bound(min, max);
}

inline texel getTexcord(const Triangle& tri, const texel& uv) {
	texel texCoord = tri.ta + (tri.tb - tri.ta) * uv.x + (tri.tc - tri.ta) * uv.y;
	while (texCoord.x > 1.0f) texCoord.x -= 1.0f;
	while (texCoord.x < 0.0f) texCoord.x += 1.0f;
	while (texCoord.y > 1.0f) texCoord.y -= 1.0f;
	while (texCoord.y < 0.0f) texCoord.y += 1.0f;
	texCoord.y = 1.0f - texCoord.y;
	return texCoord;
}

HittableType Object::getType() const { return HittableType::MESH; }

bool Object::hit(const Ray& ray, Packet& result) const {
	const Triangle* closest_triangle = nullptr;
	vec3 invertRayDir = { 1 / ray.d.x(), 1 / ray.d.y(), 1 / ray.d.z() };
	uint32_t iter = 0, stack_size = 0;
	uint32_t stack[64];

	auto& bvh = tree.bvh;

	float tMax = ray.tMax;
	bool hasHit = false;
	vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	// Delete this
	result.shape_check--;

	while (true) {
		// Delete this
		result.aabb_check++;

		if (bvh[iter].bound.hit(ray.o, invertRayDir, ray.tMax)) {
			if (bvh[iter].next_index == 0) {
				auto& tri = mesh[bvh[iter].obj_index];

				result.shape_check++;
				if (tri.hit(ray, result)) {

					if (material == nullptr) closest_triangle = &tri;
					else if (material->texture_map.size() == 0) closest_triangle = &tri;
					else {
						color = material->getTexture(getTexcord(tri, { result.u, result.v }));
						if (color.w > 0.70f) {
							closest_triangle = &tri;
						}
						else {
							ray.tMax = tMax;
						}
					}
				}
				if (stack_size < 1) break;
				iter = stack[--stack_size];
			}
			else {
				if (invertRayDir.x() > 0) {
					stack[stack_size++] = bvh[iter++].next_index;
				}
				else {
					stack[stack_size++] = iter + 1;
					iter = bvh[iter].next_index;
				}
			}
		}
		else {
			if (stack_size < 1) break;
			iter = stack[--stack_size];
		}
	}

	if (closest_triangle != nullptr) {
		if (material == nullptr) result.color = closest_triangle->col;
		else {
			texel texCoord = getTexcord(*closest_triangle, { result.u, result.v });
			result.u = texCoord.x;
			result.v = texCoord.y;
			result.material = material;
		}
		result.normal = closest_triangle->normal;
		return true;	
	}
	return false;
}

Bound Object::getBound() const {
	return bound;
}

float Object::getCost() const {
	return mesh.size();
}

void Object::buildTree() {
	std::deque<std::unique_ptr<BVHNode>> nodes;
	for (long i = 0; i < mesh.size(); i++) {
		nodes.emplace_back(std::make_unique<BVHNode>(mesh[i].getBound(), i));
	}
	auto root = constructBVH(std::move(nodes), BuildStrat::TOPDOWN);
	size_t i = 0;
	constructLinearBVH(root, tree, i);
}
